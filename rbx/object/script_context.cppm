module;

#include <cstdint>

export module rbx.object.script_context;

import mem;

import rbx.lua.object;

import rbx.object.instance;

namespace rbx {
	class script_context;

	auto get_lua_state = reinterpret_cast<rbx::state * (__thiscall*)(rbx::script_context*, std::uint32_t&)>(mem::rebase(0x7C33F0, 0x400000));

	export class script_context : public rbx::instance {
	public:
		rbx::state* get_lua_state() {
			std::uint32_t level = 0;
			return rbx::get_lua_state(this, level);
		}
	};
}
