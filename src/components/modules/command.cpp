#include "std_include.hpp"

namespace components
{
	std::vector<game::cmd_function_s*> command::functions;
	std::map<std::string, utils::Slot<command::callback>> command::function_map;

	const char* command::params::operator[](size_t index)
	{
		if (index >= this->length()) 
		{
			return "";
		}

		return game::cmd_args->argv[this->command_id][index];
	}

	size_t command::params::length()
	{
		return game::cmd_args->argc[this->command_id];
	}

	void command::add(const char* name, utils::Slot<command::callback> callback)
	{
		command::function_map[utils::str_to_lower(name)] = callback;
		game::Cmd_AddCommand(name, command::main_callback, command::allocate(), 0);
	}

	void command::add(const char* name, const char* args, const char* description, utils::Slot<command::callback> callback)
	{
		command::function_map[utils::str_to_lower(name)] = callback;
		game::Cmd_AddCommand(name, args, description, command::main_callback, command::allocate(), 0);
	}

	void command::execute(std::string command, bool sync)
	{
		command.append("\n"); // Make sure it's terminated

		if (sync)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, command.data());
		}
		else
		{
			game::Cbuf_AddText(command.data(), 0);
		}
	}

	game::cmd_function_s* command::allocate()
	{
		const auto cmd = new game::cmd_function_s;
		command::functions.push_back(cmd);

		return cmd;
	}

	void command::main_callback()
	{
		command::params params(game::cmd_args->nesting);
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
