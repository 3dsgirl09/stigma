module;

export module rbx.lua.vm;

import rbx.lua.object;
import rbx.lua.constants;
import rbx.lua.api;

import rbx.lua.hash;
import rbx.lua.string;

export namespace rbx::vm {
	void get_table(rbx::state* thread, rbx::tvalue* object, rbx::tvalue* key, rbx::tvalue* value) {
		rbx::push_object(thread, object);

		if (rbx::get_metatable(thread, -1)) {
			rbx::tvalue* metamethod = rbx::hash::get_string(rbx::table_value(rbx::index2adr(thread, -1)), rbx::str::new_string(thread, "__index"));

			if (metamethod->type == rbx::function_t) {
				rbx::push_object(thread, metamethod);
				rbx::push_object(thread, object);
				rbx::push_object(thread, key);
				rbx::pcall(thread, 2, 1);

				rbx::set_object(value, rbx::index2adr(thread, -1));
				return;
			}
		}

		rbx::pop(thread, 1);

		rbx::get_global(thread, "rawget");
		rbx::push_object(thread, object);
		rbx::push_object(thread, key);
		rbx::pcall(thread, 2, 1);

		rbx::set_object(value, rbx::index2adr(thread, -1));
	}

	void set_table(rbx::state* thread, rbx::tvalue* object, rbx::tvalue* key, rbx::tvalue* value) {
		rbx::push_object(thread, object);

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
}

