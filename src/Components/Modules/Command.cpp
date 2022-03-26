#include "STDInclude.hpp"

namespace components
{
	std::vector<Game::cmd_function_s*> command::functions;
	std::map<std::string, utils::Slot<command::callback>> command::function_map;

	const char* command::params::operator[](size_t index)
	{
		if (index >= this->length()) 
		{
			return "";
		}

		return Game::cmd_argv[this->command_id][index];
	}

	size_t command::params::length()
	{
		return Game::cmd_argc[this->command_id];
	}

	void command::add(const char* name, utils::Slot<command::callback> callback)
	{
		command::function_map[utils::str_to_lower(name)] = callback;
		Game::Cmd_AddCommand(name, command::main_callback, command::allocate(), 0);
	}

	void command::add(const char* name, const char* args, const char* description, utils::Slot<command::callback> callback)
	{
		command::function_map[utils::str_to_lower(name)] = callback;
		Game::Cmd_AddCommand(name, args, description, command::main_callback, command::allocate(), 0);
	}

	void command::execute(std::string command, bool sync)
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

	Game::cmd_function_s* command::allocate()
	{
		const auto cmd = new Game::cmd_function_s;
		command::functions.push_back(cmd);

		return cmd;
	}

	void command::main_callback()
	{
		command::params params(*Game::cmd_id);
		const std::string command = utils::str_to_lower(params[0]);

		if (command::function_map.contains(command))
		{
			command::function_map[command](params);
		}
	}

	command::~command()
	{
		for (const auto command : command::functions)
		{
			delete command;
		}

		command::functions.clear();
	}
}
