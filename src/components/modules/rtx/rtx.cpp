#include "std_include.hpp"

namespace components
{
	/**
	 * @brief - send camera matrices down the fixed-function pipeline so that the remix runtime finds the camera
	 *		  - update fixed-function debug lights (spawned via the dev-gui)
	 *		  - force certain dvar's 
	 */
	void setup_rtx()
	{
		const auto dev = game::get_device();
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

		/*if (flags::has_flag("thirdperson"))
		{
			rtx::player_origin_model();
		}*/

		if (rtx_gui::mesh_handle) {
			rtx_api::bridge.DrawMeshInstance(rtx_gui::mesh_handle, &rtx_gui::mesh_transform, rtx_gui::mesh_double_sided);
		}

		if (rtx_gui::portal0_handle) {
			rtx_api::bridge.DrawMeshInstance(rtx_gui::portal0_handle, &rtx_gui::portal0_transform, rtx_gui::mesh_double_sided);
		}

		if (rtx_gui::portal1_handle) {
			rtx_api::bridge.DrawMeshInstance(rtx_gui::portal1_handle, &rtx_gui::portal1_transform, rtx_gui::mesh_double_sided);
		}

		if (rtx_gui::light_handles[0]) {
			rtx_api::bridge.DrawLightInstance(rtx_gui::light_handles[0]);
		}

		if (rtx_gui::light_handles[1]) {
			rtx_api::bridge.DrawLightInstance(rtx_gui::light_handles[1]);
		}

		if (rtx_api::g_light_handle) {
			rtx_api::bridge.DrawLightInstance(rtx_api::g_light_handle);
		}

		if (rtx_api::g_light_handle2) {
			rtx_api::bridge.DrawLightInstance(rtx_api::g_light_handle2);
		}

		rtx_gui::skysphere_frame();

		if (!flags::has_flag("no_fog"))
		{
			const auto s = rtx_map_settings::settings();
			const float fog_start = 1.0f;

			dev->SetRenderState(D3DRS_FOGENABLE, TRUE);
			dev->SetRenderState(D3DRS_FOGCOLOR, s->fog_color.packed);
			dev->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
			dev->SetRenderState(D3DRS_FOGSTART, *(DWORD*)&fog_start);
			dev->SetRenderState(D3DRS_FOGEND, *(DWORD*)&s->fog_distance);
		}

		if (!flags::has_flag("no_sun"))
		{
			// only set sun if debuglight 0 is not active
			if (auto l0 = &rtx_lights::rtx_debug_lights[0]; !l0->enable)
			{
				const auto s = rtx_map_settings::settings();

				D3DLIGHT9 light = {};
				light.Type = D3DLIGHT_DIRECTIONAL;
				light.Diffuse.r = s->sun_color[0] * s->sun_intensity;
				light.Diffuse.g = s->sun_color[1] * s->sun_intensity;
				light.Diffuse.b = s->sun_color[2] * s->sun_intensity;

				D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, (const D3DXVECTOR3*)&s->sun_direction);

				dev->SetLight(0, &light);
				dev->LightEnable(0, TRUE);

				// update gui settings for debug light 0
				l0->color_scale = s->sun_intensity;
				l0->color[0] =	s->sun_color[0];		l0->color[1] = s->sun_color[1];		l0->color[2] = s->sun_color[2];
				l0->dir[0] =	s->sun_direction[0];	l0->dir[1] = s->sun_direction[1];	l0->dir[2] = s->sun_direction[2];
				l0->type = D3DLIGHT_DIRECTIONAL;
			}
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
		const auto dev = game::get_device();

		// disable fog before rendering UI (who wants foggy UI elements right?)
		// ^ can happen if no skysphere is rendered, which is rendered last and thus disables fog for everything afterwards
		dev->SetRenderState(D3DRS_FOGENABLE, FALSE);

		// normally done in R_ToggleSmpFrame but should be fine here
		rtx::textureOverrideCount = 0;
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

	// model:	[1] techset - [2] material
	// bsp:		[3] techset - [4] material
	// bmodel:	[5] techset - [6] material
	void rtx::rb_show_tess(game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, const float* center, const char* name, const float* color, game::DebugGlobals* manual_debug_glob)
	{
		const auto debug_glob = manual_debug_glob ? manual_debug_glob : &source->input.data->debugGlobals;

		float offset_center[3];
		offset_center[0] = center[0];
		offset_center[1] = center[1];
		offset_center[2] = center[2];

		const auto dist_to_str = utils::vector::distance3(source->eyeOffset, offset_center);

		if (dvars::r_showTessDist)
		{
			if (dist_to_str > dvars::r_showTessDist->current.value && dvars::r_showTessDist->current.value != 0.0f)
			{
				return;
			}
		}

		bool viewmodel_string = false;
		auto font_scale = 0.25f;
		if (dist_to_str < 25.0f)
		{
			viewmodel_string = true;
			font_scale = 0.025f;
		}

		const game::MaterialTechnique* tech = nullptr;
		if (state->material && state->material->techniqueSet->techniques[static_cast<std::uint8_t>(state->techType)])
		{
			tech = state->material->techniqueSet->techniques[static_cast<std::uint8_t>(state->techType)];
		}

		if (const auto r_showTess = game::Dvar_FindVar("r_showTess"); r_showTess && tech)
		{
			switch (r_showTess->current.integer)
			{
			case 1: // techset model
			case 3: // techset bsp
			case 5: // techset bmodel
			{
				// offset_center[2] = (((float)state->techType - 16.0f) * 0.3f) + offset_center[2];
				// header
				game::R_AddDebugString(debug_glob, offset_center, color, font_scale, utils::va("%s: %s", name, tech->name));
				font_scale *= 0.5f;

				offset_center[2] -= viewmodel_string ? 0.25f : 2.5f;
				game::R_AddDebugString(debug_glob, offset_center, color, font_scale, utils::va("> [TQ]: %s", state->material->techniqueSet->name));

				offset_center[2] -= viewmodel_string ? 0.25f : 2.5f;
				game::R_AddDebugString(debug_glob, offset_center, color, font_scale, utils::va("> [VS] %s", tech->passArray[0].vertexShader ? tech->passArray[0].vertexShader->name : "<NONE>"));

				offset_center[2] -= viewmodel_string ? 0.25f : 2.5f;
				game::R_AddDebugString(debug_glob, offset_center, color, font_scale, utils::va("> [PS] %s", tech->passArray[0].pixelShader ? tech->passArray[0].pixelShader->name : "<NONE>"));
				break;
			}

			case 2: // material model
			case 4: // material bsp
			case 6: // material bmodel
			{
				// header
				game::R_AddDebugString(debug_glob, offset_center, color, font_scale, utils::va("%s: %s", name, state->material->info.name));
				font_scale *= 0.5f;

				for (auto i = 0; i < state->material->textureCount; i++)
				{
					if (&state->material->textureTable[i] && state->material->textureTable[i].u.image && state->material->textureTable[i].u.image->name)
					{
						const auto img = state->material->textureTable[i].u.image;
						offset_center[2] -= viewmodel_string ? 0.25f : 2.5f;

						const char* semantic_str;
						switch (static_cast<std::uint8_t>(img->semantic))
						{
						case 0: semantic_str = "2D"; break;
						case 1: semantic_str = "F"; break;
						case 2: semantic_str = "C"; break;
						case 5: semantic_str = "N"; break;
						case 8: semantic_str = "S"; break;
						case 11: semantic_str = "W"; break;
						default: semantic_str = "C+"; break;
						}

						game::R_AddDebugString(debug_glob, offset_center, color, font_scale, utils::va("> [%s] %s", semantic_str, img->name)); // static_cast<std::uint8_t>(img->semantic)
					}
				}
				break;
			}

			default:
				break;
			}
		}
	}

	/**
	 * @brief spawns a little triangle at the origin of the player that is marked as 'player model body texture'
	 *		  - triangle then acts as the origin for the bounding box that culls meshes marked with the 'player model' category
	 *		  - not really working .. better technique below this func
	 */
	void rtx::player_origin_model()
	{
		if (game::clc.demoplaying)
		{
			return;
		}

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

	int R_AllocTextureOverride(game::Material* material, std::uint32_t model_index_mask, game::GfxImage* img1, int prev_override)
	{
		const auto override_idx = rtx::textureOverrideCount++;
		if (override_idx < 256u)
		{
			rtx::textureOverrides[override_idx].material = material;
			rtx::textureOverrides[override_idx].img1 = img1;
			rtx::textureOverrides[override_idx].img2 = img1; //img2;
			rtx::textureOverrides[override_idx].dobjModelMask = static_cast<std::uint16_t>(model_index_mask);
			rtx::textureOverrides[override_idx].prev = static_cast<std::int16_t>(prev_override);
			return override_idx;
		}

		//R_WarnOncePerFrame(R_WARN_TEXTURE_OVERRIDES, 256);
		rtx::textureOverrideCount = 256;
		return -1;
	}

	// this adds a unique material on the playermodel dobj and all of its submodels including the worldmodel of the gun
	// textureoverride is then checked and set via rtx_fixed_function::apply_texture_overrides
	int cg_player_add_obj_to_scene(const game::DObj_s* obj, const game::centity_s* cent)
	{
		int tex_override = -1;

		const bool not_local = !game::svs_header->clients; // client is not hosting the game
		const auto spectator_num = not_local ? -2 : game::g_clients[game::cgs->clientNum].spectatorClient;
		const auto cl_ent = game::clients->snap.ps.clientNum;

		// hide body of our player and body of the spectated player (but show our own body in the spectator cam)
		if ((cent->nextState.clientNum == game::cgs->clientNum && spectator_num == -1) // centity = self but NOT spectating someone
			|| spectator_num == cent->nextState.clientNum // centity = the spectated player
			|| cl_ent == cent->nextState.clientNum) // non-local servers: centity = the spectated player 
		{
			if (const auto	material = game::Material_RegisterHandle("rtx_player_shadow", 3); material
				&& material->textureTable
				&& material->textureTable->u.image
				&& material->textureTable->u.image->name
				&& !std::string_view(material->textureTable->u.image->name)._Equal("default"))
			{
				std::uint16_t model_hash = 1337u;
				for (auto i = 0u; i < static_cast<std::uint8_t>(obj->numModels); i++)
				{
					tex_override = R_AllocTextureOverride(material, model_hash, material->textureTable->u.image, i == 0 ? -1 : tex_override);
					model_hash += 2;
				}
			}
		}

		return tex_override;
	}

	int m_cg_player_stub_helper = 0;
	__declspec(naked) void cg_player_stub()
	{
		const static uint32_t retn_addr = 0x445478;
		__asm
		{
			// og
			mov     ecx, [ebp + 0];

			pushad;
			push	ebx;
			push	esi; // dobj
			call	cg_player_add_obj_to_scene;
			mov		m_cg_player_stub_helper, eax;
			add		esp, 8;
			popad;

			fild	m_cg_player_stub_helper;
			jmp		retn_addr;
		}
	}


	// *
	// material related

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
	// https://github.com/doitsujin/dxvk/issues/3214

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
		int _last_active_valid_cell = 0; //-1;
		void R_AddWorldSurfacesPortalWalk_hk(int camera_cell_index, game::DpvsView* dpvs)
		{
			//const auto dpvsGlob = reinterpret_cast<game::DpvsGlob*>(0x3957100);

			const auto dpvsGlob = game::dpvsGlob;

			// never show the complete map, only the last valid cell
			if (camera_cell_index < 0)
			{
				const auto cell = &game::rgp->world->cells[_last_active_valid_cell];
				const auto cell_index = cell - game::rgp->world->cells;
				game::R_AddCellSurfacesAndCullGroupsInFrustumDelayed(cell, dpvs->frustumPlanes, dpvs->frustumPlaneCount, dpvs->frustumPlaneCount);
				dpvsGlob->cellVisibleBits[(cell_index >> 5) + 3] |= (1 << (cell_index & 0x1F)); // is the +3 correct here?
			}
			else
			{
				_last_active_valid_cell = camera_cell_index;

				// hack - disable most frustum culling
				dpvsGlob->views[0][0].frustumPlanes[0].coeffs[3] += rtx::m_frustum_plane_offsets[0]; //5000.0f;
				dpvsGlob->views[0][0].frustumPlanes[1].coeffs[3] += rtx::m_frustum_plane_offsets[1];
				dpvsGlob->views[0][0].frustumPlanes[2].coeffs[3] += rtx::m_frustum_plane_offsets[2];
				dpvsGlob->views[0][0].frustumPlanes[3].coeffs[3] += rtx::m_frustum_plane_offsets[3];
				dpvsGlob->views[0][0].frustumPlanes[4].coeffs[3] += rtx::m_frustum_plane_offsets[4];
				dpvsGlob->views[0][0].frustumPlanes[5].coeffs[3] += rtx::m_frustum_plane_offsets[5];
				dpvsGlob->viewPlane.coeffs[3] += rtx::m_frustum_plane_offsets[6]; //5000.0f;
				// ^ needs to be viewPlane here?

				const auto& var = game::Dvar_FindVar("r_singleCell");
				const bool single_cell = (var && var->current.enabled) ? true : false;

				// #
				// always add full cell the player is in (same as r_singlecell)
				const auto cell = &game::rgp->world->cells[camera_cell_index];
				const auto cell_index = cell - game::rgp->world->cells;

				if (single_cell)
				{
					dpvsGlob->farPlane = nullptr;
				}

				game::R_AddCellSurfacesAndCullGroupsInFrustumDelayed(cell, dpvs->frustumPlanes, dpvs->frustumPlaneCount, dpvs->frustumPlaneCount); // dpvs->frustumPlaneCount
				dpvsGlob->cellVisibleBits[(cell_index >> 5) + 3] |= (1 << (cell_index & 0x1F));


				// #
				// draw cell index at the center of the current cell
				if (dvars::r_showCellIndex && dvars::r_showCellIndex->current.enabled)
				{
					const game::vec3_t center =
					{
						(cell->mins[0] + cell->maxs[0]) * 0.5f,
						(cell->mins[1] + cell->maxs[1]) * 0.5f,
						(cell->mins[2] + cell->maxs[2]) * 0.5f
					};

					game::R_AddDebugString(&game::get_frontenddata()->debugGlobals, center, game::COLOR_GREEN, 1.0f, utils::va("Cell Index: %d", camera_cell_index));

					const game::vec4_t col = { 1.0f, 1.0f, 1.0f, 1.0f };
					game::draw_text_with_engine(
						/* x	*/ 50.0f,
						/* y	*/ 500.0f,
						/* scaX */ 1,
						/* scaY */ 1,
						/* font */ _cg::get_font_for_style(dvars::pm_hud_fontStyle->current.integer),
						/* colr */ col,
						/* txt	*/ utils::va("Current Cell: %d", camera_cell_index));
				}


				// #
				// force cells defined in map_settings.ini

				if (!single_cell && rtx_map_settings::settings()->cell_overrides_exist && camera_cell_index < 1024)
				{
					const auto& c_ow = rtx_map_settings::settings()->cell_settings[camera_cell_index];
					if (c_ow.active)
					{
						for (const auto& i : c_ow.forced_cell_indices)
						{
							const auto forced_cell = &game::rgp->world->cells[i];
							const auto c_index = forced_cell - game::rgp->world->cells;
							game::R_AddCellSurfacesAndCullGroupsInFrustumDelayed(forced_cell, dpvs->frustumPlanes, dpvs->frustumPlaneCount, dpvs->frustumPlaneCount);
							dpvsGlob->cellVisibleBits[(c_index >> 5) + 3] |= (1 << (c_index & 0x1F));
						}
					}
				}

				if (!single_cell)
				{
					// R_VisitPortals
					game::R_VisitPortals(dpvs->frustumPlaneCount, cell, &dpvsGlob->viewPlane, dpvs->frustumPlanes); // viewplane here .. or is that the nearplane?
				}
			}
		}

		__declspec(naked) void cell_stub()
		{
			const static uint32_t retn_addr = 0x60B08F;
			__asm
			{
				// ebx = world
				// esi = cameraCellIndex
				// eax = DpvsView

				push	eax;
				push	esi;
				call	R_AddWorldSurfacesPortalWalk_hk;
				add		esp, 8;
				jmp		retn_addr;
			}
		}

		// #
		// # tweakable culling dvars
		
		__declspec(naked) void disable_mins_culling_stub()
		{
			const static uint32_t stock_addr = 0x643B08;
			const static uint32_t disable_culling_addr = 0x643B0E;
			__asm
			{
				// jump if culling is less or disabled
				cmp		rtx::loc_culling_tweak_mins, 1;
				je		SKIP;

				fnstsw  ax;
				test    ah, 0x41;
				jmp		stock_addr;

				SKIP:
				jmp		disable_culling_addr;
			}
		}

		__declspec(naked) void disable_maxs_culling_stub()
		{
			const static uint32_t stock_addr = 0x643B39;
			const static uint32_t disable_culling_addr = 0x643B48;
			__asm
			{
				// jump if culling is less or disabled
				cmp		rtx::loc_culling_tweak_maxs, 1;
				je		SKIP;

				fnstsw  ax;
				test    ah, 1;
				jmp		stock_addr;

			SKIP:
				jmp		disable_culling_addr;
			}
		}

		__declspec(naked) void disable_frustum_culling_stub()
		{
			const static uint32_t stock_addr = 0x643D44;
			const static uint32_t disable_culling_addr = 0x643D80;
			__asm
			{
				// og
				xor		ecx, ecx;

				// jump if culling is less or disabled
				cmp		rtx::loc_culling_tweak_frustum, 1;
				je		SKIP;

				cmp		[esp + 0x54], ecx;
				jmp		stock_addr;

			SKIP:
				jmp		disable_culling_addr;
			}
		}

		__declspec(naked) void disable_smodel_culling_stub()
		{
			const static uint32_t stock_addr = 0x643C79;
			const static uint32_t disable_culling_addr = 0x643CB5;
			__asm
			{
				// og
				xor		ecx, ecx;

				// jump if culling is less or disabled
				cmp		rtx::loc_culling_tweak_smodel, 1;
				je		SKIP;

				cmp		[esp + 0x54], ecx;
				jmp		stock_addr;

			SKIP:
				jmp		disable_culling_addr;
			}
		}

#pragma region OLD_CULLING
		// #
		// old anti culling - active if flag 'old_anti_culling' is set

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
#pragma endregion
	}


	// *
	// fx
	
	// returns true if inside radius
	int fx_cullsphere_radius_check(const float* camera_pos, const float* fx_world_pos)
	{
		if (const auto var = game::Dvar_FindVar("fx_cull_elem_draw");
			var && var->current.enabled)
		{
			const auto dist = utils::vector::distance3(fx_world_pos, camera_pos);

			if (dvars::rtx_fx_cull_elem_draw_radius &&
				dist < dvars::rtx_fx_cull_elem_draw_radius->current.value)
			{
				return 1;
			}
		}

		return 0;
	}

	int fx_cullsphere_global_helper = 0;
	__declspec(naked) void fx_cullsphere_stub()
	{
		const static uint32_t og_continue_addr = 0x4A6297;
		const static uint32_t og_retn_addr = 0x4A62DE;
		__asm
		{
			pushad;
			push	edi;					// float* fx worldpos
			push	ecx;					// FxCamera*
			call	fx_cullsphere_radius_check;
			add		esp, 8;
			mov		fx_cullsphere_global_helper, eax;
			popad;
			xor		eax, eax;

			cmp		fx_cullsphere_global_helper, 1;

			push    ecx;
			je		loc_4A62DE;				// do not cull if within radius

			// OG_LOGIC
			xor		edx, edx;
			test    esi, esi;
			jbe		loc_4A62DE;
			jmp		og_continue_addr;

		loc_4A62DE:
			jmp		og_retn_addr;
		}
	}


	// *
	// dvars

	void rtx::force_dvars_on_frame()
	{
		// update culling vars at the end of a frame (so we don't change culling behaviour mid-frame -> not safe)
		{
			if (dvars::rtx_culling_tweak_mins) loc_culling_tweak_mins = dvars::rtx_culling_tweak_mins->current.enabled;
			if (dvars::rtx_culling_tweak_maxs) loc_culling_tweak_maxs = dvars::rtx_culling_tweak_maxs->current.enabled;
			if (dvars::rtx_culling_tweak_frustum) loc_culling_tweak_frustum = dvars::rtx_culling_tweak_frustum->current.enabled;
			if (dvars::rtx_culling_tweak_smodel) loc_culling_tweak_smodel = dvars::rtx_culling_tweak_smodel->current.enabled;


			// #
			// old anti culling - activ if flag 'old_anti_culling' is set

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
	}

	//_common::force_dvars_on_init()
	void rtx::set_dvars_defaults_on_init()
	{
		rtx::force_dvars_on_frame();
		dvars::bool_override("r_multiGpu", true); // fps ++
		dvars::bool_override("r_floatz", true);
		dvars::bool_override("r_autopriority", false);
		dvars::bool_override("r_d3d9ex", false);
		dvars::bool_override("r_vsync", false);
		dvars::bool_override("r_preloadShaders", false);
		dvars::bool_override("r_altModelLightingUpdate", false); // prevents loading into a map
		dvars::bool_override("r_zfeather", false);
		dvars::bool_override("r_aaAlpha", false);
		dvars::int_override("r_aaSamples", 1);
		dvars::int_override("r_dlightLimit", 0);
		dvars::bool_override("sm_enable", false);
	}

	// rtx::on_map_load()
	void rtx::set_dvars_defaults_on_mapload()
	{
		if (!flags::has_flag("no_forced_lod"))
		{
			dvars::int_override("r_forceLod", 0);
		}

		//dvars::bool_override("fx_drawClouds", false);
		dvars::bool_override("r_pretess", true); // bsp rendering fps ++
		dvars::float_override("r_znear", 4.00195f);
		dvars::float_override("r_znear_depthhack", 4.0f);
		dvars::bool_override("r_glow", false);
		dvars::bool_override("r_glow_allowed", false);
		dvars::bool_override("r_depthPrepass", false);
		dvars::bool_override("r_dof_enable", false);

		if (dvars::rtx_culling_plane_dist)
		{
			for (auto& plane : rtx::m_frustum_plane_offsets)
			{
				plane = dvars::rtx_culling_plane_dist->current.value;
			}
		}
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
		rtx_map_settings::get()->set_settings_for_loaded_map();
		rtx::set_dvars_defaults_on_mapload();

		// increase culling distances for certain objects
		if (game::gfx_world)
		{
			for (auto m = 0u; m < game::gfx_world->dpvs.smodelCount; m++)
			{
				if (std::string_view(game::gfx_world->dpvs.smodelDrawInsts[m].model->name)._Equal("cs_cargoship_wall_light_on"))
				{
					game::gfx_world->dpvs.smodelDrawInsts[m].cullDist = 10000.0f;
				}
			}
		}
	}

	// > fixed_function::free_fixed_function_buffers_stub
	void rtx::on_map_shutdown()
	{
		rtx_gui::skysphere_reset();
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

		if (!flags::has_flag("no_playershadow"))
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
			utils::hook::nop(0x45674A, 2); // DrawBulletImpacts :: calculate impact mark position accounting for gun idle sway
			utils::hook::nop(0x457054, 6); // CG_AddViewWeapon: do not disable viewmodel
			utils::hook::nop(0x451D8E, 2); // CG_UpdateThirdPerson: always enable "thirdperson"

			utils::hook(0x445473, cg_player_stub, HOOK_JUMP).install()->quick(); // set unique texture for all parts of the model incl. gun 
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

		// modellight alloc
		//utils::hook::set<BYTE>(0x62ED7B + 2, 0x7F); // inc. modellight alloc break from 32 to 127
		utils::hook::set(0x62ECA0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // always "alloc" and return 1 as lightingHandle

		// disable loading of specular and normalmaps (de-clutter remix ui)
		if (!flags::has_flag("load_normal_spec"))
		{
			utils::hook::nop(0x616E65, 5);
			utils::hook::nop(0x616F42, 5);
			utils::hook::nop(0x6424E4, 7); utils::hook(0x6424E4, load_image_stub, HOOK_JUMP).install()->quick();
		}

		// *
		// allow 4k images (skysphere)

		// set img alloc size from 0x600000 to 0x4000000 (67mb)
		utils::hook::set<BYTE>(0x641CC7 + 3, 0x00);
		utils::hook::set<BYTE>(0x641CC7 + 4, 0x04);

		// ^ cmp -> error if image larger
		utils::hook::set<BYTE>(0x641C89 + 3, 0x00);
		utils::hook::set<BYTE>(0x641C89 + 4, 0x04);
		//utils::hook::set<BYTE>(0x641C8F, 0xEB); // disable warning completely

		// dxvk's 'EnumAdapterModes' returns a lot of duplicates and the games array only has a capacity of 256 which is not enough depending on max res. and refreshrate
		// fix resolution issues by removing duplicates returned by EnumAdapterModes - then write the array ourselfs
		utils::hook(0x5F4182, resolution::R_EnumDisplayModes_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x5F41C9, resolution::R_EnumDisplayModes_stub2, HOOK_JUMP).install()->quick();
		utils::hook::set<BYTE>(0x5F4170 + 2, 0x04); // set max array size check to 1024 (check within loop)


		// *
		// culling

		if (flags::has_flag("old_anti_culling"))
		{
			OLD_CULLING_ACTIVE = true;

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
		else // new way of culling
		{
			// rewrite some logic in 'R_AddWorldSurfacesPortalWalk'
			utils::hook::nop(0x60B03F, 9); utils::hook(0x60B03F, cull::cell_stub, HOOK_JUMP).install()->quick();

			// ^ - never show all cells at once when the camera cell index is < 0, we handle that in the func above
			utils::hook::nop(0x60B02E, 2);


			// #
			// tweakable culling via dvars

			// disable mins culling - 0x643B08 nop 6
			utils::hook(0x643B03, cull::disable_mins_culling_stub, HOOK_JUMP).install()->quick();

			// disable maxs culling - 0x643B39 to jmp
			utils::hook(0x643B34, cull::disable_maxs_culling_stub, HOOK_JUMP).install()->quick();

			// disable (most) frustum culling - 0x643D44 to jmp
			utils::hook::nop(0x643D3E, 6); utils::hook(0x643D3E, cull::disable_frustum_culling_stub, HOOK_JUMP).install()->quick();

			// ^ for smodels - 0x643C79 to jmp
			utils::hook::nop(0x643C73, 6); utils::hook(0x643C73, cull::disable_smodel_culling_stub, HOOK_JUMP).install()->quick();

			dvars::rtx_culling_tweak_mins = game::Dvar_RegisterBool(
				/* name		*/ "rtx_culling_tweak_mins",
				/* desc		*/ "Disable dpvs mins check (reduces culling)",
				/* default	*/ false,
				/* flags	*/ game::dvar_flags::saved);

			dvars::rtx_culling_tweak_maxs = game::Dvar_RegisterBool(
				/* name		*/ "rtx_culling_tweak_maxs",
				/* desc		*/ "Disable dpvs maxs check (reduces culling)",
				/* default	*/ false,
				/* flags	*/ game::dvar_flags::saved);

			dvars::rtx_culling_tweak_frustum = game::Dvar_RegisterBool(
				/* name		*/ "rtx_culling_tweak_frustum",
				/* desc		*/ "Disable (most) frustum culling (reduces culling)",
				/* default	*/ true,
				/* flags	*/ game::dvar_flags::saved);

			dvars::rtx_culling_tweak_smodel = game::Dvar_RegisterBool(
				/* name		*/ "rtx_culling_tweak_smodel",
				/* desc		*/ "Disable static model frustum culling (reduces culling)",
				/* default	*/ true,
				/* flags	*/ game::dvar_flags::saved);

			dvars::rtx_culling_plane_dist = game::Dvar_RegisterFloat(
				/* name		*/ "rtx_culling_plane_dist",
				/* desc		*/ "Distance of dpvs culling-planes",
				/* default	*/ 1250.0f,
				/* minVal	*/ -50000.0f,
				/* maxVal	*/ 50000.0f,
				/* flags	*/ game::dvar_flags::saved);


			// never cull brushmodels via dpvs
			utils::hook::nop(0x64D35B, 2);
			utils::hook::set<BYTE>(0x64D368, 0xEB); // ^

			// ^ scene ents (spawned map markers (script models))
			utils::hook::nop(0x64D17A, 2);
			utils::hook::set<BYTE>(0x64D1A9, 0xEB); // ^
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
		// fx

		// hook FX_CullSphere to implement an additional radius check
		utils::hook(0x4A6290, fx_cullsphere_stub, HOOK_JUMP).install()->quick();


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

		static const char* r_showTess_enum[] = { "off", "model_techset", "model_material", "bsp_techset", "bsp_material", "bmodel_techset", "bmodel_material" };
		dvars::r_showTess = game::Dvar_RegisterEnum(
			/* name		*/ "r_showTess",
			/* desc		*/ "surface data info",
			/* default	*/ 0,
			/* enumSize */ 7,
			/* enumData */ r_showTess_enum,
			/* flags	*/ game::dvar_flags::none
		);

		dvars::r_showTessSkin = game::Dvar_RegisterBool(
			/* name		*/ "r_showTessSkin",
			/* desc		*/ "show skinned surface info",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none);

		dvars::r_showTessDist = game::Dvar_RegisterFloat(
			/* name		*/ "r_showTessDist",
			/* desc		*/ "radius in which to draw r_showTess debug strings",
			/* default	*/ 400.0f,
			/* min		*/ 0.0f,
			/* max		*/ 10000.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::r_showCellIndex = game::Dvar_RegisterBool(
			/* name		*/ "r_showCellIndex",
			/* desc		*/ "draw cell index at the center of current cell (useful for map_settings)",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none);

		dvars::rtx_fx_cull_elem_draw_radius = game::Dvar_RegisterFloat(
			/* name		*/ "rtx_fx_cull_elem_draw_radius",
			/* desc		*/ "fx elements inside this radius are not affected by culling (fx_cull_elem_draw)",
			/* default	*/ 1200.0f,
			/* min		*/ 0.0f,
			/* max		*/ 10000.0f,
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
