#include "std_include.hpp"

// Notes:
// * skinned meshes have unstable hashes

// * motion vectors are broken (flickering static meshes)
// ^ Game Setup -> Parameter Tuning -> Fused World-View Mode set to 'In View Transform' fixes motion vectors but rotates the distant light
// ^ same applies when distant light is placed via usd.mod

// * running the game with a debug build of remix will throw an assert
// * running the game with all culling disabled and quickly teleporting around (spawning counts as teleporting) can crash the runtime
// * dynamic meshes (dynEnts, destr. cars and some other static meshes) are not 'static' (debug view) and create a smear effect (motion vectors)

// * 'r_preTess' (surface batching) set to false + 'rtx_disable_world_culling' set to less (cull full portals only) = almost stable geo hashes

namespace components
{
	/**
	 * @brief - send camera matrices down the fixed-function pipeline so that the remix runtime finds the camera
	 *		  - update fixed-function debug lights (spawned via the dev-gui)
	 *		  - force certain dvar's 
	 */
	void setup_rtx()
	{
		const auto dev = game::glob::d3d9_device;
		const auto data = game::get_backenddata();

		// populate viewParms3D because R_Set3D needs it
		game::gfxCmdBufSourceState->viewParms3D = &data->viewInfo->viewParms;

		// update world matrix
		rtx::r_set_3d();

		// directly set matrices on the device so that rtx-remix finds the camera
		dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->matrices.matrix[0].m));
		dev->SetTransform(D3DTS_VIEW, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->viewParms.viewMatrix.m));
		dev->SetTransform(D3DTS_PROJECTION, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->viewParms.projectionMatrix.m));

		rtx_lights::spawn_light();
		rtx::force_dvars_on_frame();

		if (flags::has_flag("thirdperson"))
		{
			rtx::player_origin_model();
		}

		if (dvars::rtx_sky_follow_player && dvars::rtx_sky_follow_player->current.enabled)
		{
			utils::vector::copy(game::cgs->predictedPlayerState.origin, rtx_gui::skysphere_model_origin, 3);
			rtx_gui::skysphere_update_pos();
		}

		if (!flags::has_flag("no_fog"))
		{
			const float fog_start = 1.0f;
			dev->SetRenderState(D3DRS_FOGENABLE, TRUE);
			dev->SetRenderState(D3DRS_FOGCOLOR, rtx_map_settings::m_color.packed);
			dev->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
			dev->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&fog_start));
			dev->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&rtx_map_settings::m_max_distance));
		}
	}

	// stub at the beginning of 'RB_Draw3DInternal' (frame begin)
	__declspec(naked) void rb_standard_drawcommands_stub()
	{
		const static uint32_t retn_addr = 0x64B7B6;
		__asm
		{
			pushad;
			call	setup_rtx;
			popad;

			// og instructions
			mov     ebp, esp;
			and		esp, 0xFFFFFFF8;
			jmp		retn_addr;
		}
	}

	void post_scene_rendering_stub()
	{
		const auto dev = game::glob::d3d9_device;

		// disable fog before rendering UI (who wants foggy UI elements right?)
		// ^ can happen if no skysphere is rendered, which is rendered last and thus disables fog for everything afterwards
		dev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	}

	// stub at the beginning of 'RB_CallExecuteRenderCommands' (before UI)
	__declspec(naked) void rb_call_execute_render_commands_stub()
	{
		const static uint32_t retn_addr = 0x6155E6;
		__asm
		{
			pushad;
			call	post_scene_rendering_stub;
			popad;

			// og instructions
			mov     ebp, esp;
			and		esp, 0xFFFFFFF8;
			jmp		retn_addr;
		}
	}

	// rewritten R_Set3D function
	void rtx::r_set_3d()
	{
		const auto state = game::gfxCmdBufSourceState;

		if (state->viewMode != game::VIEW_MODE_3D)
		{
			state->viewMode = game::VIEW_MODE_3D;
			memcpy(&state->viewParms, state->viewParms3D, sizeof(state->viewParms));

			if (state->viewParms.origin[3] == 0.0f)
			{
				state->eyeOffset[0] = 0.0f;
				state->eyeOffset[1] = 0.0f;
				state->eyeOffset[2] = 0.0f;
			}
			else
			{
				state->eyeOffset[0] = state->viewParms.origin[0];
				state->eyeOffset[1] = state->viewParms.origin[1];
				state->eyeOffset[2] = state->viewParms.origin[2];
			}

			state->eyeOffset[3] = 1.0f;

			//
			// inlined R_CmdBufSet3D;

			++state->matrixVersions[0];
			++state->matrixVersions[1];
			++state->matrixVersions[2];
			++state->matrixVersions[4];
			++state->matrixVersions[3];
			++state->matrixVersions[5];
			++state->matrixVersions[7];
			state->constVersions[58] = state->matrixVersions[0];

			//memcpy(state, &identity_matrix, 0x40u);
			const auto mtx = state->matrices.matrix[0].m;
			mtx[0][0] = 1.0f;  mtx[0][1] = 0.0f; mtx[0][2] = 0.0f; mtx[0][3] = 0.0f;
			mtx[1][0] = 0.0f;  mtx[1][1] = 1.0f; mtx[1][2] = 0.0f; mtx[1][3] = 0.0f;
			mtx[2][0] = 0.0f;  mtx[2][1] = 0.0f; mtx[2][2] = 1.0f; mtx[2][3] = 0.0f;
			mtx[3][0] = 0.0f;  mtx[3][1] = 0.0f; mtx[3][2] = 0.0f; mtx[3][3] = 1.0f;

			mtx[3][0] = mtx[3][0] - state->eyeOffset[0];
			mtx[3][1] = mtx[3][1] - state->eyeOffset[1];
			mtx[3][2] = mtx[3][2] - state->eyeOffset[2];
		}
	}

	

	/**
	 * @brief spawns a little triangle at the origin of the player that is marked as 'player model body texture'
	 *		  - triangle then acts as the origin for the bounding box that culls meshes marked with the 'player model' category
	 *		  - not really working .. 
	 */
	void rtx::player_origin_model()
	{
		const auto index = game::G_ModelIndex("rtx_player_origin");

		if (  !axis_spawned
			|| axis_model == nullptr
			|| axis_model->classname == 0
			|| axis_model->model != index)
		{
			axis_model = game::G_Spawn();
			axis_model->classname = game::scr_const->script_model;
			axis_model->model = index;
			axis_model->s.index = index;
			axis_model->r.svFlags = 0x04;
			axis_model->r.linked = 0x1;

			// G_SetOrigin
			game::G_SetOrigin(axis_model, game::vec3_origin);
			game::G_SetAngles(axis_model, game::vec3_origin);
			game::G_CallSpawnEntity(axis_model);
			axis_spawned = true;
		}

		if (axis_spawned && axis_model && axis_model->model == index)
		{
			const game::vec3_t new_org = 
			{
				game::glob::lpmove_camera_origin.x,
				game::glob::lpmove_camera_origin.y,
				game::glob::lpmove_camera_origin.z - 10.0f,
			};

			game::G_SetOrigin(axis_model, new_org);
		}
	}


	void rtx::sky_material_update(std::string_view buffer, bool use_dvar)
	{
		if (use_dvar && dvars::rtx_sky_materials && *dvars::rtx_sky_materials->current.string)
		{
			sky_material_addons = utils::split(dvars::rtx_sky_materials->current.string, ' ');
		}
		else
		{
			sky_material_addons = utils::split(std::string(buffer), ' ');
		}
	}

	/**
	 * @brief function called by _renderer::R_SetMaterial,
	 *		  triggered each time the game sets up the material pass for the next object to be rendered
	 *		  - can be used to switch the material or the shaders used for rendering the next obj
	 */
	bool rtx::r_set_material_stub(game::switch_material_t* swm, const game::GfxCmdBufState* state)
	{
		if (dvars::rtx_hacks && dvars::rtx_hacks->current.enabled)
		{
			if (dvars::rtx_sky_hacks && dvars::rtx_sky_hacks->current.enabled)
			{
				if (sky_material_addons.empty())
				{
					sky_material_update("", true);
				}

				if (!sky_material_addons.empty())
				{
					for (const std::string_view s : sky_material_addons)
					{
						// remove 'wc/' - 'mc/' from materials
						if (s == std::string(swm->current_material->info.name).substr(3))
						{
							swm->technique_type = game::TECHNIQUE_UNLIT;
							_renderer::switch_material(swm, "rtx_sky");
							return false;
						}
					}
				}
			}

			if (state->material && state->material->info.sortKey == 5 || utils::starts_with(swm->current_material->info.name, "wc/sky_"))
			{
				swm->technique_type = game::TECHNIQUE_UNLIT;
				_renderer::switch_material(swm, "rtx_sky");
				return false;
			}
		}

		return true;
	}

	
	// *
	// general stubs
	
	/*
	0063AF3F call    R_AllocStaticModelLighting
	0063AF44 add     esp, 8
	0063AF47 test    al, al
	0063AF49 jnz     short loc_63AF62
	*/

	__declspec(naked) void alloc_smodel_lighting_stub()
	{
		const static uint32_t retn_addr = 0x63AF49;
		const static uint32_t draw_model_addr = 0x63AF62;
		__asm
		{
			pushad;
			push	eax;
			mov		eax, dvars::rtx_extend_smodel_drawing;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			jne		OG_LOGIC;
			popad;

			add     esp, 8 // og instruction overwritten by hook
			jmp		draw_model_addr;

		OG_LOGIC:
			popad;

			// og instructions
			add     esp, 8;
			test    al, al;
			jmp		retn_addr;
		}
	}
	
	// *
	// fix resolution issues by removing duplicates returned by EnumAdapterModes

	namespace resolution
	{
		auto hash = [](const _D3DDISPLAYMODE& d) { return d.Width + 10 * d.Height + d.RefreshRate; };
		auto equal = [](const _D3DDISPLAYMODE& d1, const _D3DDISPLAYMODE& d2) { return d1.Width == d2.Width && d1.Height == d2.Height && d1.RefreshRate == d2.RefreshRate; };
		std::unordered_set<_D3DDISPLAYMODE, decltype(hash), decltype(equal)> modes(256, hash, equal);

		int enum_adapter_modes_intercept(std::uint32_t adapter_index, std::uint32_t mode_index)
		{
			_D3DDISPLAYMODE current = {};
			const auto hr = game::dx->d3d9->EnumAdapterModes(adapter_index, D3DFMT_X8R8G8B8, mode_index, &current) < 0;
			modes.emplace(current);
			return hr;
		}

		__declspec(naked) void R_EnumDisplayModes_stub()
		{
			const static uint32_t retn_addr = 0x5F4192;
			__asm
			{
				push	esi; // mode index
				push	ebx; // adapter index
				call	enum_adapter_modes_intercept;
				add		esp, 8;
				jmp		retn_addr;
			}
		}

		void enum_adapter_modes_write_array()
		{
			std::uint32_t idx = 0;
			for (auto& m : modes)
			{
				if (idx >= 256)
				{
					game::Com_PrintMessage(0, "EnumAdapterModes : Failed to grab all possible resolutions. Array to small!\n", 0);
					break;
				}

				memcpy(&game::dx->displayModes[idx], &m, sizeof(_D3DDISPLAYMODE));
				idx++;
			}
		}

		__declspec(naked) void R_EnumDisplayModes_stub2()
		{
			const static uint32_t R_CompareDisplayModes_addr = 0x5F4110;
			const static uint32_t retn_addr = 0x5F41CE;
			__asm
			{
				pushad;
				call	enum_adapter_modes_write_array;
				popad;

				push	R_CompareDisplayModes_addr;
				jmp		retn_addr;
			}
		}
	}


	// *
	// LOD: altered forceLod logic to actually force a SPECIFIC LOD at ALL times

	int xmodel_get_lod_for_dist(const game::XModel* model, const float* base_dist)
	{
		const auto lod_count = model->numLods;
		const auto& r_forceLod = game::Dvar_FindVar("r_forceLod");

		// 4 = none - disabled
		if (r_forceLod->current.integer == r_forceLod->reset.integer)
		{
			for (auto lod_index = 0; lod_index < lod_count; ++lod_index)
			{
				const auto lod_dist = model->lodInfo[lod_index].dist;

				if (lod_dist == 0.0f || lod_dist > *base_dist)
				{
					return lod_index;
				}
			}
		}
		// r_forcelod enabled
		else
		{
			if (r_forceLod->current.integer > lod_count // force lowest possible LOD
				|| (dvars::r_forceLod_second_lowest->current.enabled && r_forceLod->current.integer >= lod_count)) // force second lowest possible LOD
			{
				return lod_count - 1 >= 0 ? lod_count - 1 : 0;
			}

			return r_forceLod->current.integer;
		}

		return -1;
	}

	int xmodel_get_lod_for_dist_global = 0;
	__declspec(naked) void xmodel_get_lod_for_dist_detour()
	{
		const static uint32_t retn_addr = 0x5911F0;
		__asm
		{
			lea		ecx, [esp + 4];
			pushad;
			push	ecx; // base_dist
			push	eax; // model
			call	xmodel_get_lod_for_dist;
			add		esp, 8;
			mov     xmodel_get_lod_for_dist_global, eax;
			popad;

			mov     eax, xmodel_get_lod_for_dist_global;
			jmp		retn_addr;
		}
	}

	// -----

	int forcelod_get_lod(const int lod_count)
	{
		const auto& r_forceLod = game::Dvar_FindVar("r_forceLod");

		if (r_forceLod->current.integer > lod_count // force lowest possible LOD
			|| (dvars::r_forceLod_second_lowest->current.enabled && r_forceLod->current.integer >= lod_count)) // force second lowest possible LOD
		{
			return lod_count - 1 >= 0 ? lod_count - 1 : 0;
		}

		return r_forceLod->current.integer;
	}

	int forcelod_is_enabled()
	{
		const auto& r_forceLod = game::Dvar_FindVar("r_forceLod");

		// 4 = none - disabled
		if (r_forceLod->current.integer == r_forceLod->reset.integer)
		{
			return 0;
		}

		return 1;
	}

	int xmodel_get_lod_for_dist_global_2 = 0;
	__declspec(naked) void xmodel_get_lod_for_dist_inlined()
	{
		const static uint32_t break_addr = 0x63AF27;
		const static uint32_t og_logic_addr = 0x63AF09;
		__asm
		{
			pushad;

			push	ecx;					// save ecx
			call	forcelod_is_enabled;
			cmp		eax, 1;
			pop		ecx;					// restore ecx
			jne		OG_LOGIC;				// if r_forceLod != 1

			push	ecx;					// holds model->numLods
			call	forcelod_get_lod;
			add		esp, 4;
			mov		xmodel_get_lod_for_dist_global_2, eax;
			popad;

			mov		esi, xmodel_get_lod_for_dist_global_2; // move returned lodindex into the register the game expects it to be
			jmp		break_addr;


			OG_LOGIC:
			popad;
			fld     dword ptr [edx];
			fcomp   st(1);
			fnstsw  ax;
			jmp		og_logic_addr;
		}
	}

	

	// *
	// Culling

	namespace cull
	{
		// R_AddWorldSurfacesPortalWalk
		__declspec(naked) void world_stub_01()
		{
			const static uint32_t retn_skip = 0x60B095;
			const static uint32_t retn_stock = 0x60B02E;
			__asm
			{
				// jump if culling is less or disabled
				cmp		rtx::loc_disable_world_culling, 1;
				jge		SKIP;

				// og code
				cmp     esi, ebp;
				mov		[esp + 0x10], eax;
				jmp		retn_stock;

			SKIP:
				mov		[esp + 0x10], eax;
				jmp		retn_skip;
			}
		}

		int _skipped_cull = 0; // helper var
		__declspec(naked) void world_stub_02_reset_helper()
		{
			const static uint32_t retn_stock = 0x643F6A;
			__asm
			{
				mov		_skipped_cull, 0;
				mov		[ecx + 0xC], edi;
				mov		[ecx + 8], edx;
				jmp		retn_stock;
			}
		}

		__declspec(naked) void world_stub_02_skip_static_model()
		{
			const static uint32_t retn_stock = 0x643B70;
			const static uint32_t retn_stock_jumped = 0x643B8F;
			__asm
			{
				// do we want to cull static models the way geo would be culled?
				cmp		rtx::loc_disable_world_culling, 3;
				jl		STOCK;

				// did we skip the culling check in 'r_cull_world_stub_02'?
				cmp		_skipped_cull, 1;
				je		SKIP;

			STOCK:		// og code
				jz		loc_643B8F;
				mov     esi, [edx + 0x24];
				jmp		retn_stock;

			loc_643B8F: // og code
				jmp		retn_stock_jumped;


			SKIP:		// skip static model rendering
				mov     esi, [edx + 0x24];
				jmp		retn_stock_jumped;
			}
		}

		// R_AddAabbTreeSurfacesInFrustum_r
		__declspec(naked) void world_stub_02()
		{
			const static uint32_t retn_skip = 0x643B0E;
			const static uint32_t retn_stock = 0x643B08;
			__asm
			{
				// jump if culling is disabled
				cmp		rtx::loc_disable_world_culling, 2;
				jge		SKIP;
				
				// og code
				fnstsw  ax;
				test    ah, 0x41;
				jmp		retn_stock;

			SKIP: // jumped here because culling is disabled 
				fnstsw  ax;
				test    ah, 0x41;
				jnp		HACKED_CULLING;
				jmp		retn_skip;

			HACKED_CULLING: // jumped here because the game would have culled this object
				mov		_skipped_cull, 1;
				jmp		retn_skip;
			}
		}

		// R_AddAabbTreeSurfacesInFrustum_r
		__declspec(naked) void world_stub_03()
		{
			const static uint32_t retn_skip = 0x643B48;
			const static uint32_t retn_stock = 0x643B39;
			__asm
			{
				// jump if culling is less or disabled
				cmp		rtx::loc_disable_world_culling, 1;
				jge		SKIP;
				
				// og code
				fnstsw  ax;
				test    ah, 1;
				jmp		retn_stock;

			SKIP:
				fnstsw  ax;
				jmp		retn_skip;
			}
		}

		// R_AddCellSceneEntSurfacesInFrustumCmd
		__declspec(naked) void entities_stub()
		{
			const static uint32_t retn_skip = 0x64D17C;
			const static uint32_t retn_stock = 0x64D17A;
			__asm
			{
				//pushad;
				cmp		rtx::loc_disable_entity_culling, 1;
				je		SKIP;
				//popad;

				// stock op's
				and		[esp + 0x18], edx;
				cmp     byte ptr[esi + eax], 0;
				jmp		retn_stock;

			SKIP:
				//popad;
				and		[esp + 0x18], edx;
				jmp		retn_skip;
			}
		}
	}


	// *
	// dvars

	void rtx::force_dvars_on_frame()
	{
		// update culling vars at the end of a frame (so we don't change culling behaviour mid-frame -> not safe)
		{
			// update world culling
			if (dvars::rtx_disable_world_culling)
			{
				loc_disable_world_culling = dvars::rtx_disable_world_culling->current.integer;
				loc_disable_world_culling = loc_disable_world_culling < 0 ? 0 :
					loc_disable_world_culling > 3 ? 3 : loc_disable_world_culling;
			}

			// update entity culling
			if (dvars::rtx_disable_entity_culling)
			{
				loc_disable_entity_culling = dvars::rtx_disable_entity_culling->current.enabled ? 1 : 0;
			}
		}

		// #
		// #

		dvars::bool_override("r_smp_backend", false); // reduces lag-spikes and generally increases fps, wobbly / laggy animated meshes
		dvars::bool_override("r_skinCache", false); // ^ fix wobbly viewmodels
		dvars::bool_override("r_smc_enable", false); // disable static model caching (stable hashes)
		dvars::bool_override("r_depthPrepass", false); // remix does not like this
		dvars::bool_override("r_zfeather", false);
		dvars::bool_override("r_dof_enable", false);
	}

	//_common::force_dvars_on_init()
	void rtx::set_dvars_defaults_on_init()
	{
		rtx::force_dvars_on_frame();
		dvars::bool_override("r_multiGpu", true); // fps ++
	}

	// rtx::on_map_load()
	void rtx::set_dvars_defaults_on_mapload()
	{
		if (!flags::has_flag("no_forced_lod"))
		{
			dvars::int_override("r_forceLod", 0);
		}

		//dvars::bool_override("fx_drawClouds", false);
		dvars::bool_override("r_pretess", true); // needed for fixed-function
		dvars::float_override("r_znear", 4.00195f);
		dvars::float_override("r_znear_depthhack", 4.0f);
	}

	void register_rtx_dvars()
	{
		// 
		// re-register LOD related dvars to extend limits

		const auto r_lodScaleRigid = (DWORD*)(0xD569790);
		*r_lodScaleRigid = (DWORD)game::Dvar_RegisterFloat(
			/* name		*/ "r_lodScaleRigid",
			/* desc		*/ "Scale the level of detail distance for rigid models (larger reduces detail)",
			/* default	*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ FLT_MAX,
			/* flags	*/ game::dvar_flags::saved);

		const auto r_lodBiasRigid = (DWORD*)(0xD56972C);
		*r_lodBiasRigid = (DWORD)game::Dvar_RegisterFloat(
			/* name		*/ "r_lodBiasRigid",
			/* desc		*/ "Bias the level of detail distance for rigid models (negative increases detail)",
			/* default	*/ 0.0f,
			/* minVal	*/ -FLT_MAX,
			/* maxVal	*/ FLT_MAX,
			/* flags	*/ game::dvar_flags::saved);

		const auto r_lodScaleSkinned = (DWORD*)(0xD569724);
		*r_lodScaleSkinned = (DWORD)game::Dvar_RegisterFloat(
			/* name		*/ "r_lodScaleSkinned",
			/* desc		*/ "Scale the level of detail distance for skinned models (larger reduces detail)",
			/* default	*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ FLT_MAX,
			/* flags	*/ game::dvar_flags::saved);

		const auto r_lodBiasSkinned = (DWORD*)(0xD569638);
		*r_lodBiasSkinned = (DWORD)game::Dvar_RegisterFloat(
			/* name		*/ "r_lodBiasSkinned",
			/* desc		*/ "Bias the level of detail distance for skinned models (negative increases detail)",
			/* default	*/ 0.0f,
			/* minVal	*/ -FLT_MAX,
			/* maxVal	*/ FLT_MAX,
			/* flags	*/ game::dvar_flags::saved);

		dvars::rtx_sky_materials = game::Dvar_RegisterString(
			/* name		*/ "rtx_sky_materials",
			/* desc		*/ "materials that get replaced by a placeholder sky texture (rtx_hacks)\nuseful for custom maps with custom skies",
			/* default	*/ "",
			/* flags	*/ game::saved);
	}

	__declspec(naked) void register_dvars_stub()
	{
		const static uint32_t stock_func = 0x62FE50;
		const static uint32_t retn_addr = 0x5F4EFF;
		__asm
		{
			pushad;
			call	register_rtx_dvars;
			popad;

			call	stock_func;
			jmp		retn_addr;
		}
	}

	int skip_image_load(game::GfxImage* img)
	{
		// 0x2 = color, 0x5 = normal, 0x8 = spec
		if (img->semantic == 0x5 || img->semantic == 0x8)
		{
			return 1;
		}

		return 0;
	}

	__declspec(naked) void load_image_stub()
	{
		const static uint32_t skip_img_addr = 0x6424F3;
		const static uint32_t og_logic_addr = 0x6424EB;
		__asm
		{
			pushad;
			push	ebx;					// img
			call	skip_image_load;
			pop		ebx;
			cmp		eax, 1;
			jne		OG_LOGIC;

			popad;
			//xor		eax, eax;
			//mov     eax, 1;
			jmp		skip_img_addr;

			// og code
		OG_LOGIC:
			popad;
			push    ebx;
			mov     edx, edi;
			lea     eax, [esp + 0x10];
			jmp		og_logic_addr;
		}
	}

	// ----------------------------------------------------

	// *
	// Event stubs

	// > _map::init_fixed_function_buffers_stub
	void rtx::on_map_load()
	{
		if (!game::clc.demoplaying)
		{
			rtx_map_settings::get()->set_settings_for_loaded_map();
		}
		
		rtx::set_dvars_defaults_on_mapload();
	}

	// > fixed_function::free_fixed_function_buffers_stub
	void rtx::on_map_shutdown()
	{
	}

	rtx::rtx()
	{
		// *
		// general

		// force fullbright (switching to fullbright in-game will result in wrong surface normals)
		utils::hook::nop(0x633259, 2);

		// hook beginning of 'RB_Draw3DInternal' to setup general stuff required for rtx-remix
		utils::hook(0x64B7B1, rb_standard_drawcommands_stub, HOOK_JUMP).install()->quick();

		// hook beginning of 'RB_CallExecuteRenderCommands' (before UI)
		utils::hook(0x6155E1, rb_call_execute_render_commands_stub, HOOK_JUMP).install()->quick();

		if (flags::has_flag("thirdperson"))
		{
			// render third person model in first person
			utils::hook::nop(0x42E187, 6); // CG_DrawFriendlyNames: do not disable name drawing
			utils::hook::nop(0x42E2BA, 6); // CG_DrawCrosshairNames: ^
			utils::hook::nop(0x4311AE, 6); // CG_DrawCrosshair: do not disable crosshair
			utils::hook::nop(0x443007, 2); // CG_UpdateCursorHints: do not disable cursor hints
			utils::hook::nop(0x45083C, 2); // CG_CalcTurretViewValues: calculate first person viewangles
			utils::hook::nop(0x4508C1, 6); // CG_ApplyViewAnimation: first person anims
			utils::hook::nop(0x451C70, 2); // CG_CalcViewValues: always call CG_OffsetFirstPersonView
			utils::hook::set<BYTE>(0x451C9E, 0xEB); // CG_CalcViewValues: do not call CG_OffsetThirdPersonView
			utils::hook::set<BYTE>(0x456D36, 0xEB); // CG_AddPlayerWeapon: do not disable bolt effects
			utils::hook::nop(0x457054, 6); // CG_AddViewWeapon: do not disable viewmodel
			utils::hook::nop(0x451D8E, 2); // CG_UpdateThirdPerson: always enable "thirdperson"
		}

		// un-cheat + saved flag for fx_enable
		utils::hook::set<BYTE>(0x4993EC + 1, 0x01); // was 0x80
		// un-cheat + saved flag for r_forceLod
		utils::hook::set<BYTE>(0x62BB41 + 1, 0x01); // was 0x80

		// mp_crash msg "too many static models ..." @ 0x63AF4D (disabled culling: the engine cant handle modellighting for so many static models, thus not drawing them)
		utils::hook::nop(0x63AF44, 5); utils::hook(0x63AF44, alloc_smodel_lighting_stub, HOOK_JUMP).install()->quick();

		// R_AddWorkerCmd :: disable dynEnt models (not needed)
		//utils::hook::nop(0x629328, 5);

		// disable 'RB_DrawSun' call
		utils::hook::nop(0x649BD8, 5);

		// disable loading of specular and normalmaps (de-clutter remix ui)
		if (!flags::has_flag("load_normal_spec"))
		{
			utils::hook::nop(0x616E65, 5);
			utils::hook::nop(0x616F42, 5);
			utils::hook::nop(0x6424E4, 7); utils::hook(0x6424E4, load_image_stub, HOOK_JUMP).install()->quick();
		}

		// *
		// allow 4k images (skysphere)

		// set img alloc size from 0x600000 to 0x1000000
		utils::hook::set<BYTE>(0x641CC7 + 3, 0x00);
		utils::hook::set<BYTE>(0x641CC7 + 4, 0x01);

		// ^ cmp -> error if image larger
		utils::hook::set<BYTE>(0x641C89 + 3, 0x00);
		utils::hook::set<BYTE>(0x641C89 + 4, 0x01);

		// dxvk's 'EnumAdapterModes' returns a lot of duplicates and the games array only has a capacity of 256 which is not enough depending on max res. and refreshrate
		// fix resolution issues by removing duplicates returned by EnumAdapterModes - then write the array ourselfs
		utils::hook(0x5F4182, resolution::R_EnumDisplayModes_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x5F41C9, resolution::R_EnumDisplayModes_stub2, HOOK_JUMP).install()->quick();
		utils::hook::set<BYTE>(0x5F4170 + 2, 0x04); // set max array size check to 1024 (check within loop)


		// *
		// culling

		{
			// R_AddWorldSurfacesPortalWalk :: less culling .. // 0x60B02E -> jl to jmp // 0x7C -> 0xEB //utils::hook::set<BYTE>(0x60B02E, 0xEB);
			utils::hook::nop(0x60B028, 6); utils::hook(0x60B028, cull::world_stub_01, HOOK_JUMP).install()->quick();

			{
				// note:
				// using 'rtx_disable_world_culling' = 'less' results in unstable world geometry hashes (but stable static model hashes)
				// -> using 'all-but-models' leaves culling logic for static models at 'less' but does not cull world geometry

				// R_AddCellStaticSurfacesInFrustumCmd :: stub used to reset the skip model check from the stub below
				utils::hook::nop(0x643F64, 6); utils::hook(0x643F64, cull::world_stub_02_reset_helper, HOOK_JUMP).install()->quick();
				// R_AddAabbTreeSurfacesInFrustum_r :: check if culling mode 'all-but-models' is active - check note above
				utils::hook(0x643B6B, cull::world_stub_02_skip_static_model, HOOK_JUMP).install()->quick();

				// R_AddAabbTreeSurfacesInFrustum_r :: disable all surface culling (bad fps) .. // 0x643B08 -> nop //utils::hook::nop(0x643B08, 6);
				utils::hook(0x643B03, cull::world_stub_02, HOOK_JUMP).install()->quick();
			}

			// 0x643B39 -> jmp ^ // 0x74 -> 0xEB //utils::hook::set<BYTE>(0x643B39, 0xEB);
			utils::hook(0x643B34, cull::world_stub_03, HOOK_JUMP).install()->quick();

			// R_AddCellSceneEntSurfacesInFrustumCmd :: active ents like destructible cars / players (disable all culling) .. // 0x64D17A -> nop // 2 bytes //utils::hook::nop(0x64D17A, 2);
			utils::hook::nop(0x64D172, 8); utils::hook(0x64D172, cull::entities_stub, HOOK_JUMP).install()->quick();

			dvars::rtx_disable_world_culling = game::Dvar_RegisterEnum(
				/* name		*/ "rtx_disable_world_culling",
				/* desc		*/ "Disable world culling. 'all' is needed for stable geometry hashes!\n"
										 "- less: reduces culling to portals only (unstable world geo hashes!)\n"
										 "- all: disable culling of all surfaces including models\n"
										 "- all-but-models: disable culling of all surfaces excluding models",
				/* default	*/ 1,
				/* enumSize	*/ rtx::rtx_disable_world_culling_enum.size(),
				/* enumData */ rtx::rtx_disable_world_culling_enum.data(),
				/* flags	*/ game::dvar_flags::saved);

			dvars::rtx_disable_entity_culling = game::Dvar_RegisterBool(
				/* name		*/ "rtx_disable_entity_culling",
				/* desc		*/ "Disable culling of game entities (script objects/destructible cars ...)",
				/* default	*/ true,
				/* flags	*/ game::dvar_flags::saved);
		}

		// *
		// LOD

		// skip original lod dvar registration
		utils::hook::nop(0x62A2A1, 5); // r_lodScaleRigid
		utils::hook::nop(0x62A2F8, 5); // r_lodBiasRigid
		utils::hook::nop(0x62A34F, 5); // r_lodScaleSkinned
		utils::hook::nop(0x62A3A6, 5); // r_lodBiasSkinned

		utils::hook::nop(0x5FAE5A, 4); // do not scale lodScaleRigid with fov
		utils::hook::nop(0x5FAE67, 2); // do not scale lodBiasRigid with fov
		

		// check if r_forceLod is enabled and force LOD's accordingly
		utils::hook(0x5911C0, xmodel_get_lod_for_dist_detour, HOOK_JUMP).install()->quick();

		// ^ but inlined ..... for all other static models (R_AddAllStaticModelSurfacesCamera)
		utils::hook::nop(0x63AF03, 6);  utils::hook(0x63AF03, xmodel_get_lod_for_dist_inlined, HOOK_JUMP).install()->quick();


		// *
		// dvars

		// stub after 'R_RegisterDvars' to re-register stock dvars
		utils::hook(0x5F4EFA, register_dvars_stub, HOOK_JUMP).install()->quick();

		dvars::rtx_hacks = game::Dvar_RegisterBool(
			/* name		*/ "rtx_hacks",
			/* desc		*/ "Enables various hacks and tweaks to make nvidia rtx work",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved);

		dvars::rtx_sky_hacks = game::Dvar_RegisterBool(
			/* name		*/ "rtx_sky_hacks",
			/* desc		*/ "Check and replace sky materials defined with dvar 'rtx_sky_materials'",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);

		dvars::rtx_sky_follow_player = game::Dvar_RegisterBool(
			/* name		*/ "rtx_sky_follow_player",
			/* desc		*/ "Sky will follow the player (can help with culling issues)",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved);

		dvars::rtx_extend_smodel_drawing = game::Dvar_RegisterBool(
			/* name		*/ "rtx_extend_smodel_drawing",
			/* desc		*/ "IW3 has a limit on how many static models it can drawn at the same time. Forcing lods and drawdistances can exceed that limit pretty fast.\n"
					       "Enabling this will force the game to still render them (at the cost of wrong lighting - rasterized only tho)",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved);

		dvars::r_forceLod_second_lowest = game::Dvar_RegisterBool(
			/* name		*/ "r_forceLod_second_lowest",
			/* desc		*/ "Force LOD of static models to the second lowest LOD (should keep grass, rocks, trees ... visible)\nSet r_forceLod to anything but none for this to work",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved);

#if DEBUG
		command::add("dump_modes", "", "", [this](command::params)
		{
			for (auto m = 0u; m < game::dx->displayModeCount; m++)
			{
				game::Com_PrintMessage(0, utils::va("[%d] %dx%d @ %dHz - D3DFORMAT: %d \n", m, game::dx->displayModes[m].Width, game::dx->displayModes[m].Height, game::dx->displayModes[m].RefreshRate, game::dx->displayModes[m].Format), 0);
			}
		});
#endif
	}
}
