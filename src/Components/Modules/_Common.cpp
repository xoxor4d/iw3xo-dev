#include "STDInclude.hpp"

namespace components
{
	// *
	// dvars

	// register additional dvars in R_Init > R_RegisterDvars
	void register_additional_dvars()
	{
		if (components::active._ui)
		{
			_ui::register_dvars();
		}
		
		if (components::active._renderer)
		{
			_renderer::register_dvars();
		}

		if (components::active.postfx_shaders)
		{
			postfx_shaders::register_dvars();
		}
	}

	__declspec(naked) void register_additional_dvars_stub()
	{
		const static uint32_t R_RegisterSunDvars_func = 0x636FC0;
		const static uint32_t retn_addr = 0x629D7F;
		__asm
		{
			pushad;
			call	register_additional_dvars;
			popad;

			call	R_RegisterSunDvars_func;
			jmp		retn_addr;
		}
	}

	void force_dvars_on_init()
	{
		if (const auto	dedicated = Game::Dvar_FindVar("dedicated"); 
						dedicated && !dedicated->current.integer)
		{
			const auto sv_pure = Game::Dvar_FindVar("sv_pure");
			const auto sv_punkbuster	 = Game::Dvar_FindVar("sv_punkbuster");
			const auto r_zFeather = Game::Dvar_FindVar("r_zFeather");
			const auto r_distortion = Game::Dvar_FindVar("r_distortion");
			const auto r_fastSkin = Game::Dvar_FindVar("r_fastSkin");

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

		if (const auto	in_mouse = Game::Dvar_FindVar("in_mouse"); 
						in_mouse && !in_mouse->current.enabled)
		{
			Game::Dvar_SetValue(in_mouse, true); // quick set the value
			Game::Cmd_ExecuteSingleCommand(0, 0, "in_mouse 1\n");
		}

		if (components::active.gui)
		{
			gui::load_settings();
		}
	}

	__declspec(naked) void R_BeginRegistration_stub()
	{
		const static uint32_t retn_addr = 0x46CB0E;
		__asm
		{
			pushad;
			call	force_dvars_on_init;
			popad;

			mov     ecx, 0Ch;	// overwritten op
			jmp		retn_addr;
		}
	}

	// disable cheat / write check
	__declspec(naked) void disable_dvar_cheats_stub()
	{
		const static uint32_t retn_addr = 0x56B3A1;
		__asm
		{
			movzx   eax, word ptr[edi + 8];	// overwritten op
			jmp		retn_addr;
		}
	}

	// *
	// FastFiles

	// load common + addon fastfiles (only on init or when changing CGame)
	void load_common_fast_files()
	{
		/*
		only unload zones with a set flag
		:: zone.name = 0;
		:: zone.allocFlags = 0;
		:: zone.freeFlags = ZONE_FLAG_TO_UNLOAD
		*/

		int i = 0;
		Game::XZoneInfo xzone_info_stack[8];

		// ------------------------------------

		xzone_info_stack[i].name = *Game::zone_code_post_gfx_mp;
		xzone_info_stack[i].allocFlags = Game::XZONE_FLAGS::XZONE_POST_GFX;
		xzone_info_stack[i].freeFlags = Game::XZONE_FLAGS::XZONE_POST_GFX_FREE;
		++i;

		// ------------------------------------

		if (*Game::zone_localized_code_post_gfx_mp)
		{
			xzone_info_stack[i].name = *Game::zone_localized_code_post_gfx_mp;
			xzone_info_stack[i].allocFlags = Game::XZONE_FLAGS::XZONE_LOC_POST_GFX;
			xzone_info_stack[i].freeFlags = Game::XZONE_FLAGS::XZONE_LOC_POST_GFX_FREE;
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
					xzone_info_stack[i].name = FF_ADDON_MENU_NAME;

					xzone_info_stack[i].allocFlags = Game::XZONE_FLAGS::XZONE_MOD | Game::XZONE_FLAGS::XZONE_DEBUG;
					++i;

					// we have to sync assets or we run into issues when the game is trying to access unloaded assets
					Game::DB_LoadXAssets(&xzone_info_stack[0], i, 0);

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
				Game::Com_PrintMessage(0, utils::va("^1DB_LoadCommonFastFiles^7:: %s.ff not found. \n", FF_ADDON_MENU_NAME), 0);
			}
		}

		// mod loading -- maybe sync assets for addon menu too?
		if (*Game::zone_mod)
		{
			xzone_info_stack[i].name = *Game::zone_mod;
			xzone_info_stack[i].allocFlags = Game::XZONE_FLAGS::XZONE_MOD;
			xzone_info_stack[i].freeFlags = Game::XZONE_FLAGS::XZONE_MOD_FREE;
			++i;

			Game::DB_LoadXAssets(&xzone_info_stack[0], i, 0);

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
			xzone_info_stack[i].name = *Game::zone_ui_mp;
			xzone_info_stack[i].allocFlags = Game::XZONE_FLAGS::XZONE_UI;
			xzone_info_stack[i].freeFlags = Game::XZONE_FLAGS::XZONE_UI_FREE;
			++i;
		}

		// ------------------------------------

		xzone_info_stack[i].name = *Game::zone_common_mp;
		xzone_info_stack[i].allocFlags = Game::XZONE_FLAGS::XZONE_COMMON;
		xzone_info_stack[i].freeFlags = Game::XZONE_FLAGS::XZONE_COMMON_FREE;
		++i;

		// ------------------------------------

		if (*Game::zone_localized_common_mp) // not loaded on when starting dedicated servers
		{
			xzone_info_stack[i].name = *Game::zone_localized_common_mp;
			xzone_info_stack[i].allocFlags = Game::XZONE_FLAGS::XZONE_LOC_COMMON;
			xzone_info_stack[i].freeFlags = Game::XZONE_FLAGS::XZONE_LOC_COMMON_FREE;
			++i;
		}

		// ------------------------------------

		// load user addon zone
		if (FF_LOAD_ADDON_OPT)
		{
			// custom addon file
			if (Game::DB_FileExists(FF_ADDON_OPT_NAME, Game::DB_FILE_EXISTS_PATH::DB_PATH_ZONE))
			{
				xzone_info_stack[i].name = FF_ADDON_OPT_NAME;
				xzone_info_stack[i].allocFlags = Game::XZONE_FLAGS::XZONE_COMMON; //Game::XZONE_FLAGS::XZONE_MOD; // free when loading mods?
				xzone_info_stack[i].freeFlags = Game::XZONE_FLAGS::XZONE_ZERO; // do not free any other fastfiles?
				++i;
			}
			else
			{
				Game::Com_PrintMessage(0, utils::va("^1DB_LoadCommonFastFiles^7:: %s.ff not found (optional user addon)\n", FF_ADDON_OPT_NAME), 0);
			}
		}

		// load required addon fastfile last, if addon_required loading enabled
		if (FF_LOAD_ADDON_REQ)
		{
			// if the fastfile exists
			if (FF_ADDON_REQ_NAME && Game::DB_FileExists(FF_ADDON_REQ_NAME, Game::DB_FILE_EXISTS_PATH::DB_PATH_ZONE))
			{
				xzone_info_stack[i].name = FF_ADDON_REQ_NAME;
				xzone_info_stack[i].allocFlags = Game::XZONE_FLAGS::XZONE_COMMON; //Game::XZONE_FLAGS::XZONE_MOD; // free when loading mods?
				xzone_info_stack[i].freeFlags = Game::XZONE_FLAGS::XZONE_ZERO; // do not free any other fastfiles?
				++i;
			}

			// if addon_required loading is enabled and file not found
			else if (FF_LOAD_ADDON_REQ)
			{
				Game::Com_PrintMessage(0, utils::va("^1DB_LoadCommonFastFiles^7:: %s.ff not found. \n", FF_ADDON_REQ_NAME), 0);
			}
		}

		// ------------------------------------

		Game::DB_LoadXAssets(&xzone_info_stack[0], i, 0);
	}

	// realloc zones that get unloaded on map load (eg. ui_mp)
	void com_start_hunk_users()
	{
		if (const auto	useFastFile = Game::Dvar_FindVar("useFastFile"); 
						useFastFile && useFastFile->current.enabled)
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
					Game::Com_PrintMessage(0, utils::va("^1Com_StartHunkUsers^7:: %s.ff not found. \n", FF_ADDON_MENU_NAME), 0);
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
	__declspec(naked) void com_start_hunk_users_stub()
	{
		const static uint32_t retn_addr = 0x500238;
		__asm
		{
			pushad;
			call	com_start_hunk_users;
			popad;

			jmp		retn_addr;
		}
	}

	// *
	// IWDs

	bool iwd_match_xcommon(const char* s0)
	{
		if (!utils::q_stricmpn(s0, "xcommon_", 8))
		{
			return false;
		}

		return true;
	}

	bool iwd_match_iw(const char* s0)
	{
		if (!utils::q_stricmpn(s0, "iw_", 3))
		{
			return false;
		}

		return true;
	}

	// load "iw_" iwds and "xcommon_" iwds as localized ones ;)
	__declspec(naked) void FS_MakeIWDsLocalized()
	{
		const static uint32_t err_msg = 0x55DBCA;
		const static uint32_t retn_addr = 0x55DBE8;
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
			jmp		err_msg;			// yeet


		MATCH:
			mov     ebx, [ebp - 4];		// whatever df that is
			mov		[ebp - 8], 1;		// set qLocalized to true ;)
			mov		[ebp - 0Ch], esi;	// whatever df that is
			jmp		retn_addr;
		}
	}


	// *
	// DB

	void _common::db_realloc_entry_pool()
	{
		AssertSize(Game::XAssetEntry, 16);

		size_t size = 789312;
		Game::XAssetEntry* entry_pool = utils::memory::GetAllocator()->allocateArray<Game::XAssetEntry>(size);

		// Apply new size
		utils::hook::set<DWORD>(0x488F50, size);

		// Apply new pool
		DWORD asset_entry_pool_patches[] =
		{
			0x488F48, 0x489178, 0x4891A5, 0x4892C4, 0x489335,
			0x489388, 0x48944E, 0x4898F4, 0x4899A4, 0x489B67,
			0x489C23, 0x489CDA, 0x489D03, 0x489DAD, 0x489EF6,
			0x48AE6D, 0x48AFC4, 0x48B049, 0x48B16C, 0x48B1E4,
			0x48B4A4, 0x48B4F8
		};

		for (int i = 0; i < ARRAYSIZE(asset_entry_pool_patches); ++i) 
		{
			utils::hook::set<Game::XAssetEntry*>(asset_entry_pool_patches[i], entry_pool);
		}

		utils::hook::set<Game::XAssetEntry*>(0x488F31, entry_pool + 1);
		utils::hook::set<Game::XAssetEntry*>(0x488F42, entry_pool + 1);
	}

	_common::_common()
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
		Game::db_realloc_xasset_pool(Game::XAssetType::ASSET_TYPE_GAMEWORLD_SP, 1); // kek


		// increase hunkTotal from 10mb to 15mb
		utils::hook::set<BYTE>(0x563A21 + 8, 0xF0);

		// gmem from 128 to 512
		utils::hook::set<BYTE>(0x4FF23B + 4, 0x20);
		//gmem prim pos ^
		utils::hook::set<BYTE>(0x4FF26B + 9, 0x20);


		// *
		// dvars

		// Stub after renderer was initialized
		utils::hook(0x46CB09, R_BeginRegistration_stub, HOOK_JUMP).install()->quick();

		// Register String dvars (doing so on module load crashes the game (SL_GetStringOfSize))
		utils::hook(0x629D7A, register_additional_dvars_stub, HOOK_JUMP).install()->quick();

		// Disable dvar cheat / write protection
		utils::hook(0x56B335, disable_dvar_cheats_stub, HOOK_JUMP).install()->quick();
		utils::hook::nop(0x56B339 + 1, 1);


		// *
		// FastFiles

		// rewritten the whole function
		utils::hook(0x5F4810, load_common_fast_files, HOOK_CALL).install()->quick();

		// ^ Com_StartHunkUsers Mid-hook (realloc files that were unloaded on map load)
		utils::hook::nop(0x50020F, 6);		utils::hook(0x50020F, com_start_hunk_users_stub, HOOK_JUMP).install()->quick();


		// *
		// IWDs

		// Remove Impure client (iwd) check 
		utils::hook::nop(0x55BFB3, 30);

		// Load "iw_" and "xcommon_" iwds as localized (works in all situations + elements in xcommon files can overwrite prior files)
		utils::hook(0x55DBB4, FS_MakeIWDsLocalized, HOOK_JUMP).install()->quick();


		// *
		// commands

		// load / reload a zonefile
		command::add("loadzone", [](command::params params) // unload zone and load zone again
		{
			if (params.length() < 2)
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
}