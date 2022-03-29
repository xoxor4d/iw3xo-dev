#include "std_include.hpp"

namespace components
{
	void _map::on_load()
	{
		memset(&game::glob::cgs_addons, 0, sizeof(game::cgs_addon_s));

		if (components::active.radiant_livelink)
		{
			memset(&game::glob::dynamic_brush_models, 0, sizeof(game::dynBrushModelsArray_t));
			memset(&game::glob::radiant_saved_brushes, 0, sizeof(game::saved_radiant_brushes_s));

			radiant_livelink::find_dynamic_bmodels();
		}

		if (components::active.draw_collision)
		{
			game::glob::debug_collision_initialized = false;
		}
	}

	void _map::on_unload()
	{
		if (components::active.radiant_livelink)
		{
			radiant_livelink::shutdown_livelink();
		}
	}

	// --------

	__declspec(naked) void sv_spawnserver_stub()
	{
		const static uint32_t retn_addr = 0x52F8A7;
		__asm
		{
			// overwritten op's
			add     esp, 8;
			and		esi, 0FFFFFFF0h;

			call	_map::on_load;
			jmp		retn_addr;
		}
	}

	__declspec(naked) void com_shutdowninternal_stub()
	{
		const static uint32_t retn_addr = 0x5004C0;
		__asm
		{
			call	_map::on_unload;
			jmp		retn_addr;
		}
	}

	struct brushside_test_s
	{
		game::cbrushside_t brush_side[6];
		game::cplane_s brushside_plane[6];
	};

	static brushside_test_s addon_brushsides = {};

	static game::cbrush_t reloc_cm_brushes[32768] = {};
	static game::cbrush_t* reloc_cm_brushes_ptr = reloc_cm_brushes;

	static game::cmodel_t reloc_cm_cmodels[4095] = {};
	static game::cmodel_t* reloc_cm_cmodels_ptr = reloc_cm_cmodels;

	// *
	// MP-SP

	std::string mpsp_map_name;
	char* mpsp_mapents_buffer = nullptr;

	void post_clipmap(game::clipMap_t* cm)
	{
		// copy cmodels + assign new pointer
		memcpy(&reloc_cm_cmodels, cm->cmodels, sizeof(game::cmodel_t) * cm->numSubModels);
		cm->cmodels = reloc_cm_cmodels_ptr;

		// copy brushes + assign new pointer
		memcpy(&reloc_cm_brushes, cm->brushes, sizeof(game::cbrush_t) * cm->numBrushes);
		cm->brushes = reloc_cm_brushes_ptr;

		// *
		// mp-sp

		if (_map::mpsp_is_sp_map)
		{
			const auto& fs_basepath = game::Dvar_FindVar("fs_basepath");
			const auto& fs_game = game::Dvar_FindVar("fs_game");

			if (fs_basepath && fs_game)
			{
				std::string mod = fs_game->current.string;
				utils::replace(mod, "/", "\\"s);

				std::string base_path;
				base_path += fs_basepath->current.string + "\\"s;
				base_path += mod + "\\mapents\\"s;

				if (std::filesystem::exists(base_path))
				{
					std::ifstream mapents;
					mapents.open(base_path + mpsp_map_name + ".ents", std::ios::in | std::ios::binary);

					if (!mapents.is_open())
					{
						game::Com_PrintMessage(0, utils::va("[!] Failed to open mapents file [%s]", (mpsp_map_name + ".ents"s).c_str()), 0);
					}

					mapents.ignore(std::numeric_limits<std::streamsize>::max());
					const auto length = static_cast<size_t>(mapents.gcount());

					mapents.clear();   //  Since ignore will have set eof.
					mapents.seekg(0, std::ios_base::beg);

					if (mpsp_mapents_buffer)
					{
						mpsp_mapents_buffer = (char*)realloc(mpsp_mapents_buffer, length + 1);
					}
					else
					{
						mpsp_mapents_buffer = (char*)malloc(length + 1);
					}

					mapents.read(mpsp_mapents_buffer, length);
					mapents.close();

					mpsp_mapents_buffer[length] = 0;

					// save original ptr
					_map::mpsp_mapents_original = cm->mapEnts->entityString;

					// replace ptr
					cm->mapEnts->entityString = mpsp_mapents_buffer;
				}
			}
		}
	}

	void __declspec(naked) post_clipmap_load_stub()
	{
		const static uint32_t retn_addr = 0x480898;
		__asm
		{
			pushad;
			mov		eax, [eax]; // ptr to clipmap ptr
			push	eax;		// clipmap ptr
			call	post_clipmap;
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
		return _map::mpsp_is_sp_map;
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
		_map::mpsp_is_sp_map = !strstr(mapname, "mp_");
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
		if (dvars::mpsp_require_gsc->current.enabled)
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
		if (dvars::mpsp_require_gsc->current.enabled)
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

	void _map::create_debug_collision()
	{
		int dynb_index = game::glob::dynamic_brush_models.mapped_bmodels;

		auto cmodel = &reloc_cm_cmodels[game::cm->numSubModels];

		utils::vector::set3(cmodel->mins, -4.80011272f, -16.5f, -33.0f);
		utils::vector::set3(cmodel->maxs, 5.19986343f, 15.5f, 33.0f);
		cmodel->radius = 37.2597466f;

		utils::vector::set3(cmodel->leaf.mins, -3.92510986f, -15.625f, -32.125f);
		utils::vector::set3(cmodel->leaf.maxs, 4.3248291f, 14.625f, 32.125f);
		cmodel->leaf.brushContents = 0x8000001;

		game::glob::dynamic_brush_models.brushes[dynb_index].is_custom = true;

		game::glob::dynamic_brush_models.brushes[dynb_index].cmodelIndex = game::cm->numSubModels;
		game::glob::dynamic_brush_models.brushes[dynb_index].cmodel = cmodel;

		++game::cm->numSubModels;


		// index = s.number
		auto ent = game::G_Spawn();
		ent->s.eType = 6;
		ent->s.solid = 0xFFFFFF;
		ent->r.bmodel = 1;
		utils::vector::copy(cmodel->mins, ent->r.mins, 3);
		utils::vector::copy(cmodel->maxs, ent->r.maxs, 3);
		ent->r.contents = cmodel->leaf.brushContents;
		ent->flags = 4096;
		ent->handler = 5;
		ent->model = 1;
		ent->s.index = 1;

		game::cgs->nextSnap->numEntities;
		auto x = game::cg_entities;
		auto asd = game::cg_entities[ent->s.number - 1];

		//const std::int16_t model_index = game::G_ModelIndex("xo_radiantcam"); // axis
		//ent->model = model_index;
		//ent->s.index = model_index;
		//ent->r.svFlags = 0x04; // hide brushmodel

		game::SV_LinkEntity(ent);

		utils::vector::set3(ent->r.currentOrigin, -634.0f, -165.0f, 992.0f);
		utils::vector::copy(ent->r.currentOrigin, game::glob::dynamic_brush_models.brushes[dynb_index].originalOrigin, 3);

		game::G_SetOrigin(ent, ent->r.currentOrigin);
		game::G_CallSpawnEntity(ent);

		auto classname = _ggame::ent_get_gscr_string(ent->classname);

		game::glob::dynamic_brush_models.brushes[dynb_index].ent = ent;
		game::glob::dynamic_brush_models.brushes[dynb_index].entityIndex = ent->s.number;

		++game::glob::dynamic_brush_models.mapped_bmodels;

		// brush

		const char* test = "\v\t\n\x2\x2\n\x3\v\v\x3\n";

		auto b = &reloc_cm_brushes[game::cm->numBrushes];
		utils::vector::copy(cmodel->mins, b->mins, 3);
		utils::vector::copy(cmodel->mins, b->maxs, 3);
		b->contents = 0x8000001;
		b->numsides = 6;
		b->baseAdjacentSide = (char*)test;
		b->sides = addon_brushsides.brush_side;

		++game::cm->numBrushes;

		game::cbrushside_t* bside;

		// 0
		bside = &addon_brushsides.brush_side[0];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[0];
		utils::vector::set3(bside->plane->normal, -0.141421348f, -0.989949465f, 0.0f);
		bside->plane->dist = 14.7502594f;
		bside->plane->type = 3;
		bside->plane->signbits = 3;

		// 1
		bside = &addon_brushsides.brush_side[1];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[1];
		utils::vector::set3(bside->plane->normal, 0.999444902f, 0.0333148278f, 0.0f);
		bside->plane->dist = 3.68115234f;
		bside->plane->type = 3;
		bside->plane->signbits = 0;

		// 2
		bside = &addon_brushsides.brush_side[2];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[2];
		utils::vector::set3(bside->plane->normal, -0.141421363f, 0.989949465f, 0.0f);
		bside->plane->dist = 13.9017105f;
		bside->plane->type = 3;
		bside->plane->signbits = 1;

		// 3
		bside = &addon_brushsides.brush_side[3];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[3];
		utils::vector::set3(bside->plane->normal, 0.999362826f, 0.035691529f, 0.0f);
		bside->plane->dist = 3.31585693f;
		bside->plane->type = 3;
		bside->plane->signbits = 3;

		// 4
		bside = &addon_brushsides.brush_side[4];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[4];
		utils::vector::set3(bside->plane->normal, -0.124034733f, 0.0f, 0.992277861f);
		bside->plane->dist = 31.2319336f;
		bside->plane->type = 3;
		bside->plane->signbits = 1;

		// 5
		bside = &addon_brushsides.brush_side[5];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[5];
		utils::vector::set3(bside->plane->normal, -0.124034733f, 0.0f, 0.992277861f);
		bside->plane->dist = 31.2318726f;
		bside->plane->type = 3;
		bside->plane->signbits = 5;
	}

	void relocate_struct_ref(const std::uintptr_t code_addr, const void* target_addr, const std::uintptr_t base_addr = 0, const std::uintptr_t dest_addr = 0)
	{
		const auto struct_offset = dest_addr - base_addr;
		const auto struct_final_addr = reinterpret_cast<std::uintptr_t>(target_addr) + struct_offset;

		utils::hook::set<std::uintptr_t>(code_addr, struct_final_addr);
	}

	void relocate_struct_ref(const std::uintptr_t* code_addr, const void* target_addr, const int patch_amount)
	{
		const auto struct_final_addr = reinterpret_cast<std::uintptr_t>(target_addr);

		for(auto i = 0; i < patch_amount; i++)
		{
			utils::hook::set<std::uintptr_t>(code_addr[i], struct_final_addr);
		}
	}

	_map::_map()
	{
		// relocate clipmap->cmodels
		uintptr_t cm_cmodel_patches[] =
		{
			0x45909A + 2,	0x490469 + 2,	0x4ED9CD + 2,	0x4EDAF0 + 1,
			0x4EDB24 + 1,	0x4EE980 + 2,	0x4F2EA0 + 2,	0x4F4D4C + 2,
			0x4F66EB + 2,	0x4F7080 + 1,	0x4F7785 + 2,	0x52D684 + 2,
			0x52D71E + 2,	0x5A67F7 + 2,	0x5A6C30 + 2,	0x5A846B + 2,
			0x5A8A71 + 2,

		}; relocate_struct_ref(cm_cmodel_patches, &reloc_cm_cmodels_ptr, ARRAYSIZE(cm_cmodel_patches));


		// relocate clipmap->brushes
		uintptr_t cm_brush_patches[] =
		{
			0x4EDC78 + 2,	0x4EE017 + 2,	0x4EE114 + 2,	0x4EE198 + 2,
			0x4EE1CC + 2,	0x4EE1FF + 2,	0x4EE233 + 2,	0x4EE28A + 2,
			0x4EE362 + 2,	0x4EE3C8 + 2,	0x4EE42D + 2,	0x4EE493 + 2,
			0x4EE51A + 2,	0x4EE7B2 + 2,	0x4EE9DB + 2,	0x4EEC44 + 1,
			0x4EEC63 + 2,	0x4EF059 + 2,	0x4EF159 + 2,	0x4EF9F3 + 2,
			0x4F2DEF + 2,	0x4F332F + 2,	0x4F3F8C + 2,	0x4F58E6 + 2,
			0x4F5C2A + 2,	0x4F6AC0 + 2,	0x5A7B2D + 2,

		}; relocate_struct_ref(cm_brush_patches, &reloc_cm_brushes_ptr, ARRAYSIZE(cm_brush_patches));

		// ^ todo
		// spawn entities, cbrushsides, cplanes

		// -----------

		// On map load
		utils::hook::nop(0x52F8A1, 6);  utils::hook(0x52F8A1, sv_spawnserver_stub, HOOK_JUMP).install()->quick(); // after SV_InitGameProgs before G_RunFrame

		// On map unload
		utils::hook(0x4FCDF8, com_shutdowninternal_stub, HOOK_JUMP).install()->quick(); // before Com_Restart

		// *
		// MP-SP

		// stub to modify clipmap after it was loaded - [MP-SP] custom mapents
		utils::hook(0x480893, post_clipmap_load_stub, HOOK_JUMP).install()->quick();

		// fix bsp path
		utils::hook(0x44AA17, mpsp_bsp_name_stub01, HOOK_JUMP).install()->quick();
		utils::hook(0x45BF8C, mpsp_bsp_name_stub02, HOOK_JUMP).install()->quick();
		utils::hook(0x52F727, mpsp_bsp_name_stub03, HOOK_JUMP).install()->quick();

		// fix gsc path
		utils::hook(0x424C7F, mpsp_fx_def_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x4CB015, mpsp_map_gsc_stub, HOOK_JUMP).install()->quick();

		// skip sv_ent visibility check on sp maps
		utils::hook(0x5346D6, mpsp_add_entities_visible_from_point_stub, HOOK_JUMP).install()->quick();

		// no longer needed because of custom mapents
		//utils::hook(0x4DFF20, mpsp_ignore_entities_stub, HOOK_JUMP).install()->quick();

		dvars::mpsp_require_gsc = game::Dvar_RegisterBool(
			/* name		*/ "mpsp_require_gsc",
			/* desc		*/ "enabled: load spmod map gsc's (mostly fx)",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved);
	}

	_map::~_map()
	{
		free(mpsp_mapents_buffer);
	}
}