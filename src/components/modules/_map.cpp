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
		game::cbrushside_t brush_side[7];
		game::cplane_s brushside_plane[7];
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

	struct scr_const_t
	{
		unsigned __int16 _;
		unsigned __int16 active;
		unsigned __int16 aim_bone;
		unsigned __int16 aim_highest_bone;
		unsigned __int16 aim_vis_bone;
		unsigned __int16 all;
		unsigned __int16 allies;
		unsigned __int16 axis;
		unsigned __int16 bad_path;
		unsigned __int16 begin_firing;
		unsigned __int16 cancel_location;
		unsigned __int16 confirm_location;
		unsigned __int16 crouch;
		unsigned __int16 current;
		unsigned __int16 damage;
		unsigned __int16 dead;
		unsigned __int16 death;
		unsigned __int16 detonate;
		unsigned __int16 direct;
		unsigned __int16 dlight;
		unsigned __int16 done;
		unsigned __int16 empty;
		unsigned __int16 end_firing;
		unsigned __int16 entity;
		unsigned __int16 explode;
		unsigned __int16 failed;
		unsigned __int16 free;
		unsigned __int16 fraction;
		unsigned __int16 goal;
		unsigned __int16 goal_changed;
		unsigned __int16 goal_yaw;
		unsigned __int16 grenade;
		unsigned __int16 grenadedanger;
		unsigned __int16 grenade_fire;
		unsigned __int16 grenade_pullback;
		unsigned __int16 info_notnull;
		unsigned __int16 invisible;
		unsigned __int16 key1;
		unsigned __int16 key2;
		unsigned __int16 killanimscript;
		unsigned __int16 left;
		unsigned __int16 light;
		unsigned __int16 movedone;
		unsigned __int16 noclass;
		unsigned __int16 none;
		unsigned __int16 normal;
		unsigned __int16 player;
		unsigned __int16 position;
		unsigned __int16 projectile_impact;
		unsigned __int16 prone;
		unsigned __int16 right;
		unsigned __int16 reload;
		unsigned __int16 reload_start;
		unsigned __int16 rocket;
		unsigned __int16 rotatedone;
		unsigned __int16 script_brushmodel;
		unsigned __int16 script_model;
		unsigned __int16 script_origin;
		unsigned __int16 snd_enveffectsprio_level;
		unsigned __int16 snd_enveffectsprio_shellshock;
		unsigned __int16 snd_channelvolprio_holdbreath;
		unsigned __int16 snd_channelvolprio_pain;
		unsigned __int16 snd_channelvolprio_shellshock;
		unsigned __int16 stand;
		unsigned __int16 suppression;
		unsigned __int16 suppression_end;
		unsigned __int16 surfacetype;
		unsigned __int16 tag_aim;
		unsigned __int16 tag_aim_animated;
		unsigned __int16 tag_brass;
		unsigned __int16 tag_butt;
		unsigned __int16 tag_clip;
		unsigned __int16 tag_flash;
		unsigned __int16 tag_flash_11;
		unsigned __int16 tag_flash_2;
		unsigned __int16 tag_flash_22;
		unsigned __int16 tag_flash_3;
		unsigned __int16 tag_fx;
		unsigned __int16 tag_inhand;
		unsigned __int16 tag_knife_attach;
		unsigned __int16 tag_knife_fx;
		unsigned __int16 tag_laser;
		unsigned __int16 tag_origin;
		unsigned __int16 tag_weapon;
		unsigned __int16 tag_player;
		unsigned __int16 tag_camera;
		unsigned __int16 tag_weapon_right;
		unsigned __int16 tag_gasmask;
		unsigned __int16 tag_gasmask2;
		unsigned __int16 tag_sync;
		unsigned __int16 target_script_trigger;
		unsigned __int16 tempEntity;
		unsigned __int16 top;
		unsigned __int16 touch;
		unsigned __int16 trigger;
		unsigned __int16 trigger_use;
		unsigned __int16 trigger_use_touch;
		unsigned __int16 trigger_damage;
		unsigned __int16 trigger_lookat;
		unsigned __int16 truck_cam;
		unsigned __int16 weapon_change;
		unsigned __int16 weapon_fired;
		unsigned __int16 worldspawn;
		unsigned __int16 flashbang;
		unsigned __int16 flash;
		unsigned __int16 smoke;
		unsigned __int16 night_vision_on;
		unsigned __int16 night_vision_off;
		unsigned __int16 mod_unknown;
		unsigned __int16 mod_pistol_bullet;
		unsigned __int16 mod_rifle_bullet;
		unsigned __int16 mod_grenade;
		unsigned __int16 mod_grenade_splash;
		unsigned __int16 mod_projectile;
		unsigned __int16 mod_projectile_splash;
		unsigned __int16 mod_melee;
		unsigned __int16 mod_head_shot;
		unsigned __int16 mod_crush;
		unsigned __int16 mod_telefrag;
		unsigned __int16 mod_falling;
		unsigned __int16 mod_suicide;
		unsigned __int16 mod_trigger_hurt;
		unsigned __int16 mod_explosive;
		unsigned __int16 mod_impact;
		unsigned __int16 script_vehicle;
		unsigned __int16 script_vehicle_collision;
		unsigned __int16 script_vehicle_collmap;
		unsigned __int16 script_vehicle_corpse;
		unsigned __int16 turret_fire;
		unsigned __int16 turret_on_target;
		unsigned __int16 turret_not_on_target;
		unsigned __int16 turret_on_vistarget;
		unsigned __int16 turret_no_vis;
		unsigned __int16 turret_rotate_stopped;
		unsigned __int16 turret_deactivate;
		unsigned __int16 turretstatechange;
		unsigned __int16 turretownerchange;
		unsigned __int16 reached_end_node;
		unsigned __int16 reached_wait_node;
		unsigned __int16 reached_wait_speed;
		unsigned __int16 near_goal;
		unsigned __int16 veh_collision;
		unsigned __int16 veh_predictedcollision;
		unsigned __int16 auto_change;
		unsigned __int16 back_low;
		unsigned __int16 back_mid;
		unsigned __int16 back_up;
		unsigned __int16 begin;
		unsigned __int16 call_vote;
		unsigned __int16 freelook;
		unsigned __int16 head;
		unsigned __int16 intermission;
		unsigned __int16 j_head;
		unsigned __int16 manual_change;
		unsigned __int16 menuresponse;
		unsigned __int16 neck;
		unsigned __int16 pelvis;
		unsigned __int16 pistol;
		unsigned __int16 plane_waypoint;
		unsigned __int16 playing;
		unsigned __int16 spectator;
		unsigned __int16 vote;
		unsigned __int16 sprint_begin;
		unsigned __int16 sprint_end;
		unsigned __int16 tag_driver;
		unsigned __int16 tag_passenger;
		unsigned __int16 tag_gunner;
		unsigned __int16 tag_wheel_front_left;
		unsigned __int16 tag_wheel_front_right;
		unsigned __int16 tag_wheel_back_left;
		unsigned __int16 tag_wheel_back_right;
		unsigned __int16 tag_wheel_middle_left;
		unsigned __int16 tag_wheel_middle_right;
		unsigned __int16 tag_detach;
		unsigned __int16 tag_popout;
		unsigned __int16 tag_body;
		unsigned __int16 tag_turret;
		unsigned __int16 tag_turret_base;
		unsigned __int16 tag_barrel;
		unsigned __int16 tag_engine_left;
		unsigned __int16 tag_engine_right;
		unsigned __int16 front_left;
		unsigned __int16 front_right;
		unsigned __int16 back_left;
		unsigned __int16 back_right;
		unsigned __int16 tag_gunner_pov;
	};


	void _map::create_debug_collision()
	{
		int dynb_index = game::glob::dynamic_brush_models.mapped_bmodels;

		auto cmodel = &reloc_cm_cmodels[game::cm->numSubModels];

		utils::vector::set3(cmodel->mins, -17.00000f, -19.18193f, -34.14550f);
		utils::vector::set3(cmodel->maxs, 17.72730f, 18.83999f, 34.02410f);
		cmodel->radius = 42.98976f;

		utils::vector::set3(cmodel->leaf.mins, -16.12500f, -18.30693f, -33.27051f);
		utils::vector::set3(cmodel->leaf.maxs, 16.85230f, 17.96499f, 33.14909f);
		cmodel->leaf.brushContents = 134217729;

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
		ent->model = 0;
		ent->s.index = 0;

		game::cgs->nextSnap->numEntities;
		auto x = game::cg_entities;
		auto asd = game::cg_entities[ent->s.number - 1];

		//const std::int16_t model_index = game::G_ModelIndex("xo_radiantcam"); // axis
		//ent->model = model_index;
		//ent->s.index = model_index;
		//ent->r.svFlags = 0x04; // hide brushmodel

		game::SV_LinkEntity(ent);

		utils::vector::set3(ent->r.currentOrigin, -1104.00f, -1008.00f, -832.00f);
		utils::vector::copy(ent->r.currentOrigin, game::glob::dynamic_brush_models.brushes[dynb_index].originalOrigin, 3);

		game::G_SetOrigin(ent, ent->r.currentOrigin);
		game::G_CallSpawnEntity(ent);

		const auto scr_const = reinterpret_cast<scr_const_t*>(0x1406E90);
		auto classname = scr_const->script_brushmodel; //_ggame::ent_get_gscr_string(ent->classname);

		game::glob::dynamic_brush_models.brushes[dynb_index].ent = ent;
		game::glob::dynamic_brush_models.brushes[dynb_index].entityIndex = ent->s.number;

		++game::glob::dynamic_brush_models.mapped_bmodels;

		// brush

		const char* test = "\v\t\n\x2\x2\n\x3\v\v\x3\n";

		auto b = &reloc_cm_brushes[game::cm->numBrushes];
		utils::vector::copy(cmodel->mins, b->mins, 3);
		utils::vector::copy(cmodel->mins, b->maxs, 3);
		b->contents = 0x8000001;
		b->numsides = 7;
		b->baseAdjacentSide = (char*)test;
		b->sides = addon_brushsides.brush_side;

		++game::cm->numBrushes;

		game::cbrushside_t* bside;

		// 0
		bside = &addon_brushsides.brush_side[0];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[0];
		utils::vector::set3(bside->plane->normal, -0.24254, -0.97014, 0.00000);
		bside->plane->dist = 13.58209f;
		bside->plane->type = 3;
		bside->plane->signbits = 3;

		// 1
		bside = &addon_brushsides.brush_side[1];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[1];
		utils::vector::set3(bside->plane->normal, 0.94471, 0.30727, 0.11451);
		bside->plane->dist = 6.42019f;
		bside->plane->type = 3;
		bside->plane->signbits = 0;

		// 2
		bside = &addon_brushsides.brush_side[2];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[2];
		utils::vector::set3(bside->plane->normal, -0.49614, 0.86824, 0.00000);
		bside->plane->dist = 13.14767f;
		bside->plane->type = 3;
		bside->plane->signbits = 1;

		// 3
		bside = &addon_brushsides.brush_side[3];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[3];
		utils::vector::set3(bside->plane->normal, -0.91381, -0.40614, 0.00000);
		bside->plane->dist = 12.18421f;
		bside->plane->type = 3;
		bside->plane->signbits = 3;

		// 4
		bside = &addon_brushsides.brush_side[4];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[4];
		utils::vector::set3(bside->plane->normal, -0.36466, -0.05882, 0.92928);
		bside->plane->dist = 28.71365f;
		bside->plane->type = 3;
		bside->plane->signbits = 3;

		// 5
		bside = &addon_brushsides.brush_side[5];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[5];
		utils::vector::set3(bside->plane->normal, -0.02158, 0.05934, -0.99800);
		bside->plane->dist = 31.63948f;
		bside->plane->type = 3;
		bside->plane->signbits = 5;

		// 6
		bside = &addon_brushsides.brush_side[6];
		bside->edgeCount = 4;
		bside->plane = &addon_brushsides.brushside_plane[6];
		utils::vector::set3(bside->plane->normal, 0.94868, 0.31623, -0.00000);
		bside->plane->dist = 10.11929f;
		bside->plane->type = 3;
		bside->plane->signbits = 0;
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


	void write_clipmap()
	{
		if (const auto& base_path = game::Dvar_FindVar("fs_basepath");
						base_path)
		{
			const std::string file_path = base_path->current.string + "\\iw3xo\\clipmap\\"s;
			if (!std::filesystem::exists(file_path))
			{
				std::filesystem::create_directories(file_path);
			}

			std::ofstream o;
			const std::string file_name = file_path + "clipmap_";

			for (auto i = 0; i < 1024; i++)
			{
				if (std::filesystem::exists(file_name + std::to_string(i) + ".txt"s))
				{
					//continue;
				}

#define FMT_VEC " [ %.5f ]"
#define FMT_VEC3 " [ %.5f, %.5f, %.5f ]"
#define ARG_VEC3(arg) (arg)[0], (arg)[1], (arg)[2]
#define LGAP " ---- "
#define ENDL std::endl

				o.open((file_name + std::to_string(i) + ".txt").c_str());
				const auto cm = game::cm;

				o << "name: " << cm->name << ENDL << ENDL;

				o << "---- planes ---- count [ " << cm->planeCount << " ] -------" << ENDL;

				for (auto p = 0u; p < cm->planeCount; p++)
				{
					o << "plane [ " << p << " ] :: ";
					o << utils::va("normal" FMT_VEC3, ARG_VEC3(cm->planes[p].normal)) << LGAP;
					o << utils::va("dist" FMT_VEC, cm->planes[p].dist) << LGAP;
					o << utils::va("type [ %d ] ---- signbits [ %d ]", cm->planes[p].type, cm->planes[p].signbits) << ENDL;
				}

				o << ENDL;
				o << "---- brushsides ---- count [ " << cm->numBrushSides << " ] -------" << ENDL;

				for (auto p = 0u; p < cm->numBrushSides; p++)
				{
					const int plane_num = ((DWORD)cm->brushsides[p].plane - (DWORD)cm->planes) / sizeof(game::cplane_s);
					o << "brushside [ " << p << " ] :: ";
					o << "using plane [ " << plane_num << " ]" << LGAP;
					o << utils::va("materialnum [ %d ] ---- first adjacent side offset [ %d ] ---- edgecount [ %d ]", cm->brushsides[p].materialNum, cm->brushsides[p].firstAdjacentSideOffset, cm->brushsides[p].edgeCount) << ENDL;
				}

				o << ENDL;
				o << "---- brush edges ---- count [ " << cm->numBrushEdges << " ] -------" << ENDL;

				o << ENDL;
				o << "---- nodes ---- count [ " << cm->numNodes << " ] -------" << ENDL;

				o << ENDL;
				o << "---- leafs ---- count [ " << cm->numLeafs << " ] -------" << ENDL;

				o << ENDL;
				o << "---- leafbrush nodes ---- count [ " << cm->leafbrushNodesCount << " ] -------" << ENDL;

				o << ENDL;
				o << "---- leafbrushes ---- count [ " << cm->numLeafBrushes << " ] -------" << ENDL;

				o << ENDL;
				o << "---- leafsurfaces ---- count [ " << cm->numLeafSurfaces << " ] -------" << ENDL;

				o << ENDL;
				o << "---- partitions ---- count [ " << cm->partitionCount << " ] -------" << ENDL;

				o << ENDL;
				o << "---- aabb trees ---- count [ " << cm->aabbTreeCount << " ] -------" << ENDL;

				o << ENDL;
				o << "---- submodels ---- count [ " << cm->numSubModels << " ] -------" << ENDL;

				for (auto p = 0u; p < cm->numSubModels; p++)
				{
					o << "submodel [ " << p << " ] :: ";
					o << utils::va("mins" FMT_VEC3, ARG_VEC3(cm->cmodels[p].mins)) << LGAP;
					o << utils::va("maxs" FMT_VEC3, ARG_VEC3(cm->cmodels[p].maxs)) << LGAP;
					o << utils::va("radius" FMT_VEC, cm->cmodels[p].radius) << ENDL;
					o << ">>> leaf :: " << LGAP;
					o << utils::va("mins" FMT_VEC3, ARG_VEC3(cm->cmodels[p].leaf.mins)) << LGAP;
					o << utils::va("maxs" FMT_VEC3, ARG_VEC3(cm->cmodels[p].leaf.maxs)) << LGAP;
					o << utils::va("brush contents [ %d ] ---- leaf brush node [ %d ] ---- cluster [ %d ]", 
						cm->cmodels[p].leaf.brushContents,
						cm->cmodels[p].leaf.leafBrushNode,
						cm->cmodels[p].leaf.cluster ) << ENDL;
				}
				
				o << ENDL;
				o << "---- brushes ---- count [ " << cm->numBrushes << " ] -------" << ENDL;

				for (auto p = 0u; p < cm->numBrushes; p++)
				{
					o << "brush [ " << p << " ] :: ";
					o << utils::va("mins" FMT_VEC3, ARG_VEC3(cm->brushes[p].mins)) << LGAP;
					o << utils::va("maxs" FMT_VEC3, ARG_VEC3(cm->brushes[p].maxs)) << LGAP;
					o << utils::va("brush contents [ %d ] ---- num sides [ %d ]", 
						cm->brushes[p].contents,
						cm->brushes[p].numsides) << ENDL;

					o << utils::va(">> first adj side offsets [ %d, %d, %d, %d, %d, %d ]",
							cm->brushes[p].firstAdjacentSideOffsets[0][0], cm->brushes[p].firstAdjacentSideOffsets[0][1], cm->brushes[p].firstAdjacentSideOffsets[0][2],
							cm->brushes[p].firstAdjacentSideOffsets[1][0], cm->brushes[p].firstAdjacentSideOffsets[1][1], cm->brushes[p].firstAdjacentSideOffsets[1][2]) << ENDL;

					o << utils::va(">> edge count [ %d, %d, %d, %d, %d, %d ]",
						cm->brushes[p].edgeCount[0][0], cm->brushes[p].edgeCount[0][1], cm->brushes[p].edgeCount[0][2],
						cm->brushes[p].edgeCount[1][0], cm->brushes[p].edgeCount[1][1], cm->brushes[p].edgeCount[1][2]) << ENDL;

					if(cm->brushes[p].numsides)
					{
						o << "using brushsides [ ";

						for (auto s = 0u; s < cm->brushes[p].numsides; s++)
						{
							const int brushside_num = ((DWORD)&cm->brushes[p].sides[s] - (DWORD)cm->brushsides) / sizeof(game::cbrushside_t);
							o << brushside_num << ", ";
						}

						o << " ]" << ENDL;
					}

					o << ENDL;
				}

				o << ENDL;
				o << "---- clusters ---- count [ " << cm->numClusters << " ] -------" << ENDL;

				o.close();

				break;
			}
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

		command::add("dump_clipmap", "", "dump clipmap into human readable format to iw3xo/clipmap", [this](command::params)
		{
			write_clipmap();
		});
	}

	_map::~_map()
	{
		free(mpsp_mapents_buffer);
	}
}