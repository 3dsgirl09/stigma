module;

#include <cstdint>

export module rbx.lua.constants;

export namespace rbx {
	enum types {
		nil_t,
		boolean_t,
		vector_t,
		number_t,
		light_userdata_t,
		string_t,
		table_t,
		function_t,
		userdata_t,
		thread_t,
		proto_t,
		upvalue_t,
	};

	enum indices {
		globals = -10002,
		environment = -10001,
		registry = -10000,
	};

	enum results {
		multiple = -1
	};

	enum ref {
		nil = -1,
		free_list,
	};

	enum limits {
		max_calls = 20000,
	};
}
