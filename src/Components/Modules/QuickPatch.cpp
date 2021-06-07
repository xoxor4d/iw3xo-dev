#include "STDInclude.hpp"

namespace Components
{
	// on renderer initialization
	void PrintLoadedModules()
	{
		Game::Com_PrintMessage(0, Utils::VA("-------------- Loaded Modules -------------- \n%s\n", Game::Globals::loadedModules.c_str()), 0);

		// Add FS Path output print
		Game::dvar_s* dedicated = Game::Dvar_FindVar("dedicated");
		
		if (dedicated && dedicated->current.integer == 0)
		{
			Game::FS_DisplayPath(1);
		}
	}

	__declspec(naked) void CL_PreInitRenderer_stub()
	{
		const static uint32_t CL_PreInitRenderer_Func = 0x46CCB0;
		const static uint32_t retnPt = 0x46FD05;
		__asm
		{
			pushad;
			call	PrintLoadedModules;
			popad;

			call	CL_PreInitRenderer_Func;
			jmp		retnPt;
		}
	}

	// r_init
	__declspec(naked) void Fix_ConsolePrints01()
	{
		const static uint32_t retnPt = 0x5F4EE6;
		const static char* print = "\n-------------- R_Init --------------\n";
		__asm
		{
			push	print;
			jmp		retnPt;
		}
	}

	// working directory
	__declspec(naked) void Fix_ConsolePrints02()
	{
		const static uint32_t retnPt = 0x5776A2;
		const static char* print = "Working directory: %s\n\n";
		__asm
		{
			push	print;
			jmp		retnPt;
		}
	}

	// server initialization
	__declspec(naked) void Fix_ConsolePrints03()
	{
		const static uint32_t retnPt = 0x52F3F3;
		const static char* print = "\n------- Server Initialization ---------\n";
		__asm
		{
			push	print;
			jmp		retnPt;
		}
	}

	// helper function because cba to do that in asm
	void PrintBuildOnInit() 
	{
		Game::Com_PrintMessage(0, "\n-------- Game Initialization ----------\n", 0);
		Game::Com_PrintMessage(0, Utils::VA("> Build: IW3xo %0.0lf :: %s\n", IW3X_BUILDNUMBER, __TIMESTAMP__), 0);
	}

	// game init + game name and version
	__declspec(naked) void Fix_ConsolePrints04()
	{
		const static uint32_t retnPt = 0x4BF04F;
		__asm
		{
			pushad;
			call	PrintBuildOnInit;
			popad;

			jmp		retnPt;
		}
	}

	QuickPatch::QuickPatch()
	{
		Dvars::load_iw3mvm = Game::Dvar_RegisterBool(
			/* name		*/ "load_iw3mvm",
			/* desc		*/ "load iw3mvm on startup",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		// Force debug logging
		Utils::Hook::Nop(0x4FCB9D, 8);

		// Enable console log
		Utils::Hook::Nop(0x4FCBA3, 2);

		// Ignore hardware changes
		Utils::Hook::Set<DWORD>(0x57676B, 0xC359C032);
		Utils::Hook::Set<DWORD>(0x576691, 0xC359C032);

		// Remove improper quit popup
		Utils::Hook::Set<BYTE>(0x5773F6, 0xEB);

		// Fix microphone shit
		Utils::Hook::Set<BYTE>(0x57AB09, 0xEB);
		Utils::Hook::Nop(0x4ED366, 5);

		// Disable AutoUpdate Check?
		Utils::Hook::Nop(0x4D76DA, 5);

		// Disable developer check for Alt + Enter
		Utils::Hook::Nop(0x57BEBD, 2);

		// Precaching beyond level load
		Utils::Hook::Nop(0x4E2216, 2); // model 1
		Utils::Hook::Set<BYTE>(0x4E2282, 0xEB); // model 2

		// (c) Snake :: Fix mouse lag by moving SetThreadExecutionState call out of event loop
		Utils::Hook::Nop(0x57BB2C, 8);
		Scheduler::on_frame([]()
		{
			SetThreadExecutionState(ES_DISPLAY_REQUIRED);
		});


		// *
		// Console prints

		// Print loaded modules to console
		Utils::Hook(0x46FD00, CL_PreInitRenderer_stub, HOOK_JUMP).install()->quick();
		
		// Remove "setstat: developer_script must be false"
		Utils::Hook::Nop(0x46FCFB, 5);

		// Add newlines 
		Utils::Hook(0x5F4EE1, Fix_ConsolePrints01, HOOK_JUMP).install()->quick();
		Utils::Hook(0x57769D, Fix_ConsolePrints02, HOOK_JUMP).install()->quick();
		Utils::Hook(0x52F3EE, Fix_ConsolePrints03, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4BF04A, Fix_ConsolePrints04, HOOK_JUMP).install()->quick(); 
		Utils::Hook::Nop(0x4BF05B, 5); // gamename
		Utils::Hook::Nop(0x4BF06C, 5); // gamedate


		// *
		// bouncepatch stuff

		//Utils::Hook::Set<BYTE>(0x40E3FA, 0xEB); Utils::Hook::Set<BYTE>(0x40E3FA + 1, 0x20); // jnp 0040E41C -> jmp 0040E41C = Bounce on everything - projVelocity
		//Utils::Hook::Set<BYTE>(0x40E3E8, 0xD9); Utils::Hook::Set<BYTE>(0x40E3E8 + 1, 0xEB); // fld1 to fldpi - projVelocity
		//Utils::Hook::Set<FLOAT>(0x6D8D84, 0.005f); // Float 0.3 -> 0.005 = ignore angles - stepslidemove

		// elevator slide along all edges
		//Utils::Hook::Set<BYTE>(0x4103E2, 0x01);


		Command::Add("patchdvars", [](Command::Params)
		{
			Dvars::cg_fovScale = Game::Dvar_RegisterFloat(
				/* name		*/ "cg_fovScale",
				/* desc		*/ "Overwritten non-cheat fovscale",
				/* default	*/ 1.125f,
				/* minVal	*/ 0.01f,
				/* maxVal	*/ 10.0f,
				/* flags	*/ Game::dvar_flags::none);

			Dvars::snaps = Game::Dvar_RegisterInt(
				/* name		*/ "snaps",
				/* desc		*/ "re-registered snaps with increased max",
				/* default	*/ 20,
				/* minVal	*/ 0,
				/* maxVal	*/ 333,
				/* flags	*/ Game::dvar_flags::saved);
		});
	}

	QuickPatch::~QuickPatch()
	{ }
}
