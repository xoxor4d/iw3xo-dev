#include "STDInclude.hpp"

// enables addon fastfiles
#define FF_LOAD_ADDON_MENU		true
#define FF_ADDON_MENU_NAME		"xcommon_iw3xo_menu"

#define FF_LOAD_ADDON_REQ		true
#define FF_ADDON_REQ_NAME		"xcommon_iw3xo"

char* MENU_CHANGELOG_TITLE_FMT = "IW3XO :: %.lf :: %s\n"; // IW3X_BUILDNUMBER, __TIMESTAMP__

namespace Components
{
	//void QuickPatch::PerformInit()
	//{
	//	//Command::Execute("set dedicated 0", true);
	//}

	//__declspec(naked) void QuickPatch::OnInitStub()
	//{
	//	__asm
	//	{
	//		pushad
	//		call QuickPatch::PerformInit
	//		popad

	//		push 4FD740h
	//		retn
	//	}
	//}

	// on renderer initialization
	void PrintLoadedModules()
	{
		Game::Com_PrintMessage(0, Utils::VA("-------------- Loading Modules -------------- \n%s\n", Game::Globals::loadedModules.c_str()), 0);
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
			jmp		retnPt			// continue exec
		}
	}

	// --------------------------------------------------------

	bool iwdMatchXCOMMON(const char* s0)
	{
		if (!Utils::Q_stricmpn(s0, "xcommon_", 8)) 
		{
			return 0;
		}

		return 1;
	}

	bool iwdMatchIW(const char* s0)
	{
		if (!Utils::Q_stricmpn(s0, "iw_", 3)) 
		{
			return 0;
		}

		return 1;
	}

	//load "iw_" iwds and "xcommon_" iwds as localized ones ;)
	__declspec(naked) void FS_MakeIWDsLocalized()
	{
		const static uint32_t errMsg	= 0x55DBCA;
		const static uint32_t hax		= 0x55DBE8;
		__asm
		{
			push	edi				// current iwd string + ext
			Call	iwdMatchIW
			add		esp, 4
			test    eax, eax
			
			je		MATCH			// jump if iwd matched iw_

									// if not, cmp to xcommon_
			push	edi				// current iwd string + ext
			Call	iwdMatchXCOMMON
			add		esp, 4
			test    eax, eax
			
			je		MATCH			// jump if iwd matched xcommon_
			jmp		errMsg			// yeet

		MATCH :
			mov     ebx, [ebp - 4]		// whatever df that is
			mov		[ebp - 8], 1		// set qLocalized to true ;)
			mov		[ebp - 0Ch], esi	// whatever df that is
			jmp		hax
		}
	}

	// --------------------------------------------------------

	// load common + addon fastfiles (only on init or when changing CGame)
	void DB_LoadCommonFastFiles()
	{
		/*
		only unload zones with a set flag
		:: zone.name = 0;
		:: zone.allocFlags = 0;
		:: zone.freeFlags = ZONE_FLAG_TO_UNLOAD
		*/

		int i = 0;
		Game::XZoneInfo XZoneInfoStack[7];

		// ------------------------------------

		XZoneInfoStack[i].name			= *Game::zone_code_post_gfx_mp;
		XZoneInfoStack[i].allocFlags	= Game::XZONE_FLAGS::XZONE_POST_GFX;
		XZoneInfoStack[i].freeFlags		= Game::XZONE_FLAGS::XZONE_POST_GFX_FREE;
		++i;

		// ------------------------------------

		if (*Game::zone_localized_code_post_gfx_mp)
		{
			XZoneInfoStack[i].name			= *Game::zone_localized_code_post_gfx_mp;
			XZoneInfoStack[i].allocFlags	= Game::XZONE_FLAGS::XZONE_LOC_POST_GFX;
			XZoneInfoStack[i].freeFlags		= Game::XZONE_FLAGS::XZONE_LOC_POST_GFX_FREE;
			++i;
		}

		// ---------------------------------------------------------------------------------------------------------

		// if addon_menu loading is enabled
		if (FF_LOAD_ADDON_MENU)
		{
			// if the fastfile exists
			if (FF_ADDON_MENU_NAME && Game::DB_FileExists(FF_ADDON_MENU_NAME, Game::DB_FILE_EXISTS_PATH::DB_PATH_ZONE))
			{
				// file exists, check if a mod was loaded
				if (!*Game::zone_mod)
				{
					// only load the menu when no mod is loaded -- unload on mod | map load
					XZoneInfoStack[i].name = FF_ADDON_MENU_NAME;

					XZoneInfoStack[i].allocFlags	= Game::XZONE_FLAGS::XZONE_MOD | Game::XZONE_FLAGS::XZONE_DEBUG;
					++i;

					// we have to sync assets or we run into issues when the game is trying to access unloaded assets
					Game::DB_LoadXAssets(&XZoneInfoStack[0], i, 0);

					Game::R_BeginRemoteScreenUpdate();
					WaitForSingleObject(Game::dbHandle, 0xFFFFFFFF);
					Game::R_EndRemoteScreenUpdate();
					Game::DB_SyncXAssets();

					// start a new zone stack
					i = 0;
				}
			}

			// if addon_menu loading is enabled and file not found
			else if (FF_LOAD_ADDON_MENU) 
			{
				Game::Com_PrintMessage(0, Utils::VA("^1DB_LoadCommonFastFiles^7:: %s.ff not found. \n", FF_ADDON_MENU_NAME), 0);
			}
		}

		// mod loading -- maybe sync assets for addon menu too?
		if (*Game::zone_mod) 
		{
			XZoneInfoStack[i].name			= *Game::zone_mod;
			XZoneInfoStack[i].allocFlags	= Game::XZONE_FLAGS::XZONE_MOD;
			XZoneInfoStack[i].freeFlags		= Game::XZONE_FLAGS::XZONE_MOD_FREE;
			++i;

			Game::DB_LoadXAssets(&XZoneInfoStack[0], i, 0);

			Game::R_BeginRemoteScreenUpdate();
			WaitForSingleObject(Game::dbHandle, 0xFFFFFFFF);

			Game::R_EndRemoteScreenUpdate();
			Game::DB_SyncXAssets();

			// start a new zone stack
			i = 0;
		}

		// ---------------------------------------------------------------------------------------------------------

		if (*Game::zone_ui_mp) // not loaded when starting dedicated servers
		{
			XZoneInfoStack[i].name			= *Game::zone_ui_mp;
			XZoneInfoStack[i].allocFlags	= Game::XZONE_FLAGS::XZONE_UI;
			XZoneInfoStack[i].freeFlags		= Game::XZONE_FLAGS::XZONE_UI_FREE;
			++i;
		}

		// ------------------------------------

		XZoneInfoStack[i].name			= *Game::zone_common_mp;
		XZoneInfoStack[i].allocFlags	= Game::XZONE_FLAGS::XZONE_COMMON;
		XZoneInfoStack[i].freeFlags		= Game::XZONE_FLAGS::XZONE_COMMON_FREE;
		++i;

		// ------------------------------------

		if (*Game::zone_localized_common_mp) // not loaded on when starting dedicated servers
		{
			XZoneInfoStack[i].name			= *Game::zone_localized_common_mp;
			XZoneInfoStack[i].allocFlags	= Game::XZONE_FLAGS::XZONE_LOC_COMMON;
			XZoneInfoStack[i].freeFlags		= Game::XZONE_FLAGS::XZONE_LOC_COMMON_FREE;
			++i;
		}

		// ------------------------------------

		// load required addon fastfile last, if addon_required loading enabled
		if (FF_LOAD_ADDON_REQ)
		{
			// if the fastfile exists
			if (FF_ADDON_REQ_NAME && Game::DB_FileExists(FF_ADDON_REQ_NAME, Game::DB_FILE_EXISTS_PATH::DB_PATH_ZONE))
			{
				XZoneInfoStack[i].name = FF_ADDON_REQ_NAME;
				XZoneInfoStack[i].allocFlags = Game::XZONE_FLAGS::XZONE_COMMON; //Game::XZONE_FLAGS::XZONE_MOD; // free when loading mods?
				XZoneInfoStack[i].freeFlags = Game::XZONE_FLAGS::XZONE_ZERO; // do not free any other fastfiles?
				++i;
			}

			// if addon_required loading is enabled and file not found
			else if (FF_LOAD_ADDON_REQ) 
			{
				Game::Com_PrintMessage(0, Utils::VA("^1DB_LoadCommonFastFiles^7:: %s.ff not found. \n", FF_ADDON_REQ_NAME), 0);
			}
		}

		// ------------------------------------

		Game::DB_LoadXAssets(&XZoneInfoStack[0], i, 0);
	}

	// --------------------------------------------------------

	// realloc zones that get unloaded on map load (eg. ui_mp)
	void Com_StartHunkUsers()
	{
		if (Game::Dvar_FindVar("useFastFile")->current.enabled)
		{
			int i = 0;
			Game::XZoneInfo XZoneInfoStack[2];

			// if addon menu loading is enabled
			if (FF_LOAD_ADDON_MENU)
			{
				// if the fastfile exists
				if (FF_ADDON_MENU_NAME && Game::DB_FileExists(FF_ADDON_MENU_NAME, Game::DB_FILE_EXISTS_PATH::DB_PATH_ZONE))
				{
					// file exists, check if a mod was loaded
					if (!*Game::zone_mod)
					{
						// only load the menu when no mod is loaded -- unload on mod | map load
						XZoneInfoStack[i].name = FF_ADDON_MENU_NAME;

						XZoneInfoStack[i].allocFlags	= Game::XZONE_FLAGS::XZONE_MOD | Game::XZONE_FLAGS::XZONE_DEBUG;
						XZoneInfoStack[i].freeFlags		= Game::XZONE_FLAGS::XZONE_UI_FREE_INGAME;
						++i;
					}
				}

				// if addon menu loading is enabled and file not found
				else if (FF_LOAD_ADDON_MENU) 
				{
					Game::Com_PrintMessage(0, Utils::VA("^1Com_StartHunkUsers^7:: %s.ff not found. \n", FF_ADDON_MENU_NAME), 0);
				}
			}

			// ------------------------------------

			XZoneInfoStack[i].name			= *Game::zone_ui_mp;
			XZoneInfoStack[i].allocFlags	= Game::XZONE_FLAGS::XZONE_UI;
			XZoneInfoStack[i].freeFlags		= Game::XZONE_FLAGS::XZONE_UI_FREE_INGAME;
			++i;

			// ------------------------------------

			Game::DB_LoadXAssets(&XZoneInfoStack[0], i, 0);
		}
	}

	// :: Com_StartHunkUsers
	__declspec(naked) void Com_StartHunkUsers_stub()
	{
		const static uint32_t retnPt = 0x500238;
		__asm
		{
			pushad
			Call	Com_StartHunkUsers
			popad

			jmp		retnPt			// continue exec
		}
	}

	// --------------------------------------------------------

	// disable cheat / write check
	__declspec(naked) void disable_dvar_cheats_stub()
	{
		const static uint32_t overjumpTo = 0x56B3A1;
		__asm
		{
			movzx   eax, word ptr[edi + 8]	// overwritten op
			jmp		overjumpTo
		}
	}

	// --------------------------------------------------------

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

	// register string dvars
	void R_RegisterStringDvars()
	{
		_UI::MainMenu_Changelog();
	}

	__declspec(naked) void R_RegisterStringDvars_stub()
	{
		const static uint32_t R_RegisterSunDvars_Func = 0x636FC0;
		const static uint32_t retnPt = 0x629D7F;
		__asm
		{
			pushad
			Call	R_RegisterStringDvars
			popad

			Call	R_RegisterSunDvars_Func
			jmp		retnPt
		}
	}

	void ForceDvarsOnInit()
	{
		// force depthbuffer
		if (!Game::Dvar_FindVar("r_zFeather")->current.enabled)
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, "r_zFeather 1\n");
		}

		if (!Game::Dvar_FindVar("r_distortion")->current.enabled)
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, "r_distortion 1\n");
		}
	}

	__declspec(naked) void R_BeginRegistration_stub()
	{
		const static uint32_t rtnPt = 0x46CB0E;
		__asm
		{
			pushad
			Call	ForceDvarsOnInit
			popad

			mov     ecx, 0Ch	// overwritten op
			jmp		rtnPt
		}
	}

	QuickPatch::QuickPatch()
	{
		// Set fs_game
		//Utils::Hook::Set<char*>(0x55E509, "mods/q3");
		//Utils::Hook(0x4FF20A, QuickPatch::OnInitStub, HOOK_CALL).install()->quick();

		// Disable dvar cheat / write protection
		Utils::Hook(0x56B335, disable_dvar_cheats_stub, HOOK_JUMP).install()->quick();
		Utils::Hook::Nop(0x56B33A, 1);

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

		// Remove "setstat: developer_script must be false"
		Utils::Hook::Nop(0x46FCFB, 5);

		// Precaching beyond level load
		Utils::Hook::Nop(0x4E2216, 2); // model 1
		Utils::Hook::Set<BYTE>(0x4E2282, 0xEB); // model 2

		// (c) Snake :: Fix mouse lag by moving SetThreadExecutionState call out of event loop
		Utils::Hook::Nop(0x57BB2C, 8);
		Scheduler::on_frame([]()
		{
			SetThreadExecutionState(ES_DISPLAY_REQUIRED);
		});


		// ----
		// Init

		 // Stub after renderer was initialized
		Utils::Hook(0x46CB09, R_BeginRegistration_stub, HOOK_JUMP).install()->quick();

		// Register String Dvars (doing so on module load crashes the game (SL_GetStringOfSize))
		Utils::Hook(0x629D7A, R_RegisterStringDvars_stub, HOOK_JUMP).install()->quick();

		// Print loaded modules to console
		Utils::Hook(0x46FD00, CL_PreInitRenderer_stub, HOOK_JUMP).install()->quick();
		
		// fix default console prints
		Utils::Hook(0x5F4EE1, Fix_ConsolePrints01, HOOK_JUMP).install()->quick();
		Utils::Hook(0x57769D, Fix_ConsolePrints02, HOOK_JUMP).install()->quick();
		Utils::Hook(0x52F3EE, Fix_ConsolePrints03, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4BF04A, Fix_ConsolePrints04, HOOK_JUMP).install()->quick(); 
		Utils::Hook::Nop(0x4BF05B, 5); Utils::Hook::Nop(0x4BF06C, 5);


		// --------
		// FastFile
	
		// rewritten the whole function
		Utils::Hook(0x5F4810, DB_LoadCommonFastFiles, HOOK_CALL).install()->quick();

		// ^ Com_StartHunkUsers Mid-hook (realloc files that were unloaded on map load)
		Utils::Hook::Nop(0x50020F, 6);  Utils::Hook(0x50020F, Com_StartHunkUsers_stub, HOOK_JUMP).install()->quick();
		

		// ----
		// IWDs

		// Remove Impure client (iwd) check 
		Utils::Hook::Nop(0x55BFB3, 30);
		
		// Load "iw_" and "xcommon_" iwds as localized (works in all situations + elements in xcommon files can overwrite prior files)
		Utils::Hook(0x55DBB4, FS_MakeIWDsLocalized, HOOK_JUMP).install()->quick();


		// --------
		// Commands

		Command::Add("iw3xo_github", [](Command::Params)
		{
			ShellExecute(0, 0, L"https://github.com/xoxor4d/iw3xo-dev/", 0, 0, SW_SHOW);
		});

		Command::Add("iw3xo_radiant_github", [](Command::Params)
		{
			ShellExecute(0, 0, L"https://github.com/xoxor4d/iw3xo-radiant/", 0, 0, SW_SHOW);
		});

		Command::Add("help", [](Command::Params)
		{
			ShellExecute(0, 0, L"https://xoxor4d.github.io/projects/iw3xo/#in-depth", 0, 0, SW_SHOW);
		});

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
