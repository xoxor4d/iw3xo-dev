#include "STDInclude.hpp"

namespace Components
{
	// *
	// Dvars

	// register additional dvars in R_Init->R_RegisterDvars
	void R_RegisterAdditionalDvars()
	{
		if (Components::active._UI)
		{
			_UI::main_menu_register_dvars();
		}
		
		if (Components::active._Renderer)
		{
			_Renderer::R_RegisterBufferDvars();
		}

		if (Components::active.RB_ShaderOverlays)
		{
			RB_ShaderOverlays::Register_StringDvars();
		}
	}

	void ForceDvarsOnInit()
	{
		const auto dedicated = Game::Dvar_FindVar("dedicated");
		//auto fs_usedevdir = Game::Dvar_FindVar("fs_usedevdir");
		auto in_mouse = Game::Dvar_FindVar("in_mouse");

		if (dedicated && !dedicated->current.integer)
		{
			auto sv_pure		= Game::Dvar_FindVar("sv_pure");
			auto sv_punkbuster	= Game::Dvar_FindVar("sv_punkbuster");
			auto r_zFeather		= Game::Dvar_FindVar("r_zFeather");
			auto r_distortion	= Game::Dvar_FindVar("r_distortion");
			auto r_fastSkin		= Game::Dvar_FindVar("r_fastSkin");

			if (sv_pure && sv_pure->current.enabled)
			{
				Game::Dvar_SetValue(sv_pure, false); // quick set the value
				Game::Cmd_ExecuteSingleCommand(0, 0, "sv_pure 0\n");
			}

			if (sv_punkbuster && sv_punkbuster->current.enabled)
			{
				Game::Dvar_SetValue(sv_punkbuster, false); // quick set the value
				Game::Cmd_ExecuteSingleCommand(0, 0, "sv_punkbuster 0\n");
			}

			// force depthbuffer
			if (r_zFeather && !r_zFeather->current.enabled)
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "r_zFeather 1\n");
			}

			// enable distortion (it creates a rendertarget thats needed)
			if (r_distortion && !r_distortion->current.enabled)
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "r_distortion 1\n");
			}

			// disable r_fastSkin in-case someone increased renderbuffers
			if (r_fastSkin && r_fastSkin->current.enabled)
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "r_fastSkin 0\n");
			}
		}

		// Do not force fs_usedevdir, some mods do not like that.
		//if (fs_usedevdir && !fs_usedevdir->current.enabled)
		//{
		//	Game::Dvar_SetValue(fs_usedevdir, true); // quick set the value
		//	Game::Cmd_ExecuteSingleCommand(0, 0, "fs_usedevdir 1\n");
		//}

		if (in_mouse && !in_mouse->current.enabled)
		{
			Game::Dvar_SetValue(in_mouse, true); // quick set the value
			Game::Cmd_ExecuteSingleCommand(0, 0, "in_mouse 1\n");
		}

		if (Components::active.Gui)
		{
			Gui::load_settings();
		}
	}

	__declspec(naked) void R_RegisterAdditionalDvars_stub()
	{
		const static uint32_t R_RegisterSunDvars_Func = 0x636FC0;
		const static uint32_t retnPt = 0x629D7F;
		__asm
		{
			pushad;
			call	R_RegisterAdditionalDvars;
			popad;

			call	R_RegisterSunDvars_Func;
			jmp		retnPt;
		}
	}

	__declspec(naked) void R_BeginRegistration_stub()
	{
		const static uint32_t rtnPt = 0x46CB0E;
		__asm
		{
			pushad;
			call	ForceDvarsOnInit;
			popad;

			mov     ecx, 0Ch;	// overwritten op
			jmp		rtnPt;
		}
	}

	// disable cheat / write check
	__declspec(naked) void disable_dvar_cheats_stub()
	{
		const static uint32_t overjumpTo = 0x56B3A1;
		__asm
		{
			movzx   eax, word ptr[edi + 8];	// overwritten op
			jmp		overjumpTo;
		}
	}

	// *
	// FastFiles

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
		Game::XZoneInfo XZoneInfoStack[8];

		// ------------------------------------

		XZoneInfoStack[i].name = *Game::zone_code_post_gfx_mp;
		XZoneInfoStack[i].allocFlags = Game::XZONE_FLAGS::XZONE_POST_GFX;
		XZoneInfoStack[i].freeFlags = Game::XZONE_FLAGS::XZONE_POST_GFX_FREE;
		++i;

		// ------------------------------------

		if (*Game::zone_localized_code_post_gfx_mp)
		{
			XZoneInfoStack[i].name = *Game::zone_localized_code_post_gfx_mp;
			XZoneInfoStack[i].allocFlags = Game::XZONE_FLAGS::XZONE_LOC_POST_GFX;
			XZoneInfoStack[i].freeFlags = Game::XZONE_FLAGS::XZONE_LOC_POST_GFX_FREE;
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

					XZoneInfoStack[i].allocFlags = Game::XZONE_FLAGS::XZONE_MOD | Game::XZONE_FLAGS::XZONE_DEBUG;
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
			XZoneInfoStack[i].name = *Game::zone_mod;
			XZoneInfoStack[i].allocFlags = Game::XZONE_FLAGS::XZONE_MOD;
			XZoneInfoStack[i].freeFlags = Game::XZONE_FLAGS::XZONE_MOD_FREE;
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
			XZoneInfoStack[i].name = *Game::zone_ui_mp;
			XZoneInfoStack[i].allocFlags = Game::XZONE_FLAGS::XZONE_UI;
			XZoneInfoStack[i].freeFlags = Game::XZONE_FLAGS::XZONE_UI_FREE;
			++i;
		}

		// ------------------------------------

		XZoneInfoStack[i].name = *Game::zone_common_mp;
		XZoneInfoStack[i].allocFlags = Game::XZONE_FLAGS::XZONE_COMMON;
		XZoneInfoStack[i].freeFlags = Game::XZONE_FLAGS::XZONE_COMMON_FREE;
		++i;

		// ------------------------------------

		if (*Game::zone_localized_common_mp) // not loaded on when starting dedicated servers
		{
			XZoneInfoStack[i].name = *Game::zone_localized_common_mp;
			XZoneInfoStack[i].allocFlags = Game::XZONE_FLAGS::XZONE_LOC_COMMON;
			XZoneInfoStack[i].freeFlags = Game::XZONE_FLAGS::XZONE_LOC_COMMON_FREE;
			++i;
		}

		// ------------------------------------

		// load user addon zone
		if (FF_LOAD_ADDON_OPT)
		{
			// custom addon file
			if (Game::DB_FileExists(FF_ADDON_OPT_NAME, Game::DB_FILE_EXISTS_PATH::DB_PATH_ZONE))
			{
				XZoneInfoStack[i].name = FF_ADDON_OPT_NAME;
				XZoneInfoStack[i].allocFlags = Game::XZONE_FLAGS::XZONE_COMMON; //Game::XZONE_FLAGS::XZONE_MOD; // free when loading mods?
				XZoneInfoStack[i].freeFlags = Game::XZONE_FLAGS::XZONE_ZERO; // do not free any other fastfiles?
				++i;
			}
			else
			{
				Game::Com_PrintMessage(0, Utils::VA("^1DB_LoadCommonFastFiles^7:: %s.ff not found (optional user addon)\n", FF_ADDON_OPT_NAME), 0);
			}
		}

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

	// realloc zones that get unloaded on map load (eg. ui_mp)
	void Com_StartHunkUsers()
	{
		auto useFastFile = Game::Dvar_FindVar("useFastFile");

		if (useFastFile && useFastFile->current.enabled)
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

						XZoneInfoStack[i].allocFlags = Game::XZONE_FLAGS::XZONE_MOD | Game::XZONE_FLAGS::XZONE_DEBUG;
						XZoneInfoStack[i].freeFlags = Game::XZONE_FLAGS::XZONE_UI_FREE_INGAME;
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

			XZoneInfoStack[i].name = *Game::zone_ui_mp;
			XZoneInfoStack[i].allocFlags = Game::XZONE_FLAGS::XZONE_UI;
			XZoneInfoStack[i].freeFlags = Game::XZONE_FLAGS::XZONE_UI_FREE_INGAME;
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
			pushad;
			call	Com_StartHunkUsers;
			popad;

			jmp		retnPt;			// continue exec
		}
	}

	// *
	// IWDs

	bool iwd_match_xcommon(const char* s0)
	{
		if (!Utils::Q_stricmpn(s0, "xcommon_", 8))
		{
			return 0;
		}

		return 1;
	}

	bool iwd_match_iw(const char* s0)
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
		const static uint32_t errMsg = 0x55DBCA;
		const static uint32_t retn_pt = 0x55DBE8;
		__asm
		{
			push	edi;				// current iwd string + ext
			call	iwd_match_iw;
			add		esp, 4;
			test    eax, eax;

			je		MATCH;				// jump if iwd matched iw_
										// if not, cmp to xcommon_
			push	edi;				// current iwd string + ext
			call	iwd_match_xcommon;
			add		esp, 4;
			test    eax, eax;

			je		MATCH;				// jump if iwd matched xcommon_
			jmp		errMsg;				// yeet


		MATCH:
			mov     ebx, [ebp - 4];		// whatever df that is
			mov		[ebp - 8], 1;		// set qLocalized to true ;)
			mov		[ebp - 0Ch], esi;	// whatever df that is
			jmp		retn_pt;
		}
	}

	// *
	// DB

	void _Common::db_realloc_entry_pool()
	{
		AssertSize(Game::XAssetEntry, 16);

		size_t size = 789312;
		Game::XAssetEntry* entry_pool = Utils::Memory::GetAllocator()->allocateArray<Game::XAssetEntry>(size);

		// Apply new size
		Utils::Hook::Set<DWORD>(0x488F50, size);

		// Apply new pool
		DWORD asset_entry_pool_patches[] =
		{
			0x488F48, 0x489178, 0x4891A5, 0x4892C4, 0x489335,
			0x489388, 0x48944E, 0x4898F4, 0x4899A4, 0x489B67,
			0x489C23, 0x489CDA, 0x489D03, 0x489DAD, 0x489EF6,
			0x48AE6D, 0x48AFC4, 0x48B049, 0x48B16C, 0x48B1E4,
			0x48B4A4, 0x48B4F8
		};

		for (int i = 0; i < ARRAYSIZE(asset_entry_pool_patches); ++i) {
			Utils::Hook::Set<Game::XAssetEntry*>(asset_entry_pool_patches[i], entry_pool);
		}

		Utils::Hook::Set<Game::XAssetEntry*>(0x488F31, entry_pool + 1);
		Utils::Hook::Set<Game::XAssetEntry*>(0x488F42, entry_pool + 1);
	}

	_Common::_Common()
	{
		// *
		// DB

		this->db_realloc_entry_pool();

		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_IMAGE, 7168);
		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_LOADED_SOUND, 2700);
		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_FX, 1200);
		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_LOCALIZE_ENTRY, 14000);
		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_XANIMPARTS, 8192);
		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_XMODEL, 5125);
		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_PHYSPRESET, 128);
		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_MENU, 1280);
		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_MENULIST, 256);
		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_MATERIAL, 8192);
		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_WEAPON, 2400);
		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_STRINGTABLE, 800);


		// increase hunkTotal from 10mb to 15mb
		Utils::Hook::Set<BYTE>(0x563A21 + 8, 0xF0);

		// gmem from 128 to 512
		Utils::Hook::Set<BYTE>(0x4FF23B + 4, 0x20);
		//gmem prim pos ^
		Utils::Hook::Set<BYTE>(0x4FF26B + 9, 0x20);


		// *
		// Dvars

		// Stub after renderer was initialized
		Utils::Hook(0x46CB09, R_BeginRegistration_stub, HOOK_JUMP).install()->quick();

		// Register String Dvars (doing so on module load crashes the game (SL_GetStringOfSize))
		Utils::Hook(0x629D7A, R_RegisterAdditionalDvars_stub, HOOK_JUMP).install()->quick();

		// Disable dvar cheat / write protection
		Utils::Hook(0x56B335, disable_dvar_cheats_stub, HOOK_JUMP).install()->quick();
		Utils::Hook::Nop(0x56B339 + 1, 1);

		// *
		// FastFiles

		// rewritten the whole function
		Utils::Hook(0x5F4810, DB_LoadCommonFastFiles, HOOK_CALL).install()->quick();

		// ^ Com_StartHunkUsers Mid-hook (realloc files that were unloaded on map load)
		Utils::Hook::Nop(0x50020F, 6);		Utils::Hook(0x50020F, Com_StartHunkUsers_stub, HOOK_JUMP).install()->quick();

		// *
		// IWDs

		// Remove Impure client (iwd) check 
		Utils::Hook::Nop(0x55BFB3, 30);

		// Load "iw_" and "xcommon_" iwds as localized (works in all situations + elements in xcommon files can overwrite prior files)
		Utils::Hook(0x55DBB4, FS_MakeIWDsLocalized, HOOK_JUMP).install()->quick();

		// *
		// Commands

		// load / reload a zonefile
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
			info[0].allocFlags = Game::XZONE_FLAGS::XZONE_ZERO;
			info[0].freeFlags = Game::XZONE_FLAGS::XZONE_MOD;

			info[1].name = zone.data();
			info[1].allocFlags = Game::XZONE_FLAGS::XZONE_MOD;
			info[1].freeFlags = Game::XZONE_FLAGS::XZONE_ZERO;

			Game::DB_LoadXAssets(info, 2, 1);
		});
	}

	_Common::~_Common()
	{ }
}