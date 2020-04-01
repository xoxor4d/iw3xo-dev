#include "STDInclude.hpp"
#include "Utils/vector.hpp"

using namespace Utils::vector;

// can grab ps from gentity-client-ps // works on dedi

#define SERVERSTATIC_STRUCT_ADDR 0x1CBFC80 // correct, we start at initialized, cba to get real start address
#define svs (*((Game::serverStatic_t*)(SERVERSTATIC_STRUCT_ADDR)))

#define SERVER_STRUCT_ADDR 0x17FC7C8 // correct
#define sv (*((Game::server_t*)(SERVER_STRUCT_ADDR))) // def. 0x17FC7C8

Game::scr_function_t *scr_methods = NULL; // only for PlayerMethods

namespace Components
{
	bool Scr_AddMethod(char *cmd_name, xfunction_t function, bool developer)
	{
		Game::scr_function_t  *cmd;

		// fail if the command already exists
		for (cmd = scr_methods; cmd; cmd = cmd->next)
		{
			if (!strcmp(cmd_name, cmd->name))
			{
				// allow completion-only commands to be silently doubled
				if (function != NULL) 
				{
					Game::Com_PrintMessage(0, Utils::VA("Scr_AddMethod: %s already defined\n", cmd_name), 0);
				}

				return false;
			}
		}

		// use a small malloc to avoid zone fragmentation
		//cmd = malloc(sizeof(Game::scr_function_t) + strlen(cmd_name) + 1);
		//strcpy((char*)(cmd + 1), cmd_name);
		//cmd->name = (char*)(cmd + 1);
		cmd = reinterpret_cast<Game::scr_function_t*> (malloc(sizeof(Game::scr_function_t) + strlen(cmd_name) + 1));
		strcpy((char*)(cmd + 1), cmd_name);
		cmd->name = (char*)(cmd + 1);

		//cmd->name		= cmd_name;
		cmd->function = function;
		cmd->developer = developer;
		cmd->next = scr_methods;
		scr_methods = cmd;

		return true;
	}

	void Scr_Error(const char *error)
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
	Game::playerState_s *SV_GameClientNum(int num) 
	{
		return (Game::playerState_s *)((BYTE*)sv.gameClients + sv.gameClientSize * (num));
	}


	// -------------------
	// Custom GScr Methods

	// *
	// Method :: wild LNR hiding in the bushes
	void PlayerCmd_LNR(scr_entref_t arg)
	{
		// just print sth to the console for now
		Game::Com_PrintMessage(0, Utils::VA("Scr_AddMethod: ^3LNR is hiding in the bushes!\n"), 0);
	}

	// *
	// Method :: SetVelocity
	void PlayerCmd_SetVelocity(scr_entref_t arg) 
	{
		Game::gentity_s* gentity	= nullptr;
		Game::playerState_s* ps		= nullptr;
		Game::client_t* cl			= nullptr;
		std::int32_t entityNum		= 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else 
		{
			entityNum	= LOWORD(arg); // arg.entnum
			gentity		= &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(Utils::VA("Entity: %i is not a player", entityNum));
			}
		}

		if( *Game::scr_numParam != 1) 
		{
			Scr_Error("Usage: self setVelocity( <Vec3> )\n");
		}

		cl = &svs.clients[entityNum];
		ps = SV_GameClientNum(cl - svs.clients);
		
		Game::Scr_GetVector(0, ps->velocity);
	}

	// *
	// Method :: SprintButtonPressed
	void PlayerCmd_SprintButtonPressed(scr_entref_t arg)
	{
		Game::gentity_s* gentity = nullptr;
		Game::client_t* cl = nullptr;
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(Utils::VA("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			Scr_Error("Usage: self sprintButtonPressed()\n");
		}

		cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(2, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: LeanLeftButtonPressed
	void PlayerCmd_LeanLeftButtonPressed(scr_entref_t arg)
	{
		Game::gentity_s* gentity = nullptr;
		Game::client_t* cl = nullptr;
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(Utils::VA("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			Scr_Error("Usage: self leanLeftButtonPressed()\n");
		}

		cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(64, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: LeanRightButtonPressed
	void PlayerCmd_LeanRightButtonPressed(scr_entref_t arg)
	{
		Game::gentity_s* gentity = nullptr;
		Game::client_t* cl = nullptr;
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(Utils::VA("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			Scr_Error("Usage: self leanRightButtonPressed()\n");
		}

		cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(128, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: ReloadButtonPressed
	void PlayerCmd_ReloadButtonPressed(scr_entref_t arg)
	{
		Game::gentity_s* gentity = nullptr;
		Game::client_t* cl = nullptr;
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(Utils::VA("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			Scr_Error("Usage: self reloadButtonPressed()\n");
		}

		cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(32, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: JumpButtonPressed
	void PlayerCmd_JumpButtonPressed(scr_entref_t arg)
	{
		Game::gentity_s* gentity = nullptr;
		Game::client_t* cl = nullptr;
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(Utils::VA("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			Scr_Error("Usage: self jumpButtonPressed()\n");
		}

		cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(1024, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: ForwardButtonPressed
	void PlayerCmd_ForwardButtonPressed(scr_entref_t arg)
	{
		Game::gentity_s* gentity = nullptr;
		Game::client_t* cl = nullptr;
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(Utils::VA("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			Scr_Error("Usage: self forwardButtonPressed()\n");
		}

		cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(127, cl->lastUsercmd.forwardmove));
	}

	// *
	// Method :: BackButtonPressed
	void PlayerCmd_BackButtonPressed(scr_entref_t arg)
	{
		Game::gentity_s* gentity = nullptr;
		Game::client_t* cl = nullptr;
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(Utils::VA("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			Scr_Error("Usage: self backButtonPressed()\n");
		}

		cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(129, cl->lastUsercmd.forwardmove));
	}

	// *
	// Method :: RightButtonPressed
	void PlayerCmd_RightButtonPressed(scr_entref_t arg)
	{
		Game::gentity_s* gentity = nullptr;
		Game::client_t* cl = nullptr;
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(Utils::VA("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			Scr_Error("Usage: self rightButtonPressed()\n");
		}

		cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(127, cl->lastUsercmd.rightmove));
	}

	// *
	// Method :: LeftButtonPressed
	void PlayerCmd_LeftButtonPressed(scr_entref_t arg)
	{
		Game::gentity_s* gentity = nullptr;
		Game::client_t* cl = nullptr;
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(Utils::VA("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			Scr_Error("Usage: self leftButtonPressed()\n");
		}

		cl = &svs.clients[entityNum];
		Game::Scr_AddInt(Game::isButtonPressed(129, cl->lastUsercmd.rightmove));
	}

	// *
	// Method :: CheckJump
	void PlayerCmd_CheckJump(scr_entref_t arg)
	{
		Game::gentity_s* gentity = nullptr;
		std::int32_t entityNum = 0;

		
		if (HIWORD(arg)) 
		{
			Game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			gentity = &Game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				Game::Scr_ObjectError(Utils::VA("Entity: %i is not a player", entityNum));
			}
		}

		if (*Game::scr_numParam != 0) 
		{
			Scr_Error("Usage: self checkJump()\n");
		}

		Game::Scr_AddInt(Game::Globals::locPmove_checkJump);
	}

	// *
	// Add GScr Methods
	void Scr_AddStockPlayerMethods() 
	{
		Scr_AddMethod("LNR",					(xfunction_t)(PlayerCmd_LNR), 0);
		Scr_AddMethod("setvelocity",			(xfunction_t)(PlayerCmd_SetVelocity), 0);
		Scr_AddMethod("sprintButtonPressed",	(xfunction_t)(PlayerCmd_SprintButtonPressed), 0);
		Scr_AddMethod("leanLeftButtonPressed",	(xfunction_t)(PlayerCmd_LeanLeftButtonPressed), 0);
		Scr_AddMethod("leanRightButtonPressed", (xfunction_t)(PlayerCmd_LeanRightButtonPressed), 0);
		Scr_AddMethod("reloadButtonPressed",	(xfunction_t)(PlayerCmd_ReloadButtonPressed), 0);
		Scr_AddMethod("jumpButtonPressed",		(xfunction_t)(PlayerCmd_JumpButtonPressed), 0);
		Scr_AddMethod("forwardButtonPressed",	(xfunction_t)(PlayerCmd_ForwardButtonPressed), 0);
		Scr_AddMethod("backButtonPressed",		(xfunction_t)(PlayerCmd_BackButtonPressed), 0);
		Scr_AddMethod("rightButtonPressed",		(xfunction_t)(PlayerCmd_RightButtonPressed), 0);
		Scr_AddMethod("leftButtonPressed",		(xfunction_t)(PlayerCmd_LeftButtonPressed), 0);
		Scr_AddMethod("checkJump",				(xfunction_t)(PlayerCmd_CheckJump), 0);
	}

	// *
	// Gets called if no stock player-method has been found
	void* Player_GetCustomMethod(const char **v_functionName)
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

		return NULL; // continue searching with the next method types
	}

	// ASM :: Scr_GetMethod :: implement Player_GetCustomMethod
	__declspec(naked) void Scr_GetMethod_stub()
	{
		const static uint32_t Player_GetMethod_Stock_Func = 0x4B1FC0;
		__asm
		{
			// esi is pushed already
			Call	Player_GetMethod_Stock_Func
			add     esp, 4
			test    eax, eax

			// Return if we got the Handle for a stock method
			jnz		GotMethodHandle

			// else look if it is a custom method
			push    esi
			Call	Player_GetCustomMethod
			add     esp, 4
			test    eax, eax
			
			// Return if we got the Handle for a custom method
			jnz		GotMethodHandle

			// else jump back to the next check in code ( ScriptEnt_GetMethod )
			push	0x4D8583;
			retn

			GotMethodHandle:
				retn
		}
	}

	GScr_Methods::GScr_Methods()
	{
		// Hook Player_GetMethod
		Utils::Hook(0x4D8577, Scr_GetMethod_stub, HOOK_JUMP).install()->quick();

		Scr_AddStockPlayerMethods();
	}

	GScr_Methods::~GScr_Methods()
	{
	}
}
