module;

#include <string>
#include <string_view>
#include <memory>
#include <vector>

export module rbx.object.player;

import rbx.object.instance;

export namespace rbx {
	class player : public instance {
	private:
		std::uint8_t _[120];
	public:
		rbx::instance* team;
	};
}
