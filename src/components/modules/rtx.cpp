#include "std_include.hpp"

// # notes

// r_z_near_depthhack >> 4
// r_znear >> 4 pushes viewmodel away the further from 0 0 0 we are - 4.002 fixes this ??
// r_smp_backend >> 1 fixes viewmodel bumping
// r_zfeather >> 0 fixes remix freaking out turning half of all textures white + makes fx work to some extend (note: 'R_SkinStaticModelsCamera' @ 0x63B024 or 0x0063AB00)

// FIXED ISSUE 1 >> mp_crash msg "too many static models ..." @ 0x63AF4D (disabled culling: the engine cant handle modellighting for so many static models, thus not drawing them)
// IMPLEMENTED 2 >> removing 'R_AddAllBspDrawSurfacesRangeCamera' (decals) call @ 0x5F9E65 fixes cold/warm light transition on mp_crash? - still present - LOD related so most likely a specific object/shader that is causing that
// FIXED ISSUE 3 >> moving infront of the construction spotlight on mp_crash (bottom hardware) with cg_drawgun enabled will completly turn off remix rendering (caused

// ISSUE 4 >> viewmodel hands and animated parts on gun have unstable hashes
// ISSUE 5 >> "dynamic" entities like explodable cars can have unstable hashes

// FPS >> disable stock rendering functions that are not useful for remix

// IDEA >> Rewrite 'R_SetupPass' and 'R_SetPassShaderStableArguments' and implement fixed-function
// ^ BSP rendering = 'R_TessTrianglesPreTessList' -> 'R_DrawBspDrawSurfsLitPreTess'
// ^ Texture set in 'R_SetPassShaderObjectArguments'
namespace components
{
	bool rtx::r_set_material_stub(game::switch_material_t* swm)
	{
		if (dvars::rtx_hacks->current.enabled)
		{
			if (utils::starts_with(swm->current_material->info.name, "wc/sky_"))
			{
				swm->technique_type = game::TECHNIQUE_UNLIT;
				_renderer::switch_material(swm, "rtx_sky"); //"rgb");
				return false;
			}
		}

		// 
		/*if (utils::starts_with(swm->current_material->techniqueSet->name, "mc_effect_falloff"))
		{
			swm->technique_type = game::TECHNIQUE_WIREFRAME_SOLID;
			swm->technique = swm->material->techniqueSet->remappedTechniqueSet->techniques[28];
			//_renderer::switch_technique(swm, "rgb");
			return false;
		}*/

		// mc/mtl_floodlight_on
		// wc/utility_light_godray
		// wc/utility_light_hotspot
		// wc/hdrportal_lighten
		// mc/gfx_floodlight_tightglow
		// mc/gfx_floodlight_beam_godray75

		return true;
	}

	// rewritten R_Set3D function
	void r_set_3d()
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

	void spawn_light()
	{
		D3DLIGHT9 light;
		ZeroMemory(&light, sizeof(D3DLIGHT9));

		light.Type = D3DLIGHT_POINT;

		for (auto i = 0; i < 8; i++)
		{
			if (gui_devgui::rtx_spawn_light[i])
			{
				light.Diffuse.r = gui_devgui::rtx_debug_light_color[i][0];
				light.Diffuse.g = gui_devgui::rtx_debug_light_color[i][1];
				light.Diffuse.b = gui_devgui::rtx_debug_light_color[i][2];

				light.Position.x = gui_devgui::rtx_debug_light_origin[i][0];
				light.Position.y = gui_devgui::rtx_debug_light_origin[i][1];
				light.Position.z = gui_devgui::rtx_debug_light_origin[i][2];

				light.Range = gui_devgui::rtx_debug_light_range[i];

				light.Attenuation0 = 0.0f;    // no constant inverse attenuation
				light.Attenuation1 = 0.125f;    // only .125 inverse attenuation
				light.Attenuation2 = 0.0f;    // no square inverse attenuation

				game::glob::d3d9_device->SetLight(i, &light);
				game::glob::d3d9_device->LightEnable(i, TRUE);
			}
			else
			{
				// only updates if raytracing is turned off and on again
				game::glob::d3d9_device->LightEnable(i, FALSE);
			}
		}

		game::glob::d3d9_device->SetRenderState(D3DRS_LIGHTING, TRUE);
	}

	void setup_dvars_rtx()
	{
		// show viewmodel
		if (const auto var = game::Dvar_FindVar("r_znear_depthhack"); 
			var&& var->current.value != 4.0f)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, "r_znear_depthhack 4\n");
		}

		// r_znear around 4 pushes viewmodel away the further from 0 0 0 we are - 4.002 fixes that
		if (const auto var = game::Dvar_FindVar("r_znear");
			var && var->current.value != 4.002f)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, "r_znear 4.002\n");
		}

		// fix viewmodel bumping
		if (const auto var = game::Dvar_FindVar("r_smp_backend"); 
			var && !var->current.enabled)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, "r_smp_backend 1\n");
		}

		// fix flickering static meshes
		if (const auto var = game::Dvar_FindVar("r_smc_enable");
			var && !var->current.enabled)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, "r_smc_enable 1\n");
		}

		// remix does not like this
		if (const auto var = game::Dvar_FindVar("r_depthPrepass");
			var && !var->current.enabled)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, "r_depthPrepass 0\n");
		}

		// ++ fps
		if (const auto var = game::Dvar_FindVar("r_multiGpu");
			var && !var->current.enabled)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, "r_multiGpu 0\n");
		}

		// fix effects or other zfeathered materials to cause remix to freak out (turning everything white)
		if (const auto var = game::Dvar_FindVar("r_zfeather");
			var && var->current.enabled)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, "r_zfeather 0\n");
		}

		// disable weapon tracers
		if (const auto var = game::Dvar_FindVar("cg_tracerlength");
			var && var->current.value != 0.0f)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, "cg_tracerlength 0\n");
		}

		// disable fx once but dont force it off the whole time
		if (static bool disable_fx_once = false; !disable_fx_once)
		{
			if (const auto var = game::Dvar_FindVar("fx_enable");
				var && var->current.enabled)
			{
				game::Cmd_ExecuteSingleCommand(0, 0, "fx_enable 0\n");
			}

			disable_fx_once = true;
		}
	}

	void setup_rtx()
	{
		const auto dev = game::glob::d3d9_device;
		const auto data = game::get_backenddata();

		// populate viewParms3D because R_Set3D needs it
		game::gfxCmdBufSourceState->viewParms3D = &data->viewInfo->viewParms;

		// update world matrix
		r_set_3d();

		// directly set matrices on the device so that rtx-remix finds the camera
		dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->matrices.matrix[0].m));
		dev->SetTransform(D3DTS_VIEW, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->viewParms.viewMatrix.m));
		dev->SetTransform(D3DTS_PROJECTION, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->viewParms.projectionMatrix.m));

		spawn_light();
		setup_dvars_rtx();
	}

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

	void register_rtx_dvars()
	{
		// 
		// register LOD related dvars

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

		//dvars::r_cullWorld = Game::Dvar_RegisterBoolWrapper(
		//	/* name		*/ "r_cullWorld",
		//	/* desc		*/ "Culls invisible world surfaces. Disabling this can be useful for vertex manipulating shaders.",
		//	/* default	*/ true,
		//	/* flags	*/ Game::dvar_flags::latched);

		//dvars::r_cullEntities = Game::Dvar_RegisterBoolWrapper(
		//	/* name		*/ "r_cullEntities",
		//	/* desc		*/ "Culls invisible entities. Disabling this can be useful for vertex manipulating shaders.",
		//	/* default	*/ true,
		//	/* flags	*/ Game::dvar_flags::latched);

		//dvars::r_drawDynents = Game::Dvar_RegisterBoolWrapper(
		//	/* name		*/ "r_drawDynents",
		//	/* desc		*/ "Draw dynamic entities.",
		//	/* default	*/ true,
		//	/* flags	*/ Game::dvar_flags::none);
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

	// -----
	// altered forceLod logic to actually force a SPECIFIC LOD at ALL times

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

	// ----------------------------------------------------

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
			cmp		byte ptr [eax + 12], 1;
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

	// ----------------------------------------------------

#if 1
	// R_AddWorldSurfacesPortalWalk
	__declspec(naked) void r_cull_world_stub_01()
	{
		const static uint32_t retn_skip = 0x60B095;
		const static uint32_t retn_stock = 0x60B02E;
		__asm
		{
			// stock op's
			cmp     esi, ebp;
			mov		[esp + 0x10], eax;

			pushad;
			push	eax;
			mov		eax, 0; //dvars::r_cullWorld;
			//cmp		byte ptr[eax + 12], 1;
			cmp		eax, 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;

			popad;
			jmp		retn_stock;

		SKIP:
			popad;
			jmp		retn_skip;
		}
	}

	// R_AddAabbTreeSurfacesInFrustum_r
	__declspec(naked) void r_cull_world_stub_02()
	{
		const static uint32_t retn_skip = 0x643B0E;
		const static uint32_t retn_stock = 0x643B08;
		__asm
		{
			// stock op's
			fnstsw  ax;
			test    ah, 0x41;

			pushad;
			push	eax;
			mov		eax, 0; //dvars::r_cullWorld;
			//cmp		byte ptr[eax + 12], 1;
			cmp		eax, 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;

			popad;
			jmp		retn_stock;

		SKIP:
			popad;
			jmp		retn_skip;
		}
	}

	// R_AddAabbTreeSurfacesInFrustum_r
	__declspec(naked) void r_cull_world_stub_03()
	{
		const static uint32_t retn_skip = 0x643B48;
		const static uint32_t retn_stock = 0x643B39;
		__asm
		{
			// stock op's
			fnstsw  ax;
			test    ah, 1;

			pushad;
			push	eax;
			mov		eax, 0; //dvars::r_cullWorld;
			//cmp		byte ptr[eax + 12], 1;
			cmp		eax, 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;

			popad;
			jmp		retn_stock;

		SKIP:
			popad;
			jmp		retn_skip;
		}
	}

	// R_AddCellSceneEntSurfacesInFrustumCmd
	__declspec(naked) void r_cull_entities_stub()
	{
		const static uint32_t retn_skip = 0x64D17C;
		const static uint32_t retn_stock = 0x64D17A;
		__asm
		{
			// stock op's
			and		[esp + 0x18], edx;
			cmp     byte ptr[esi + eax], 0;

			pushad;
			push	eax;
			mov		eax, 0; //dvars::r_cullEntities;
			//cmp		byte ptr[eax + 12], 1;
			cmp		eax, 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;

			popad;
			jmp		retn_stock;

		SKIP:
			popad;
			jmp		retn_skip;
		}
	}
#endif

	// ----------------------------------------------------

	rtx::rtx()
	{
		// set debug light defaults
		for (auto i = 0u; i < 8; i++)
		{
			gui_devgui::rtx_debug_light_color[i][0] = 1.0f;
			gui_devgui::rtx_debug_light_color[i][1] = 1.0f;
			gui_devgui::rtx_debug_light_color[i][2] = 1.0f;
		}

		// hook beginning of 'RB_Draw3DInternal' to setup general stuff required for rtx-remix
		utils::hook(0x64B7B1, rb_standard_drawcommands_stub, HOOK_JUMP).install()->quick();

		// ----------------------------------------------------

		if (flags::has_flag("disable_culling"))
		{
			// R_AddWorldSurfacesPortalWalk :: less culling
			// 0x60B02E -> jl to jmp // 0x7C -> 0xEB //utils::hook::set<BYTE>(0x60B02E, 0xEB);
			utils::hook::nop(0x60B028, 6); utils::hook(0x60B028, r_cull_world_stub_01, HOOK_JUMP).install()->quick(); // crashes on release

			// R_AddAabbTreeSurfacesInFrustum_r :: disable all surface culling (bad fps)
			// 0x643B08 -> nop //utils::hook::nop(0x643B08, 6);
			utils::hook(0x643B03, r_cull_world_stub_02, HOOK_JUMP).install()->quick();

			// 0x643B39 -> jmp ^ // 0x74 -> 0xEB //utils::hook::set<BYTE>(0x643B39, 0xEB);
			utils::hook(0x643B34, r_cull_world_stub_03, HOOK_JUMP).install()->quick();

			// R_AddCellSceneEntSurfacesInFrustumCmd :: active ents like destructible cars / players (disable all culling)
			// 0x64D17A -> nop // 2 bytes //utils::hook::nop(0x64D17A, 2);
			utils::hook::nop(0x64D172, 8); utils::hook(0x64D172, r_cull_entities_stub, HOOK_JUMP).install()->quick();
		}

		// R_AddWorkerCmd :: disable dynEnt models
		utils::hook::nop(0x629328, 5);

		// removing 'R_AddAllBspDrawSurfacesRangeCamera' (decals) call @ 0x5F9E65 fixes cold/warm light transition on mp_crash?
		// ^ also "fixes" black cars - same behaviour as enabling r_fullbright (also disables "hdr" tool-texture drawing)
		// NOTE: this might be needed for decals to work
		// TODO: dvar?
		utils::hook::nop(0x5F9E65, 5); 

		// mp_crash msg "too many static models ..." @ 0x63AF4D (disabled culling: the engine cant handle modellighting for so many static models, thus not drawing them)
		utils::hook::nop(0x63AF44, 5); utils::hook(0x63AF44, alloc_smodel_lighting_stub, HOOK_JUMP).install()->quick();

		// un-cheat + saved flag for fx_enable
		utils::hook::set<BYTE>(0x4993EC + 1, 0x01); // was 0x80

		//
		// LOD

		// skip original lod dvar registration
		utils::hook::nop(0x62A2A1, 5); // r_lodScaleRigid
		utils::hook::nop(0x62A2F8, 5); // r_lodBiasRigid
		utils::hook::nop(0x62A34F, 5); // r_lodScaleSkinned
		utils::hook::nop(0x62A3A6, 5); // r_lodBiasSkinned

		// check if r_forceLod is enabled and force LOD's accordingly (only scene entities, dynamic entities and static models affected by sun shadows?)
		utils::hook(0x5911C0, xmodel_get_lod_for_dist_detour, HOOK_JUMP).install()->quick();

		// ^ but inlined ..... for all other static models (R_AddAllStaticModelSurfacesCamera)
		utils::hook::nop(0x63AF03, 6);  utils::hook(0x63AF03, xmodel_get_lod_for_dist_inlined, HOOK_JUMP).install()->quick();

		// stub after 'R_RegisterDvars' to re-register stock dvars
		utils::hook(0x5F4EFA, register_dvars_stub, HOOK_JUMP).install()->quick();

		dvars::rtx_hacks = game::Dvar_RegisterBool(
			/* name		*/ "rtx_hacks",
			/* desc		*/ "Enables various hacks and tweaks to make nvidia rtx work",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);

		dvars::rtx_extend_smodel_drawing = game::Dvar_RegisterBool(
			/* name		*/ "rtx_extend_smodel_drawing",
			/* desc		*/ "IW3 has a limit on how many static models it can drawn at the same time. Forcing lods and drawdistances can exceed that limit pretty fast.\n"
					       "Enabling this will force the game to still render them (at the cost of wrong lighting - rasterized only tho)",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);

		dvars::r_forceLod_second_lowest = game::Dvar_RegisterBool(
			/* name		*/ "r_forceLod_second_lowest",
			/* desc		*/ "Force LOD of static models to the second lowest LOD (should keep grass, rocks, trees ... visible)\nSet r_forceLod to anything but none for this to work",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);


		// doesnt help
		//if (flags::has_flag("disable_unused_rendering"))
		//{
			// DynEntPieces_AddDrawSurfs - nop call @ 0x5F9F89
			// R_DrawAllDynEnt - nop call @ 5F9FA8
			// R_AddAllBspDrawSurfacesSunShadow - nop call @ 0x5F9FF5
			// R_AddAllStaticModelSurfacesRangeSunShadow - nop calls @ 0x0x5FA006 && 0x5FA012
			// R_DrawShadowCookies - nop call @ 0x5FA20F
			// DynEntCl_ProcessEntities - nop call @ 0x5FA38D
		//}
	}
}
