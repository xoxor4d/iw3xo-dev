#include "STDInclude.hpp"

// show tooltip after x seconds
#define TTDELAY 1.0f 

// tooltip with delay
#define TT(tooltip) if (ImGui::IsItemHoveredDelay(TTDELAY)) { ImGui::SetTooltip(tooltip); }

// spacing dummy
#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 

// seperator with spacing
#define SEPERATORV(spacing) SPACING(0.0f, spacing); ImGui::Separator(); SPACING(0.0f, spacing); 

// execute a single command
#define CMDEXEC(command) Game::Cmd_ExecuteSingleCommand(0, 0, command)

namespace components
{
	// *
	// the "main" menu
	void gui_devgui::create_devgui(Game::gui_menus_t& menu)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg].w = dvars::_imgui_window_alpha->current.value;
		style.WindowMinSize.x = 580.0f;
		style.WindowMinSize.y = 400.0f;

		ImGuiIO& io = ImGui::GetIO();
		io.WantCaptureKeyboard = true;
		io.WantCaptureMouse = true;

		if (dvars::_imgui_devgui_pos && dvars::_imgui_devgui_size)
		{
			const auto savedpos = dvars::_imgui_devgui_pos->current.vector;
			const auto savedsize = dvars::_imgui_devgui_size->current.vector;

			// set window size once
			if (!menu.one_time_init)
			{
				// cap size
				if (savedsize[0] > io.DisplaySize.x)
				{
					savedsize[0] = io.DisplaySize.x; dvars::_imgui_devgui_size->modified = true;
				}

				if (savedsize[1] > io.DisplaySize.y)
				{
					savedsize[1] = io.DisplaySize.y; dvars::_imgui_devgui_size->modified = true;
				}

				// cap position
				if (savedpos[0] + savedsize[0] > io.DisplaySize.x)
				{
					savedpos[0] = io.DisplaySize.x - savedsize[0]; dvars::_imgui_devgui_pos->modified = true;
				}

				if (savedpos[1] + savedsize[1] > io.DisplaySize.y)
				{
					savedpos[1] = io.DisplaySize.y - savedsize[1]; dvars::_imgui_devgui_pos->modified = true;
				}

				ImGui::SetNextWindowPos(ImVec2(savedpos[0], savedpos[1]), ImGuiCond_FirstUseEver);
				ImGui::SetNextWindowSize(ImVec2(savedsize[0], savedsize[1]), ImGuiCond_FirstUseEver);
			}

			// check if window position has changed and update dvar
			if (menu.position[0] != savedpos[0] || menu.position[1] != savedpos[1])
			{
				utils::vector::_VectorCopy(menu.position, dvars::_imgui_devgui_pos->current.vector, 2);
				utils::vector::_VectorCopy(menu.position, dvars::_imgui_devgui_pos->latched.vector, 2);
				dvars::_imgui_devgui_pos->modified = true;
			}

			// check if window size has changed and update dvar
			if (menu.size[0] != savedsize[0] || menu.size[1] != savedsize[1])
			{
				utils::vector::_VectorCopy(menu.size, dvars::_imgui_devgui_size->current.vector, 2);
				utils::vector::_VectorCopy(menu.size, dvars::_imgui_devgui_size->latched.vector, 2);
				dvars::_imgui_devgui_size->modified = true;
			}
		}

		// early out if the window is collapsed, as an optimization.
		if (!ImGui::Begin("Devgui", &menu.menustate, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		

		if (ImGui::BeginTabBar("devgui_tabs"))
		{
			// save current window parms
			menu.position[0] = ImGui::GetWindowPos().x;
			menu.position[1] = ImGui::GetWindowPos().y;
			menu.size[0] = ImGui::GetWindowWidth();
			menu.size[1] = ImGui::GetWindowHeight();

			if (const auto& cl_ingame = Game::Dvar_FindVar("cl_ingame"); 
							cl_ingame && cl_ingame->current.enabled)
			{
				if ((components::active.cgaz || components::active.compass || components::active.movement || components::active._cg || components::active._pmove) && ImGui::BeginTabItem("Movement"))
				{
					gui_devgui::menu_tab_movement(menu);
					ImGui::EndTabItem();
				}

				if (components::active.draw_collision && ImGui::BeginTabItem("Collision/Export"))
				{
					gui_devgui::menu_tab_collision(menu);
					ImGui::EndTabItem();
				}

				if (components::active.RB_ShaderOverlays && ImGui::BeginTabItem("Shaders"))
				{
					gui_devgui::menu_tab_shaders(menu);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Visuals"))
				{
					gui_devgui::menu_tab_visuals(menu);
					ImGui::EndTabItem();
				}

				if (components::active.ocean && ImGui::BeginTabItem("Ocean"))
				{
					ocean::devgui_tab(menu);
					ImGui::EndTabItem();
				}

				if (components::active.daynight_cycle && ImGui::BeginTabItem("DayNight"))
				{
					daynight_cycle::devgui_tab(menu);
					ImGui::EndTabItem();
				}
			}

			if (components::active.radiant_livelink && ImGui::BeginTabItem("Radiant"))
			{
				radiant_livelink::devgui_tab(menu);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Settings"))
			{
				gui_devgui::menu_tab_settings(menu);
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	// *
	// movement tab
	void gui_devgui::menu_tab_movement(Game::gui_menus_t& menu)
	{
		if (components::active.movement)
		{
			// *
			if (ImGui::CollapsingHeader("Presets", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

				if (ImGui::Button("Stock movement")) { CMDEXEC("pm_preset_stock"); } TT("pm_preset_stock");

				ImGui::SameLine();
				if (ImGui::Button("Quake movement")) { CMDEXEC("pm_preset_q3"); } TT("pm_preset_q3");

				ImGui::SameLine();
				if (ImGui::Button("Source movement")) { CMDEXEC("pm_preset_cs"); } TT("pm_preset_cs");

				SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
			}

			// *
			if (ImGui::CollapsingHeader("Movement Tweaks", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

				const char* movementType_items[] = { "Stock", "Quake", "Source" };
				ImGui::Combo("Movement Type", &dvars::pm_movementType->current.integer, movementType_items, IM_ARRAYSIZE(movementType_items)); TT("pm_movementType");

				SPACING(0.0f, 4.0f);

				// *
				if (ImGui::BeginTabBar("movement_tabs"))
				{
					// *
					if (ImGui::BeginTabItem("General"))
					{
						ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

						ImGui::Checkbox("Bhop Auto", &dvars::pm_bhop_auto->current.enabled); TT("pm_bhop_auto");

						ImGui::SameLine();
						ImGui::Checkbox("Bhop Slowdown", &dvars::pm_bhop_slowdown->current.enabled); TT("pm_bhop_slowdown, disable landing induced velocity decreases");

						if (const auto& jump_slowdownenable = Game::Dvar_FindVar("jump_slowdownenable"); 
										jump_slowdownenable)
						{
							ImGui::SameLine();
							ImGui::Checkbox("Jump Slowdown", &jump_slowdownenable->current.enabled); TT("jump_slowdownenable");
						}

						ImGui::Checkbox("Crashland", &dvars::pm_crashland->current.enabled); TT("pm_crashland, completely disable the crashland function and its side-effects");

						ImGui::SameLine();
						ImGui::Checkbox("Terrain Edge Bounces", &dvars::pm_terrainEdgeBounces->current.enabled); TT("pm_terrainEdgeBounces, ability to bounce on terrain edges");

						if (dvars::pm_movementType && dvars::pm_movementType->current.integer == 1) // Only avail. if quake movement is used
						{
							ImGui::SameLine();
							ImGui::Checkbox("Disable Sprint", &dvars::pm_disableSprint->current.enabled); TT("pm_disableSprint");
						}

						SPACING(0.0f, 4.0f);

						ImGui::DragFloat("Rocketjump Knockback", &dvars::pm_rocketJumpHeight->current.value, 1.0f, dvars::pm_rocketJumpHeight->domain.value.min, dvars::pm_rocketJumpHeight->domain.value.max, "%.0f"); TT("pm_rocketJumpHeight");
				
						SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
						ImGui::EndTabItem();
					}

					// *
					if (ImGui::BeginTabItem("Quake"))
					{
						ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

						ImGui::SliderFloat("Airstop Accelerate", &dvars::pm_cpm_airstopAccelerate->current.value, dvars::pm_cpm_airstopAccelerate->domain.value.min, dvars::pm_cpm_airstopAccelerate->domain.value.max, "%.0f"); TT("pm_cpm_airstopAccelerate");
						ImGui::SliderFloat("Strafe Accelerate", &dvars::pm_cpm_strafeAccelerate->current.value, dvars::pm_cpm_strafeAccelerate->domain.value.min, dvars::pm_cpm_strafeAccelerate->domain.value.max, "%.0f"); TT("pm_cpm_strafeAccelerate");
						ImGui::SliderFloat("Air Accelerate", &dvars::pm_cpm_airAccelerate->current.value, dvars::pm_cpm_airAccelerate->domain.value.min, dvars::pm_cpm_airAccelerate->domain.value.max, "%.0f"); TT("pm_cpm_airAccelerate");
						ImGui::SliderFloat("Air Control", &dvars::pm_cpm_airControl->current.value, dvars::pm_cpm_airControl->domain.value.min, dvars::pm_cpm_airControl->domain.value.max, "%.0f"); TT("pm_cpm_airControl");
						ImGui::SliderFloat("Damage Knockback", &dvars::pm_cpm_damageKnockback->current.value, dvars::pm_cpm_damageKnockback->domain.value.min, dvars::pm_cpm_damageKnockback->domain.value.max, "%.0f"); TT("pm_cpm_damageKnockback");

						SPACING(0.0f, 4.0f);

						ImGui::Checkbox("Quake Damage", &dvars::pm_cpm_useQuakeDamage->current.enabled); TT("pm_cpm_useQuakeDamage");

						ImGui::SameLine();
						ImGui::Checkbox("Enable Bouncing", &dvars::pm_cpm_useBouncing->current.enabled); TT("pm_cpm_useBouncing");

						SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
						ImGui::EndTabItem();
					}

					// *
					if (ImGui::BeginTabItem("Source"))
					{
						ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

						ImGui::SliderFloat("Air Accelerate", &dvars::pm_cs_airAccelerate->current.value, dvars::pm_cs_airAccelerate->domain.value.min, dvars::pm_cs_airAccelerate->domain.value.max, "%.0f"); TT("pm_cs_airAccelerate");
						ImGui::SliderFloat("Airspeed Cap", &dvars::pm_cs_airspeedCap->current.value, dvars::pm_cs_airspeedCap->domain.value.min, dvars::pm_cs_airspeedCap->domain.value.max, "%.0f"); TT("pm_cs_airspeedCap");

						SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
						ImGui::EndTabItem();
					}

					ImGui::EndTabBar();
				}

				SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
			}
		}

		// *
		if (ImGui::CollapsingHeader("Debug"))
		{
			const char* hudFont_items[] = { "FONT_SMALL_DEV", "FONT_BIG_DEV", "FONT_CONSOLE", "FONT_BIG", "FONT_SMALL", "FONT_BOLD", "FONT_NORMAL", "FONT_EXTRA_BIG", "FONT_OBJECTIVE" };

			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);
			if (components::active.movement && components::active._cg)
			{
				// ---------------

				ImGui::Checkbox("Enable Hud", gui::dvar_get_set<bool*>(dvars::pm_hud_enable)); TT("pm_hud_enable");
				ImGui::SliderFloat("Hud Position X", gui::dvar_get_set<float*>(dvars::pm_hud_x), dvars::pm_hud_x->domain.value.min, dvars::pm_hud_x->domain.value.max, "%.0f"); TT("pm_hud_x");
				ImGui::SliderFloat("Hud Position Y", gui::dvar_get_set<float*>(dvars::pm_hud_y), dvars::pm_hud_y->domain.value.min, dvars::pm_hud_y->domain.value.max, "%.0f"); TT("pm_hud_y");
				ImGui::SliderFloat("Hud Font Scale", gui::dvar_get_set<float*>(dvars::pm_hud_fontScale), dvars::pm_hud_fontScale->domain.value.min, dvars::pm_hud_fontScale->domain.value.max, "%.1f"); TT("pm_hud_fontScale");
				ImGui::ColorEdit4("Hud Font Color", gui::dvar_get_set<float*>(dvars::pm_hud_fontColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("pm_hud_fontColor");
				ImGui::Combo("Hud Font", gui::dvar_get_set<int*>(dvars::pm_hud_fontStyle), hudFont_items, IM_ARRAYSIZE(hudFont_items)); TT("pm_hud_fontStyle");

				// ---------------
				SEPERATORV(4.0f);
			}

			if (components::active._cg)
			{
				ImGui::Checkbox("Enable Position Hud", gui::dvar_get_set<bool*>(dvars::pm_origin_hud)); TT("pm_origin_hud");
				ImGui::SliderFloat("Origin Hud Position X", gui::dvar_get_set<float*>(dvars::pm_origin_hud_x), dvars::pm_origin_hud_x->domain.value.min, dvars::pm_origin_hud_x->domain.value.max, "%.0f"); TT("pm_origin_hud_x");
				ImGui::SliderFloat("Origin Hud Position Y", gui::dvar_get_set<float*>(dvars::pm_origin_hud_y), dvars::pm_origin_hud_y->domain.value.min, dvars::pm_origin_hud_y->domain.value.max, "%.0f"); TT("pm_origin_hud_y");
				ImGui::SliderFloat("Origin Hud Font Scale", gui::dvar_get_set<float*>(dvars::pm_origin_hud_fontScale), dvars::pm_origin_hud_fontScale->domain.value.min, dvars::pm_origin_hud_fontScale->domain.value.max, "%.1f"); TT("pm_origin_hud_fontScale");
				ImGui::ColorEdit4("Origin Hud Font Color", gui::dvar_get_set<float*>(dvars::pm_origin_hud_fontColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("pm_origin_hud_fontColor");
				ImGui::Combo("Hud Font", gui::dvar_get_set<int*>(dvars::pm_origin_hud_fontStyle), hudFont_items, IM_ARRAYSIZE(hudFont_items)); TT("pm_origin_hud_fontStyle");

				// ---------------
				SEPERATORV(4.0f);
			}

			if (components::active.compass)
			{
				ImGui::Checkbox("Enable mDd world compass", gui::dvar_get_set<bool*>(dvars::mdd_compass)); TT("mdd_compass");
				ImGui::SliderFloat2("Compass position and thickness", gui::dvar_get_set<float*>(dvars::mdd_compass_yh), dvars::mdd_compass_yh->domain.value.min, dvars::mdd_compass_yh->domain.value.max, "%.0f"); TT("mdd_compass_yh");
				ImGui::ColorEdit4("Color 0 to 90 quadrant", gui::dvar_get_set<float*>(dvars::mdd_compass_quadrant_rgbas0), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("mdd_compass_quadrant_rgbas0");
				ImGui::ColorEdit4("Color 90 to 180 quadrant", gui::dvar_get_set<float*>(dvars::mdd_compass_quadrant_rgbas1), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("mdd_compass_quadrant_rgbas1");
				ImGui::ColorEdit4("Color 180 to 270 quadrant", gui::dvar_get_set<float*>(dvars::mdd_compass_quadrant_rgbas2), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("mdd_compass_quadrant_rgbas2");
				ImGui::ColorEdit4("Color 270 to 360 quadrant", gui::dvar_get_set<float*>(dvars::mdd_compass_quadrant_rgbas3), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("mdd_compass_quadrant_rgbas3");
				ImGui::ColorEdit4("Color for ticks", gui::dvar_get_set<float*>(dvars::mdd_compass_ticks_rgba), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("mdd_compass_ticks_rgba");

				// ---------------
				SEPERATORV(4.0f);
			}

			if (components::active.cgaz)
			{
				ImGui::Checkbox("Display mDd CampingGaz HUD", gui::dvar_get_set<bool*>(dvars::mdd_cgaz)); TT("mdd_cgaz");
				ImGui::Checkbox("Show true ground zones", gui::dvar_get_set<bool*>(dvars::mdd_cgaz_ground)); TT("mdd_cgaz_ground");
				ImGui::SliderInt("Min speed value for draw CGaz", gui::dvar_get_set<int*>(dvars::mdd_cgaz_min_speed), dvars::mdd_cgaz_min_speed->domain.integer.min, dvars::mdd_cgaz_min_speed->domain.integer.max); TT("mdd_cgaz_min_speed");
				ImGui::SliderFloat2("CGaz position and thickness", gui::dvar_get_set<float*>(dvars::mdd_cgaz_yh), dvars::mdd_cgaz_yh->domain.value.min, dvars::mdd_cgaz_yh->domain.value.max, "%.0f"); TT("mdd_cgaz_yh");
				ImGui::ColorEdit4("Color for no accel zone", gui::dvar_get_set<float*>(dvars::mdd_cgaz_rgbaNoAccel), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("mdd_cgaz_rgbaNoAccel");
				ImGui::ColorEdit4("Color for partial accel zone", gui::dvar_get_set<float*>(dvars::mdd_cgaz_rgbaPartialAccel), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("mdd_cgaz_rgbaPartialAccel");
				ImGui::ColorEdit4("Color for full accel zone", gui::dvar_get_set<float*>(dvars::mdd_cgaz_rgbaFullAccel), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("mdd_cgaz_rgbaFullAccel");
				ImGui::ColorEdit4("Color for turn zone", gui::dvar_get_set<float*>(dvars::mdd_cgaz_rgbaTurnZone), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("mdd_cgaz_rgbaTurnZone");

				// ---------------
				SEPERATORV(4.0f);
			}

			if (components::active._pmove)
			{
				const char* drawAxis_items[] = { "disabled", "axis only", "fps zone circle" };
				ImGui::Combo("Draw Axis", &dvars::pm_debug_drawAxis->current.integer, drawAxis_items, IM_ARRAYSIZE(drawAxis_items)); TT("pm_debug_drawAxis");

				ImGui::SliderFloat("Radius", gui::dvar_get_set<float*>(dvars::pm_debug_drawAxis_radius), dvars::pm_debug_drawAxis_radius->domain.value.min, dvars::pm_debug_drawAxis_radius->domain.value.max, "%.0f"); TT("pm_debug_drawAxis_radius");
				ImGui::SliderFloat("Height", gui::dvar_get_set<float*>(dvars::pm_debug_drawAxis_height), dvars::pm_debug_drawAxis_height->domain.value.min, dvars::pm_debug_drawAxis_height->domain.value.max, "%.0f"); TT("pm_debug_drawAxis_height");

				SPACING(0.0f, 4.0f);

				ImGui::ColorEdit4("125 FPS Zone", gui::dvar_get_set<float*>(dvars::pm_debug_drawAxis_col125), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("pm_debug_drawAxis_col125");
				ImGui::ColorEdit4("250 FPS Zone", gui::dvar_get_set<float*>(dvars::pm_debug_drawAxis_col250), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("pm_debug_drawAxis_col250");
				ImGui::ColorEdit4("333 FPS Zone", gui::dvar_get_set<float*>(dvars::pm_debug_drawAxis_col333), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("pm_debug_drawAxis_col333");

				// ---------------
				SEPERATORV(4.0f);

				const char* traceOrigin_items[] = { "disabled", "trace and display for duration", "trace and display for duration when not on the ground" };
				ImGui::Combo("Trace Origin", &dvars::pm_debug_traceOrigin->current.integer, traceOrigin_items, IM_ARRAYSIZE(traceOrigin_items)); TT("pm_debug_traceOrigin");

				const char* traceVelocity_items[] = { "disabled", "trace and display for duration", "trace and display for duration when not on the ground", "trace in realtime" };
				ImGui::Combo("Trace Velocity", &dvars::pm_debug_traceVelocity->current.integer, traceVelocity_items, IM_ARRAYSIZE(traceVelocity_items)); TT("pm_debug_traceVelocity");

				ImGui::SliderInt("Line Width", gui::dvar_get_set<int*>(dvars::pm_debug_lineWidth), dvars::pm_debug_lineWidth->domain.integer.min, dvars::pm_debug_lineWidth->domain.integer.max, "%.0f"); TT("pm_debug_lineWidth");
				ImGui::SliderInt("Line Duration", gui::dvar_get_set<int*>(dvars::pm_debug_lineDuration), dvars::pm_debug_lineDuration->domain.integer.min, dvars::pm_debug_lineDuration->domain.integer.max, "%.0f"); TT("pm_debug_lineDuration");
				ImGui::ColorEdit4("Line Color", gui::dvar_get_set<float*>(dvars::pm_debug_lineColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("pm_debug_lineColor");
				ImGui::Checkbox("Line Depth", gui::dvar_get_set<bool*>(dvars::pm_debug_lineDepth)); TT("pm_debug_lineDepth");

				// ---------------
				SEPERATORV(4.0f);
			}

			if (components::active.movement)
			{
				ImGui::Checkbox("Debug Prints", &dvars::pm_debug_prints->current.enabled); TT("pm_debug_prints");
			}

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}
	}

	// *
	// collision tab
	void gui_devgui::menu_tab_collision(Game::gui_menus_t& menu)
	{
		ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

		const char* collision_type_items[] = { "Off", "Outlines Only", "Polygons Only", "Both" };
		ImGui::Combo("Debug Collision Mode", &dvars::r_drawCollision->current.integer, collision_type_items, IM_ARRAYSIZE(collision_type_items)); TT("r_drawCollision");

		// only show settings when collision drawing is enabled
		if (dvars::r_drawCollision->current.integer != 0)
		{
			ImGui::SliderInt("Brush Amount", gui::dvar_get_set<int*>(dvars::r_drawCollision_brushAmount), dvars::r_drawCollision_brushAmount->domain.integer.min, dvars::r_drawCollision_brushAmount->domain.integer.max); TT("r_drawCollision_brushAmount");
			ImGui::SliderFloat("Brush Distance", &dvars::r_drawCollision_brushDist->current.value, dvars::r_drawCollision_brushDist->domain.value.min, dvars::r_drawCollision_brushDist->domain.value.max, "%.0f"); TT("r_drawCollision_brushDist");

			const char* brush_sorting_items[] = { "Off", "Far to Near", "Near to Far" };
			ImGui::Combo("Brush Sorting", gui::dvar_get_set<int*>(dvars::r_drawCollision_brushSorting), brush_sorting_items, IM_ARRAYSIZE(brush_sorting_items)); TT("r_drawCollision_brushSorting");

			SPACING(0.0f, 4.0f);
			ImGui::Text("Amount of brushes to render: %d\nAmount of brushsides to render: %d", Game::Globals::debug_collision_rendered_brush_amount, Game::Globals::debug_collision_rendered_planes_amount);
#if DEBUG
			SPACING(0.0f, 4.0f);
			ImGui::Checkbox("Brush Debug Prints", gui::dvar_get_set<bool*>(dvars::r_drawCollision_brushDebug)); TT("r_drawCollision_brushDebug");
#endif
			SPACING(0.0f, 4.0f);

			// *
			if (ImGui::CollapsingHeader("Material Filtering", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

				ImGui::Text("Map Material List:");
				ImGui::InputTextMultiline("\n", (char*)Game::Globals::r_drawCollision_materialList_string.c_str(), Game::Globals::r_drawCollision_materialList_string.size(), ImVec2(ImGui::GetWindowWidth() - 54.0f, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly);
				ImGui::DragInt("Material Index Filter", &dvars::r_drawCollision_material->current.integer, 0.25f, dvars::r_drawCollision_material->domain.integer.min, dvars::r_drawCollision_material->domain.integer.max); TT("r_drawCollision_material");

				const char* material_include_string[] = { "None", "Clip", "Mantle", "Trigger", "All", "All - No Tools", "All - No Tools/Clip" };
				ImGui::Combo("Material Include", &dvars::r_drawCollision_materialInclude->current.integer, material_include_string, IM_ARRAYSIZE(material_include_string)); TT("r_drawCollision_materialInclude");

				SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
			}

			// *
			if (ImGui::CollapsingHeader("Brush Filtering"))
			{
				static char indexfilter_string[1024] = { "null" };

				ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

				ImGui::Checkbox("Draw Brush Indices as 3D Text", &dvars::r_drawCollision_brushIndexVisible->current.enabled); TT("r_drawCollision_brushIndexVisible");

				if (dvars::r_drawCollision_brushIndexFilter)
				{
					ImGui::SameLine();
					SPACING(80.0f, 0.0f);

					ImGui::SameLine();
					if(ImGui::Button("Apply Filter"))
					{
						CMDEXEC(utils::va("set r_drawCollision_brushIndexFilter %s", indexfilter_string));
					}

					ImGui::SameLine();
					if(ImGui::Button("Reset Filter"))
					{
						CMDEXEC("set r_drawCollision_brushIndexFilter null");
						memset(&indexfilter_string, 0, IM_ARRAYSIZE(indexfilter_string));
					}

					SPACING(0.0f, 4.0f);

					ImGui::Text("Brush Index Filter. Format <number number number ...>");
					ImGui::InputTextMultiline("\n\n", indexfilter_string, IM_ARRAYSIZE(indexfilter_string), ImVec2(ImGui::GetWindowWidth() - 54.0f, ImGui::GetTextLineHeight() * 3));  TT("r_drawCollision_brushIndexFilter");
				}

				SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
			}

			// *
			if (ImGui::CollapsingHeader("Brush Visuals"))
			{
				ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

				ImGui::SliderInt("Line Width", gui::dvar_get_set<int*>(dvars::r_drawCollision_lineWidth), dvars::r_drawCollision_lineWidth->domain.integer.min, dvars::r_drawCollision_lineWidth->domain.integer.max); TT("r_drawCollision_lineWidth");

				ImGui::ColorEdit4("Line Color", gui::dvar_get_set<float*>(dvars::r_drawCollision_lineColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("r_drawCollision_lineColor");

				// --------------- 
				SEPERATORV(4.0f);

				ImGui::Checkbox("Polygon Depth", gui::dvar_get_set<bool*>(dvars::r_drawCollision_polyDepth)); TT("r_drawCollision_polyDepth");

				ImGui::SameLine();
				ImGui::Checkbox("Polygon 2-Sided", gui::dvar_get_set<bool*>(dvars::r_drawCollision_polyFace)); TT("r_drawCollision_polyFace");

				ImGui::SameLine();
				ImGui::Checkbox("Polygon Fake-Light", gui::dvar_get_set<bool*>(dvars::r_drawCollision_polyLit)); TT("r_drawCollision_polyLit");

				SPACING(0.0f, 4.0f);

				ImGui::SliderFloat("Polygon Alpha", gui::dvar_get_set<float*>(dvars::r_drawCollision_polyAlpha), dvars::r_drawCollision_polyAlpha->domain.value.min, dvars::r_drawCollision_polyAlpha->domain.value.max, "%.2f"); TT("r_drawCollision_polyAlpha");

				SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
			}

			// *
			if (ImGui::CollapsingHeader("Hud"))
			{
				ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

				ImGui::Checkbox("Enable Hud", gui::dvar_get_set<bool*>(dvars::r_drawCollision_hud)); TT("r_drawCollision_hud");

				SPACING(0.0f, 4.0f);

				ImGui::SliderFloat2("Hud Position", gui::dvar_get_set<float*>(dvars::r_drawCollision_hud_position), dvars::r_drawCollision_hud_position->domain.value.min, dvars::r_drawCollision_hud_position->domain.value.max, "%.2f"); TT("r_drawCollision_hud_position");
				ImGui::SliderFloat("Font Scale", gui::dvar_get_set<float*>(dvars::r_drawCollision_hud_fontScale), dvars::r_drawCollision_hud_fontScale->domain.value.min, dvars::r_drawCollision_hud_fontScale->domain.value.max, "%.1f"); TT("r_drawCollision_hud_fontScale");

				const char* hudFont_items[] = { "FONT_SMALL_DEV", "FONT_BIG_DEV", "FONT_CONSOLE", "FONT_BIG", "FONT_SMALL", "FONT_BOLD", "FONT_NORMAL", "FONT_EXTRA_BIG", "FONT_OBJECTIVE" };
				ImGui::Combo("Font Style", gui::dvar_get_set<int*>(dvars::r_drawCollision_hud_fontStyle), hudFont_items, IM_ARRAYSIZE(hudFont_items)); TT("pm_hud_fontStyle");

				ImGui::ColorEdit4("Font Color", gui::dvar_get_set<float*>(dvars::r_drawCollision_hud_fontColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("r_drawCollision_hud_fontColor");

				SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
			}

			// *
			if (ImGui::CollapsingHeader("Map Export", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

				ImGui::Text("Note:");
				SPACING(0.0f, 1.0f);

				ImGui::Text("Only selected brushes will be exported. Select brushes with debug-collision settings above.\nIf you want to export all brushes, set Brush Amount to \"0\" and Material Include to \"all\".");

				ImGui::SameLine();
				if (ImGui::Button("Export Map"))
				{
					CMDEXEC("mapexport");
				} TT("mapexport");

				// --------------- 
				SEPERATORV(4.0f);

				ImGui::Checkbox("Write Triangles", gui::dvar_get_set<bool*>(dvars::mapexport_writeTriangles)); TT("mapexport_writeTriangles");

				ImGui::SameLine();
				ImGui::Checkbox("Write Quads", gui::dvar_get_set<bool*>(dvars::mapexport_writeQuads)); TT("mapexport_writeQuads");

				ImGui::SameLine();
				ImGui::Checkbox("Write Entities", gui::dvar_get_set<bool*>(dvars::mapexport_writeEntities)); TT("mapexport_writeEntities");

				ImGui::Checkbox("Write Static Models", gui::dvar_get_set<bool*>(dvars::mapexport_writeModels)); TT("mapexport_writeModels");

				ImGui::SameLine();
				ImGui::Checkbox("Write Dynamic Models", gui::dvar_get_set<bool*>(dvars::mapexport_writeDynModels)); TT("mapexport_writeDynModels");

				SPACING(0.0f, 4.0f);

				ImGui::SliderFloat("Brush Minimum Size", gui::dvar_get_set<float*>(dvars::mapexport_brushMinSize), dvars::mapexport_brushMinSize->domain.value.min, dvars::mapexport_brushMinSize->domain.value.max, "%.1f"); TT("mapexport_brushMinSize :: only export brushes (with more then 6 sides) if their diagonal length is greater then <this>");
				ImGui::SliderFloat("Brushside Epsilon 1", gui::dvar_get_set<float*>(dvars::mapexport_brushEpsilon1), dvars::mapexport_brushEpsilon1->domain.value.min, dvars::mapexport_brushEpsilon1->domain.value.max, "%.1f"); TT("mapexport_brushEpsilon1 :: use 0.3 to export more but prob. broken brushes");
				ImGui::SliderFloat("Brushside Epsilon 2", gui::dvar_get_set<float*>(dvars::mapexport_brushEpsilon2), dvars::mapexport_brushEpsilon2->domain.value.min, dvars::mapexport_brushEpsilon2->domain.value.max, "%.1f"); TT("mapexport_brushEpsilon2 :: use 0.4 to export more but prob. broken brushes");

				// --------------- 
				SEPERATORV(4.0f);

				const char* selection_mode_items[] = { "Off", "Bounding box" };
				ImGui::Combo("Brush Selection Mode", gui::dvar_get_set<int*>(dvars::mapexport_selectionMode), selection_mode_items, IM_ARRAYSIZE(selection_mode_items)); TT("mapexport_selectionMode :: Bounding box needs 2 defined points <mapexport_selectionAdd>");

				if (dvars::mapexport_selectionMode->current.integer != 0)
				{
					SPACING(0.0f, 4.0f);

					if (ImGui::Button("Add point to selection"))
					{
						CMDEXEC("mapexport_selectionAdd");
					} TT("mapexport_selectionAdd");

					ImGui::SameLine();
					if (ImGui::Button("Clear selection"))
					{
						CMDEXEC("mapexport_selectionClear");
					} TT("mapexport_selectionClear");
				}

				SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
			}
		}

		SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
	}

	// *
	// shader tab
	void gui_devgui::menu_tab_shaders(Game::gui_menus_t& menu)
	{
		ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

		const char* postfx_shader_items[] = { "None", "SSAO", "Cellshading", "Outliner", "Depth Fullscreen" };
		ImGui::Combo("Post Fx Shaders", &dvars::xo_shaderoverlay->current.integer, postfx_shader_items, IM_ARRAYSIZE(postfx_shader_items)); TT("xo_shaderoverlay");

		// ssao settings
		if (dvars::xo_shaderoverlay->current.integer == 1)
		{
			SPACING(0.0f, 4.0f);

			ImGui::Checkbox("Debug Normals", &dvars::xo_ssao_debugnormal->current.enabled); TT("xo_ssao_debugnormal");

			SPACING(0.0f, 4.0f);

			ImGui::SliderFloat("Noisescale", &dvars::xo_ssao_noisescale->current.value, dvars::xo_ssao_noisescale->domain.value.min, dvars::xo_ssao_noisescale->domain.value.max, "%.1f"); TT("xo_ssao_noisescale");
			ImGui::SliderFloat("Quality", &dvars::xo_ssao_quality->current.value, 0.0f, 1.0f, "%.0f"); TT("xo_ssao_quality");
			ImGui::SliderFloat("Radius", &dvars::xo_ssao_radius->current.value, dvars::xo_ssao_radius->domain.value.min, dvars::xo_ssao_radius->domain.value.max, "%.1f"); TT("xo_ssao_radius");
			ImGui::SliderFloat("Contrast", &dvars::xo_ssao_contrast->current.value, dvars::xo_ssao_contrast->domain.value.min, dvars::xo_ssao_contrast->domain.value.max, "%.1f"); TT("xo_ssao_contrast");
			ImGui::SliderFloat("Attenuation", &dvars::xo_ssao_attenuation->current.value, dvars::xo_ssao_attenuation->domain.value.min, dvars::xo_ssao_attenuation->domain.value.max, "%.1f"); TT("xo_ssao_attenuation");
			ImGui::SliderFloat("Angle Bias", &dvars::xo_ssao_angleBias->current.value, dvars::xo_ssao_angleBias->domain.value.min, dvars::xo_ssao_angleBias->domain.value.max, "%.1f"); TT("xo_ssao_angleBias");
		}

		// outliner settings
		if (dvars::xo_shaderoverlay->current.integer == 3)
		{
			SPACING(0.0f, 4.0f);

			ImGui::SliderFloat("Outline Scale", &dvars::xo_outliner_scale->current.value, dvars::xo_outliner_scale->domain.value.min, dvars::xo_outliner_scale->domain.value.max, "%.1f"); TT("xo_outliner_scale");
			ImGui::SliderFloat("Depth Difference Scale", &dvars::xo_outliner_depthDiffScale->current.value, dvars::xo_outliner_depthDiffScale->domain.value.min, dvars::xo_outliner_depthDiffScale->domain.value.max, "%.1f"); TT("xo_outliner_depthDiffScale");
			ImGui::SliderFloat("Depth Threshold", &dvars::xo_outliner_depthThreshold->current.value, dvars::xo_outliner_depthThreshold->domain.value.min, dvars::xo_outliner_depthThreshold->domain.value.max, "%.1f"); TT("xo_outliner_depthThreshold");

			// --------------- 
			SEPERATORV(4.0f);

			ImGui::Checkbox("Enable Toon", &dvars::xo_outliner_toonEnable->current.enabled); TT("xo_outliner_toonEnable");
			ImGui::SliderFloat("Shade Amount", &dvars::xo_outliner_toonShades->current.value, dvars::xo_outliner_toonShades->domain.value.min, dvars::xo_outliner_toonShades->domain.value.max, "%.0f"); TT("xo_outliner_toonShades");
		
		}

#ifdef DEVGUI_XO_BLUR
		SPACING(0.0f, 4.0f);

		ImGui::DragFloat("xo_blur_directions", &Game::Globals::xo_blur_directions, 0.25, -2000.0f, 2000.0f, "%.1f");
		ImGui::DragFloat("xo_blur_quality", &Game::Globals::xo_blur_quality, 0.25, 0.0001f, 50.0f, "%.1f");
		ImGui::DragFloat("xo_blur_size", &Game::Globals::xo_blur_size, 0.25, 0.0001f, 50.0f, "%.1f");
		ImGui::DragFloat("xo_blur_alpha", &Game::Globals::xo_blur_alpha, 0.25, -50.0f, 50.0f, "%.1f");
#endif
		SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
	}

	void gui_devgui::menu_tab_visuals(Game::gui_menus_t& menu)
	{
		if (ImGui::CollapsingHeader("Film Tweaks", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const auto& r_filmTweakEnable = Game::Dvar_FindVar("r_filmTweakEnable");
			const auto& r_filmUseTweaks = Game::Dvar_FindVar("r_filmUseTweaks");

			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			if (r_filmTweakEnable && r_filmUseTweaks)
			{
				ImGui::Checkbox("Enable Filmtweaks", &r_filmTweakEnable->current.enabled);

				ImGui::SameLine();
				ImGui::Checkbox("Use Filmtweaks", &r_filmUseTweaks->current.enabled);

				if (r_filmTweakEnable->current.enabled && r_filmUseTweaks->current.enabled)
				{
					const auto& r_filmTweakInvert = Game::Dvar_FindVar("r_filmTweakInvert");
					const auto& r_filmTweakLightTint = Game::Dvar_FindVar("r_filmTweakLightTint");
					const auto& r_filmTweakDarkTint = Game::Dvar_FindVar("r_filmTweakDarkTint");
					const auto& r_filmTweakDesaturation = Game::Dvar_FindVar("r_filmTweakDesaturation");
					const auto& r_filmTweakContrast = Game::Dvar_FindVar("r_filmTweakContrast");
					const auto& r_filmTweakBrightness = Game::Dvar_FindVar("r_filmTweakBrightness");

					if (r_filmTweakInvert && r_filmTweakLightTint && r_filmTweakDarkTint && r_filmTweakDesaturation && r_filmTweakContrast && r_filmTweakBrightness)
					{
						ImGui::SameLine();
						ImGui::Checkbox("Invert", &r_filmTweakInvert->current.enabled);

						SPACING(0.0f, 4.0f);

						// light and darktint should be capped at 2.0 since hdr allows for much higher values
						ImGui::ColorEdit3("Light Tint", r_filmTweakLightTint->current.vector, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel); TT("r_filmTweakLightTint");
						ImGui::ColorEdit3("Dark Tint", r_filmTweakDarkTint->current.vector, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel); TT("r_filmTweakDarkTint");
						ImGui::DragFloat("Desaturation", &r_filmTweakDesaturation->current.value, 0.005f, 0.0f, 1.0f); TT("r_filmTweakDesaturation");
						ImGui::DragFloat("Contrast", &r_filmTweakContrast->current.value, 0.005f, 0.0f, 4.0f); TT("r_filmTweakContrast");
						ImGui::DragFloat("Brightness", &r_filmTweakBrightness->current.value, 0.005f, -1.0f, 1.0f); TT("r_filmTweakBrightness");
					}
				}
			}
			
			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		if (ImGui::CollapsingHeader("Light Tweaks", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const auto& r_lightTweakSunColor = Game::Dvar_FindVar("r_lightTweakSunColor");
			const auto& r_lightTweakSunDirection = Game::Dvar_FindVar("r_lightTweakSunDirection");
			const auto& r_lightTweakSunLight = Game::Dvar_FindVar("r_lightTweakSunLight");

			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			if (r_lightTweakSunColor && r_lightTweakSunDirection && r_lightTweakSunLight)
			{
				// unpack the color and use a swap var
				float sun_color_swap[4] = { 0.0f }; 
				utils::byte4_unpack_rgba(r_lightTweakSunColor->current.color, sun_color_swap);
				
				if (ImGui::ColorEdit3("Sun Color", sun_color_swap, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel))
				{
					utils::byte4_pack_rgba(sun_color_swap, r_lightTweakSunColor->current.color);

					for (auto i = 0; i < 4; i++) 
					{
						r_lightTweakSunColor->latched.color[i] = r_lightTweakSunColor->current.color[i];
					}

					r_lightTweakSunColor->modified = true;

				} TT("r_lightTweakSunColor");

				ImGui::DragFloat3("Sun Direction", gui::dvar_get_set<float*>(r_lightTweakSunDirection), 0.25f, -360.0f, 360.0f, "%.2f"); TT("r_lightTweakSunDirection");
				ImGui::SliderFloat("Sun Light", gui::dvar_get_set<float*>(r_lightTweakSunLight), r_lightTweakSunLight->domain.value.min, r_lightTweakSunLight->domain.value.max, "%.2f");
			}

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		if (ImGui::CollapsingHeader("Fog Tweaks", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			ImGui::Checkbox("Tweak Framefog", gui::dvar_get_set<bool*>(dvars::r_fogTweaks));
			ImGui::ColorEdit4("Fog Color", gui::dvar_get_set<float*>(dvars::r_fogTweaksColor), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
			ImGui::DragFloat("Fog Start", gui::dvar_get_set<float*>(dvars::r_fogTweaksStart), 5.0f, -1000.0f, 30000.0f, "%.4f");
			ImGui::DragFloat("Fog Density", gui::dvar_get_set<float*>(dvars::r_fogTweaksDensity), 0.0001f, 0.0f, 1.0f, "%.4f");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

		ImGui::Checkbox("Tweak Gun FOV", gui::dvar_get_set<bool*>(dvars::cg_fov_tweaks));
		ImGui::DragFloat("Gun FOV", gui::dvar_get_set<float*>(dvars::cg_fov_gun), 0.05f, 20.0f, 160.0f, "%.2f");

		const auto& cg_fov = Game::Dvar_FindVar("cg_fov");
		const auto& cg_fovScale = Game::Dvar_FindVar("cg_fovScale");
		const auto& cg_gun_x = Game::Dvar_FindVar("cg_gun_x");

		if(cg_fov && cg_fovScale && cg_gun_x)
		{
			ImGui::DragFloat("cg_fov", gui::dvar_get_set<float*>(cg_fov), 0.05f, 65.0f, 80.0f, "%.2f");
			ImGui::DragFloat("cg_fovScale", gui::dvar_get_set<float*>(cg_fovScale), 0.05f, 0.2f, 2.0f, "%.2f");
			ImGui::DragFloat("cg_gun_x", gui::dvar_get_set<float*>(cg_gun_x), 0.05f, -30.0f, 30.0f, "%.2f");
		}
		
		SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
	}


	// *
	// settings tab
	void gui_devgui::menu_tab_settings(Game::gui_menus_t& menu)
	{
		if (ImGui::CollapsingHeader("Window", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			ImGui::SliderFloat("Background Alpha", gui::dvar_get_set<float*>(dvars::_imgui_window_alpha), 0.0f, 1.0f, "%.2f"); TT("_imgui_window_alpha");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		if (components::active._ui && ImGui::CollapsingHeader("Main Menu", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			ImGui::DragFloat("Highlight Radius", gui::dvar_get_set<float*>(dvars::ui_button_highlight_radius), 0.001f, dvars::ui_button_highlight_radius->domain.value.min, dvars::ui_button_highlight_radius->domain.value.max, "%.4f"); TT("ui_button_highlight_radius");
			ImGui::DragFloat("Highlight Brightness", gui::dvar_get_set<float*>(dvars::ui_button_highlight_brightness), 0.001f, dvars::ui_button_highlight_brightness->domain.value.min, dvars::ui_button_highlight_brightness->domain.value.max, "%.4f"); TT("ui_button_highlight_brightness");
			ImGui::DragFloat("Outline Radius", gui::dvar_get_set<float*>(dvars::ui_button_outline_radius), 0.001f, dvars::ui_button_outline_radius->domain.value.min, dvars::ui_button_outline_radius->domain.value.max, "%.4f"); TT("ui_button_outline_radius");
			ImGui::DragFloat("Outline Brightness", gui::dvar_get_set<float*>(dvars::ui_button_outline_brightness), 0.001f, dvars::ui_button_outline_brightness->domain.value.min, dvars::ui_button_outline_brightness->domain.value.max, "%.4f"); TT("ui_button_outline_brightness");

			SEPERATORV(4.0f);

			ImGui::DragFloat2("Eyes Position", gui::dvar_get_set<float*>(dvars::ui_eyes_position), 0.001f, dvars::ui_eyes_position->domain.value.min, dvars::ui_eyes_position->domain.value.max, "%.4f"); TT("ui_eyes_position");
			ImGui::DragFloat("Eyes Size", gui::dvar_get_set<float*>(dvars::ui_eyes_size), 0.001f, dvars::ui_eyes_size->domain.value.min, dvars::ui_eyes_size->domain.value.max, "%.4f"); TT("ui_eyes_size");
			ImGui::DragFloat("Eyes Alpha", gui::dvar_get_set<float*>(dvars::ui_eyes_alpha), 0.001f, dvars::ui_eyes_alpha->domain.value.min, dvars::ui_eyes_alpha->domain.value.max, "%.4f"); TT("ui_eyes_alpha");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}
	}

	gui_devgui::gui_devgui()
	{
		// *
		// dvars

		dvars::_imgui_devgui_pos = Game::Dvar_RegisterVec2(
			/* name		*/ "_imgui_devgui_pos",
			/* desc		*/ "position of the devgui",
			/* x		*/ 20.0f,
			/* y		*/ 20.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 4096.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::_imgui_devgui_size = Game::Dvar_RegisterVec2(
			/* name		*/ "_imgui_devgui_size",
			/* desc		*/ "size of the devgui",
			/* x		*/ 550.0f,
			/* y		*/ 680.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 4096.0f,
			/* flags	*/ Game::dvar_flags::saved);
	}
}