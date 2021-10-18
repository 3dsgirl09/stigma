module;

#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <format>

export module rbx.object.instance;

export namespace rbx {
	namespace reflection {
		class class_descriptor {
		private:
			std::uint8_t _[4];
		public:
			std::string& class_name;
		};
	}

	class instance {
	private:
		std::uint8_t _[12];
	public:
		rbx::reflection::class_descriptor& descriptor;
	private:
		std::uint8_t __[24];
	public:
		std::string& name;
		std::shared_ptr<std::vector<std::shared_ptr<rbx::instance>>> children;
		rbx::instance* parent;
	public:
		using child_iterator = std::vector<std::shared_ptr<rbx::instance>>::iterator;

		child_iterator begin() {
			return children->begin();
		}

		child_iterator end() {
			return children->end();
		}

		rbx::instance* find_first_child(const std::string_view& name) {
			if (this != nullptr) {
				for (const auto& child : *this) {
					if (child->name == name) {
						return child.get();
					}
				}

				return nullptr;
			}

			throw std::runtime_error(std::format("Attempt to search for \"{}\" in a nil instance", name.data()));
		}

		rbx::instance* find_first_child_of_class(const std::string_view& class_name) {
			if (this != nullptr) {
				for (const auto& child : *this) {
					if (child->descriptor.class_name == class_name) {
						return child.get();
					}
				}

				return nullptr;
			}

			throw std::runtime_error(std::format("Attempt to search for class of \"{}\" in a nil instance", class_name.data()));
		}
	};
}
