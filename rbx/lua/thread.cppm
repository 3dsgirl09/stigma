module;

#include <cstdint>
#include <cstddef>
#include <cstring>

export module rbx.lua.thread;

import rbx.lua.constants;
import rbx.lua.object;

import rbx.lua.memory;
import rbx.lua.gc;

export namespace rbx::thread {
	rbx::state* new_thread(rbx::state* thread) {
		auto new_thread = reinterpret_cast<rbx::state*>(rbx::memory::alloc(thread, 112));
		std::memset(new_thread, 0, 112);

		rbx::gc::link(thread, reinterpret_cast<rbx::gcobject*>(new_thread), rbx::thread_t);

		new_thread->global_state = reinterpret_cast<rbx::global_state*>(reinterpret_cast<std::uintptr_t>(&new_thread->global_state) + reinterpret_cast<std::uintptr_t>(rbx::get_global_state(thread)));
		new_thread->modkey = thread->modkey;

		new_thread->base_ci = reinterpret_cast<rbx::call_info*>(rbx::memory::alloc(thread, 192));
		new_thread->ci = new_thread->base_ci;	
		new_thread->size_ci = 8;
		
		constexpr std::size_t basic_stack_size = 45;
		new_thread->stack = reinterpret_cast<rbx::tvalue*>(rbx::memory::alloc(thread, basic_stack_size * sizeof(rbx::tvalue)));
		new_thread->stack_size = basic_stack_size - reinterpret_cast<std::uintptr_t>(&new_thread->stack_size);

		for (std::size_t i = 0; i < basic_stack_size; i++) {
			rbx::set_nil_value(new_thread->stack + i);
		}

		new_thread->l_gt = thread->l_gt;
		new_thread->top = new_thread->stack;
		new_thread->stack_last = new_thread->stack + ((reinterpret_cast<std::uintptr_t>(&new_thread->stack_size) + new_thread->stack_size) - 6);
		new_thread->ci->func = new_thread->stack;
		rbx::set_nil_value(new_thread->top++);
		new_thread->ci->base = thread->top;
		new_thread->base = new_thread->top;
		new_thread->ci->top = new_thread->top + 20;

		return new_thread;
	}
}
