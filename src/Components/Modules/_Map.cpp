#include "STDInclude.hpp"

namespace Components
{
	void _Map::OnLoad()
	{
		memset(&Game::Globals::cgsAddons, 0, sizeof(Game::cgsAddon));

		if (Components::active.RadiantRemote)
		{
			memset(&Game::Globals::dynBrushModels, 0, sizeof(Game::dynBrushModelsArray_t));
			memset(&Game::Globals::rad_savedBrushes, 0, sizeof(Game::savedRadiantBrushes));

			RadiantRemote::CM_FindDynamicBrushModels();
		}

		if (Components::active.RB_DrawCollision)
		{
			Game::Globals::dbgColl_initialized = false;
		}
	}

	void _Map::OnUnload()
	{
		if (Components::active.RadiantRemote)
		{
			RadiantRemote::SV_Shutdown();
		}
	}

	// --------

	__declspec(naked) void sv_spawnserver_stub()
	{
		const static uint32_t retnPt = 0x52F8A7;
		__asm
		{
			// overwritten op's
			add     esp, 8;
			and		esi, 0FFFFFFF0h;

			call	_Map::OnLoad;
			jmp		retnPt;
		}
	}

	__declspec(naked) void com_shutdowninternal_stub()
	{
		const static uint32_t Com_Restart_Jmp = 0x5004C0;
		__asm
		{
			call	_Map::OnUnload;
			jmp		Com_Restart_Jmp;
		}
	}


	// *
	// MP-SP

	std::string mpsp_map_name = "";
	char* mpsp_mapents_buffer = nullptr;

#if 0
	int mpsp_ignore_entities(char* entity, [[maybe_unused]] const char* unused_str, [[maybe_unused]] int unused_int)
	{
		if (Utils::StartsWith(entity, "dyn_"))
		{
			return false;
		}

		//if (mpsp_is_sp_map)
		//{
		//	//strcpy(Game::cm->mapEnts->entityString, test);

		//	if (Utils::StartsWith(entity, "info_player_start"))
		//	{
		//		strcpy(entity, "mp_dm_spawn"); // kek works
		//		return true;
		//	}

		//	if (Utils::StartsWith(entity, "worldspawn"))
		//	{
		//		return true;
		//	}

		//	if (Utils::StartsWith(entity, "script_struct"))
		//	{
		//		return true;
		//	}

		//	// ignore all other entities
		//	return false;
		//}

		return true;
	}

	void __declspec(naked) mpsp_ignore_entities_stub()
	{
		const static uint32_t retn_addr = 0x4DFF25;
		__asm
		{
			// already pushed: entity str
			call	mpsp_ignore_entities;
			jmp		retn_addr;
		}
	}
#endif

	void mpsp_replace_mapents(Game::clipMap_t* cm)
	{
		if (_Map::mpsp_is_sp_map)
		{
			const auto& fs_basepath = Game::Dvar_FindVar("fs_basepath");
			const auto& fs_game = Game::Dvar_FindVar("fs_game");

			if (fs_basepath && fs_game)
			{
				std::string mod = fs_game->current.string;
				Utils::Replace(mod, "/", "\\"s);

				std::string base_path;
				base_path += fs_basepath->current.string + "\\"s;
				base_path += mod + "\\mapents\\"s;

				if (std::filesystem::exists(base_path))
				{
					std::ifstream mapents;
					mapents.open(base_path + mpsp_map_name + ".ents", std::ios::in | std::ios::binary);

					if (!mapents.is_open())
					{
						Game::Com_PrintMessage(0, Utils::VA("[!] Failed to open mapents file [%s]", (mpsp_map_name + ".ents"s).c_str()), 0);
					}

					mapents.ignore(std::numeric_limits<std::streamsize>::max());
					const size_t length = static_cast<size_t>(mapents.gcount());
					mapents.clear();   //  Since ignore will have set eof.
					mapents.seekg(0, std::ios_base::beg);

					if (mpsp_mapents_buffer)
					{
						mpsp_mapents_buffer = (char*)realloc(mpsp_mapents_buffer, length + 1);
					}
					else
					{
						mpsp_mapents_buffer = (char*)malloc(length + 1); //new char[length];
					}

					mapents.read(mpsp_mapents_buffer, length);
					mapents.close();

					mpsp_mapents_buffer[length] = 0;

					// save original ptr
					_Map::mpsp_mapents_original = cm->mapEnts->entityString;

					// replace ptr
					cm->mapEnts->entityString = mpsp_mapents_buffer;
				}
			}
		}
	}

	void __declspec(naked) mpsp_replace_mapents_stub()
	{
		const static uint32_t retn_addr = 0x480898;
		__asm
		{
			pushad;
			mov		eax, [eax]; // ptr to clipmap ptr
			push	eax;		// clipmap ptr
			call	mpsp_replace_mapents;
			add		esp, 4;
			popad;

			mov     esi, eax;	// og
			mov     eax, [eax]; // og
			push    eax;		// og
			jmp		retn_addr;
		}
	}

	bool skip_visible_from_point_check()
	{
		return _Map::mpsp_is_sp_map;
	}

	void __declspec(naked) mpsp_add_entities_visible_from_point_stub()
	{
		const static uint32_t func_addr_mp_map = 0x52D540;
		const static uint32_t retn_addr_mp_map = 0x5346DB;
		const static uint32_t retn_addr_sp_map = 0x5347B2;
		__asm
		{
			pushad;
			call	skip_visible_from_point_check;
			test	al, al;
			jne		SKIP;

			popad;
			call	func_addr_mp_map; // og
			jmp		retn_addr_mp_map; // og

		SKIP:
			popad;
			jmp		retn_addr_sp_map;
		}
	}

	void mpsp_get_bsp_name(char* filename, int size, [[maybe_unused]] const char* original_format, const char* mapname)
	{
		_Map::mpsp_is_sp_map = !strstr(mapname, "mp_");
		mpsp_map_name = mapname;

		sprintf_s(filename, size, "maps/%s%s.d3dbsp", strstr(mapname, "mp_") ? "mp/" : "", mapname);
	}

	void __declspec(naked) mpsp_bsp_name_stub01()
	{
		const static uint32_t retn_addr = 0x44AA1C;
		__asm
		{
			// already pushed: mapname
			// already pushed: "maps/mp/%s.d3dbsp"

			push	esi; // size
			push	edi; // dest
			call	mpsp_get_bsp_name;
			add		esp, 8;

			jmp		retn_addr;
		}
	}

	void __declspec(naked) mpsp_bsp_name_stub02()
	{
		const static uint32_t retn_addr = 0x45BF91;
		__asm
		{
			// already pushed: mapname
			// already pushed: "maps/mp/%s.d3dbsp"

			push	esi; // size
			push	edi; // dest
			call	mpsp_get_bsp_name;
			add		esp, 8;

			jmp		retn_addr;
		}
	}

	void __declspec(naked) mpsp_bsp_name_stub03()
	{
		const static uint32_t retn_addr = 0x52F72C;
		__asm
		{
			// already pushed: mapname 
			// already pushed: "maps/mp/%s.d3dbsp"

			push	esi; // size
			push	edi; // dest
			call	mpsp_get_bsp_name;
			add		esp, 8;

			jmp		retn_addr;
		}
	}

	void mpsp_get_fx_def(char* filename, int size, [[maybe_unused]] const char* original_format, const char* mapname)
	{
		if (Dvars::mpsp_require_gsc->current.enabled)
		{
			sprintf_s(filename, size, "maps/%s%s_fx.gsc", strstr(mapname, "mp_") ? "mp/" : "", mapname);
		}
		else
		{
			sprintf_s(filename, size, "maps/mp/%s_fx.gsc", mapname);
		}

	}

	void __declspec(naked) mpsp_fx_def_stub()
	{
		const static uint32_t retn_addr = 0x424C84;
		__asm
		{
			// already pushed: mapname 
			// already pushed: "maps/mp/%s.d3dbsp"

			push	esi; // size
			push	edi; // dest
			call	mpsp_get_fx_def;
			add		esp, 8;

			jmp		retn_addr;
		}
	}

	void mpsp_get_map_gsc(char* filename, int size, [[maybe_unused]] const char* original_format, const char* mapname)
	{
		if (Dvars::mpsp_require_gsc->current.enabled)
		{
			sprintf_s(filename, size, "maps/%s%s", strstr(mapname, "mp_") ? "mp/" : "", mapname);
		}
		else
		{
			sprintf_s(filename, size, "maps/mp/%s", mapname);
		}
	}

	void __declspec(naked) mpsp_map_gsc_stub()
	{
		const static uint32_t retn_addr = 0x4CB01A;
		__asm
		{
			// already pushed: mapname 
			// already pushed: "maps/mp/%s.d3dbsp"

			push	esi; // size
			push	edi; // dest
			call	mpsp_get_map_gsc;
			add		esp, 8;

			jmp		retn_addr;
		}
	}


	// --------

	_Map::_Map()
	{ 
		// On map load
		Utils::Hook::Nop(0x52F8A1, 6);  Utils::Hook(0x52F8A1, sv_spawnserver_stub, HOOK_JUMP).install()->quick(); // after SV_InitGameProgs before G_RunFrame

		// On map unload
		Utils::Hook(0x4FCDF8, com_shutdowninternal_stub, HOOK_JUMP).install()->quick(); // before Com_Restart

		// *
		// MP-SP

		// fix bsp path
		Utils::Hook(0x44AA17, mpsp_bsp_name_stub01, HOOK_JUMP).install()->quick();
		Utils::Hook(0x45BF8C, mpsp_bsp_name_stub02, HOOK_JUMP).install()->quick();
		Utils::Hook(0x52F727, mpsp_bsp_name_stub03, HOOK_JUMP).install()->quick();

		// fix gsc path
		Utils::Hook(0x424C7F, mpsp_fx_def_stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4CB015, mpsp_map_gsc_stub, HOOK_JUMP).install()->quick();

		// custom mapents
		Utils::Hook(0x480893, mpsp_replace_mapents_stub, HOOK_JUMP).install()->quick();

		// skip sv_ent visibility check on sp maps
		Utils::Hook(0x5346D6, mpsp_add_entities_visible_from_point_stub, HOOK_JUMP).install()->quick();

		// no longer needed because of custom mapents
		//Utils::Hook(0x4DFF20, mpsp_ignore_entities_stub, HOOK_JUMP).install()->quick();

		Dvars::mpsp_require_gsc = Game::Dvar_RegisterBool(
			/* name		*/ "mpsp_require_gsc",
			/* desc		*/ "enabled: load spmod map gsc's (mostly fx)",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);
	}

	_Map::~_Map()
	{
		free(mpsp_mapents_buffer);
	}
}