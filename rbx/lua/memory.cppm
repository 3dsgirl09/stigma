module;

#include <cstddef>
#include <cstdint>
#include <cstring>

export module rbx.lua.memory;

import mem;

import rbx.lua.object;
import rbx.lua.constants;

import rbx.lua.gc;

namespace rbx::memory {
	auto nhash = reinterpret_cast<uint8_t*>(mem::rebase(0x2DBCBD8, 0x400000));
	auto alt_realloc = reinterpret_cast<void* (*__fastcall)(rbx::state*, char)>(mem::rebase(0x1764CF0, 0x400000));
	auto too_big = reinterpret_cast<void(__cdecl*)(rbx::state*)>(mem::rebase(0x17919F0, 0x400000));
	auto throw_error = reinterpret_cast<void(__fastcall*)(rbx::state*, int)>(mem::rebase(0x172F320, 0x400000));

	void* realloc(rbx::state* thread, std::size_t new_size, std::uint32_t mod_key) {
		rbx::global_state* global_state = rbx::get_global_state(thread);
		auto global_state_ptr = reinterpret_cast<std::uintptr_t>(global_state);
		void* block = ([=]() {
			if ((new_size - 1) >= 0x200 || nhash[new_size] < 0) {
				return global_state->frealloc(thread, global_state->ud, 0, 0, new_size);
			}

			return alt_realloc(thread, nhash[new_size]);
		})();

		if (block == nullptr && new_size) {
			throw_error(thread, 4);
		}

		*reinterpret_cast<std::uintptr_t*>(global_state_ptr + 64) += new_size;
		*reinterpret_cast<std::uintptr_t*>(global_state_ptr + 4 * mod_key + 204) += new_size;

		return block;
	}

	export inline void* alloc(rbx::state* thread, std::size_t size) {
		return rbx::memory::realloc(thread, size, *reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(thread) + 104));
	};
}
