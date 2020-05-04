#include "STDInclude.hpp"

namespace Components
{
	// on renderer initialization
	void PrintLoadedModules()
	{
		Game::Com_PrintMessage(0, Utils::VA("-------------- Loading Modules -------------- \n%s\n", Game::Globals::loadedModules.c_str()), 0);

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
			pushad
			Call	PrintLoadedModules
			popad

			Call	CL_PreInitRenderer_Func
			jmp		retnPt
		}
	}

	// r_init
	__declspec(naked) void Fix_ConsolePrints01()
	{
		const static uint32_t retnPt = 0x5F4EE6;
		const static char* print = "\n-------------- R_Init --------------\n";
		__asm
		{
			push	print
			jmp		retnPt
		}
	}

	// working directory
	__declspec(naked) void Fix_ConsolePrints02()
	{
		const static uint32_t retnPt = 0x5776A2;
		const static char* print = "Working directory: %s\n\n";
		__asm
		{
			push	print
			jmp		retnPt
		}
	}

	// server initialization
	__declspec(naked) void Fix_ConsolePrints03()
	{
		const static uint32_t retnPt = 0x52F3F3;
		const static char* print = "\n------- Server Initialization ---------\n";
		__asm
		{
			push	print
			jmp		retnPt
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
			pushad
			Call	PrintBuildOnInit
			popad 

			jmp		retnPt
		}
	}

	void codesampler_error(Game::MaterialShaderArgument* arg, Game::GfxCmdBufSourceState* source, Game::GfxCmdBufState* state, const char* sampler, int droptype, const char* msg, ...)
	{
		if (!sampler || !state || !state->material || !state->technique) {
			return;
		}

		//__debugbreak();

		Game::Com_PrintMessage(0, Utils::VA(
			"^1Tried to use sampler <%s> when it isn't valid!\n"
			"^7[Passdump]\n"
			"|-> Material ----- %s\n"
			"|-> Technique ----	%s\n"
			"|-> RenderTarget - %s\n"
			"|-> Not setting sampler using R_SetSampler!\n", 
			sampler, state->material->info.name, state->technique->name, Game::RendertargetStringFromID(state->renderTargetId)), 0);
	}

	// R_SetupPassPerObjectArgs
	__declspec(naked) void codesampler_error01_stub()
	{
		const static uint32_t rtnPt = 0x64BD0F; // offset after call to R_SetSampler
		__asm
		{
			// args pushed before:
			// push    fmt
			// push    msg
			// push    dropType

			mov     eax, [esp + 8h]	// move sampler string into eax
			push	eax					// decreased esp by 4
			mov     eax, [esp + 14h]	// move GfxCmdBufState* into eax (now at 14h)
			push	eax					// GfxCmdBufState*
			push	ebx					// GfxCmdBufSourceState*
			push	edi					// MaterialShaderArgument*
#if DEBUG
			Call	codesampler_error	// only dump info on debug builds
#endif
			add		esp, 28

			mov     eax, [esp + 14h]
			mov     ecx, [esp + 24h]
			movzx   esi, word ptr[edi + 2]
			push    eax
			push    ecx
			push    ebx
			mov     eax, ebp
			//call    R_SetSampler		// skip call on null sampler
			jmp		rtnPt
		}
	}

	// R_SetPassShaderStableArguments (not used anymore?)
	__declspec(naked) void codesampler_error02_stub()
	{
		const static uint32_t rtnPt = 0x64C36C; // offset after call to R_SetSampler
		__asm
		{
			// args pushed before:
			// push    fmt
			// push    msg
			// push    dropType

			// just skip the error and R_SetSampler
			//push	ebx // GfxCmdBufState* <- wrong
			//push	ebp // GfxCmdBufSourceState*
			//push	edi // MaterialShaderArgument*
			//Call	codesampler_error
			add		esp, 12

			mov     eax, [esp + 20h]
			movzx   esi, word ptr[edi + 2]
			push    eax
			mov     eax, [esp + 18h]
			push    ebx
			push    ebp
			//call    R_SetSampler

			jmp		rtnPt
		}
	}


	QuickPatch::QuickPatch()
	{
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
		// Renderer
		
		// Hook "Com_Error(1, "Tried to use '%s' when it isn't valid\n", codeSampler)" to skip a call to R_SetSampler
		Utils::Hook(0x64BCF1, codesampler_error01_stub, HOOK_JUMP).install()->quick(); // R_SetupPassPerObjectArgs
		Utils::Hook(0x64C350, codesampler_error02_stub, HOOK_JUMP).install()->quick(); // R_SetPassShaderStableArguments (not really used)


		// *
		// Commands

		// extend
		Command::Add("ent_rotateTo", [](Command::Params params)
		{
			if (params.Length() < 3) 
			{
				Game::Com_PrintMessage(0, "Usage :: ent_rotateTo entityID <angles vec3>\n", 0);
				return;
			}

			float angles[3] = { 0.0f, 0.0f, 0.0f };

			angles[0] = Utils::try_stof(params[2], true);

			if (params.Length() >= 4)
			{
				angles[1] = Utils::try_stof(params[3], true);
			}
				
			if (params.Length() >= 5)
			{
				angles[2] = Utils::try_stof(params[4], true);
			}
			
			int entIdx = Utils::try_stoi(params[1], true);
			float vRot[3];

			for (auto i = 0; i < 3; ++i)
			{
				float v2 = Utils::vector::_AngleNormalize180(angles[i] - Game::scr_g_entities[entIdx].r.currentAngles[i]);
				vRot[i] = Game::scr_g_entities[entIdx].r.currentAngles[i] + v2;
			}

			float tAngles[3] =
			{
				Game::scr_g_entities[entIdx].r.currentAngles[0],
				Game::scr_g_entities[entIdx].r.currentAngles[1],
				Game::scr_g_entities[entIdx].r.currentAngles[2],
			};

			Game::ScriptMover_SetupMove(
				&Game::scr_g_entities[entIdx].s.lerp.apos,
				vRot,
				4.0f,
				0.1f,
				0.1f,
				tAngles,
				&Game::scr_g_entities[entIdx].___u30.mover.aSpeed,
				&Game::scr_g_entities[entIdx].___u30.mover.aMidTime,
				&Game::scr_g_entities[entIdx].___u30.mover.aDecelTime,
				Game::scr_g_entities[entIdx].___u30.mover.apos1,
				Game::scr_g_entities[entIdx].___u30.mover.apos2,
				Game::scr_g_entities[entIdx].___u30.mover.apos3);

			Game::SV_LinkEntity(&Game::scr_g_entities[entIdx]);
		});

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

		// needs work :x
		Command::Add("loadzone", [](Command::Params params) // unload zone and load zone again
		{
			if (params.Length() < 2) 
			{
				Game::Com_PrintMessage(0, "Usage :: loadzone <zoneName>\n", 0);
				return;
			}
			
			Game::XZoneInfo info[2];
			std::string zone = params[1];

			// unload
			info[0].name = 0;
			info[0].allocFlags	= Game::XZONE_FLAGS::XZONE_ZERO;
			info[0].freeFlags	= Game::XZONE_FLAGS::XZONE_MOD;

			info[1].name = zone.data();
			info[1].allocFlags	= Game::XZONE_FLAGS::XZONE_MOD;
			info[1].freeFlags	= Game::XZONE_FLAGS::XZONE_ZERO;

			Game::DB_LoadXAssets(info, 2, 1);
		});
	}

	QuickPatch::~QuickPatch()
	{ }
}
