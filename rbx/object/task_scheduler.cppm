module;

#include <vector>
#include <string>
#include <memory>
#include <functional>

#include <Windows.h>

export module rbx.object.task_scheduler;

import rbx.object.instance;
import rbx.object.script_context;

using task_caller_t = char(*)();

static std::vector<std::function<void()>> call_stack{};
static task_caller_t original_caller = nullptr;

static char task_caller() {
	if (!call_stack.empty()) {
		for (const auto& function : call_stack) {
			function();
		}

		call_stack.clear();
	}

	return original_caller();
}

export namespace rbx {
	class task_scheduler {
	public:
		class job {
		private:
			void* vtable;
			std::uint8_t _[12];
		public:
			std::string name;

			void hook() {
				constexpr std::size_t vtable_size = 40;
				constexpr std::size_t ptr_size = sizeof std::uintptr_t;

				if (original_caller == nullptr) {
					auto vtable = new std::uintptr_t[vtable_size];
					DWORD old_protection{};

					memcpy(vtable, this->vtable, vtable_size);
					original_caller = reinterpret_cast<task_caller_t>(vtable[1]);
					vtable[1] = reinterpret_cast<std::uintptr_t>(task_caller);

					VirtualProtect(this, ptr_size, PAGE_READWRITE, &old_protection);
					this->vtable = vtable;
					VirtualProtect(this, ptr_size, old_protection, &old_protection);
				}
			}
		};

		class waiting_script_job : public job {
		private:
			std::uint8_t __[264];
		public:
			rbx::script_context* script_context;
		};

		using iterator = std::vector<std::shared_ptr<job>>::iterator;
	private:
		std::uint8_t _[300];
		iterator _begin, _end;
	public:
		task_scheduler() = delete;
		task_scheduler(const task_scheduler&) = delete;
		task_scheduler(const task_scheduler&&) = delete;

		iterator begin() {
			return _begin;
		}

		iterator end() {
			return _end;
		}

		static void push(const std::function<void()>& function) {
			call_stack.push_back(function);
		}

		void hook_job() {
			if (original_caller == nullptr) {
				for (const auto& job : *this) {
					if (job->name == "WaitingScriptJob") {
						job->hook();
						return;
					}
				}
			}
		}
	};
}