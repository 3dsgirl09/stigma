module;

#include <string_view>

export module rbx.lua.gc;

import mem;

import rbx.lua.object;
import rbx.lua.constants;

export namespace rbx {
	rbx::global_state* get_global_state(rbx::state* thread) {
		return reinterpret_cast<rbx::global_state*>(reinterpret_cast<std::uintptr_t>(thread->global_state) - reinterpret_cast<std::uintptr_t>(&thread->global_state));
	}

	namespace gc {
		auto step = reinterpret_cast<std::uintptr_t(__fastcall*)(rbx::state*, char)>(mem::rebase(0x175D950, 0x400000));
		auto really_mark_object = reinterpret_cast<void(__fastcall*)(rbx::global_state*, rbx::gcobject*)>(mem::rebase(0x175C600, 0x400000));

		inline std::uint8_t white(rbx::global_state* g) {
			return g->currentwhite & 3;
		};

		void link(rbx::state* thread, rbx::gcobject* obj, std::uint8_t type) {
			rbx::global_state* global_state = rbx::get_global_state(thread);
			obj->gch.next = global_state->rootgc;
			global_state->rootgc = obj;
			obj->gch.type = type;
			obj->gch.marked = white(global_state);
		};

		inline void check(rbx::state* thread) {
			if (const rbx::global_state* global_state = rbx::get_global_state(thread); global_state->totalbytes >= global_state->gcthreshold) {
				rbx::gc::step(thread, 1);
			}
		};

		inline void make_white(rbx::global_state* global_state, rbx::gcobject* object) {
			object->gch.marked &= 0xFB | global_state->currentwhite & 3;
		}

		void barrierf(rbx::state* thread, rbx::gcobject* object, rbx::gcobject* value) {
			rbx::global_state* global_state = rbx::get_global_state(thread);

			if (global_state->gcstate == 1) {
				rbx::gc::really_mark_object(global_state, value);
			} else {
				rbx::gc::make_white(global_state, object);
			}
		}

		void barrierback(rbx::state* thread, rbx::table* t) {
			rbx::global_state* global_state = rbx::get_global_state(thread);
			auto gc_object = reinterpret_cast<rbx::gcobject*>(t);

			gc_object->gch.marked &= 0xFB;
			//t->gclist = global_state->grayagain;
			global_state->grayagain = gc_object;
		};

		inline bool iscollectible(rbx::gcobject* obj) {
			return obj->gch.type <= rbx::string_t;
		};

		inline std::uint8_t isgray(rbx::gcobject* obj) {
			return (obj->gch.marked & 7 ) == 0;
		}

		inline std::uint8_t isblack(rbx::gcobject* obj) {
			return obj->gch.marked & 3;
		};

		inline std::uint8_t iswhite(rbx::gcobject* obj) {
			return obj->gch.marked & 4;
		};

		inline std::uint8_t isdead(rbx::global_state* global_state, rbx::gcobject* obj) {
			return global_state->currentwhite & 3;
		}

		inline std::uint8_t changewhite(rbx::gcobject* obj) {
			return obj->gch.marked ^ 3;
		}

		inline void gray2black(rbx::gcobject* obj) {
			obj->gch.marked |= 4;
		}

		inline void objbarrier(rbx::state* thread, rbx::gcobject* obj, rbx::gcobject* value) {
			if (rbx::gc::iswhite(obj) && rbx::gc::isblack(value)) {
				rbx::gc::barrierf(thread, obj, value);
			}
		};

		inline void barriert(rbx::state* thread, rbx::table* t, rbx::gcobject* obj) {
			if (rbx::gc::iscollectible(obj) && rbx::gc::iswhite(obj) && rbx::gc::isblack(obj)) {
				rbx::gc::barrierback(thread, t);
			}
		};

		void link_upvalue(rbx::state* thread, rbx::upval* upvalue) {
			rbx::global_state* global_state = rbx::get_global_state(thread);
			auto object = reinterpret_cast<rbx::gcobject*>(upvalue);

			upvalue->gch.next = global_state->rootgc;
			global_state->rootgc = object;

			if (isgray(object)) {
				if (global_state->gcstate == 1) {
					rbx::gc::gray2black(object);
					rbx::gc::barrierf(thread, object, upvalue->value->value.gc);
				}
				else {
					rbx::gc::make_white(global_state, object);
				}
			}
		}
	}
}
