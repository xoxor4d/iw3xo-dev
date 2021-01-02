#pragma once

namespace Components
{
	class Command : public Component
	{
	public:
		class Params
		{
		public:
			Params(DWORD id) : CommandId(id) {};
			Params(const Params &obj) { this->CommandId = obj.CommandId; };
			Params() : Params(*Game::cmd_id) {};

			const char* operator[](size_t index);
			size_t Length();

		private:
			DWORD CommandId;
		};

		typedef void(Callback)(Command::Params params);

		Command();
		~Command();
		const char* getName() override { return "Command"; };

		static void Add(const char* name, Utils::Slot<Callback> callback);
		static void Add(const char* name, const char* args, const char* description, Utils::Slot<Command::Callback> callback);
		static void Execute(std::string command, bool sync = true);

	private:
		static Game::cmd_function_s* Allocate();
		static std::vector<Game::cmd_function_s*> Functions;
		static std::map<std::string, Utils::Slot<Callback>> FunctionMap;
		static void MainCallback();
	};
}
