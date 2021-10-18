module;

#include <string_view>

export module rbx.lua.api;

import mem;

import rbx.lua.object;
import rbx.lua.constants;
import rbx.lua.gc;
import rbx.lua.string;
import rbx.lua.call;
import rbx.lua.function;
import rbx.lua.hash;
import rbx.lua.aux;
import rbx.lua.thread;

import rbx.object.instance;
import rbx.object.script_context;
import rbx.object.task_scheduler;

import rbx.bypass.retcheck;

static rbx::script_context* script_context = nullptr;

export namespace rbx {
	auto get_task_scheduler = reinterpret_cast<rbx::task_scheduler* (*)()>(mem::rebase(0x11D35E0, 0x400000));
	//auto deserialize = reinterpret_cast<int(__fastcall*)(rbx::state*, const char*, const char*, std::size_t, std::uintptr_t) > (rbx::bypass::retcheck(mem::rebase(0x1717D00, 0x400000)));

	enum class scan_option {
		scan_once,
		rescan,
	};

	rbx::script_context* get_script_context(scan_option option = scan_option::scan_once) {
		if (option == scan_option::rescan || ::script_context == nullptr) {
			for (const auto& job : *get_task_scheduler()) {
				if (job->name == "WaitingScriptJob") {
					const auto waiting_script_job = reinterpret_cast<rbx::task_scheduler::waiting_script_job*>(job.get());
					::script_context = waiting_script_job->script_context;
					break;
				}
			}
		}

		return ::script_context;
	}

	rbx::tvalue* index2adr(rbx::state* thread, int index) {
		if (index > 0) {
			return thread->base + (index - 1);
		}

		return rbx::aux::index2adr(thread, index);
	}

	inline int type(rbx::state* thread, int index) {
		return rbx::index2adr(thread, index)->type;
	}

	inline int get_top(rbx::state* thread) {
		return thread->top - thread->base;
	}

	void set_top(rbx::state* thread, int index) {
		if (index >= 0) {
			while (thread->top < thread->base + index) {
				rbx::set_nil_value(thread->top++);
			}

			thread->top = thread->base + index;
		} else {
			thread->top += index + 1;
		}
	}

	inline void pop(rbx::state* thread, int n) {
		thread->top -= n;
	}

	void insert(rbx::state* thread, int index) {
		const rbx::tvalue* position = rbx::index2adr(thread, index);

		for (rbx::tvalue* value = thread->top; value > position; value--) {
			rbx::set_object(value, value - 1);
		}
	}

	void remove(rbx::state* thread, int index) {
		rbx::tvalue* object = rbx::index2adr(thread, index);

		while (++object < thread->top) {
			rbx::set_object(object - 1, object);
		}

		thread->top--;
	}

	auto f_call = reinterpret_cast<void(*)(rbx::state*, void*)>(mem::rebase(0x17159E0, 0x400000));

	int pcall(rbx::state* thread, std::size_t args, int results) {
		auto savestack = [](rbx::state* thread, rbx::tvalue* object) {
			return reinterpret_cast<std::uintptr_t>(object) - reinterpret_cast<std::uintptr_t>(thread->stack);
		};

		auto call = std::make_pair(thread->top - (args + 1), results);
		int result = rbx::call::pcall(thread, f_call, call, savestack(thread, call.first));

		if (results == rbx::multiple && thread->top >= thread->ci->top) {
			thread->ci->top = thread->top;
		}

		return result;
	}

	std::string_view to_string(rbx::state* thread, int index) {
		const auto value = rbx::string_value(rbx::index2adr(thread, index));
		return { &value->buffer, value->length + reinterpret_cast<std::uintptr_t>(&value->length) };
	}

	inline double to_number(rbx::state* thread, int index) {
		return rbx::number_value(rbx::index2adr(thread, index));
	}

	inline int to_integer(rbx::state* thread, int index) {
		return static_cast<int>(rbx::to_number(thread, index));
	}

	inline bool to_boolean(rbx::state* thread, int index) {
		return rbx::boolean_value(rbx::index2adr(thread, index));
	}

	inline rbx::vector to_vector(rbx::state* thread, int index) {
		return rbx::vector_value(rbx::index2adr(thread, index));
	}

	inline void* to_userdata(rbx::state* thread, int index) {
		const rbx::tvalue* object = rbx::index2adr(thread, index);
		void* ptr = object->value.pointer;

		if (object->type == rbx::userdata_t) {
			return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) + 0x10);
		}

		return ptr;
	}

	inline rbx::state* to_thread(rbx::state* thread, int index) {
		return rbx::thread_value(rbx::index2adr(thread, index));
	}

	inline void push_object(rbx::state* thread, rbx::tvalue* value) {
		if (value == nullptr) {
			rbx::set_nil_value(thread->top);
		} else {
			rbx::set_object(thread->top, value);
		}

		rbx::incr_top(thread);
	}

	inline void push_nil(rbx::state* thread) {
		rbx::set_nil_value(thread->top);
		rbx::incr_top(thread);
	}

	inline void push_value(rbx::state* thread, int index) {
		rbx::set_object(thread->top, rbx::index2adr(thread, index));
		rbx::incr_top(thread);
	}

	inline void push_string(rbx::state* thread, const std::string_view& string) {
		rbx::gc::check(thread);
		
		if (string.data() != nullptr) {
			rbx::set_string_value(thread->top, rbx::str::new_string(thread, string));
		} else {
			rbx::set_nil_value(thread->top);
		}

		rbx::incr_top(thread);
	}

	inline void push_number(rbx::state* thread, double number) {
		rbx::set_number_value(thread->top, number);
		rbx::incr_top(thread);
	}

	inline void push_boolean(rbx::state* thread, bool boolean) {
		rbx::set_boolean_value(thread->top, boolean);
		rbx::incr_top(thread);
	}

	inline void push_vector(rbx::state* thread, const rbx::vector& vector) {
		rbx::set_vector_value(thread->top, vector);
		rbx::incr_top(thread);
	}

	inline void push_light_userdata(rbx::state* thread, void* pointer) {
		rbx::set_pointer_value(thread->top, pointer);
		rbx::incr_top(thread);
	}

	inline void push_cclosure(rbx::state* thread, rbx::cfunction function, std::size_t upvalues) {
		rbx::gc::check(thread);

		auto get_current_env = [](rbx::state* thread) {
			if (thread->ci == thread->base_ci) {
				return reinterpret_cast<rbx::table*>(&thread->l_gt);
			}

			return rbx::closure_value(thread->ci->func)->c.environment;
		};

		rbx::closure* closure = rbx::function::new_cclosure(thread, upvalues, get_current_env(thread));
		//closure->c.obf_f = rbx::callcheck_address - reinterpret_cast<std::uintptr_t>(closure) + 24;
		closure->c.obf_f = 0 - reinterpret_cast<std::uintptr_t>(closure) + 24;
		closure->c.extra_space = function;

		while (upvalues--) {
			rbx::set_object(&(&closure->c.upvals)[upvalues], thread->top + upvalues);
		}

		rbx::set_closure_value(thread->top, closure);
		rbx::incr_top(thread);
	}

	int get_metatable(rbx::state* thread, int index) {
		rbx::tvalue* obj = rbx::index2adr(thread, index);
		if (rbx::table* metatable = ([=]() {
				auto obj_ptr = *reinterpret_cast<std::uintptr_t*>(obj);

				if (obj->type == rbx::table_t) {
					return reinterpret_cast<rbx::table*>(*reinterpret_cast<std::uintptr_t*>(obj_ptr + 20) - (obj_ptr + 20));
				} else if (obj->type == rbx::userdata_t) {
					return reinterpret_cast<rbx::table*>((obj_ptr + 12) + *reinterpret_cast<std::uintptr_t*>(obj_ptr + 12));
				}

				return rbx::get_global_state(thread)->metatables[obj->type];
			})()
		) {
			rbx::set_table_value(thread->top, metatable);
			rbx::incr_top(thread);
			return true;
		}

		return false;
	}

	void get_field(rbx::state* thread, int index, const char* field) {
		rbx::tvalue* object = rbx::index2adr(thread, index);
		rbx::push_object(thread, object);

		if (rbx::get_metatable(thread, -1)) {
			rbx::tvalue* metamethod = rbx::hash::get_string(rbx::table_value(rbx::index2adr(thread, -1)), rbx::str::new_string(thread, "__index"));

			if (metamethod->type == rbx::function_t) {
				rbx::push_object(thread, metamethod);
				rbx::push_object(thread, object);
				rbx::push_string(thread, field);
				rbx::pcall(thread, 2, 1);
				
				return;
			}
		}
		
		rbx::pop(thread, 1);

		if (object->type == rbx::table_t) {
			rbx::push_object(thread, rbx::hash::get_string(rbx::table_value(object), rbx::str::new_string(thread, field)));
		}
	}

	inline void get_global(rbx::state* thread, const char* field) {
		return rbx::get_field(thread, rbx::globals, field);
	}

	inline void get_registry(rbx::state* thread, const char* field) {
		return rbx::get_field(thread, rbx::registry, field);
	}
		
	void raw_geti(rbx::state* thread, int index, int n) {
		rbx::get_global(thread, "rawget");
		rbx::push_object(thread, rbx::index2adr(thread, index));
		rbx::push_number(thread, n);
		rbx::pcall(thread, 2, 1);
	}

	void get_table(rbx::state* thread, int index) {
		rbx::tvalue* object = rbx::index2adr(thread, index);
		rbx::tvalue* key = thread->top - 1;

		rbx::remove(thread, -2);
		rbx::push_object(thread, object);

		if (rbx::get_metatable(thread, -1)) {
			rbx::tvalue* metamethod = rbx::hash::get_string(rbx::table_value(rbx::index2adr(thread, -1)), rbx::str::new_string(thread, "__index"));

			if (metamethod->type == rbx::function_t) {
				rbx::push_object(thread, metamethod);
				rbx::push_object(thread, object);
				rbx::push_object(thread, key);
				rbx::pcall(thread, 2, 1);

				return;
			}
		}

		rbx::pop(thread, 1);

		rbx::get_global(thread, "rawget");
		rbx::push_object(thread, object);
		rbx::push_object(thread, key);
		rbx::pcall(thread, 2, 1);
	}

	int get_upvalue(rbx::state* thread, int index, int n) {
		if (rbx::tvalue* upvalue = rbx::aux::get_upvalue(rbx::closure_value(rbx::index2adr(thread, index)), n)) {
			rbx::set_object(thread->top, upvalue);
			rbx::incr_top(thread);
		} else {
			rbx::push_nil(thread);
		}
	}

	int set_metatable(rbx::state* thread, int index) {
		return 0;
	}

	void set_field(rbx::state* thread, int index, const char* field) {
		rbx::tvalue* object = rbx::index2adr(thread, index);
		rbx::tvalue* value = thread->top - 1;

		if (rbx::get_metatable(thread, index)) {
			rbx::tvalue* metamethod = rbx::hash::get_string(rbx::table_value(rbx::index2adr(thread, -1)), rbx::str::new_string(thread, "__newindex"));

			if (metamethod->type == rbx::function_t) {
				rbx::push_object(thread, metamethod);
				rbx::push_object(thread, object);
				rbx::push_string(thread, field);
				rbx::push_object(thread, value);
				rbx::pcall(thread, 3, 0);

				rbx::remove(thread, index);
				rbx::pop(thread, 1);

				return;
			}
		}

		rbx::pop(thread, 1);

		rbx::get_global(thread, "rawset");
		rbx::push_object(thread, object);
		rbx::push_string(thread, field);
		rbx::push_object(thread, value);
		rbx::pcall(thread, 3, 0);
	}

	inline void set_global(rbx::state* thread, const char* field) {
		return rbx::set_field(thread, rbx::globals, field);
	}

	inline void set_registry(rbx::state* thread, const char* field) {
		return rbx::set_field(thread, rbx::registry, field);
	}

	void raw_seti(rbx::state* thread, int index, int n) {
		rbx::tvalue* value = thread->top - 1;

		rbx::remove(thread, -2);

		rbx::get_global(thread, "rawset");
		rbx::push_object(thread, rbx::index2adr(thread, index));
		rbx::push_number(thread, n);
		rbx::push_object(thread, value);
		rbx::pcall(thread, 3, 0);
	}

	void set_table(rbx::state* thread, int index) {
		rbx::tvalue* object = rbx::index2adr(thread, index);
		rbx::tvalue* key = thread->top - 1;
		rbx::tvalue* value = thread->top - 2;

		rbx::push_object(thread, object);
		rbx::remove(thread, -3);

		if (rbx::get_metatable(thread, -1)) {
			rbx::tvalue* metamethod = rbx::hash::get_string(rbx::table_value(rbx::index2adr(thread, -1)), rbx::str::new_string(thread, "__newindex"));

			if (metamethod->type == rbx::function_t) {
				rbx::push_object(thread, metamethod);
				rbx::push_object(thread, object);
				rbx::push_object(thread, key);
				rbx::push_object(thread, value);
				rbx::pcall(thread, 3, 0);

				return;
			}
		}

		rbx::pop(thread, 1);

		rbx::get_global(thread, "rawset");
		rbx::push_object(thread, object);
		rbx::push_object(thread, key);
		rbx::push_object(thread, value);
		rbx::pcall(thread, 3, 0);
	}

	int set_upvalue(rbx::state* thread, int index, int n) {
		rbx::closure* closure = rbx::closure_value(rbx::index2adr(thread, index));

		if (rbx::tvalue* upvalue = rbx::aux::get_upvalue(closure, n)) {
			rbx::decr_top(thread);

			rbx::set_object(upvalue, thread->top);
			rbx::gc::barrierf(thread, reinterpret_cast<rbx::gcobject*>(rbx::aux::get_upvalue(closure, n)), rbx::gc_value(thread->top));
		}
	}

	namespace aux {
		int ref(rbx::state* thread, int index) {
			if (index == 0 || rbx::type(thread, -1) == rbx::nil_t) {
				rbx::pop(thread, 1);
				return static_cast<int>(rbx::ref::nil);
			}

			rbx::raw_geti(thread, index, rbx::ref::free_list);

			int ref = rbx::to_integer(thread, -1);

			rbx::pop(thread, 1);

			if (ref) {
				rbx::raw_geti(thread, index, ref);
				rbx::raw_seti(thread, index, rbx::ref::free_list);
			}
			else {
				rbx::get_registry(thread, "getn");
				rbx::push_object(thread, rbx::index2adr(thread, index));
				rbx::pcall(thread, 1, 1);
				ref = rbx::number_value(rbx::index2adr(thread, index)) + 1;
			}

			rbx::raw_seti(thread, index, ref);
			return ref;
		}
	}

	void new_table(rbx::state* thread) {
		rbx::get_registry(thread, "createTable");
		rbx::pcall(thread, 0, 1);
	}

	rbx::state* new_thread(rbx::state* thread) {
		rbx::state* new_thread = rbx::thread::new_thread(thread);
		rbx::gc::check(thread);
		
		rbx::set_thread_value(thread->top, new_thread);
		rbx::incr_top(thread);

		if (auto userstatethread = rbx::get_global_state(thread)->userstatethread) {
			userstatethread(thread, new_thread);
		}

		return new_thread;
	}

	void* new_userdata(rbx::state* thread, std::size_t size) {
		rbx::gc::check(thread);
		auto userdata = reinterpret_cast<rbx::userdata*>(rbx::str::new_userdata(thread, size, 0));
		rbx::set_userdata_value(thread->top, userdata);
		rbx::incr_top(thread);

		return userdata + 1;
	}

	int next(rbx::state* thread, int index) {
		rbx::tvalue* key = thread->top - 1;

		rbx::get_global(thread, "next");
		rbx::push_object(thread, rbx::index2adr(thread, index));
		rbx::push_object(thread, key);
		rbx::pcall(thread, 2, 2);

		bool is_nil = rbx::type(thread, -1) != rbx::nil_t;

		if (is_nil) {
			rbx::pop(thread, 1);
		}

		return is_nil;
	}
}
