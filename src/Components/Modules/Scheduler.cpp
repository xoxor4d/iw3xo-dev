#include "STDInclude.hpp"

namespace Components
{
	std::mutex Scheduler::mutex_;
	std::queue<std::pair<std::string, int>> Scheduler::errors_;
	Utils::concurrent_list<std::pair<std::function<void()>, Scheduler::thread>> Scheduler::callbacks_;
	Utils::concurrent_list<std::pair<std::function<void()>, Scheduler::thread>> Scheduler::single_callbacks_;
	Utils::concurrent_list<std::pair<std::function<Scheduler::evaluation()>, Scheduler::thread>> Scheduler::condition_callbacks_;

	void Scheduler::on_frame(const std::function<void()>& callback, const thread thread)
	{
		callbacks_.add({ callback, thread });
	}

	void Scheduler::delay(const std::function<void()>& callback, const std::chrono::milliseconds delay, const thread thread)
	{
		const auto begin = std::chrono::high_resolution_clock::now();

		until([=]()
		{
			if ((std::chrono::high_resolution_clock::now() - begin) > delay)
			{
				callback();
				return evaluation::remove;
			}

			return evaluation::reschedule;
		}, thread);
	}

	void Scheduler::once(const std::function<void()>& callback, const thread thread)
	{
		single_callbacks_.add({ callback, thread });
	}

	void Scheduler::until(const std::function<evaluation()>& callback, thread thread)
	{
		condition_callbacks_.add({ callback, thread });
	}

	void Scheduler::error(const std::string& message, int level)
	{
		std::lock_guard _(mutex_);
		errors_.emplace(message, level);
	}

	__declspec(naked) void Scheduler::main_frame_stub()
	{
		static const int execution_thread = thread::main;

		__asm
		{
			pushad;
			push	execution_thread;
			call	execute;
			pop		eax;
			popad;

			push	0x4FFF30;
			retn;
		}
	}

	// -------

	__declspec(naked) void Scheduler::renderer_frame_stub_stock()
	{
		static const int execution_thread = thread::renderer;

		__asm
		{
			pushad;
			push	execution_thread;
			call	execute;
			pop		eax;
			popad;

			push	0x45CEF0;
			retn;
		}
	}

	__declspec(naked) void Scheduler::renderer_frame_stub_con_addon()
	{
		static const int execution_thread = thread::renderer;

		__asm
		{
			pushad;
			push	execution_thread;
			call	execute;
			pop		eax;
			popad;

			call	XO_Console::xo_con_CheckResize;
			retn;
		}
	}

	// -------

	__declspec(naked) void Scheduler::execute(thread)
	{
		__asm
		{
			push	[esp + 4h];

			call	execute_error;
			call	execute_safe;

			add		esp, 4h;
			retn;
		}
	}

	void Scheduler::execute_safe(const thread thread)
	{
		for (const auto callback : callbacks_)
		{
			if (callback->second == thread)
			{
				callback->first();
			}
		}

		for (auto callback : single_callbacks_)
		{
			if (callback->second == thread)
			{
				single_callbacks_.remove(callback);
				callback->first();
			}
		}

		for (auto callback : condition_callbacks_)
		{
			if (callback->second == thread)
			{
				if (callback->first() == evaluation::remove)
				{
					condition_callbacks_.remove(callback);
				}
			}
		}
	}

	void Scheduler::execute_error(thread thread)
	{
		const char* message;
		int level;

		if (get_next_error(&message, &level) && message)
		{
			Game::Com_Error(level, "%s", message);
		}
	}

	bool Scheduler::get_next_error(const char** error_message, int* error_level)
	{
		std::lock_guard _(mutex_);
		if (errors_.empty())
		{
			*error_message = nullptr;
			return false;
		}

		const auto error = errors_.front();
		errors_.pop();

		*error_level = error.second;
		*error_message = Utils::VA("%s", error.first.data());

		return true;
	}

	Scheduler::Scheduler()
	{
		Utils::Hook(0x500328, main_frame_stub, HOOK_CALL).install()->quick();
		
		if (Components::active.XO_Console)
		{
			Utils::Hook(0x475052, renderer_frame_stub_con_addon, HOOK_CALL).install()->quick(); // call xo_con_CheckResize if console addon is loaded
		}
		else
		{
			Utils::Hook(0x475052, renderer_frame_stub_stock, HOOK_CALL).install()->quick(); // call stock Con_CheckResize otherwise
		}
	}

	Scheduler::~Scheduler()
	{ }
}