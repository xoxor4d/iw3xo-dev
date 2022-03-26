#include "STDInclude.hpp"

#define SERVERSTATIC_STRUCT_ADDR 0x1CBFC80 // correct, we start at initialized, cba to get real start address
#define svs (*((Game::serverStatic_t*)(SERVERSTATIC_STRUCT_ADDR)))

#define SERVER_STRUCT_ADDR 0x17FC7C8 // correct
#define sv (*((Game::server_t*)(SERVER_STRUCT_ADDR))) // def. 0x17FC7C8

Game::scr_function_t* scr_functions = nullptr; // only for PlayerMethods
Game::scr_function_t *scr_methods = nullptr; // only for PlayerMethods


namespace components
{
	bool scr_add_function(const char* cmd_name, xfunction_t function, bool developer) 
	{
		Game::scr_function_t* cmd;

		// fail if the command already exists
		for (cmd = scr_functions; cmd; cmd = cmd->next) 
		{
			if (!strcmp(cmd_name, cmd->name)) 
			{
				// allow completion-only commands to be silently doubled
				if (function != nullptr)
				{
					Game::Com_PrintMessage(0, utils::va("scr_add_function: %s already defined\n", cmd_name), 0);
				}

				return false;
			}
		}

		// use a small malloc to avoid zone fragmentation
		cmd = static_cast<Game::scr_function_t*> (malloc(sizeof(Game::scr_function_t) + strlen(cmd_name) + 1));

		if (cmd && cmd + 0x1)
		{
			strcpy((char*)(cmd + 1), cmd_name);
			cmd->name = (char*)(cmd + 1);
		}
		else
		{
			return false;
		}

		cmd->function = function;
		cmd->developer = developer;
		cmd->next = scr_methods;
		scr_methods = cmd;

		return true;
	}

	bool add_method(const char *cmd_name, xfunction_t function, bool developer)
	{
		Game::scr_function_t* cmd;

		// fail if the command already exists
		for (cmd = scr_methods; cmd; cmd = cmd->next)
		{
			if (!strcmp(cmd_name, cmd->name))
			{
				// allow completion-only commands to be silently doubled
				if (function != nullptr) 
				{
					Game::Com_PrintMessage(0, utils::va("scr_add_method: %s already defined\n", cmd_name), 0);
				}

				return false;
			}
		}

		// use a small malloc to avoid zone fragmentation
		cmd = static_cast<Game::scr_function_t*> (malloc(sizeof(Game::scr_function_t) + strlen(cmd_name) + 1));

		if (cmd && cmd + 0x1)
		{
			strcpy((char*)(cmd + 1), cmd_name);
			cmd->name = (char*)(cmd + 1);
		}
		else
		{
			return false;
		}

		cmd->function = function;
		cmd->developer = developer;
		cmd->next = scr_methods;
		scr_methods = cmd;

		return true;
	}

	void scr_error(const char *error)
	{
		*Game::scrVarPub_p4 = 1;								// display type?

		if (!*Game::scrVarPub)									// if no pointer to error_message
		{
			strncpy(*&Game::error_message, error, 1023u);		// copy errorText to location of error_message
			*Game::errortype = 0;								// what does that do?
			*Game::scrVarPub = (DWORD)(Game::error_message);	// scrVarPub pointing to location of error_message
		}

		Game::Scr_Error_Internal();
	}

	// *
	// Get playerstate for clientNum
	Game::playerState_s *get_playerstate_for_sv_client(int num) 
	{
		return (Game::playerState_s *)((BYTE*)sv.gameClients + sv.gameClientSize * (num));
	}


	// -------------------
	// Custom GScr Methods

	// *
	// Method :: wild LNR hiding in the bushes
	void playercmd_lnr(scr_entref_t arg)
	{
		// just print sth to the console for now
		Game::Com_PrintMessage(0, utils::va("Scr_AddMethod: ^3LNR is hiding in the bushes!\n"), 0);
	}

	// *
	// method :: SetVelocity
	void playercmd_set_velocity(scr_entref_t arg) 
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else 
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity	= &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if(*Game::scr_numParam != 1) 
		{
			scr_error("Usage: self setVelocity( <Vec3> )\n");
		}

		const auto cl = &svs.clients[entityNum];
		Game::Scr_GetVector(0, get_playerstate_for_sv_client(cl - svs.clients)->velocity);
	}

	// *
	// method :: SprintLeftButtonPressed
	void playercmd_sprint_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			scr_error("Usage: self sprintButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(2, cl->lastUsercmd.buttons));
	}

	// *
	// method :: LeanLeftButtonPressed
	void playercmd_lean_left_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			scr_error("Usage: self leanLeftButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(64, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: LeanRightButtonPressed
	void playercmd_lean_right_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			scr_error("Usage: self leanRightButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(128, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: ReloadButtonPressed
	void playercmd_reload_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			scr_error("Usage: self reloadButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(32, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: JumpButtonPressed
	void playercmd_jump_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			scr_error("Usage: self jumpButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(1024, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: ForwardButtonPressed
	void playercmd_forward_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			scr_error("Usage: self forwardButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(127, cl->lastUsercmd.forwardmove));
	}

	// *
	// Method :: BackButtonPressed
	void playercmd_back_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			scr_error("Usage: self backButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(129, cl->lastUsercmd.forwardmove));
	}

	// *
	// Method :: RightButtonPressed
	void playercmd_right_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			scr_error("Usage: self rightButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(127, cl->lastUsercmd.rightmove));
	}

	// *
	// Method :: LeftButtonPressed
	void playercmd_left_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			scr_error("Usage: self leftButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		Game::Scr_AddInt(cl->lastUsercmd.rightmove == 129);
	}

	// *
	// Method :: CheckJump
	void playercmd_check_jump(scr_entref_t arg)
	{
		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			const std::int32_t entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			scr_error("Usage: self checkJump()\n");
		}

		Game::Scr_AddInt(Game::Globals::lpmove_check_jump);
	}

	// *
	// Add GScr Methods
	void add_stock_player_methods() 
	{
		add_method("LNR",						(xfunction_t)(playercmd_lnr), 0);
		add_method("setvelocity",				(xfunction_t)(playercmd_set_velocity), 0);
		add_method("sprintButtonPressed",		(xfunction_t)(playercmd_sprint_button_pressed), 0);
		add_method("leanLeftButtonPressed",		(xfunction_t)(playercmd_lean_left_button_pressed), 0);
		add_method("leanRightButtonPressed",	(xfunction_t)(playercmd_lean_right_button_pressed), 0);
		add_method("reloadButtonPressed",		(xfunction_t)(playercmd_reload_button_pressed), 0);
		add_method("jumpButtonPressed",			(xfunction_t)(playercmd_jump_button_pressed), 0);
		add_method("forwardButtonPressed",		(xfunction_t)(playercmd_forward_button_pressed), 0);
		add_method("backButtonPressed",			(xfunction_t)(playercmd_back_button_pressed), 0);
		add_method("rightButtonPressed",		(xfunction_t)(playercmd_right_button_pressed), 0);
		add_method("leftButtonPressed",			(xfunction_t)(playercmd_left_button_pressed), 0);
		add_method("checkJump",					(xfunction_t)(playercmd_check_jump), 0);
	}

	// *
	// Gets called if no stock player-method has been found
	void* player_get_custom_method(const char **v_functionName)
	{
		Game::scr_function_t *cmd;

		for (cmd = scr_methods; cmd != NULL; cmd = cmd->next)
		{
			if (!_stricmp(*v_functionName, cmd->name))
			{
				//*v_developer = cmd->developer;
				*v_functionName = cmd->name;
				return cmd->function;
			}
		}

		return nullptr; // continue searching with the next method types
	}

	// ASM :: Scr_GetMethod :: implement Player_GetCustomMethod
	__declspec(naked) void Scr_GetMethod_stub()
	{
		const static uint32_t Player_GetMethod_Stock_func = 0x4B1FC0;
		const static uint32_t retn_addr = 0x4D8583;
		__asm
		{
			// esi is pushed already
			call	Player_GetMethod_Stock_func;
			add     esp, 4;
			test    eax, eax;

			// Return if we got the Handle for a stock method
			jnz		VALID_METHOD_HANDLE;

			// else look if it is a custom method
			push    esi;
			call	player_get_custom_method;
			add     esp, 4;
			test    eax, eax;
			
			// Return if we got the Handle for a custom method
			jnz		VALID_METHOD_HANDLE;

			// else jump back to the next check in code ( ScriptEnt_GetMethod )
			jmp		retn_addr;

		VALID_METHOD_HANDLE:
			retn;
		}
	}

	gscr_methods::gscr_methods()
	{
		// hook Player_GetMethod
		utils::hook(0x4D8577, Scr_GetMethod_stub, HOOK_JUMP).install()->quick();

		add_stock_player_methods();
	}
}
