module;

#include <cstddef>
#include <cstdint>

export module rbx.lua.function;

import mem;

import rbx.lua.object;
import rbx.lua.constants;
import rbx.lua.gc;
import rbx.lua.memory;

export namespace rbx::function {
	rbx::closure* new_cclosure(rbx::state* thread, std::size_t elements, rbx::table* environment) {
		auto closure = reinterpret_cast<rbx::closure*>(rbx::memory::alloc(thread, (sizeof(rbx::tvalue) * elements) + 40));

		rbx::gc::link(thread, reinterpret_cast<rbx::gcobject*>(closure), rbx::function_t);

		closure->c.isC = true;
		closure->c.environment = environment;
		closure->c.upvalues = elements;

		return closure;
	}

	void unlink_upvalue(rbx::upval* upvalue) {
		upvalue->u.l.next->u.l.previous = upvalue->u.l.previous;
		upvalue->u.l.previous->u.l.next = upvalue->u.l.next;
	}

	auto alloc_flag = *reinterpret_cast<std::uint8_t*>(mem::rebase(0x2DBCBF8, 0x400000));
	auto foo = reinterpret_cast<void(__fastcall*)(rbx::state*, int, rbx::gcobject*)>(mem::rebase(0x1764DE0, 0x400000));

	void free_upvalue(rbx::state* thread, rbx::upval* upvalue) {
		rbx::global_state* global_state = rbx::get_global_state(thread);
		auto global_state_ptr = reinterpret_cast<std::uintptr_t>(global_state);

		if (upvalue->value != &upvalue->u.value) {
			rbx::function::unlink_upvalue(upvalue);
		}

		if (alloc_flag < 0) {
			global_state->frealloc(thread, global_state->ud, upvalue, sizeof rbx::upval, 0);
		}
		else {
			foo(thread, alloc_flag, reinterpret_cast<rbx::gcobject*>(upvalue));
		}

		*reinterpret_cast<std::uintptr_t*>(global_state_ptr + 64) -= 32;
		*reinterpret_cast<std::uintptr_t*>(global_state_ptr + 4 * upvalue->gch.marked + 204) -= 32;
	}

	void close(rbx::state* thread, rbx::tvalue* level) {
		rbx::global_state* global_state = rbx::get_global_state(thread);
		rbx::upval* upvalue = nullptr;

			if (thread->open_upvalue != nullptr && (upvalue = &thread->open_upvalue->upvalue)->value >= level) {
			auto object = reinterpret_cast<rbx::gcobject*>(upvalue);

			thread->open_upvalue = reinterpret_cast<rbx::gcobject*>(upvalue->u.l.next);

			if (rbx::gc::isdead(global_state, object)) {
				rbx::function::free_upvalue(thread, upvalue);
			}
			else {
				rbx::function::unlink_upvalue(upvalue);
				rbx::set_object(&upvalue->u.value, upvalue->value);
				upvalue->value = &upvalue->u.value;
				rbx::gc::link_upvalue(thread, upvalue);
			}
		}
	}
}
