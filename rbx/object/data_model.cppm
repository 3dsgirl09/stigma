module;

#include <iostream>
#include <string>
#include <memory>
#include <Windows.h>

export module rbx.object.data_model;

import rbx.object.instance;

char test() {
	std::cout << "hello\n";
	return 0;
}

export namespace rbx {
	class data_model : public rbx::instance {
	private:
		std::uint8_t _[656];
	public:
		std::string job_id; //0x02C4
	private:
		std::uint8_t ___[116]; //0x02DC
	public:
		std::string server_address; //0x0354
	};
}
