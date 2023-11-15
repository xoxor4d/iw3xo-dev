#include "std_include.hpp"

// # notes

// r_z_near_depthhack >> 4
// r_znear >> 4 pushes viewmodel away the further from 0 0 0 we are - 4.002 fixes this ??
// r_smp_backend >> 1 fixes viewmodel bumping
// r_zfeather >> 0 fixes remix freaking out turning half of all textures white + makes fx work to some extend (note: 'R_SkinStaticModelsCamera' @ 0x63B024 or 0x0063AB00)

// FIXED ISSUE 1 >> mp_crash msg "too many static models ..." @ 0x63AF4D (disabled culling: the engine cant handle modellighting for so many static models, thus not drawing them)
// ISSUE 2 >> removing 'R_AddAllBspDrawSurfacesRangeCamera' (decals) call @ 0x5F9E65 fixes cold/warm light transition on mp_crash? - still present - LOD related so most likely a specific object/shader that is causing that
// ISSUE 3 >> moving infront of the construction spotlight on mp_crash (bottom hardware) with cg_drawgun enabled will completly turn off remix rendering

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
		light.Diffuse.r = gui_devgui::rtx_debug_light_color[0];
		light.Diffuse.g = gui_devgui::rtx_debug_light_color[1];
		light.Diffuse.b = gui_devgui::rtx_debug_light_color[2];

		light.Position.x = gui_devgui::rtx_debug_light_origin[0];
		light.Position.y = gui_devgui::rtx_debug_light_origin[1];
		light.Position.z = gui_devgui::rtx_debug_light_origin[2];

		light.Range = gui_devgui::rtx_debug_light_range;

		game::glob::d3d9_device->SetLight(0, &light);
		game::glob::d3d9_device->LightEnable(0, TRUE);

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

		if (gui_devgui::rtx_spawn_light)
		{
			spawn_light();
		}
		
		setup_dvars_rtx();
	}

	__declspec(naked) void rb_standard_drawcommands_stub()
	{
		const static uint32_t rb_standard_drawcommands_func = 0x64AFB0;
		const static uint32_t retn_addr = 0x64B7D0;
		__asm
		{
			pushad;
			call	setup_rtx;
			popad;

			call	rb_standard_drawcommands_func;
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

	void xmodel_set_test_lods(int lod_level, float dist)
	{
		game::g_testLods[lod_level].dist = dist;
		game::g_testLods[lod_level].enabled = dist >= 0.0f;
	}

	void r_set_test_lods()
	{
		const auto& r_forceLod = game::Dvar_FindVar("r_forceLod");
		if (r_forceLod && r_forceLod->current.integer == r_forceLod->reset.integer)
		{
			const auto& r_highLodDist = game::Dvar_FindVar("r_highLodDist");
			const auto& r_mediumLodDist = game::Dvar_FindVar("r_mediumLodDist");
			const auto& r_lowLodDist = game::Dvar_FindVar("r_lowLodDist");
			const auto& r_lowestLodDist = game::Dvar_FindVar("r_lowestLodDist");

			xmodel_set_test_lods(0, r_highLodDist->current.value);
			xmodel_set_test_lods(1, r_mediumLodDist->current.value);
			xmodel_set_test_lods(2, r_lowLodDist->current.value);
			xmodel_set_test_lods(3, r_lowestLodDist->current.value);
		}
		else
		{
			float dist;
			for (auto i = 0; i < 4; ++i)
			{
				if (i == r_forceLod->current.integer)
				{
					dist = 0.0f;
				}
				else
				{
					dist = 0.001f;
				}

				xmodel_set_test_lods(i, dist);
			}
		}
	}

	__declspec(naked) void r_set_test_lods_stub()
	{
		const static uint32_t retn_addr = 0x5F7515;
		__asm
		{
			pushad;
			call	r_set_test_lods;
			popad;

			// og instructions
			mov     eax, [eax + 0xC];
			shl     eax, 4;
			jmp		retn_addr;
		}
	}

	// -------

	float xmodel_lodinfo_get_dist(const game::XModelLodInfo* lod_info, const int lod_index)
	{
		auto dist = lod_info->dist;

		if (game::g_testLods[lod_index].enabled)
		{
			dist = game::g_testLods[lod_index].dist;
		}
		
		return dist;
	}

	int xmodel_get_lod_for_dist(const game::XModel* model, const float* base_dist)
	{
		const auto lod_count = model->numLods;

		for (auto lod_index = 0; lod_index < lod_count; ++lod_index)
		{
			const auto lod_dist = xmodel_lodinfo_get_dist(&model->lodInfo[lod_index], lod_index);

			if (lod_dist == 0.0f || lod_dist > *base_dist)
			{
				return lod_index;
			}
		}
		return -1;
	}

	int xmodel_get_lod_for_dist_global = 0;
	__declspec(naked) void xmodel_get_lod_for_dist_detour()
	{
		const static uint32_t retn_addr = 0x5911F0;
		__asm
		{
			pushad;
			lea		ecx, [eax + 0x28];
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

	// ouch ... this was inlined
	__declspec(naked) void xmodel_get_lod_for_dist_inlined()
	{
		const static uint32_t break_addr = 0x63AF27;
		const static uint32_t retn_addr = 0x63AF14;
		__asm
		{
			pushad;
			push	esi; // index
			push	edx; // lodinfo
			call	xmodel_lodinfo_get_dist; // returns lod_dist in st0
			add		esp, 8;
			popad;

			
			fldz;		// st0 = 0
						// st1 = lod_dist
			fcompp;		// compare and pop both floats
			fstsw ax;	// move fpu status word into ax
			sahf;		// move into eflags to we can check with conditional jumps

			jnz	CONTINUE;
			jmp break_addr;

		CONTINUE:
			// og instructions
			add     esi, 1;
			add     edx, 0x1C;

			jmp		retn_addr;
		}
	}

	/* ---------------------------------------------------------- */

#if 1 // disabled for now
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

	// R_AddWorkerCmd
	__declspec(naked) void r_draw_dynents_stub()
	{
		const static uint32_t R_AddCellDynModelSurfacesInFrustumCmd_func = 0x64D4C0;
		const static uint32_t retn_stock = 0x62932D;
		__asm
		{
			pushad;
			push	eax;
			mov		eax, 0; //dvars::r_drawDynents;
			//cmp		byte ptr[eax + 12], 1;
			cmp		eax, 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;

			popad;
			call	R_AddCellDynModelSurfacesInFrustumCmd_func;

		SKIP:
			popad;
			jmp		retn_stock;
		}
	}
#endif

#define DISABLE_CULLING

	rtx::rtx()
	{
		// hook 'RB_StandardDrawCommands' call in 'RB_Draw3DInternal'
		utils::hook(0x64B7CB, rb_standard_drawcommands_stub, HOOK_JUMP).install()->quick();

		/* ---------------------------------------------------------- */


#ifdef DISABLE_CULLING

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
#endif

		// R_AddWorkerCmd :: disable dynEnt models // 0x629328 -> nop
		utils::hook::nop(0x629328, 5); //utils::hook(0x629328, r_draw_dynents_stub, HOOK_JUMP).install()->quick(); // popad makes it worse

		// nop calls to 'R_UpdateLodParms'
		//utils::hook::nop(0x5FAEB3, 5);
		//utils::hook::nop(0x5FAFCE, 5);

		// removing 'R_AddAllBspDrawSurfacesRangeCamera' (decals) call @ 0x5F9E65 fixes cold/warm light transition on mp_crash?
		//utils::hook::nop(0x5F9E65, 5);

		// stub after 'R_RegisterDvars' to register our own or re-register stock dvars
		utils::hook(0x5F4EFA, register_dvars_stub, HOOK_JUMP).install()->quick();

		// skip original lod dvar registration
		utils::hook::nop(0x62A2A1, 5); // r_lodScaleRigid
		utils::hook::nop(0x62A2F8, 5); // r_lodBiasRigid
		utils::hook::nop(0x62A34F, 5); // r_lodScaleSkinned
		utils::hook::nop(0x62A3A6, 5); // r_lodBiasSkinned

		// un-cheat + saved flag for fx_enable
		utils::hook::set<BYTE>(0x4993EC + 1, 0x01); // was 0x80


		//
		// LOD

		// check if testlods are active and overwrite lod-dist
		utils::hook(0x5911C0, xmodel_get_lod_for_dist_detour, HOOK_JUMP).install()->quick();

		// ^ but inlined ..... (R_AddAllStaticModelSurfacesCamera)
		utils::hook::nop(0x63AF0E, 6);  utils::hook(0x63AF0E, xmodel_get_lod_for_dist_inlined, HOOK_JUMP).install()->quick();

		// update testlods via 'r_forceLod' dvar
		utils::hook::nop(0x5F750F, 6); utils::hook(0x5F750F, r_set_test_lods_stub, HOOK_JUMP).install()->quick();

		// TODO - fix test lods for inlined stuff .. not working

		// mp_crash msg "too many static models ..." @ 0x63AF4D (disabled culling: the engine cant handle modellighting for so many static models, thus not drawing them)
		// jnz -> jmp (0x75 -> 0xEB) instead of jnz at 0x63AF49 (boolean check on return val from 'R_AllocStaticModelLighting')

		// TODO - create dvar for that ^

		dvars::rtx_hacks = game::Dvar_RegisterBool(
			/* name		*/ "rtx_hacks",
			/* desc		*/ "Enables various hacks and tweaks to make nvidia rtx work",
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
