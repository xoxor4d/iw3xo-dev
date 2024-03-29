#include "std_include.hpp"

namespace components
{
	void rtx_gui::gui()
	{
		ImGui::Indent(8.0f);

		if (ImGui::CollapsingHeader("General Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			const auto& fx_enable = game::Dvar_FindVar("fx_enable");
			ImGui::Checkbox("Enable FX", &fx_enable->current.enabled);

			const auto& r_drawdecals = game::Dvar_FindVar("r_drawdecals");
			ImGui::Checkbox("Enable Decals", &r_drawdecals->current.enabled);
			TT("Assign material categories 'Decal' and 'Dynamic Decal' to all decal surfaces\n"
			   "Tip: You can disable 'Stochachstic Alpha Blending' to select decals within the 'world'");

			const auto& rtx_hacks = game::Dvar_FindVar("rtx_hacks");
			ImGui::Checkbox("Skybox hack", &rtx_hacks->current.enabled);
			TT("Replaces the skybox with a placeholder texture that can be set as sky.\n"
			   "It`s recommended to keep this enabled!");

			const auto& rtx_extend_smodel_drawing = game::Dvar_FindVar("rtx_extend_smodel_drawing");
			ImGui::Checkbox("Increase static model limit", &rtx_extend_smodel_drawing->current.enabled);
			TT("The game has a hard limit on how many static models it can draw at once (warning print in console).\n"
			   "Enabling this setting will disable that limit but might cause instability.");

			if (dvars::rtx_disable_entity_culling)
			{
				ImGui::Checkbox("Disable Entity Culling", &dvars::rtx_disable_entity_culling->current.enabled);
				TT(dvars::rtx_disable_entity_culling->description);
			}

			if (dvars::rtx_disable_world_culling)
			{
				ImGui::SliderInt("World Culling", &dvars::rtx_disable_world_culling->current.integer, 0, 3,
												  rtx::rtx_disable_world_culling_enum[dvars::rtx_disable_world_culling->current.integer]);
				TT(dvars::rtx_disable_world_culling->description);
			}

#if DEBUG
			ImGui::DragInt("D3D Alpha Blend Setting", &rtx_gui::d3d_alpha_blend, 0.025f, 0, 16);
#endif

			ImGui::Indent(-8.0f); SPACING(0.0f, 4.0f);
		}

		if (ImGui::CollapsingHeader("Skybox", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			const auto sky_valid = skysphere_is_valid();
			constexpr auto BUTTON_WIDTH = 100.0f;

			static float skygroup01_width = 0.0f;
			gui::center_horz_begin(skygroup01_width);
			{
				if (ImGui::Button("Hill", ImVec2(BUTTON_WIDTH, 0)))
				{
					skysphere_spawn(SKY::CLEAR);
				}

				ImGui::SameLine();
				if (ImGui::Button("Desert", ImVec2(BUTTON_WIDTH, 0)))
				{
					skysphere_spawn(SKY::DESERT);
				}

				ImGui::SameLine();
				if (ImGui::Button("Night", ImVec2(BUTTON_WIDTH, 0)))
				{
					skysphere_spawn(SKY::NIGHT);
				}

				ImGui::SameLine();
				if (ImGui::Button("Night 2", ImVec2(BUTTON_WIDTH, 0)))
				{
					skysphere_spawn(SKY::NIGHT2);
				}

				gui::center_horz_end(skygroup01_width);
			}

			static float skygroup02_width = 0.0f;
			gui::center_horz_begin(skygroup02_width);
			{
				if (ImGui::Button("Overcast", ImVec2(BUTTON_WIDTH, 0)))
				{
					skysphere_spawn(SKY::OVERCAST);
				}

				ImGui::SameLine();
				if (ImGui::Button("Sunset", ImVec2(BUTTON_WIDTH, 0)))
				{
					skysphere_spawn(SKY::SUNSET);
				}

				ImGui::SameLine();
				if (ImGui::Button("Galaxy 1", ImVec2(BUTTON_WIDTH, 0)))
				{
					skysphere_spawn(SKY::GALAXY1);
				}

				ImGui::SameLine();
				if (ImGui::Button("Galaxy 2", ImVec2(BUTTON_WIDTH, 0)))
				{
					skysphere_spawn(SKY::GALAXY2);
				}

				gui::center_horz_end(skygroup02_width);
			}

			static float skygroup03_width = 0.0f;
			gui::center_horz_begin(skygroup03_width);
			{
				SPACING(4.0f, 0);

				if (!sky_valid)
				{
					ImGui::TextUnformatted("No valid sky found. Limited settings...");
				}

				ImGui::PushItemWidth(90.0f);
				ImGui::DragFloat("Sky Scale", &skysphere_scale, 0.01f, 1.0f, 10000.0f, "%.0f");
				ImGui::PopItemWidth();

				if (sky_valid)
				{
					if (dvars::rtx_sky_follow_player)
					{
						ImGui::SameLine(0, 30.0f);
						ImGui::Checkbox("Follow player", &dvars::rtx_sky_follow_player->current.enabled);
					}
				}
				gui::center_horz_end(skygroup03_width);
			}

			if (sky_valid)
			{
				SPACING(4.0f, 0);

				if (ImGui::DragFloat3("Sphere Origin", rtx_gui::skysphere_model_origin, 1.0f, -360.0f, 360.0f, "%.2f"))
				{
					rtx_gui::skysphere_update_pos();
				}

				if (ImGui::DragFloat3("Sphere Rotation", skysphere_model_rotation, 0.25f, -FLT_MAX, FLT_MAX, "%.2f"))
				{
					skysphere_update_pos();
				}

				//ImGui::SameLine();
				ImGui::Checkbox("Auto Rotation", &skysphere_auto_rotation);
				ImGui::SameLine();

				ImGui::PushItemWidth(90.0f);
				ImGui::DragFloat("Speed", &skysphere_auto_rotation_speed, 0.01f, 0.01f, 10.0f, "%.2f");
				ImGui::PopItemWidth();
			}

			if (dvars::rtx_sky_hacks)
			{
				if (ImGui::CollapsingHeader("Additional Sky Marking"))
				{
					if (ImGui::Checkbox("Enable Additional Sky Marking", &dvars::rtx_sky_hacks->current.enabled))
					{
						dvars::rtx_sky_hacks->latched.enabled = dvars::rtx_sky_hacks->current.enabled;
						dvars::rtx_sky_hacks->modified = true;
					} TT("This can be used to tell iw3xo about any custom skies it might not have picked up for replacement");

					if (dvars::rtx_sky_hacks->current.enabled)
					{
						SPACING(0.0f, 4.0f);

						ImGui::Text("Map Material List");
						ImGui::InputTextMultiline("##map_mat_list", (char*)map_materials.c_str(), map_materials.size(), ImVec2(ImGui::GetWindowWidth() - 54.0f, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly);

						
						if (ImGui::Button("Refresh List", ImVec2(ImGui::GetWindowWidth() - 54.0f, 30.0f)) || map_materials.empty())
						{
							map_materials_update();
						}

						SPACING(0.0f, 4.0f);

						if (dvars::rtx_sky_materials)
						{
							static char edit_buffer[1024] = {};
							static bool edit_buffer_update = *dvars::rtx_sky_materials->current.string;
							static bool edit_buffer_update_init = false;

							ImGui::Text("Sky Materials (editable). Format <string string string ...>");
							ImGui::InputTextMultiline("##edit_mats", edit_buffer, IM_ARRAYSIZE(edit_buffer), ImVec2(ImGui::GetWindowWidth() - 54.0f, ImGui::GetTextLineHeight() * 3));

							if ((ImGui::Button("Refresh") && edit_buffer_update) || (!edit_buffer_update_init && edit_buffer_update))
							{
								edit_buffer_update_init = true;
								memset(&edit_buffer, 0, IM_ARRAYSIZE(edit_buffer));
								memcpy(&edit_buffer, dvars::rtx_sky_materials->current.string, strlen(dvars::rtx_sky_materials->current.string));
							}

							ImGui::SameLine();
							if (ImGui::Button("Apply"))
							{
								CMDEXEC(utils::va("set rtx_sky_materials %s", edit_buffer));
								rtx::sky_material_update(edit_buffer);
							}
						}
					}
				}
			}

			ImGui::Indent(-8.0f); SPACING(0.0f, 4.0f);
		}

		if (ImGui::CollapsingHeader("Debug Light", ImGuiTreeNodeFlags_None))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			for (auto i = 0; i < rtx_lights::RTX_DEBUGLIGHT_AMOUNT; i++)
			{
				ImGui::PushID(i);

				if (ImGui::CollapsingHeader(utils::va("Light %d", i), !i ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None))
				{
					bool on_edit = false;

					if (ImGui::Checkbox("Spawn light", &rtx_lights::rtx_debug_lights[i].enable))
					{
						on_edit = true;

						// default light settings (spawn on player when untouched)
						if (rtx_lights::rtx_debug_lights[i].virgin)
						{
							rtx_lights::rtx_debug_lights[i].color_scale = 3.0f;
							rtx_lights::rtx_debug_lights[i].range = 500.0f;

							utils::vector::copy(game::cgs->predictedPlayerState.origin, rtx_lights::rtx_debug_lights[i].origin, 3);
							rtx_lights::rtx_debug_lights[i].origin[2] += 60.0f;

							if (rtx_lights::rtx_debug_lights[i].type == D3DLIGHT_SPOT)
							{
								utils::vector::angle_to_forward(game::cgs->predictedPlayerState.viewangles, rtx_lights::rtx_debug_lights[i].dir);
							}
						}

						// it was ... touched
						rtx_lights::rtx_debug_lights[i].virgin = false;
					}

					if (!rtx_lights::rtx_debug_lights[i].enable)
					{
						const char* LIGHT_TYPES[4] = { "None", "Point", "Spot", "Directional" };
						if (ImGui::SliderInt("Type", (int*)&rtx_lights::rtx_debug_lights[i].type, 1, 3, LIGHT_TYPES[rtx_lights::rtx_debug_lights[i].type]))
						{
							rtx_lights::rtx_debug_lights[i].virgin = true;
							on_edit = true;
						}
					}
					else
					{
						ImGui::SameLine();
						ImGui::TextUnformatted("  Turn off the light to change the light type.");
					}

					if (rtx_lights::rtx_debug_lights[i].enable)
					{
						if (rtx_lights::rtx_debug_lights[i].type != D3DLIGHT_DIRECTIONAL)
						{
							ImGui::DragFloat3("Position", rtx_lights::rtx_debug_lights[i].origin, 0.25f);

							game::vec2_t screen_pos = {};
							if (utils::world_to_screen(rtx_lights::rtx_debug_lights[i].origin, screen_pos))
							{
								ImGui::GetOverlayDrawList()->AddCircle(ImVec2(screen_pos[0], screen_pos[1]), 6.0f, IM_COL32(0, 255, 0, 255), 8, 4.0f);

								if (rtx_lights::rtx_debug_lights[i].type == D3DLIGHT_SPOT)
								{
									game::vec3_t dir_normalized = {};
									game::vec2_t screen_pos_dir = {};
									game::vec3_t pos_in_direction = {};

									utils::vector::normalize_to(rtx_lights::rtx_debug_lights[i].dir, dir_normalized);
									utils::vector::multiply_add(rtx_lights::rtx_debug_lights[i].origin, 20.0f, dir_normalized, pos_in_direction);

									if (utils::world_to_screen(pos_in_direction, screen_pos_dir))
									{
										ImGui::GetOverlayDrawList()->AddLine(
											ImVec2(screen_pos[0], screen_pos[1]),
											ImVec2(screen_pos_dir[0], screen_pos_dir[1]),
											IM_COL32(0, 128, 255, 255), 4.0f);
									}

									const auto draw_spotlight_debug_circle = [](const rtx_lights::rtx_debug_light* light, const float distance, std::uint8_t r, std::uint8_t g, std::uint8_t b, const bool draw_inner = true)
									{
										game::vec3_t dn = {};
										game::vec3_t pos = {};

										utils::vector::normalize_to(light->dir, dn);
										utils::vector::multiply_add(light->origin, distance, dn, pos);

										game::vec2_t screen_pos = {};
										if (utils::world_to_screen(pos, screen_pos))
										{
											const float lenx = utils::vector::distance3(game::cgs->predictedPlayerState.origin, pos);

											if (draw_inner)
											{
												// inner angle
												ImGui::GetOverlayDrawList()->AddCircle(
													ImVec2(screen_pos[0], screen_pos[1]),
													((distance * 5.0f * tanf(utils::vector::deg_to_rad(light->inner_angle * 0.5f))) / (lenx * 0.0055f)),
													IM_COL32(255, 128, 0, 255), 8, 4.0f);
											}

											// outer angle
											ImGui::GetOverlayDrawList()->AddCircle(
												ImVec2(screen_pos[0], screen_pos[1]),
												((distance * 5.0f * tanf(utils::vector::deg_to_rad(light->outer_angle * 0.5f))) / (lenx * 0.0055f)),
												IM_COL32(r, g, b, 255), 8, 4.0f);
										}
									};

									draw_spotlight_debug_circle(&rtx_lights::rtx_debug_lights[i], 20.0f * 1.00f, 0, 255, 0, true);
									//draw_spotlight_debug_circle(&rtx_lights[i], 20.0f * 0.95f, 0, 200, 0, false);
								}
							}
						}

						if (rtx_lights::rtx_debug_lights[i].type > D3DLIGHT_POINT)
						{
							on_edit = ImGui::DragFloat3("Direction", rtx_lights::rtx_debug_lights[i].dir, 0.05f) ? true : on_edit;

							if (rtx_lights::rtx_debug_lights[i].attach_to_head)
							{
								on_edit = ImGui::DragFloat3("Direction Offset", rtx_lights::rtx_debug_lights[i].dir_offset, 0.05f) ? true : on_edit;
							}
						}

						if (rtx_lights::rtx_debug_lights[i].type != D3DLIGHT_DIRECTIONAL)
						{
							if (rtx_lights::rtx_debug_lights[i].type == D3DLIGHT_SPOT)
							{
								on_edit = ImGui::DragFloat("Inner Angle", &rtx_lights::rtx_debug_lights[i].inner_angle, 0.25f) ? true : on_edit;
								on_edit = ImGui::DragFloat("Outer Angle", &rtx_lights::rtx_debug_lights[i].outer_angle, 0.25f) ? true : on_edit;
							}

							on_edit = ImGui::DragFloat("Range", &rtx_lights::rtx_debug_lights[i].range, 0.25f) ? true : on_edit;
						}

						on_edit = ImGui::ColorEdit3("Color", rtx_lights::rtx_debug_lights[i].color, ImGuiColorEditFlags_Float) ? true : on_edit;
						on_edit = ImGui::DragFloat("Color Scale", &rtx_lights::rtx_debug_lights[i].color_scale, 0.1f) ? true : on_edit;

						if (rtx_lights::rtx_debug_lights[i].type != D3DLIGHT_DIRECTIONAL)
						{
							if (ImGui::Checkbox("Attach light to head", &rtx_lights::rtx_debug_lights[i].attach_to_head))
							{
								rtx_lights::rtx_debug_lights[i].attach_to_weapon = false;
							}

							ImGui::SameLine(0, 20.0f);
							if (ImGui::Checkbox("Attach light to weapon", &rtx_lights::rtx_debug_lights[i].attach_to_weapon))
							{
								rtx_lights::rtx_debug_lights[i].attach_to_head = false;
							}

							if (ImGui::Button("Move light to player"))
							{
								utils::vector::copy(game::cgs->predictedPlayerState.origin, rtx_lights::rtx_debug_lights[i].origin, 3);
								rtx_lights::rtx_debug_lights[i].origin[2] += 60.0f;

								if (rtx_lights::rtx_debug_lights[i].type > D3DLIGHT_POINT)
								{
									utils::vector::angle_to_forward(game::cgs->predictedPlayerState.viewangles, rtx_lights::rtx_debug_lights[i].dir);
								}
							}
						}

						if (on_edit)
						{
							switch (rtx_lights::rtx_debug_lights[i].type)
							{
							default:
							case D3DLIGHT_POINT:
								rtx_lights::rtx_debug_lights[i].origin[2] += 0.0001f; break;

							case D3DLIGHT_SPOT:
								rtx_lights::rtx_debug_lights[i].outer_angle += 0.0001f; break;

							case D3DLIGHT_DIRECTIONAL:
								rtx_lights::rtx_debug_lights[i].dir[2] += 0.0001f; break;
							}
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

	// ---------------------------------------------------------------------
	// #####################################################################
	// ---------------------------------------------------------------------

	void rtx_gui::map_materials_update()
	{
		map_materials.clear();

		for (auto num = 0u; num < game::cm->numMaterials; num++)
		{
			if (game::cm->materials[num].material[0])
			{
				map_materials += std::to_string(num) + ": " + std::string(game::cm->materials[num].material) + "\n";
			}
		}
	}

	// called from rtx::setup_rtx()
	void rtx_gui::skysphere_frame()
	{
		if (skysphere_is_valid())
		{
			if (dvars::rtx_sky_follow_player && dvars::rtx_sky_follow_player->current.enabled)
			{
				utils::vector::copy(game::cgs->predictedPlayerState.origin, rtx_gui::skysphere_model_origin, 3);
				rtx_gui::skysphere_update_pos();
			}

			if (skysphere_auto_rotation)
			{
				if (skysphere_model_rotation[1] >= 360.0f)
				{
					skysphere_model_rotation[1] = 0.0f;
				}

				const auto timescale = game::Dvar_FindVar("timescale")->current.value;

				if (game::clc.demoplaying)
				{
					skysphere_model_rotation[1] += (float)game::cgs->frametime * 0.0001f * skysphere_auto_rotation_speed * timescale;
				}
				else
				{
					skysphere_model_rotation[1] += (float)game::glob::lpmove_server_frame_time * 0.0001f * skysphere_auto_rotation_speed * timescale;
					game::G_SetAngles(skysphere_model, skysphere_model_rotation);
				}
			}
		}
	}

	const char* rtx_gui::skysphere_get_name_for_variant(std::uint32_t variant)
	{
		if (variant > SKY::COUNT)
		{
			return SKY_STRINGS[SKY::SUNSET];
		}

		return SKY_STRINGS[variant];
	}

	bool rtx_gui::skysphere_is_valid()
	{
		if (game::clc.demoplaying)
		{
			if (skysphere_model_fx)
			{
				return true;
			}

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

	void rtx_gui::skysphere_update_pos()
	{
		if (game::clc.demoplaying && skysphere_model_fx)
		{
			game::vec4_t quat = {};
			game::vec3_t axis[3] = {};
			utils::vector::angles_to_axis(skysphere_model_rotation, axis);
			utils::vector::axis_to_quat(axis, quat);

			utils::vector::copy(game::clients->cgameOrigin, skysphere_model_fx->frameAtSpawn.origin, 4);
			utils::vector::copy(quat,						skysphere_model_fx->frameAtSpawn.quat, 4);

			utils::vector::copy(game::clients->cgameOrigin, skysphere_model_fx->frameNow.origin, 4);
			utils::vector::copy(quat,						skysphere_model_fx->frameNow.quat, 4);

			return;
		}

		if (skysphere_spawned)
		{
			skysphere_model->r.svFlags = 0x04; // visible
			game::G_SetOrigin(skysphere_model, skysphere_model_origin);
			game::G_SetAngles(skysphere_model, skysphere_model_rotation);
		}
	}

	void rtx_gui::skysphere_toggle_vis()
	{
		if (rtx_gui::skysphere_is_valid())
		{
			skysphere_model->r.svFlags = skysphere_model->r.svFlags == 0x04 ? 0x01 : 0x04;
			game::G_SetOrigin(skysphere_model, skysphere_model_origin);
		}
	}

	void rtx_gui::skysphere_change_model(int variant)
	{
		const std::int16_t model_index = game::G_ModelIndex(skysphere_get_name_for_variant(variant));

		skysphere_model->model = model_index;
		skysphere_model->s.index = model_index;

		skysphere_variant = variant;
	}

	void rtx_gui::skysphere_spawn_fx(int variant)
	{
		const auto fx = game::DB_FindXAssetHeader(game::XAssetType::ASSET_TYPE_FX, rtx_gui::skysphere_get_name_for_variant(variant)).fx;

		if (fx)
		{
			game::vec3_t axis[3] = {};
			utils::vector::angles_to_axis(skysphere_model_rotation, axis);
			skysphere_model_fx = game::FX_SpawnOrientedEffect(axis[0], fx, 0, skysphere_model_origin);
		}
	}

	// rtx_gui::SKY enum
	void rtx_gui::skysphere_spawn(int variant)
	{
		if (game::clc.demoplaying)
		{
			if (skysphere_model_fx)
			{
				game::FX_KillEffect(skysphere_model_fx);
			}

			skysphere_spawn_fx(variant);
			return;
		}

		// #
		// #

		if (skysphere_is_valid())
		{
			skysphere_change_model(variant);
			return;
		}

		const std::int16_t model_index = game::G_ModelIndex(skysphere_get_name_for_variant(variant));

		skysphere_model = game::G_Spawn();
		skysphere_model->classname = game::scr_const->script_model;
		skysphere_model->model = model_index;
		skysphere_model->s.index = model_index;
		skysphere_model->r.svFlags = 0x04;

		game::G_SetOrigin(skysphere_model, skysphere_model_origin);
		game::G_SetAngles(skysphere_model, skysphere_model_rotation);

		game::G_CallSpawnEntity(skysphere_model);

		skysphere_spawned = true;
		skysphere_variant = variant;
	}

	void rtx_gui::skysphere_reset()
	{
		skysphere_spawned = false;
		skysphere_model = nullptr;
		skysphere_model_fx = nullptr;
	}

	rtx_gui::rtx_gui()
	{ }
}