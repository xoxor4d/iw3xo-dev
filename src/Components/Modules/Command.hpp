#pragma once

namespace Components
{
	class command final : public Component
	{
	public:
		class params
		{
		public:
			params(DWORD id) : command_id(id) {};
			params(const params &obj) { this->command_id = obj.command_id; };
			params() : params(*Game::cmd_id) {};

			const char* operator[](size_t index);
			size_t length();

		private:
			DWORD command_id;
		};

		typedef void(callback)(command::params params);

		command() = default; ~command() override;
		const char* getName() override { return "command"; };

		static void add(const char* name, utils::Slot<callback> callback);
		static void add(const char* name, const char* args, const char* description, utils::Slot<command::callback> callback);
		static void execute(std::string command, bool sync = true);

	private:
		static Game::cmd_function_s* allocate();
		static std::vector<Game::cmd_function_s*> functions;
		static std::map<std::string, utils::Slot<callback>> function_map;
		static void main_callback();
	};
}
