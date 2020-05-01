#include "STDInclude.hpp"

#define devgui Game::Globals::g_devgui
#define DEVGUI_OPEN Game::Globals::g_devgui.imgui.menustate
#define DEVGUI_WASOPEN Game::Globals::g_devgui.imgui.wasOpen
#define DEVGUI_READY Game::Globals::g_devgui.imgui.initialized

// tooltip
#define TTDELAY 1.0f // show tooltip after x seconds
#define TT(tooltip) if (ImGui::IsItemHoveredDelay(TTDELAY)) { ImGui::SetTooltip(tooltip); }
// spacing dummy
#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 
// seperator with spacing
#define SEPERATORV(spacing) SPACING(0.0f, spacing); ImGui::Separator(); SPACING(0.0f, spacing); 

#define CMDEXEC(command) Game::Cmd_ExecuteSingleCommand(0, 0, command)

namespace Components
{
	bool test1 = false;
	bool test2 = false;
	char buffer[256] = { 0 };

	// *
	// set latched values + modified flag and return value pointer 
	template <typename T>
	T DvarGetSet(Game::dvar_s* dvar)
	{
		if (dvar)
		{
			auto vecSize = 0;

			switch (dvar->type)
			{
			case Game::dvar_type::boolean:
				dvar->latched.enabled = dvar->current.enabled;
				dvar->modified = true;
				return reinterpret_cast<T>(&dvar->current.enabled);

			case Game::dvar_type::value:
				dvar->latched.value = dvar->current.value;
				dvar->modified = true;
				return reinterpret_cast<T>(&dvar->current.value);

			case Game::dvar_type::enumeration:
			case Game::dvar_type::integer:
				dvar->latched.integer = dvar->current.integer;
				dvar->modified = true;
				return reinterpret_cast<T>(&dvar->current.integer);

			default:
				Game::Com_Error(0, "unhandled dvar_type!");
				return nullptr;


			case Game::dvar_type::vec2:
				vecSize = 2;
				break;
			case Game::dvar_type::vec3:
				vecSize = 3;
				break;
			case Game::dvar_type::vec4:
				vecSize = 4;
				break;
			}

			// vectors here
			Utils::vector::_VectorCopy(dvar->current.vector, dvar->latched.vector, vecSize);
			dvar->modified = true;
			return reinterpret_cast<T>(dvar->current.vector);
		}

		assert("dvar was null!");
		return nullptr;
	}

	// *
	// collision tab
	void Devgui::menu_tab_collision()
	{
		ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

		const char* collision_type_items[] = { "Off", "Outlines Only", "Polygons Only", "Both" };
		ImGui::Combo("Debug Collision Mode", &Dvars::r_drawCollision->current.integer, collision_type_items, IM_ARRAYSIZE(collision_type_items)); TT("r_drawCollision");

		// only show settings when collision drawing is enabled
		if (Dvars::r_drawCollision->current.integer != 0)
		{
			ImGui::SliderInt("Brush Amount", DvarGetSet<int*>(Dvars::r_drawCollision_brushAmount), Dvars::r_drawCollision_brushAmount->domain.integer.min, Dvars::r_drawCollision_brushAmount->domain.integer.max); TT("r_drawCollision_brushAmount");

			ImGui::SliderFloat("Brush Distance", &Dvars::r_drawCollision_brushDist->current.value, Dvars::r_drawCollision_brushDist->domain.value.min, Dvars::r_drawCollision_brushDist->domain.value.max, "%.0f"); TT("r_drawCollision_brushDist");

			const char* brush_sorting_items[] = { "Off", "Far to Near", "Near to Far" };
			ImGui::Combo("Brush Sorting", DvarGetSet<int*>(Dvars::r_drawCollision_brushSorting), brush_sorting_items, IM_ARRAYSIZE(brush_sorting_items)); TT("r_drawCollision_brushSorting");

#if DEBUG
			SPACING(0.0f, 4.0f);
			ImGui::Checkbox("Brush Debug Prints", DvarGetSet<bool*>(Dvars::r_drawCollision_brushDebug)); TT("r_drawCollision_brushDebug");
#endif
			SPACING(0.0f, 4.0f);

			// *
			if (ImGui::CollapsingHeader("Material Filtering", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

				ImGui::InputTextMultiline("Map Material List", (char*)Game::Globals::r_drawCollision_materialList_string.c_str(), Game::Globals::r_drawCollision_materialList_string.size(), ImVec2(-1.0f, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly);

				ImGui::DragInt("Material Index Filter", &Dvars::r_drawCollision_material->current.integer, 0.25f, Dvars::r_drawCollision_material->domain.integer.min, Dvars::r_drawCollision_material->domain.integer.max); TT("r_drawCollision_material");

				const char* material_include_string[] = { "None", "Clip", "Mantle", "Trigger", "All", "All - No Tools", "All - No Tools/Clip" };
				ImGui::Combo("Material Include", &Dvars::r_drawCollision_materialInclude->current.integer, material_include_string, IM_ARRAYSIZE(material_include_string)); TT("r_drawCollision_materialInclude");

				SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
			}

			// *
			if (ImGui::CollapsingHeader("Brush Filtering"))
			{
				ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

				ImGui::Checkbox("Draw Brush Indices as 3D Text", &Dvars::r_drawCollision_brushIndexVisible->current.enabled); TT("r_drawCollision_brushIndexVisible");

				if (Dvars::r_drawCollision_brushIndexFilter)
				{
					static char indexfilter_string[1024] = { "null" };
					if (ImGui::InputTextMultiline("Brush Index Filter", indexfilter_string, IM_ARRAYSIZE(indexfilter_string), ImVec2(ImGui::GetWindowWidth() - 8.0f, ImGui::GetTextLineHeight() * 3)))
					{
						CMDEXEC(Utils::VA("set r_drawCollision_brushIndexFilter %s", indexfilter_string));
					} 
					TT("r_drawCollision_brushIndexFilter");
				}

				SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
			}

			// *
			if (ImGui::CollapsingHeader("Brush Visuals"))
			{
				ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

				ImGui::SliderInt("Line Width", DvarGetSet<int*>(Dvars::r_drawCollision_lineWidth), Dvars::r_drawCollision_lineWidth->domain.integer.min, Dvars::r_drawCollision_lineWidth->domain.integer.max); TT("r_drawCollision_lineWidth");

				ImGui::ColorEdit4("Line Color", DvarGetSet<float*>(Dvars::r_drawCollision_lineColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("r_drawCollision_lineColor");

				// --------------- 
				SEPERATORV(4.0f);

				ImGui::Checkbox("Polygon Depth", DvarGetSet<bool*>(Dvars::r_drawCollision_polyDepth)); TT("r_drawCollision_polyDepth");
				ImGui::SameLine();
				ImGui::Checkbox("Polygon 2-Sided", DvarGetSet<bool*>(Dvars::r_drawCollision_polyFace)); TT("r_drawCollision_polyFace");
				ImGui::SameLine();
				ImGui::Checkbox("Polygon Fake-Light", DvarGetSet<bool*>(Dvars::r_drawCollision_polyLit)); TT("r_drawCollision_polyLit");

				SPACING(0.0f, 4.0f);

				ImGui::SliderFloat("Polygon Alpha", DvarGetSet<float*>(Dvars::r_drawCollision_polyAlpha), Dvars::r_drawCollision_polyAlpha->domain.value.min, Dvars::r_drawCollision_polyAlpha->domain.value.max, "%.1f"); TT("r_drawCollision_polyAlpha");

				SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
			}

			// *
			if (ImGui::CollapsingHeader("Hud"))
			{
				ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

				ImGui::Checkbox("Enable Hud", DvarGetSet<bool*>(Dvars::r_drawCollision_hud)); TT("r_drawCollision_hud"); 
				
				SPACING(0.0f, 4.0f);

				ImGui::SliderFloat2("Hud Position", DvarGetSet<float*>(Dvars::r_drawCollision_hud_position), Dvars::r_drawCollision_hud_position->domain.value.min, Dvars::r_drawCollision_hud_position->domain.value.max, "%.2f"); TT("r_drawCollision_hud_position");
				
				ImGui::SliderFloat("Font Scale", DvarGetSet<float*>(Dvars::r_drawCollision_hud_fontScale), Dvars::r_drawCollision_hud_fontScale->domain.value.min, Dvars::r_drawCollision_hud_fontScale->domain.value.max, "%.1f"); TT("r_drawCollision_hud_fontScale");
				
				const char* hudFont_items[] = { "FONT_SMALL_DEV", "FONT_BIG_DEV", "FONT_CONSOLE", "FONT_BIG", "FONT_SMALL", "FONT_BOLD", "FONT_NORMAL", "FONT_EXTRA_BIG", "FONT_OBJECTIVE" };
				ImGui::Combo("Font Style", DvarGetSet<int*>(Dvars::r_drawCollision_hud_fontStyle), hudFont_items, IM_ARRAYSIZE(hudFont_items)); TT("pm_hud_fontStyle");

				ImGui::ColorEdit4("Font Color", DvarGetSet<float*>(Dvars::r_drawCollision_hud_fontColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("r_drawCollision_hud_fontColor");

				SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
			}

			// *
			if (ImGui::CollapsingHeader("Map Export", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

				

				SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
			}
		}

		SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
	}

	// *
	// radiant tab
	void Devgui::menu_tab_radiant()
	{
		// *
		if (ImGui::CollapsingHeader("General Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			ImGui::Checkbox("Enable Radiant Live-Link", DvarGetSet<bool*>(Dvars::radiant_live)); TT("radiant_live");
			ImGui::SameLine();
			ImGui::Checkbox("Enable Live-Link Debug", DvarGetSet<bool*>(Dvars::radiant_liveDebug)); TT("radiant_liveDebug");

			SPACING(0.0f, 4.0f);

			ImGui::InputInt("Live-Link Port", DvarGetSet<int*>(Dvars::radiant_livePort)); TT("radiant_livePort");

			const char* radiant_cam_items[] = { "Disabled", "Radiant To Game", "Game To Radiant", "Both" };
			ImGui::Combo("Camera Synchronization", DvarGetSet<int*>(Dvars::radiant_syncCamera), radiant_cam_items, IM_ARRAYSIZE(radiant_cam_items)); TT("radiant_syncCamera");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		// *
		if (ImGui::CollapsingHeader("Brush Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			ImGui::Checkbox("Brush Collision", DvarGetSet<bool*>(Dvars::radiant_brushCollision)); TT("radiant_brushCollision");
			ImGui::SameLine();
			ImGui::Checkbox("Brush Fake-Light", DvarGetSet<bool*>(Dvars::radiant_brushLit)); TT("radiant_brushLit");
			ImGui::SameLine();
			ImGui::Checkbox("Brush Wireframe", DvarGetSet<bool*>(Dvars::radiant_brushWireframe)); TT("radiant_brushWireframe");

			SPACING(0.0f, 4.0f);

			ImGui::ColorEdit4("Brush Color", DvarGetSet<float*>(Dvars::radiant_brushColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("radiant_brushColor");
			
			ImGui::ColorEdit4("Wireframe Color", DvarGetSet<float*>(Dvars::radiant_brushWireframeColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("radiant_brushWireframeColor");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		// *
		if (ImGui::CollapsingHeader("Commands", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			if (ImGui::Button("Save Current Selection")) { CMDEXEC("radiant_saveSelection"); } TT("radiant_saveSelection");
			ImGui::SameLine();
			if (ImGui::Button("Clear Saved Selection")) { CMDEXEC("radiant_clearSaved"); } TT("radiant_clearSaved");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}
	}

	// *
	// shader tab
	void Devgui::menu_tab_shaders()
	{
		ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

		const char* postfx_shader_items[] = { "None", "SSAO", "Cellshading", "Outliner", "Depth Fullscreen" };
		ImGui::Combo("Post Fx Shaders", &Dvars::xo_shaderoverlay->current.integer, postfx_shader_items, IM_ARRAYSIZE(postfx_shader_items)); TT("xo_shaderoverlay");

		// ssao settings
		if (Dvars::xo_shaderoverlay->current.integer == 1)
		{
			SPACING(0.0f, 4.0f);

			ImGui::Checkbox("Debug Normals", &Dvars::xo_ssao_debugnormal->current.enabled); TT("xo_ssao_debugnormal");
			ImGui::SameLine();
			ImGui::Checkbox("Debug Rendertargets", &Dvars::xo_ssao_debugTargets->current.enabled); TT("xo_ssao_debugTargets");
			ImGui::SameLine();
			ImGui::Checkbox("Custom Depth Prepass", &Dvars::xo_ssao_depthprepass->current.enabled); TT("xo_ssao_depthprepass");

			SPACING(0.0f, 4.0f);

			ImGui::SliderFloat("Noisescale", &Dvars::xo_ssao_noisescale->current.value, Dvars::xo_ssao_noisescale->domain.value.min, Dvars::xo_ssao_noisescale->domain.value.max, "%.1f"); TT("xo_ssao_noisescale");

			ImGui::SliderFloat("Quality", &Dvars::xo_ssao_quality->current.value, 0.0f, 1.0f, "%.0f"); TT("xo_ssao_quality");

			ImGui::SliderFloat("Radius", &Dvars::xo_ssao_radius->current.value, Dvars::xo_ssao_radius->domain.value.min, Dvars::xo_ssao_radius->domain.value.max, "%.1f"); TT("xo_ssao_radius");

			ImGui::SliderFloat("Contrast", &Dvars::xo_ssao_contrast->current.value, Dvars::xo_ssao_contrast->domain.value.min, Dvars::xo_ssao_contrast->domain.value.max, "%.1f"); TT("xo_ssao_contrast");

			ImGui::SliderFloat("Attenuation", &Dvars::xo_ssao_attenuation->current.value, Dvars::xo_ssao_attenuation->domain.value.min, Dvars::xo_ssao_attenuation->domain.value.max, "%.1f"); TT("xo_ssao_attenuation");

			ImGui::SliderFloat("Angle Bias", &Dvars::xo_ssao_angleBias->current.value, Dvars::xo_ssao_angleBias->domain.value.min, Dvars::xo_ssao_angleBias->domain.value.max, "%.1f"); TT("xo_ssao_angleBias");
		}

		// outliner settings
		if (Dvars::xo_shaderoverlay->current.integer == 3)
		{
			SPACING(0.0f, 4.0f);

			ImGui::SliderFloat("Outline Scale", &Dvars::xo_outliner_scale->current.value, Dvars::xo_outliner_scale->domain.value.min, Dvars::xo_outliner_scale->domain.value.max, "%.1f"); TT("xo_outliner_scale");

			ImGui::SliderFloat("Depth Difference Scale", &Dvars::xo_outliner_depthDiffScale->current.value, Dvars::xo_outliner_depthDiffScale->domain.value.min, Dvars::xo_outliner_depthDiffScale->domain.value.max, "%.1f"); TT("xo_outliner_depthDiffScale");

			ImGui::SliderFloat("Depth Threshold", &Dvars::xo_outliner_depthThreshold->current.value, Dvars::xo_outliner_depthThreshold->domain.value.min, Dvars::xo_outliner_depthThreshold->domain.value.max, "%.1f"); TT("xo_outliner_depthThreshold");

			// --------------- 
			SEPERATORV(4.0f);

			ImGui::Checkbox("Enable Toon", &Dvars::xo_outliner_toonEnable->current.enabled); TT("xo_outliner_toonEnable");

			ImGui::SliderFloat("Shade Amount", &Dvars::xo_outliner_toonShades->current.value, Dvars::xo_outliner_toonShades->domain.value.min, Dvars::xo_outliner_toonShades->domain.value.max, "%.0f"); TT("xo_outliner_toonShades");
		}

		SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
	}

	// *
	// movement tab
	void Devgui::menu_tab_movement()
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
			ImGui::Combo("Movement Type", &Dvars::pm_movementType->current.integer, movementType_items, IM_ARRAYSIZE(movementType_items)); TT("pm_movementType");

			SPACING(0.0f, 4.0f);
			
			// *
			if (ImGui::BeginTabBar("movement_tabs"))
			{
				// *
				if (ImGui::BeginTabItem("General"))
				{
					ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

					ImGui::Checkbox("Bhop Auto", &Dvars::pm_bhop_auto->current.enabled); TT("pm_bhop_auto");
					ImGui::SameLine();
					ImGui::Checkbox("Bhop Slowdown", &Dvars::pm_bhop_slowdown->current.enabled); TT("pm_bhop_slowdown");

					auto jump_slowdownenable = Game::Dvar_FindVar("jump_slowdownenable");
					if (jump_slowdownenable)
					{
						ImGui::SameLine();
						ImGui::Checkbox("Jump Slowdown", &jump_slowdownenable->current.enabled); TT("jump_slowdownenable");
					}
						
					ImGui::Checkbox("Crashland", &Dvars::pm_crashland->current.enabled); TT("pm_crashland");
					ImGui::SameLine();
					ImGui::Checkbox("Disable Sprint", &Dvars::pm_disableSprint->current.enabled); TT("pm_disableSprint");

					SPACING(0.0f, 4.0f);

					ImGui::SliderFloat("Rocketjump Knockback", &Dvars::pm_rocketJumpHeight->current.value, Dvars::pm_rocketJumpHeight->domain.value.min, Dvars::pm_rocketJumpHeight->domain.value.max, "%.0f"); TT("pm_rocketJumpHeight");

					// --------------- 
					SEPERATORV(4.0f);

					ImGui::Checkbox("Enable Hud", DvarGetSet<bool*>(Dvars::pm_hud_enable)); TT("pm_hud_enable");

					ImGui::SliderFloat("Hud Position X", DvarGetSet<float*>(Dvars::pm_hud_x), Dvars::pm_hud_x->domain.value.min, Dvars::pm_hud_x->domain.value.max, "%.0f"); TT("pm_hud_x");

					ImGui::SliderFloat("Hud Position Y", DvarGetSet<float*>(Dvars::pm_hud_y), Dvars::pm_hud_y->domain.value.min, Dvars::pm_hud_y->domain.value.max, "%.0f"); TT("pm_hud_y");

					ImGui::SliderFloat("Hud Font Scale", DvarGetSet<float*>(Dvars::pm_hud_fontScale), Dvars::pm_hud_fontScale->domain.value.min, Dvars::pm_hud_fontScale->domain.value.max, "%.1f"); TT("pm_hud_fontScale");

					ImGui::ColorEdit4("Hud Font Color", DvarGetSet<float*>(Dvars::pm_hud_fontColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("pm_hud_fontColor");

					const char* hudFont_items[] = { "FONT_SMALL_DEV", "FONT_BIG_DEV", "FONT_CONSOLE", "FONT_BIG", "FONT_SMALL", "FONT_BOLD", "FONT_NORMAL", "FONT_EXTRA_BIG", "FONT_OBJECTIVE" };
					ImGui::Combo("Hud Font", DvarGetSet<int*>(Dvars::pm_hud_fontStyle), hudFont_items, IM_ARRAYSIZE(hudFont_items)); TT("pm_hud_fontStyle");

					SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
					ImGui::EndTabItem();
				}

				// *
				if (ImGui::BeginTabItem("Quake"))
				{
					ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

					ImGui::SliderFloat("Airstop Accelerate", &Dvars::pm_cpm_airstopAccelerate->current.value, Dvars::pm_cpm_airstopAccelerate->domain.value.min, Dvars::pm_cpm_airstopAccelerate->domain.value.max, "%.0f"); TT("pm_cpm_airstopAccelerate");

					ImGui::SliderFloat("Strafe Accelerate", &Dvars::pm_cpm_strafeAccelerate->current.value, Dvars::pm_cpm_strafeAccelerate->domain.value.min, Dvars::pm_cpm_strafeAccelerate->domain.value.max, "%.0f"); TT("pm_cpm_strafeAccelerate");

					ImGui::SliderFloat("Air Accelerate", &Dvars::pm_cpm_airAccelerate->current.value, Dvars::pm_cpm_airAccelerate->domain.value.min, Dvars::pm_cpm_airAccelerate->domain.value.max, "%.0f"); TT("pm_cpm_airAccelerate");

					ImGui::SliderFloat("Air Control", &Dvars::pm_cpm_airControl->current.value, Dvars::pm_cpm_airControl->domain.value.min, Dvars::pm_cpm_airControl->domain.value.max, "%.0f"); TT("pm_cpm_airControl");

					ImGui::SliderFloat("Damage Knockback", &Dvars::pm_cpm_damageKnockback->current.value, Dvars::pm_cpm_damageKnockback->domain.value.min, Dvars::pm_cpm_damageKnockback->domain.value.max, "%.0f"); TT("pm_cpm_damageKnockback");

					SPACING(0.0f, 4.0f);

					ImGui::Checkbox("Quake Damage", &Dvars::pm_cpm_useQuakeDamage->current.enabled); TT("pm_cpm_useQuakeDamage");
					ImGui::SameLine();
					ImGui::Checkbox("Enable Bouncing", &Dvars::pm_cpm_useBouncing->current.enabled); TT("pm_cpm_useBouncing");
		
					SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
					ImGui::EndTabItem();
				}

				// *
				if (ImGui::BeginTabItem("Source"))
				{
					ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

					ImGui::SliderFloat("Air Accelerate", &Dvars::pm_cs_airAccelerate->current.value, Dvars::pm_cs_airAccelerate->domain.value.min, Dvars::pm_cs_airAccelerate->domain.value.max, "%.0f"); TT("pm_cs_airAccelerate");
	
					ImGui::SliderFloat("Airspeed Cap", &Dvars::pm_cs_airspeedCap->current.value, Dvars::pm_cs_airspeedCap->domain.value.min, Dvars::pm_cs_airspeedCap->domain.value.max, "%.0f"); TT("pm_cs_airspeedCap");

					SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		// *
		if (ImGui::CollapsingHeader("Debug"))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			const char* drawAxis_items[] = { "disabled", "axis only", "fps zone circle" };
			ImGui::Combo("Draw Axis", &Dvars::pm_debug_drawAxis->current.integer, drawAxis_items, IM_ARRAYSIZE(drawAxis_items)); TT("pm_debug_drawAxis");

			ImGui::SliderFloat("Radius", DvarGetSet<float*>(Dvars::pm_debug_drawAxis_radius), Dvars::pm_debug_drawAxis_radius->domain.value.min, Dvars::pm_debug_drawAxis_radius->domain.value.max, "%.0f"); TT("pm_debug_drawAxis_radius");

			ImGui::SliderFloat("Height", DvarGetSet<float*>(Dvars::pm_debug_drawAxis_height), Dvars::pm_debug_drawAxis_height->domain.value.min, Dvars::pm_debug_drawAxis_height->domain.value.max, "%.0f"); TT("pm_debug_drawAxis_height");

			SPACING(0.0f, 4.0f);

			ImGui::ColorEdit4("125 FPS Zone", DvarGetSet<float*>(Dvars::pm_debug_drawAxis_col125), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("pm_debug_drawAxis_col125");

			ImGui::ColorEdit4("250 FPS Zone", DvarGetSet<float*>(Dvars::pm_debug_drawAxis_col250), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("pm_debug_drawAxis_col250");

			ImGui::ColorEdit4("333 FPS Zone", DvarGetSet<float*>(Dvars::pm_debug_drawAxis_col333), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("pm_debug_drawAxis_col333");

			// --------------- 
			SEPERATORV(4.0f);

			const char* traceOrigin_items[] = { "disabled", "trace and display for duration", "trace and display for duration when not on the ground" };
			ImGui::Combo("Trace Origin", &Dvars::pm_debug_traceOrigin->current.integer, traceOrigin_items, IM_ARRAYSIZE(traceOrigin_items)); TT("pm_debug_traceOrigin");

			const char* traceVelocity_items[] = { "disabled", "trace and display for duration", "trace and display for duration when not on the ground", "trace in realtime" };
			ImGui::Combo("Trace Velocity", &Dvars::pm_debug_traceVelocity->current.integer, traceVelocity_items, IM_ARRAYSIZE(traceVelocity_items)); TT("pm_debug_traceVelocity");

			ImGui::SliderInt("Line Width", DvarGetSet<int*>(Dvars::pm_debug_lineWidth), Dvars::pm_debug_lineWidth->domain.integer.min, Dvars::pm_debug_lineWidth->domain.integer.max, "%.0f"); TT("pm_debug_lineWidth");

			ImGui::SliderInt("Line Duration", DvarGetSet<int*>(Dvars::pm_debug_lineDuration), Dvars::pm_debug_lineDuration->domain.integer.min, Dvars::pm_debug_lineDuration->domain.integer.max, "%.0f"); TT("pm_debug_lineDuration");

			ImGui::ColorEdit4("Line Color", DvarGetSet<float*>(Dvars::pm_debug_lineColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("pm_debug_lineColor");

			ImGui::Checkbox("Line Depth", DvarGetSet<bool*>(Dvars::pm_debug_lineDepth)); TT("pm_debug_lineDepth");

			// --------------- 
			SEPERATORV(4.0f);

			ImGui::Checkbox("Debug Prints", &Dvars::pm_debug_prints->current.enabled); TT("pm_debug_prints");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}
	}

	// *
	// settings tab
	void Devgui::menu_tab_settings()
	{
		if (ImGui::CollapsingHeader("Window", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			ImGui::SliderFloat("Background Alpha", DvarGetSet<float*>(Dvars::_imgui_window_alpha), 0.0f, 1.0f, "%.2f"); TT("_imgui_window_alpha");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}
	}

	// *
	// the "main" menu
	void Devgui::create_devgui(bool* p_open)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImGuiIO& io = ImGui::GetIO();

		style.Colors[ImGuiCol_WindowBg].w = Dvars::_imgui_window_alpha->current.value;
		style.WindowMinSize.x = 480.0f;
		style.WindowMinSize.y = 320.0f;

		io.WantCaptureKeyboard = true;
		io.WantCaptureMouse = true;

		// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only do it to make the Demo applications a little more welcoming.
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

		// early out if the window is collapsed, as an optimization.
		if (!ImGui::Begin("Devgui", p_open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		auto cl_ingame = Game::Dvar_FindVar("cl_ingame");
				
		if (ImGui::BeginTabBar("devgui_tabs"))
		{
			if (cl_ingame && cl_ingame->current.enabled)
			{
				if (ImGui::BeginTabItem("Movement"))
				{
					Devgui::menu_tab_movement();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Collision"))
				{
					Devgui::menu_tab_collision();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Shaders"))
				{
					Devgui::menu_tab_shaders();
					ImGui::EndTabItem();
				}
			}

			if (ImGui::BeginTabItem("Radiant"))
			{
				Devgui::menu_tab_radiant();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Settings"))
			{
				Devgui::menu_tab_settings();
				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();
		ImGui::End();
	}

	// *
	// toggle imgui by command (or key (scheduler))
	void Devgui::toggle(bool onCommand = false)
	{
		if (!DEVGUI_READY)
		{
			return;
		}

		static auto is_down = false, is_clicked = false;

		if (Game::playerKeys->keys[KEYCATCHER_END].down) 
		{
			is_clicked = false;
			is_down = true;
		}
		else if (!Game::playerKeys->keys[KEYCATCHER_END].down && is_down) 
		{
			is_clicked = true;
			is_down = false;
		}
		else 
		{
			is_clicked = false;
			is_down = false;
		}

		// toggle menu
		if (is_clicked || onCommand)
		{
			ImGuiIO& io = ImGui::GetIO();
			std::fill_n(io.KeysDown, 512, 0); // fix keys getting stuck on close / vid_restart

			DEVGUI_OPEN = !DEVGUI_OPEN;

			// free the mouse
			static auto in_mouse = Game::Dvar_FindVar("in_mouse");
			if (in_mouse)
			{
				Game::Dvar_SetValue(in_mouse, DEVGUI_OPEN ? 0 : 1);
				
				Utils::function<void()>(0x575E90)(); // In_Shutdown
				*Game::mouse_enabled = in_mouse->current.enabled;
			}
		}
	}

	// *
	// unused
	void Devgui::update_hWnd(void* hwnd)
	{
		if (DEVGUI_READY)
		{
			ImGui_SethWnd(hwnd);
		}
	}

	// *
	// reset the mouse
	void Devgui::reset_mouse()
	{
		static auto in_mouse = Game::Dvar_FindVar("in_mouse");

		if (in_mouse)
		{
			Game::Dvar_SetValue(in_mouse, true);
			*Game::mouse_enabled = in_mouse->current.enabled;
		}
	}

	// *
	// shutdown imgui when game window resets (Window::CreateMainWindow)
	void Devgui::reset()
	{
		if (DEVGUI_READY)
		{
			ImGui_ImplDX9_Shutdown();
			ImGui_ImplWin32_Shutdown();

			Devgui::reset_mouse();
			memset(&Game::Globals::g_devgui.imgui, 0, sizeof(Game::imgui_t));
		}
	}

	// *
	//
	void Devgui::begin_frame()
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	// *
	//
	void Devgui::end_frame()
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	// asign dvar value pointers (take latched)
	// called on init from Commands::ForceDvarsOnInit()
	void Devgui::load_settings()
	{
		// window settings
		//devgui.settings.window.alpha = &Dvars::_imgui_window_alpha->latched.value;

		// end
		devgui.settings.settingsLoaded = true;
	}

	// *
	// save dvars to disk (only modified dvars are saved)
	void Devgui::save_settings()
	{
		// window settings
		//Dvars::_imgui_window_alpha->modified = true;

		// trigger config writing
		Game::Cmd_ExecuteSingleCommand(0, 0, Utils::VA("set _imgui_saving %d", !Dvars::_imgui_saving->current.enabled));
	}

	// *
	// main rendering loop (D3D9Ex::D3D9Device::EndScene())
	void Devgui::render_loop()
	{
		if (/**/ !devgui.settings.settingsLoaded) 
		{
			return;
		}

		if (!DEVGUI_READY)
		{
			Devgui::imgui_init();
		}

		Devgui::begin_frame();

		if (DEVGUI_OPEN) // devgui toggle
		{
			if (Dvars::devgui_demo->current.enabled)
			{
				// demo menu
				ImGui::ShowDemoWindow(&DEVGUI_OPEN);
			}
			else
			{
				// devgui
				Devgui::create_devgui(&DEVGUI_OPEN);
			}

			DEVGUI_WASOPEN = true;
		}
		else if (DEVGUI_WASOPEN) // on closing
		{
			Devgui::reset_mouse();
			Devgui::save_settings();

			DEVGUI_WASOPEN = false;
		}

		Devgui::end_frame();
	}

	// *
	// initialize imgui
	void Devgui::imgui_init()
	{
		IDirect3DDevice9* device;

		if (Components::active.D3D9Ex && Dvars::r_d3d9ex && Dvars::r_d3d9ex->current.enabled)
		{
			// get the device from D3D9Ex::D3D9::CreateDevice
			device = Game::Globals::d3d9_device;
		}
		else
		{
			// get the device from the game
			device = *Game::dx9_device_ptr;
		}

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(Window::GetWindow());
		ImGui_ImplDX9_Init(device);

		// Style
		ImGui::StyleColorsDevgui();

		DEVGUI_READY = true;
	}

	// *
	// 
	Devgui::Devgui()
	{ 
		Command::Add("devgui", "", "opens the devgui", [this](Command::Params)
		{
			if (!Dvars::r_d3d9ex->current.enabled)
			{
				Game::Com_PrintMessage(0, "Please enable <r_d3d9ex> to use the devgui!", 0);
				return;
			}

			Devgui::toggle(true);
		});

//#if DEBUG
		Scheduler::on_frame([this]()
		{
			Devgui::toggle();
		}, Scheduler::thread::main);
//#endif

		Dvars::devgui_demo = Game::Dvar_RegisterBool(
			/* name		*/ "devgui_demo",
			/* desc		*/ "display the imgui demo instead of devgui",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::_imgui_saving = Game::Dvar_RegisterBool(
			/* name		*/ "_imgui_saving",
			/* desc		*/ "saving flag",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::_imgui_window_alpha = Game::Dvar_RegisterFloat(
			/* name		*/ "_imgui_window_alpha",
			/* desc		*/ "imgui window alpha",
			/* default	*/ 0.8f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);
	}

	Devgui::~Devgui()
	{ }
}