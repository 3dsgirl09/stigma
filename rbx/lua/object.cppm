module;

#include <cstdint>
#include <cstddef>
#include <ostream>
#include <string_view>

export module rbx.lua.object;

import mem;
import rbx.lua.constants;

auto number_xor = *reinterpret_cast<std::uint64_t*>(mem::rebase(0x2F69820, 0x400000));

export namespace rbx {
	union gcobject;
	struct state;
	struct closed;
	struct table;
	struct string;

	struct vector {
		float x, y, z;
	};

#pragma pack(1)
	struct tvalue {
		union {
			rbx::gcobject* gc;
			void* pointer;
			bool boolean;
			std::uint64_t xored_number;
			rbx::vector vector;
		} value;
		std::uint32_t type;
	}; 
#pragma pack(pop)

	struct call_info {
		rbx::tvalue* base;
		rbx::tvalue* func;
		rbx::tvalue* top;
		std::uint8_t _[4];
	};

	struct string_table {
		std::size_t size;
		std::size_t elements;
		rbx::gcobject** hash;
	};

	struct gcheader {
		rbx::gcobject* next;
		std::uint8_t marked;
		std::uint8_t modkey;
		std::uint8_t type;
	};

	struct upval {
		rbx::gcheader gch; // 0 + 7
		std::uint8_t _; // 7 + 1
		rbx::tvalue* value; // 8 + 4
		std::uint8_t __[4];
		
		union value_t {
			rbx::tvalue value; // 16 + 16
			struct linked_t {
				rbx::upval* previous;
				rbx::upval* next;
			} l;
		} u;
	};
	
	using alloc_t = void* (*)(rbx::state*, void*, void*, size_t, size_t);
	using yielding_t = void(*)(rbx::state*);
	using userstatethread_t = void(*)(rbx::state*, rbx::state*);

	struct global_state {
		rbx::string_table string_table;
		rbx::alloc_t frealloc;
		void* ud;
		std::uint8_t currentwhite;
		std::uint8_t gcstate;
		std::uint8_t __[2];
		rbx::gcobject* weak;
		rbx::gcobject* grayagain;
		int sweepstrgc;
		rbx::gcobject* rootgc;
		rbx::gcobject** sweepgc;
		rbx::gcobject* gray;
		std::uint8_t ___[12];
		std::size_t totalbytes;
		std::size_t gcthreshold;
		std::uint8_t ____[1236];
		rbx::upval uvhead;
		std::uint8_t _____[40];
		rbx::table* metatables[rbx::thread_t + 1];
		rbx::state* mainthread;
		std::uint8_t _______[4];
		rbx::tvalue registry;
		std::uint8_t ________[568];
		rbx::userstatethread_t userstatethread;
		std::uint8_t _________[16];
		rbx::yielding_t yielding;
	};

	struct state {
		rbx::gcheader gch;
		std::uint8_t status;
		std::uint8_t _[4];
		rbx::tvalue* stack;
		rbx::tvalue* stack_last;
		rbx::tvalue* top;
		rbx::tvalue* base;
		rbx::global_state* global_state;
		rbx::call_info* ci;
		std::uint8_t ___[4];
		rbx::call_info* base_ci;
		std::size_t stack_size;
		std::size_t size_ci;
		std::uint16_t nCcalls;
		std::uint8_t _____[2];
		rbx::tvalue l_gt;
		rbx::tvalue env;
		rbx::gcobject* open_upvalue;
		std::uint8_t ______[4];
		rbx::string* namecall;
		std::uint8_t _______[4];
		std::uint8_t modkey;
	};

	struct node {
		rbx::tvalue value;
		rbx::tvalue key;
	};

	struct table {
		rbx::gcheader gch;
		std::uint8_t _[2];
		std::uint8_t readonly;
		std::uint8_t __;
		std::uint8_t log2_sizenode;
		std::uint8_t ___[8];
		rbx::table* obf_metatable;
		rbx::node* node;
	};

	struct string {
		rbx::gcheader gch; // 0 + 7
		std::uint8_t _; // 7 + 1
		std::uint8_t __[4]; // 8 + 4
		std::size_t hash; // 12 + 4
		std::size_t length; // 16 + 4
		char buffer;

		friend std::ostream& operator<<(std::ostream&, const rbx::string&);
	};

	std::ostream& operator<<(std::ostream& os, const rbx::string& string) {
		os << std::string_view(&string.buffer, string.length - reinterpret_cast<std::uintptr_t>(&string.length));
		return os;
	}

	struct userdata {
		rbx::gcheader gch;
		std::uint8_t flag;
		std::size_t size;
		rbx::table* metatable;
	};

	struct closed {
		rbx::upval* next;
		rbx::upval* prev;
		rbx::tvalue* value;
	};

	struct lclosure {
		rbx::gcheader gch;
		std::uint8_t isC; // [7] + 1
		std::uint8_t upvalues; // [8] + 1
		std::uint8_t maxstacksize; // [9] + 1
		std::uint8_t __[6]; // [10] + 6
		rbx::table* environment;
		std::uint8_t ___[4];
		std::uintptr_t obf_p;
		std::uint8_t ____[4];
		rbx::upval upvals;
	};

	using cfunction = int(*)(rbx::state*);

	struct cclosure {
		rbx::gcheader gch;
		std::uint8_t isC;
		std::uint8_t upvalues;
		rbx::gcobject* gclist;
		rbx::table* environment;
		std::uintptr_t obf_f;
		rbx::cfunction extra_space;
		rbx::tvalue upvals;
	};

	union closure {
		rbx::lclosure l;
		rbx::cclosure c;
	};

	union gcobject {
		rbx::gcheader gch;
		rbx::closure closure;
		rbx::table table;
		rbx::string string;
		rbx::userdata userdata;
		rbx::state thread;
		rbx::upval upvalue;
	};

	inline void incr_top(rbx::state* thread) {
		thread->top++;
	}

	inline void decr_top(rbx::state* thread) {
		thread->top--;
	}

	inline rbx::gcobject* gc_value(rbx::tvalue* value) {
		return value->value.gc;
	}

	inline rbx::table* table_value(rbx::tvalue* value) {
		return &value->value.gc->table;
	}

	inline rbx::closure* closure_value(rbx::tvalue* value) {
		return &value->value.gc->closure;
	}

	inline rbx::string* string_value(rbx::tvalue* value) {
		return &value->value.gc->string;
	}

	inline rbx::userdata* userdata_value(rbx::tvalue* value) {
		return &value->value.gc->userdata;
	}

	inline rbx::state* thread_value(rbx::tvalue* value) {
		return &value->value.gc->thread;
	}

	inline bool boolean_value(rbx::tvalue* value) {
		return value->value.boolean;
	}

	inline double number_value(rbx::tvalue* value) {
		double result{};
		*reinterpret_cast<uint64_t*>(&result) = value->value.xored_number ^ number_xor;
		return result;
	}

	inline void* pointer_value(rbx::tvalue* value) {
		return value->value.pointer;
	}

	inline rbx::vector vector_value(rbx::tvalue* value) {
		return value->value.vector;
	}

	inline void set_object(rbx::tvalue* object, rbx::tvalue* value) {
		*object = *value;
	}

	inline void set_string_value(rbx::tvalue* object, rbx::string* string) {
		object->value.gc = reinterpret_cast<rbx::gcobject*>(string);
		object->type = rbx::string_t;
	}

	inline void set_closure_value(rbx::tvalue* object, rbx::closure* closure) {
		object->value.gc = reinterpret_cast<rbx::gcobject*>(closure);
		object->type = rbx::function_t;
	}

	inline void set_table_value(rbx::tvalue* object, rbx::table* table) {
		object->value.gc = reinterpret_cast<rbx::gcobject*>(table);
		object->type = rbx::table_t;
	}

	inline void set_userdata_value(rbx::tvalue* object, rbx::userdata* userdata) {
		object->value.gc = reinterpret_cast<rbx::gcobject*>(userdata);
		object->type = rbx::userdata_t;
	}

	inline void set_thread_value(rbx::tvalue* object, rbx::state* thread) {
		object->value.gc = reinterpret_cast<rbx::gcobject*>(thread);
		object->type = rbx::thread_t;
	}

	inline void set_number_value(rbx::tvalue* object, double number) {
		object->value.xored_number = number_xor ^ *reinterpret_cast<std::uint64_t*>(&number);
		object->type = rbx::number_t;
	}
	
	inline void set_boolean_value(rbx::tvalue* object, bool boolean) {
		object->value.boolean = boolean;
		object->type = rbx::boolean_t;
	}
	
	inline void set_pointer_value(rbx::tvalue* object, void* pointer) {
		object->value.pointer = pointer;
		object->type = rbx::light_userdata_t;
	}

	inline void set_vector_value(rbx::tvalue* object, const rbx::vector& vector) {
		object->value.vector = vector;
		object->type = rbx::vector_t;
	}

	inline void set_nil_value(rbx::tvalue* object) {
		object->type = rbx::nil_t;
	}

	rbx::tvalue nil_object;
}
