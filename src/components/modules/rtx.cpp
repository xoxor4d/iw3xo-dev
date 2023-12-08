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
	

	// hide with
	// rtx_skysphere_model->r.svFlags = 0x01;

	// show with
	// rtx_skysphere_model->r.svFlags = 0x04;

	// called from r_set_frame_fog > daynight
	void rtx::skysphere_frame()
	{
		if (rtx::skysphere_is_model_valid())
		{
			if (skysphere_auto_rotation)
			{
				if (skysphere_model_rotation[1] >= 360.0f)
				{
					skysphere_model_rotation[1] = 0.0f;
				}

				const auto timescale = game::Dvar_FindVar("timescale")->current.value;

				skysphere_model_rotation[1] += (float)game::glob::lpmove_server_frame_time * 0.0001f * skysphere_auto_rotation_speed * timescale;
				game::G_SetAngles(skysphere_model, skysphere_model_rotation);
			}
		}
	}

	const char* rtx::skysphere_get_name_for_variant(int variant)
	{
		switch (variant)
		{
		default:
		case 0: return "rtx_skysphere_oceanrock";
		case 1: return "rtx_skysphere_desert";
		case 2: return "rtx_skysphere_overcast_city";
		case 3: return "rtx_skysphere_night";
		}
	}

	bool rtx::skysphere_is_model_valid()
	{
		// if not spawned an entity yet
		if (!skysphere_spawned)
		{
			return false;
		}

		// check if the entity is valid (player changed level etc.)
		if (skysphere_model == nullptr || skysphere_model->classname == 0 
			|| skysphere_model->model != game::G_ModelIndex(skysphere_get_name_for_variant(skysphere_variant)))
		{
			return false;
		}

		return true;
	}

	void rtx::skysphere_update_pos()
	{
		if (skysphere_spawned)
		{
			skysphere_model->r.svFlags = 0x04; // visible
			game::G_SetOrigin(skysphere_model, skysphere_model_origin);
			game::G_SetAngles(skysphere_model, skysphere_model_rotation);
		}
	}

	void rtx::skysphere_toggle_vis()
	{
		if (rtx::skysphere_is_model_valid())
		{
			skysphere_model->r.svFlags = skysphere_model->r.svFlags == 0x04 ? 0x01 : 0x04;
			game::G_SetOrigin(skysphere_model, skysphere_model_origin);
		}
	}

	void rtx::skysphere_change_model(int variant)
	{
		const std::int16_t model_index = game::G_ModelIndex(skysphere_get_name_for_variant(variant));

		skysphere_model->model = model_index;
		skysphere_model->s.index = model_index;

		skysphere_variant = variant;
	}

	void rtx::skysphere_spawn(int variant)
	{
		if (rtx::skysphere_is_model_valid())
		{
			skysphere_change_model(variant);
			return;
		}

		// needs :: 
		// s->index = modelIndex
		// linked = 0x1;
		// svFlags = 0x04; // even = visible, uneven = hidden

		const std::int16_t model_index = game::G_ModelIndex(skysphere_get_name_for_variant(variant));

		skysphere_model = game::G_Spawn();
		skysphere_model->model = model_index;
		skysphere_model->s.index = model_index;
		skysphere_model->r.svFlags = 0x04;
		skysphere_model->r.linked = 0x1;

		game::G_SetOrigin(skysphere_model, skysphere_model_origin);
		game::G_SetAngles(skysphere_model, skysphere_model_rotation);

		game::G_CallSpawnEntity(skysphere_model);

		skysphere_spawned = true;
		skysphere_variant = variant;
	}


	// ---------------------------------------------------------------------
	// #####################################################################
	// ---------------------------------------------------------------------


	bool rtx::r_set_material_stub(game::switch_material_t* swm, const game::GfxCmdBufState* state)
	{
		if (dvars::rtx_hacks->current.enabled)
		{
			if (utils::starts_with(swm->current_material->info.name, "wc/sky_"))
			{
				swm->technique_type = game::TECHNIQUE_UNLIT;
				_renderer::switch_material(swm, "rtx_sky");
				return false;
			}

			// fix remix normals for viewmodels - needs the material string check because the weapon is in both depth-ranges for some reason
			if (state->depthRangeType == game::GFX_DEPTH_RANGE_VIEWMODEL || utils::starts_with(swm->current_material->info.name, "mc/mtl_weapon_"))
			{
				swm->technique_type = game::TECHNIQUE_LIT;
				swm->switch_technique_type = true;

				return false;
			}
		}

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


		const auto setup_light_settings = [](D3DLIGHT9* ff_light, rtx::rtx_debug_light* setting)
		{
			if (!ff_light || !setting)
			{
				game::Com_Error(game::ERR_DISCONNECT, "spawn_light::setup_light_settings (!ff_light || !setting)");
				return;
			}

			ff_light->Type = setting->type;

			if (setting->enable)
			{
				// used to turn off the light once rtx_lights[i].enable gets set to false
				setting->disable_hack = 0;

				ff_light->Diffuse.r = setting->color[0] * setting->color_scale;
				ff_light->Diffuse.g = setting->color[1] * setting->color_scale;
				ff_light->Diffuse.b = setting->color[2] * setting->color_scale;
			}
			else
			{
				if (setting->type == D3DLIGHT_DIRECTIONAL && setting->disable_hack && setting->disable_hack < 9)
				{
					const float p = setting->color_scale * 0.5f;

					ff_light->Diffuse.r = setting->color[0] * setting->color_scale / (p * (float)setting->disable_hack);
					ff_light->Diffuse.g = setting->color[1] * setting->color_scale / (p * (float)setting->disable_hack);
					ff_light->Diffuse.b = setting->color[2] * setting->color_scale / (p * (float)setting->disable_hack);
				}
				else
				{
					ff_light->Diffuse.r = 0.0f;
					ff_light->Diffuse.g = 0.0f;
					ff_light->Diffuse.b = 0.0f;
				}
			}

			utils::vector::copy(setting->origin, &ff_light->Position.x, 3);

			ff_light->Range = setting->range;

			ff_light->Attenuation0 = 0.0f;    // no constant inverse attenuation
			ff_light->Attenuation1 = 0.125f;  // only .125 inverse attenuation
			ff_light->Attenuation2 = 0.0f;    // no square inverse attenuation

			// if not a pointlight
			if (setting->type > D3DLIGHT_POINT)
			{
				utils::vector::normalize_to(setting->dir, &ff_light->Direction.x);
				ff_light->Phi = 3.14f / 4.0f;
				ff_light->Theta = 3.14f / 8.0f;
			}

			if (setting->attach && setting->enable)
			{
				game::vec3_t fwd, rt, up = {};
				utils::vector::angle_vectors(game::cgs->predictedPlayerState.viewangles, fwd, rt, up);

				setting->origin[0] = game::glob::lpmove_camera_origin.x + (utils::vector::dot3(fwd, setting->dir_offset));
				setting->origin[1] = game::glob::lpmove_camera_origin.y + (utils::vector::dot3(rt,  setting->dir_offset));
				setting->origin[2] = game::glob::lpmove_camera_origin.z + (utils::vector::dot3(up,  setting->dir_offset));

				utils::vector::copy(setting->origin, &ff_light->Position.x, 3);

				ff_light->Direction.x = fwd[0];
				ff_light->Direction.y = fwd[1];
				ff_light->Direction.z = fwd[2];

				// update gui settings
				utils::vector::copy(fwd, setting->dir, 3);
			}
		};


		for (auto i = 0; i < rtx::RTX_DEBUGLIGHT_AMOUNT; i++)
		{
			if (rtx::rtx_lights[i].enable)
			{
				setup_light_settings(&light, &rtx::rtx_lights[i]);

				game::glob::d3d9_device->SetLight(i, &light);
				game::glob::d3d9_device->LightEnable(i, TRUE);
			}
			else
			{
				// slightly change position/dir over the next 10 frames so that remix realizes that the light was updated
				// we have to use almost the exact same parameters for this to work
				// fun fact: remix duplicates a light when offsetting the light position by a huge amount within 1 frame
				if (rtx::rtx_lights[i].disable_hack < 10)
				{
					setup_light_settings(&light, &rtx::rtx_lights[i]);

					light.Position.z += 0.001f;
					light.Direction.z += 0.001f;

					game::glob::d3d9_device->SetLight(i, &light);
					game::glob::d3d9_device->LightEnable(i, TRUE);

					rtx::rtx_lights[i].disable_hack++;
				}
				else
				{
					// only updates if raytracing is turned off and on again
					game::glob::d3d9_device->LightEnable(i, FALSE);
				}
			}
		}

		game::glob::d3d9_device->SetRenderState(D3DRS_LIGHTING, TRUE);
	}

	void rtx::gui()
	{
		ImGui::Indent(8.0f);

		if (ImGui::CollapsingHeader("General Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			const auto& fx_enable = game::Dvar_FindVar("fx_enable");
			ImGui::Checkbox("Enable FX", &fx_enable->current.enabled);

			ImGui::Indent(-8.0f); SPACING(0.0f, 4.0f);
		}

		if (ImGui::CollapsingHeader("Skybox", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			if (ImGui::Button("Hill"))
			{
				rtx::skysphere_spawn(0);
			}

			ImGui::SameLine();
			if (ImGui::Button("Desert"))
			{
				rtx::skysphere_spawn(1);
			}

			ImGui::SameLine();
			if (ImGui::Button("Overcast City"))
			{
				rtx::skysphere_spawn(2);
			}

			ImGui::SameLine();
			if (ImGui::Button("Night"))
			{
				rtx::skysphere_spawn(3);
			}

			if (rtx::skysphere_is_model_valid())
			{
				/*ImGui::SameLine();
				if (ImGui::Button("Toggle Skysphere"))
				{
					rtx::skysphere_toggle_vis();
				}*/

				ImGui::Checkbox("Auto Rotation", &rtx::skysphere_auto_rotation);
				ImGui::SameLine();

				ImGui::PushItemWidth(90.0f);
				ImGui::DragFloat("Speed", &rtx::skysphere_auto_rotation_speed, 0.01f, 0.01f, 10.0f, "%.2f");
				ImGui::PopItemWidth();

				/*if (ImGui::DragFloat3("Sphere Origin", rtx::skysphere_model_origin, 1.0f, -360.0f, 360.0f, "%.2f"))
				{
					rtx::skysphere_update_pos();
				}*/

				if (ImGui::DragFloat3("Sphere Rotation", rtx::skysphere_model_rotation, 0.25f, -360.0f, 360.0f, "%.2f"))
				{
					rtx::skysphere_update_pos();
				}
			}

			ImGui::Indent(-8.0f); SPACING(0.0f, 4.0f);
		}

		if (ImGui::CollapsingHeader("Debug Light", ImGuiTreeNodeFlags_None))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			for (auto i = 0; i < rtx::RTX_DEBUGLIGHT_AMOUNT; i++)
			{
				ImGui::PushID(i);

				if (ImGui::CollapsingHeader(utils::va("Light %d", i), !i ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None))
				{
					bool on_edit = false;

					if (ImGui::Checkbox("Spawn light", &rtx::rtx_lights[i].enable))
					{
						on_edit = true;

						// default light settings (spawn on player when untouched)
						if (rtx::rtx_lights[i].virgin)
						{
							rtx::rtx_lights[i].color_scale = 3.0f;
							rtx::rtx_lights[i].range = 500.0f;

							utils::vector::copy(game::cgs->predictedPlayerState.origin, rtx::rtx_lights[i].origin, 3);
							rtx::rtx_lights[i].origin[2] += 60.0f;

							if (rtx::rtx_lights[i].type == D3DLIGHT_SPOT)
							{
								utils::vector::angle_to_forward(game::cgs->predictedPlayerState.viewangles, rtx::rtx_lights[i].dir);
							}
						}

						// it was ... touched
						rtx::rtx_lights[i].virgin = false;
					}

					if (!rtx::rtx_lights[i].enable)
					{
						const char* LIGHT_TYPES[4] = { "None", "Point", "Spot", "Directional" };
						if (ImGui::SliderInt("Type", (int*)&rtx::rtx_lights[i].type, 1, 3, LIGHT_TYPES[rtx::rtx_lights[i].type]))
						{
							rtx::rtx_lights[i].virgin = true;
							on_edit = true;
						}
					}
					else
					{
						ImGui::SameLine();
						ImGui::TextUnformatted("  Turn off the light to change the light type.");
					}

					if (rtx::rtx_lights[i].enable)
					{
						if (rtx::rtx_lights[i].type != D3DLIGHT_DIRECTIONAL)
						{
							ImGui::DragFloat3("Position", rtx::rtx_lights[i].origin, 0.25f);
						}

						if (rtx::rtx_lights[i].type > D3DLIGHT_POINT)
						{
							on_edit = ImGui::DragFloat3("Direction", rtx::rtx_lights[i].dir, 0.05f) ? true : on_edit;

							if (rtx::rtx_lights[i].attach)
							{
								ImGui::DragFloat3("Direction Offset", rtx::rtx_lights[i].dir_offset, 0.05f);
							}
						}

						if (rtx::rtx_lights[i].type != D3DLIGHT_DIRECTIONAL)
						{
							on_edit = ImGui::DragFloat("Range", &rtx::rtx_lights[i].range, 0.25f) ? true : on_edit;
						}
						
						on_edit = ImGui::ColorEdit3("Color", rtx::rtx_lights[i].color, ImGuiColorEditFlags_Float) ? true : on_edit;
						on_edit = ImGui::DragFloat("Color Scale", &rtx::rtx_lights[i].color_scale, 0.1f) ? true : on_edit;

						if (rtx::rtx_lights[i].type != D3DLIGHT_DIRECTIONAL)
						{
							ImGui::Checkbox("Attach light to head", &rtx::rtx_lights[i].attach);

							if (ImGui::Button("Move light to player"))
							{
								utils::vector::copy(game::cgs->predictedPlayerState.origin, rtx::rtx_lights[i].origin, 3);
								rtx::rtx_lights[i].origin[2] += 60.0f;

								if (rtx::rtx_lights[i].type > D3DLIGHT_POINT)
								{
									utils::vector::angle_to_forward(game::cgs->predictedPlayerState.viewangles, rtx::rtx_lights[i].dir);
								}
							}
						}

						if (on_edit)
						{
							rtx::rtx_lights[i].type == D3DLIGHT_DIRECTIONAL
								? rtx::rtx_lights[i].dir[2] += 0.0001f
								: rtx::rtx_lights[i].origin[2] += 0.0001f;
						}
					}
				}
				ImGui::PopID();
			}

			ImGui::Indent(-8.0f); SPACING(0.0f, 4.0f);
		}

		if (ImGui::CollapsingHeader("LOD", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			const auto& r_lodScaleRigid = game::Dvar_FindVar("r_lodScaleRigid");
			const auto& r_lodBiasRigid = game::Dvar_FindVar("r_lodBiasRigid");
			const auto& r_lodScaleSkinned = game::Dvar_FindVar("r_lodScaleSkinned");
			const auto& r_lodBiasSkinned = game::Dvar_FindVar("r_lodBiasSkinned");
			ImGui::DragFloat("r_lodScaleRigid", &r_lodScaleRigid->current.value, 0.1f, 0.0f);
			ImGui::DragFloat("r_lodBiasRigid", &r_lodBiasRigid->current.value, 0.1f, 0.0f);
			ImGui::DragFloat("r_lodScaleSkinned", &r_lodScaleSkinned->current.value, 0.1f);
			ImGui::DragFloat("r_lodBiasSkinned", &r_lodBiasSkinned->current.value, 0.1f);

			//

			const auto& r_forceLod = game::Dvar_FindVar("r_forceLod");
			const char* force_lod_strings[] = { "High", "Medium", "Low", "Lowest", "None" };
			ImGui::SliderInt("Force LOD", &r_forceLod->current.integer, 0, 4, force_lod_strings[r_forceLod->current.integer]);
			ImGui::Checkbox("Force second lowest LOD", &dvars::r_forceLod_second_lowest->current.enabled); TT(dvars::r_forceLod_second_lowest->description);

			// no longer used
			//const auto& r_highLodDist = game::Dvar_FindVar("r_highLodDist");
			//const auto& r_mediumLodDist = game::Dvar_FindVar("r_mediumLodDist");
			//const auto& r_lowLodDist = game::Dvar_FindVar("r_lowLodDist");
			//const auto& r_lowestLodDist = game::Dvar_FindVar("r_lowestLodDist");
			//ImGui::DragFloat("r_highLodDist", &r_highLodDist->current.value, 0.1f, 0.0f);
			//ImGui::DragFloat("r_mediumLodDist", &r_mediumLodDist->current.value, 0.1f, 0.0f);
			//ImGui::DragFloat("r_lowLodDist", &r_lowLodDist->current.value, 0.1f);
			//ImGui::DragFloat("r_lowestLodDist", &r_lowestLodDist->current.value, 0.1f);

			ImGui::Indent(-8.0f); SPACING(0.0f, 4.0f);
		}
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
			var && var->current.value != 4.00195f)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, "r_znear 4.00195\n");
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
			var && var->current.enabled)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, "r_depthPrepass 0\n");
		}

		// ++ fps
		if (const auto var = game::Dvar_FindVar("r_multiGpu");
			var && var->current.enabled)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, "r_multiGpu 0\n");
		}

		if (const auto var = game::Dvar_FindVar("r_dof_enable");
			var && var->current.enabled)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, "r_dof_enable 0\n");
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
		// 4k hacks

		// set img alloc size from 0x600000 to 0x1000000
		utils::hook::set<BYTE>(0x641CC7 + 3, 0x00);
		utils::hook::set<BYTE>(0x641CC7 + 4, 0x01);

		// ^ cmp -> error if image larger
		utils::hook::set<BYTE>(0x641C89 + 3, 0x00);
		utils::hook::set<BYTE>(0x641C89 + 4, 0x01);

		// force fullbright (switching to fullbright in-game will result in wrong surface normals)
		utils::hook::nop(0x633259, 2);

		// set default technique to fakelight normal
		//utils::hook::set<BYTE>(0x633265 + 1, 0x18);

		// enable techniqueset loading
		//utils::hook::set<BYTE>(0x6AFBB4 + 24, 0x1); // fakelight normal
		//utils::hook::set<BYTE>(0x6AFBB4 + 25, 0x1); // fakelight view
		//utils::hook::set<BYTE>(0x6AFBB4 + 26, 0x1); // sunlight preview
		//utils::hook::set<BYTE>(0x6AFBB4 + 27, 0x1); // case texture

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

		// R_AddWorkerCmd :: disable dynEnt models (not needed)
		//utils::hook::nop(0x629328, 5);

		// removing 'R_AddAllBspDrawSurfacesRangeCamera' (decals) call @ 0x5F9E65 fixes cold/warm light transition on mp_crash (not needed with forced fullbright)
		//utils::hook::nop(0x5F9E65, 5); 

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
