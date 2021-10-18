module;

#include <cstddef>
#include <cstdint>
#include <utility>

export module rbx.lua.call;

import mem;

import rbx.lua.object;
import rbx.lua.constants;

import rbx.lua.gc;
import rbx.lua.function;

namespace rbx::call {
	using protected_function_t = void(*)(rbx::state*, void*);

	auto realloc_call_info = reinterpret_cast<void(__fastcall*)(rbx::state*, std::size_t)>(mem::rebase(0x1758b30, 0x400000));
	auto rawrunprotected = reinterpret_cast<int(__fastcall*)(rbx::state*, protected_function_t, void*)>(mem::rebase(0x172F270, 0x400000));
	auto set_error_object = reinterpret_cast<rbx::tvalue*(__fastcall*)(rbx::state*, int, rbx::tvalue*)>(mem::rebase(0x172F5A0, 0x400000));

	void restore_stack_limit(rbx::state* thread) {
		if (thread->size_ci > rbx::max_calls) {
			if (static_cast<std::uintptr_t>(thread->ci - thread->base_ci) + 1 < rbx::max_calls) {
				realloc_call_info(thread, rbx::max_calls);
			}
		}
	}

	std::ptrdiff_t save_call_info(rbx::state* thread, rbx::call_info* call_info) {
		return reinterpret_cast<std::ptrdiff_t>(call_info) - reinterpret_cast<std::ptrdiff_t>(thread->base_ci);
	}

	rbx::call_info* restore_call_info(rbx::state* thread, std::ptrdiff_t old_call_info) {
		return reinterpret_cast<rbx::call_info*>(reinterpret_cast<std::ptrdiff_t>(thread->base_ci) + old_call_info);
	}

	rbx::tvalue* restore_stack(rbx::state* thread, std::ptrdiff_t old_top) {
		return reinterpret_cast<rbx::tvalue*>(reinterpret_cast<std::ptrdiff_t>(thread->stack) + old_top);
	}

	export int pcall(rbx::state* thread, protected_function_t function, std::pair<rbx::tvalue*, int>& ud, std::ptrdiff_t old_top) {
		std::uint16_t old_c_calls = thread->nCcalls;
		std::ptrdiff_t old_call_info = save_call_info(thread, thread->ci);

		int status = rbx::call::rawrunprotected(thread, function, &ud);

		if (status) {
			if (rbx::global_state* global_state = rbx::get_global_state(thread); global_state->yielding) {
				global_state->yielding(thread);

				if (thread->status == 6) {
					return 0;
				}
			}

			rbx::tvalue* restored_top = restore_stack(thread, old_top);
			rbx::function::close(thread, restored_top);
			rbx::call::set_error_object(thread, status, restored_top);

			thread->nCcalls = old_c_calls;
			thread->ci = restore_call_info(thread, old_call_info);
			thread->base = thread->ci->base;

			restore_stack_limit(thread);
		}

		return status;
	}
}
