#include "STDInclude.hpp"

namespace Components
{
	std::vector<Game::cmd_function_s*> Command::Functions;
	std::map<std::string, Utils::Slot<Command::Callback>> Command::FunctionMap;

	char* Command::Params::operator[](size_t index)
	{
		if (index >= this->Length()) 
		{
			return "";
		}

		return Game::cmd_argv[this->CommandId][index];
	}

	size_t Command::Params::Length()
	{
		return Game::cmd_argc[this->CommandId];
	}

	void Command::Add(const char* name, Utils::Slot<Command::Callback> callback)
	{
		Command::FunctionMap[Utils::StrToLower(name)] = callback;
		Game::Cmd_AddCommand(name, Command::MainCallback, Command::Allocate(), 0);
	}

	void Command::Add(const char* name, const char* args, const char* description, Utils::Slot<Command::Callback> callback)
	{
		Command::FunctionMap[Utils::StrToLower(name)] = callback;
		Game::Cmd_AddCommand(name, args, description, Command::MainCallback, Command::Allocate(), 0);
	}

	void Command::Execute(std::string command, bool sync)
	{
		command.append("\n"); // Make sure it's terminated

		if (sync)
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, command.data());
		}
		else
		{
			Game::Cbuf_AddText(command.data(), 0);
		}
	}

	Game::cmd_function_s* Command::Allocate()
	{
		Game::cmd_function_s* cmd = new Game::cmd_function_s;
		Command::Functions.push_back(cmd);

		return cmd;
	}

	void Command::MainCallback()
	{
		Command::Params params(*Game::cmd_id);

		std::string command = Utils::StrToLower(params[0]);

		if (Command::FunctionMap.find(command) != Command::FunctionMap.end())
		{
			Command::FunctionMap[command](params);
		}
	}

	Command::Command()
	{
		// TODO: Add commands here?
	}

	Command::~Command()
	{
		for (auto command : Command::Functions)
		{
			delete command;
		}

		Command::Functions.clear();
	}
}
