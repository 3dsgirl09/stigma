module;

#include <string_view>

export module rbx.lua.string;

import mem;

import rbx.lua.object;
import rbx.lua.constants;

import rbx.lua.gc;
import rbx.lua.memory;

namespace rbx::str {
	export auto new_userdata = reinterpret_cast<void* (__fastcall*)(rbx::state*, std::size_t, char)>(mem::rebase(0x175F610, 0x400000));
	auto resize = reinterpret_cast<std::uintptr_t(__fastcall*)(rbx::state*, std::uintptr_t)>(mem::rebase(0x178C2F0, 0x400000));

	export rbx::string* get_cached_string(rbx::state* thread, const std::string_view& string, std::size_t hash) {
		rbx::global_state* global_state = rbx::get_global_state(thread);
		hash &= global_state->string_table.size - 1;

		for (rbx::gcobject* itr = global_state->string_table.hash[hash]; itr != nullptr; itr = itr->gch.next) {
			if (auto cached_string = reinterpret_cast<rbx::string*>(itr); cached_string->length + reinterpret_cast<std::uintptr_t>(&cached_string->length) == string.size() && string == (&cached_string->buffer)) {
				if (rbx::gc::isdead(global_state, itr)) {
					rbx::gc::changewhite(itr);
				}

				return cached_string;
			}
		}

		return nullptr;
	}

	export inline std::size_t calculate_hash(const std::string_view& string) {
		std::size_t hash = string.size();
		std::size_t length = string.size();

		for (std::size_t i = string.size(); i >= 1; i--) {
			hash ^= (hash >> 2) + (hash << 5) + static_cast<std::uint8_t>(string[i - 1]);
		}

		return hash;
	}

	export rbx::string* new_string(rbx::state* thread, const std::string_view& string) {
		std::size_t hash = calculate_hash(string);
		rbx::global_state* global_state = rbx::get_global_state(thread);

		if (rbx::string* cached_string = get_cached_string(thread, string, hash)) {
			return cached_string;
		}

		auto new_string = reinterpret_cast<rbx::string*>(rbx::memory::alloc(thread, string.size() + 21));

		new_string->gch.type = rbx::string_t;
		new_string->gch.marked = rbx::gc::white(global_state);
		new_string->gch.modkey = thread->modkey;
		new_string->hash = reinterpret_cast<std::uintptr_t>(&new_string->hash) - hash;
		new_string->length = string.size() - reinterpret_cast<std::uintptr_t>(&new_string->length);

		strncpy(&new_string->buffer, string.data(), string.size());
		(&new_string->buffer)[string.size()] = 0;

		const std::size_t cache_size = global_state->string_table.size;
		hash &= cache_size - 1;

		new_string->gch.next = global_state->string_table.hash[hash];
		global_state->string_table.hash[hash] = reinterpret_cast<rbx::gcobject*>(new_string);

		if (global_state->string_table.elements++ > cache_size && cache_size <= 0x3FFFFFFF) {
			rbx::str::resize(thread, 2 * cache_size);
		}

		return new_string;
	}
}
