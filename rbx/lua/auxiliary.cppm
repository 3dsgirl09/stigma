module;

export module rbx.lua.aux;

import mem;

import rbx.lua.object;
import rbx.lua.constants;

export namespace rbx::aux {
	auto index2adr = reinterpret_cast<rbx::tvalue * (__fastcall*)(rbx::state*, int)>(mem::rebase(0x1715080, 0x400000));

	rbx::tvalue* get_upvalue(rbx::closure* cl, int n) {
		if (n >= 1 && n <= cl->c.upvalues) {
			if (cl->c.isC) {
				return &(&cl->c.upvals)[n - 1];
			}

			rbx::upval upvalue = (&cl->l.upvals)[n - 1];

			if (upvalue.value->type == rbx::upvalue_t) {
				return &upvalue.u.value;
			}

			return upvalue.value;
		}

		return nullptr;
	}
}
