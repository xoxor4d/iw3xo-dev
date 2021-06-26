#include "STDInclude.hpp"

// spacing dummy
#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 

namespace Components
{
	// fog
	bool		 framefog_tweaks = false;
	Game::vec4_t framefog_color = { 0.0f };


	// get shader constants vars
	Game::vec4_t sundiffuse_constant = { 0.0f };
	Game::vec4_t sunposition_constant = { 0.0f };
	Game::vec4_t colorbias_constant = { 0.0f };

	// devgui/shader constants
	Game::vec3_t outscatter_color01 = { 0.859f, 0.654f, 0.362f };
	float		 outscatter_scale01 = 0.61f;

	Game::vec3_t outscatter_color02 = { 1.0f, 1.00f, 1.00f };
	float		 outscatter_scale02 = 15.0f;

	float		 dn_skydome_scale = 80.0f;
	float		 dn_cloud_coverage = 0.55f;
	float		 dn_cloud_thickness = 10.0f;
	float		 dn_cloud_absorbtion = 0.36f;

	float		 dn_star_scale = 0.02f;
	float		 dn_cloud_step_distance_xz = 150.0f;
	float		 dn_cloud_steps_after_threshold = 6.0f; // dn_cloud_steps * 0.5
	float		 dn_cloud_exposure = 0.65f;

	Game::vec3_t dn_cloud_wind_vec = { 0.01f, 0.00f, 0.01f };
	float		 dn_cloud_wind_speed = 1.3f;

	float		 dn_sky_sun_intensity = 22.0f;
	Game::vec3_t dn_sky_rayleigh_coeff = { 7.52f, 13.0f, 22.4f };

	float		 dn_sky_mie_coeff = 7.00f;
	float		 dn_sky_rayleigh_scale = 1000.0f;
	float		 dn_sky_mie_scale = 400.0f;
	float		 dn_sky_mie_scatter_dir = 0.99f;


	// *
	// setup time/transitions

	//float		state_minute = 0.0f; // current minute of the day
	//float		state_hours = 0.0f;
	//int		state_hours_24 = 0;
	//int		state_minutes_24 = 0;

	float	time_scalar = 1.0f;
	bool	time_running = false;
	bool	time_rotates_sun = true;

	void vinterp_to(float* output, int vector_size, const float* current, const float* target, const float delta_time, const float interpolation_speed)
	{
		if (output)
		{
			// If no interp speed, jump to target value
			if (interpolation_speed <= 0.0f)
			{
				Utils::vector::_VectorCopy(target, output, vector_size);
				return;
			}

			for (auto i = 0; i < vector_size && i < 4; i++)
			{
				// distance to reach
				const float distance = target[i] - current[i];

				// If distance is too small, just set the desired location
				if (distance * distance < 1.e-8f)
				{
					output[i] = target[i];
					continue;
				}

				// Delta Move, Clamp so we do not over shoot.
				const float delta_move = distance * std::clamp(delta_time * interpolation_speed, 0.0f, 1.0f);

				output[i] = current[i] + delta_move;
			}
		}
	}

	float finterp_to(const float current, const float target, const float delta_time, const float interpolation_speed)
	{
		// If no interp speed, jump to target value
		if (interpolation_speed <= 0.0f)
		{
			return target;
		}

		// distance to reach
		const float distance = target - current;

		// If distance is too small, just set the desired location
		if (distance * distance < 1.e-8f)
		{
			return target;
		}

		// Delta Move, Clamp so we do not over shoot.
		const float delta_move = distance * std::clamp(delta_time * interpolation_speed, 0.0f, 1.0f);

		return current + delta_move;
	}


	// TODO:
	// create dvars for these

	DayNightCycle::time_of_day_settings tod_night = {
		/*float film_brightness		*/	-0.075f,
		/*float film_contrast		*/	1.06f,
		/*float film_desaturation	*/	0.375f,
		/*float film_darktint[3]	*/	{ 0.413f, 0.638f, 1.008f },
		/*float film_lighttint[3]	*/	{ 0.838f, 0.689f, 0.618f },
		/*float film_interpol_spd	*/	0.2f,

		/*float sun_light			*/	0.6f,
		/*float sun_color[3]		*/	{ 0.400f, 0.400f, 0.700f },
		/*float sun_interpol_spd	*/	0.1f
	};

	DayNightCycle::time_of_day_settings tod_day = {
		/*float film_brightness		*/	0.00f,
		/*float film_contrast		*/	1.0f,
		/*float film_desaturation	*/	0.1f,
		/*float film_darktint[3]	*/	{ 1.000f, 1.000f, 1.000f },
		/*float film_lighttint[3]	*/	{ 1.000f, 1.000f, 1.000f },
		/*float film_interpol_spd	*/	0.05f,

		/*float sun_light			*/	1.5f,
		/*float sun_color[3]		*/	{ 1.000f, 0.840f, 0.750f },
		/*float sun_interpol_spd	*/	0.05f
	};

	DayNightCycle::time_of_day_settings tod_sunset = {
		/*float film_brightness		*/	-0.03f,
		/*float film_contrast		*/	1.155f,
		/*float film_desaturation	*/	0.0f,
		/*float film_darktint[3]	*/	{ 1.197f, 1.008f, 1.004f },
		/*float film_lighttint[3]	*/	{ 1.204f, 1.200f, 0.992f },
		/*float film_interpol_spd	*/	0.1f,

		/*float sun_light			*/	1.3f,
		/*float sun_color[3]		*/	{ 1.000f, 0.800f, 0.900f },
		/*float sun_interpol_spd	*/	0.1f
	};

	// suncolor lerp variable because steps are so small that they get discarded when packing to color
	float tod_suncolor_lerp[3] = { 0.0f };


	// *
	// suntweaks over time

	void DayNightCycle::lerp_suntweaks(const DayNightCycle::time_of_day_settings* lerp_to, const float delta_time)
	{
		auto r_lightTweakSunColor = Game::Dvar_FindVar("r_lightTweakSunColor");
		auto r_lightTweakSunLight = Game::Dvar_FindVar("r_lightTweakSunLight");

		if (r_lightTweakSunColor && r_lightTweakSunLight)
		{
			if (lerp_to)
			{
				if (lerp_to->film_interpolation_speed <= 0.0f)
				{
					// sunlight
					Game::Dvar_SetValue(r_lightTweakSunLight, lerp_to->sun_light);		
					r_lightTweakSunLight->modified = true;

					// pack suncolor and modify dvar
					Utils::byte3_pack_rgba(lerp_to->sun_color, r_lightTweakSunColor->current.color);

					for (auto i = 0; i < 3; i++)
					{
						r_lightTweakSunColor->latched.color[i] = r_lightTweakSunColor->current.color[i];
					}

					r_lightTweakSunColor->modified = true;

					return;
				}

				// sunlight
				Game::Dvar_SetValue(r_lightTweakSunLight, finterp_to(r_lightTweakSunLight->current.value, lerp_to->sun_light, delta_time, lerp_to->sun_interpolation_speed));
				r_lightTweakSunLight->modified = true;

				// unpack current suncolor
				float temp_suncolor[3] = { 0.0f };
				Utils::byte3_unpack_rgba(r_lightTweakSunColor->current.color, temp_suncolor);

				// update tod_suncolor_lerp if != temp_suncolor + epsilon (steps are so small that they get discarded when packing)
				if (!Utils::vector::_VectorCompareEpsilon(temp_suncolor, tod_suncolor_lerp, 0.05f))
				{
					Utils::vector::_VectorCopy(temp_suncolor, tod_suncolor_lerp, 3);
				}

				// lerp suncolor
				vinterp_to(tod_suncolor_lerp, 3, tod_suncolor_lerp, lerp_to->sun_color, delta_time, lerp_to->sun_interpolation_speed);

				// pack suncolor and modify dvar
				Utils::byte3_pack_rgba(tod_suncolor_lerp, r_lightTweakSunColor->current.color);

				for (auto i = 0; i < 3; i++) 
				{
					r_lightTweakSunColor->latched.color[i] = r_lightTweakSunColor->current.color[i];
				}

				r_lightTweakSunColor->modified = true;

				
			}
		}
	}


	// *
	// filmtweaks over time

	void DayNightCycle::lerp_filmtweaks(const DayNightCycle::time_of_day_settings* lerp_to, const float delta_time)
	{
		auto r_filmTweakBrightness = Game::Dvar_FindVar("r_filmTweakBrightness");
		auto r_filmTweakContrast = Game::Dvar_FindVar("r_filmTweakContrast");
		auto r_filmTweakDesaturation = Game::Dvar_FindVar("r_filmTweakDesaturation");
		auto r_filmTweakDarkTint = Game::Dvar_FindVar("r_filmTweakDarkTint");
		auto r_filmTweakLightTint = Game::Dvar_FindVar("r_filmTweakLightTint");

		if (r_filmTweakBrightness && r_filmTweakContrast && r_filmTweakDesaturation && r_filmTweakDarkTint && r_filmTweakLightTint)
		{
			if (lerp_to)
			{
				if (lerp_to->film_interpolation_speed <= 0.0f)
				{
					Game::Dvar_SetValue(r_filmTweakBrightness, lerp_to->film_brightness);		r_filmTweakBrightness->modified = true;
					Game::Dvar_SetValue(r_filmTweakContrast, lerp_to->film_contrast);			r_filmTweakContrast->modified = true;
					Game::Dvar_SetValue(r_filmTweakDesaturation, lerp_to->film_desaturation);	r_filmTweakDesaturation->modified = true;
					Game::Dvar_SetValue(r_filmTweakDarkTint, lerp_to->film_darktint, 3);		r_filmTweakDarkTint->modified = true;
					Game::Dvar_SetValue(r_filmTweakLightTint, lerp_to->film_lighttint, 3);		r_filmTweakLightTint->modified = true;

					return;
				}

				Game::Dvar_SetValue(r_filmTweakBrightness, finterp_to(r_filmTweakBrightness->current.value, lerp_to->film_brightness, delta_time, lerp_to->film_interpolation_speed));
				r_filmTweakBrightness->modified = true;

				Game::Dvar_SetValue(r_filmTweakContrast, finterp_to(r_filmTweakContrast->current.value, lerp_to->film_contrast, delta_time, lerp_to->film_interpolation_speed));
				r_filmTweakContrast->modified = true;

				Game::Dvar_SetValue(r_filmTweakDesaturation, finterp_to(r_filmTweakDesaturation->current.value, lerp_to->film_desaturation, delta_time, lerp_to->film_interpolation_speed));
				r_filmTweakDesaturation->modified = true;

				
				vinterp_to(r_filmTweakDarkTint->current.vector, 3, r_filmTweakDarkTint->current.vector, lerp_to->film_darktint, delta_time, lerp_to->film_interpolation_speed);
				r_filmTweakDarkTint->modified = true;

				vinterp_to(r_filmTweakLightTint->current.vector, 3, r_filmTweakLightTint->current.vector, lerp_to->film_lighttint, delta_time, lerp_to->film_interpolation_speed);
				r_filmTweakDarkTint->modified = true;
			}
		}
	}


	// *
	// called from _Client::on_set_cgame_time()

	void DayNightCycle::set_world_time()
	{
		auto cui = Game::clientUI;
		auto cl = Game::clients;

		if (cui && cl)
		{
			if (cui->connectionState == Game::CA_ACTIVE)
			{
				auto r_lightTweakSunDirection = Game::Dvar_FindVar("r_lightTweakSunDirection");

				if ( r_lightTweakSunDirection && time_running)
				{
					float sun_current_rot = r_lightTweakSunDirection->current.vector[0];
					float delta = float(cl->serverTimeDelta) * 0.0001f * time_scalar;
					float delta_abs = fabsf(delta);

					// ----------
					// transitions

					auto r_filmTweakEnable = Game::Dvar_FindVar("r_filmTweakEnable");
					auto r_filmUseTweaks = Game::Dvar_FindVar("r_filmUseTweaks");
					
					if (r_filmTweakEnable && r_filmUseTweaks && (!r_filmTweakEnable->current.enabled || !r_filmUseTweaks->current.enabled))
					{
						Game::Dvar_SetValue(r_filmTweakEnable, true);	r_filmTweakEnable->modified = true;
						Game::Dvar_SetValue(r_filmUseTweaks, true);		r_filmUseTweaks->modified = true;
					}

					auto r_drawSun = Game::Dvar_FindVar("r_drawSun");

					if (r_drawSun && r_drawSun->current.enabled)
					{
						Game::Dvar_SetValue(r_drawSun, false);	r_drawSun->modified = true;
					}

					// longer day / sunsets
					delta *= 0.7f;
					delta_abs *= 0.7f;

					// night time (360 equals 0 so start night transition slightly before 360/0)
					if (sun_current_rot >= 355.0f || sun_current_rot >= 0.0f && sun_current_rot <= 175.0f)
					{
						DayNightCycle::lerp_suntweaks(&tod_night, delta_abs);
						DayNightCycle::lerp_filmtweaks(&tod_night, delta_abs);

						// shorter night
						delta *= 1.3f;
						delta_abs *= 1.3f;
					}
					// sunrise
					else if (sun_current_rot <= 200.0f)
					{
						DayNightCycle::lerp_suntweaks(&tod_sunset, delta_abs);
						DayNightCycle::lerp_filmtweaks(&tod_sunset, delta_abs);
					}
					// day time
					else if (sun_current_rot <= 340.0f)
					{
						DayNightCycle::lerp_suntweaks(&tod_day, delta_abs);
						DayNightCycle::lerp_filmtweaks(&tod_day, delta_abs);
					}
					// sunset
					else
					{
						DayNightCycle::lerp_suntweaks(&tod_sunset, delta_abs);
						DayNightCycle::lerp_filmtweaks(&tod_sunset, delta_abs);
					}

					// ----------
					// sun position

					if (sun_current_rot >= 360.0f)
					{
						r_lightTweakSunDirection->current.vector[0] -= 360.0f;
						r_lightTweakSunDirection->latched.vector[0] -= 360.0f;
						r_lightTweakSunDirection->modified = true;
					}
					else if (sun_current_rot < 0.0f)
					{
						r_lightTweakSunDirection->current.vector[0] = 359.999f;
						r_lightTweakSunDirection->latched.vector[0] = 359.999f;
						r_lightTweakSunDirection->modified = true;
					}

					if (time_rotates_sun)
					{
						r_lightTweakSunDirection->current.vector[0] += delta;
						r_lightTweakSunDirection->latched.vector[0] += delta;
						r_lightTweakSunDirection->modified = true;
					}

					// setup times
					//state_minute = (r_lightTweakSunDirection->current.vector[0] - 90.0f) * (1440.0f / 270.0f); // 1 degree = 4 minutes
					//state_hours = state_minute * (1.0f / 60.0f);

					//state_hours_24 = static_cast<int>(floor(state_hours));
					//state_minutes_24 = static_cast<int>(fmodf(state_hours, 1) * 60.0f);
				}
			}
		}
	}


	// *
	// devgui

	void DayNightCycle::devgui_tab(Game::gui_menus_t& menu)
	{
		const float SLIDER_SPEED = 0.01f;

		if (ImGui::Button("Reload xcommon_iw3xo_addon"))
		{
			Game::Cbuf_AddText("loadzone xcommon_iw3xo_addon\n", 0);
		}

		ImGui::SameLine();

		if (ImGui::Button("Toggle Day/Night"))
		{
			const auto& dvar = Game::Dvar_FindVar("r_dayAndNight");
			if (dvar)
			{
				Game::Dvar_SetValue(dvar, !dvar->current.enabled);
			}
		}

		if (ImGui::CollapsingHeader("Cloud Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			ImGui::ColorEdit3("Cloud Sunset Color", outscatter_color01, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
			ImGui::DragFloat("Extinction Scale", &outscatter_scale01, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");

			ImGui::DragFloat3("Cloud Day Color", outscatter_color02, SLIDER_SPEED, 0.0f, 4.0f, "%.4f");
			ImGui::DragFloat("Cloud Horizon Fade", &outscatter_scale02, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");

			SPACING(0.0f, 4.0f);

			ImGui::Text("FilterTap[2]:");
			ImGui::DragFloat("Skydome Scale", &dn_skydome_scale, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");
			ImGui::DragFloat("Cloud Coverage", &dn_cloud_coverage, SLIDER_SPEED, 0.0f, 10.0f, "%.4f");
			ImGui::DragFloat("Cloud Thickness", &dn_cloud_thickness, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");
			ImGui::DragFloat("Cloud Absorbtion", &dn_cloud_absorbtion, SLIDER_SPEED, 0.0f, 10.0f, "%.4f");

			SPACING(0.0f, 4.0f);

			ImGui::Text("FilterTap[3]:");
			ImGui::DragFloat("Star Scale",					&dn_star_scale, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");
			ImGui::DragFloat("Cloud Step Distance XZ",		&dn_cloud_step_distance_xz, SLIDER_SPEED, 0.0f, 500.0f, "%.4f");
			ImGui::DragFloat("Not used",					&dn_cloud_steps_after_threshold, SLIDER_SPEED, 0.0f, 30.0f, "%.4f");
			ImGui::DragFloat("Cloud Exposure",				&dn_cloud_exposure, SLIDER_SPEED, 0.0f, 10.0f, "%.4f");

			SPACING(0.0f, 4.0f);

			ImGui::Text("FilterTap[4]:");
			ImGui::DragFloat3("Cloud Wind Direction", dn_cloud_wind_vec, SLIDER_SPEED, 0.0f, 10.0f, "%.4f");
			ImGui::DragFloat("Cloud Wind Speed",	 &dn_cloud_wind_speed, SLIDER_SPEED, 0.0f, 10.0f, "%.4f");

			SPACING(0.0f, 4.0f);

			ImGui::Text("FilterTap[5]:");
			ImGui::DragFloat("Sky Sun Intensity",  &dn_sky_sun_intensity, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");
			ImGui::DragFloat3("Sky Rayleigh Coeff", dn_sky_rayleigh_coeff, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");

			SPACING(0.0f, 4.0f);

			ImGui::Text("FilterTap[6]:");
			ImGui::DragFloat("Sky Mie Coeff",		&dn_sky_mie_coeff, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");
			ImGui::DragFloat("Sky Rayleigh Scale",	&dn_sky_rayleigh_scale, SLIDER_SPEED, 0.0f, 2000.0f, "%.4f");
			ImGui::DragFloat("Sky Mie Scale",		&dn_sky_mie_scale, SLIDER_SPEED, 0.0f, 500.0f, "%.4f");
			ImGui::DragFloat("Sky Mie Scatter Dir", &dn_sky_mie_scatter_dir, SLIDER_SPEED, 0.0f, 10.0f, "%.4f");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}


		// dupe of film/lighttweaks tab
		if (ImGui::CollapsingHeader("Filmtweaks", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto r_filmTweakEnable = Game::Dvar_FindVar("r_filmTweakEnable");
			auto r_filmUseTweaks = Game::Dvar_FindVar("r_filmUseTweaks");

			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			if (r_filmTweakEnable && r_filmUseTweaks)
			{
				ImGui::Checkbox("Enable Filmtweaks", &r_filmTweakEnable->current.enabled);
				ImGui::SameLine();
				ImGui::Checkbox("Use Filmtweaks", &r_filmUseTweaks->current.enabled);

				if (r_filmTweakEnable->current.enabled && r_filmUseTweaks->current.enabled)
				{
					auto r_filmTweakInvert = Game::Dvar_FindVar("r_filmTweakInvert");
					auto r_filmTweakLightTint = Game::Dvar_FindVar("r_filmTweakLightTint");
					auto r_filmTweakDarkTint = Game::Dvar_FindVar("r_filmTweakDarkTint");
					auto r_filmTweakDesaturation = Game::Dvar_FindVar("r_filmTweakDesaturation");
					auto r_filmTweakContrast = Game::Dvar_FindVar("r_filmTweakContrast");
					auto r_filmTweakBrightness = Game::Dvar_FindVar("r_filmTweakBrightness");

					if (r_filmTweakInvert && r_filmTweakLightTint && r_filmTweakDarkTint && r_filmTweakDesaturation && r_filmTweakContrast && r_filmTweakBrightness)
					{
						ImGui::SameLine();
						ImGui::Checkbox("Invert", &r_filmTweakInvert->current.enabled);

						SPACING(0.0f, 4.0f);

						// light and darktint should be capped at 2.0 since hdr allows for much higher values
						ImGui::ColorEdit3("Light Tint", r_filmTweakLightTint->current.vector, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
						ImGui::ColorEdit3("Dark Tint", r_filmTweakDarkTint->current.vector, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
						ImGui::DragFloat("Desaturation", &r_filmTweakDesaturation->current.value, 0.005f, 0.0f, 1.0f);
						ImGui::DragFloat("Contrast", &r_filmTweakContrast->current.value, 0.005f, 0.0f, 4.0f);
						ImGui::DragFloat("Brightness", &r_filmTweakBrightness->current.value, 0.005f, -1.0f, 1.0f);
					}
				}
			}

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		if (ImGui::CollapsingHeader("Lighttweaks", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto r_lightTweakSunColor = Game::Dvar_FindVar("r_lightTweakSunColor");
			auto r_lightTweakSunDirection = Game::Dvar_FindVar("r_lightTweakSunDirection");
			auto r_lightTweakSunLight = Game::Dvar_FindVar("r_lightTweakSunLight");

			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			if (r_lightTweakSunColor && r_lightTweakSunDirection && r_lightTweakSunLight)
			{
				// unpack the color and use a swap var
				float sun_color_swap[4] = { 0.0f };
				Utils::byte4_unpack_rgba(r_lightTweakSunColor->current.color, sun_color_swap);

				if (ImGui::ColorEdit3("Sun Color", sun_color_swap, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel))
				{
					Utils::byte4_pack_rgba(sun_color_swap, r_lightTweakSunColor->current.color);

					for (auto i = 0; i < 4; i++) {
						r_lightTweakSunColor->latched.color[i] = r_lightTweakSunColor->current.color[i];
					}

					r_lightTweakSunColor->modified = true;

				}

				ImGui::Checkbox("Enable Day/Night Cycle", &time_running);
				ImGui::SameLine();
				ImGui::Checkbox("Enable Sun Rotation", &time_rotates_sun);
				ImGui::DragFloat("Day/Night Time Scalar", &time_scalar, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");

				SPACING(0.0f, 4.0f);

				ImGui::DragFloat3("Sun Direction", Gui::DvarGetSet<float*>(r_lightTweakSunDirection), 0.25f, -360.0f, 360.0f, "%.2f");
				ImGui::SliderFloat("Sun Light", Gui::DvarGetSet<float*>(r_lightTweakSunLight), r_lightTweakSunLight->domain.value.min, r_lightTweakSunLight->domain.value.max, "%.2f");
			
				if (ImGui::CollapsingHeader("Debug Constants when in use", ImGuiTreeNodeFlags_CollapsingHeader))
				{
					SPACING(0.0f, 4.0f);

					/*ImGui::Text("Debug Times: %.4f   %.4f   %.4f",
						debug_time01, debug_time02, debug_time03);

					ImGui::Text("Time of Day: %d:%d", 
						state_hours_24, state_minutes_24);*/

					ImGui::Text("Sun Position: %.2f %.2f %.2f %.2f",
						sunposition_constant[0], sunposition_constant[1], sunposition_constant[2], sunposition_constant[3]);

					ImGui::Text("Sun Diffuse: %.2f %.2f %.2f %.2f",
						sundiffuse_constant[0], sundiffuse_constant[1], sundiffuse_constant[2], sundiffuse_constant[3]);

					ImGui::Text("Filmtweak Colorbias: %.2f %.2f %.2f %.2f",
						colorbias_constant[0], colorbias_constant[1], colorbias_constant[2], colorbias_constant[3]);
				}

				SPACING(0.0f, 4.0f);

				ImGui::Checkbox("Tweak Framefog", &framefog_tweaks);

				ImGui::ColorEdit4("Fog Color", framefog_color, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);

			}

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}
	}


	// *
	// skybox constants

	// called from _Renderer::pixelshader_custom_constants
	void DayNightCycle::set_pixelshader_constants(Game::GfxCmdBufState* state, Game::MaterialShaderArgument* const arg_def)
	{
		if (state->pass->pixelShader && !Utils::Q_stricmp(state->pass->pixelShader->name, "iw3xo_daynight"))
		{
			// *
			// get constants for debugging purposes
			if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_SUN_DIFFUSE)
			{
				(*Game::dx9_device_ptr)->GetPixelShaderConstantF(arg_def->dest, sundiffuse_constant, 1);
			}

			if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_SUN_POSITION)
			{
				(*Game::dx9_device_ptr)->GetPixelShaderConstantF(arg_def->dest, sunposition_constant, 1);
			}

			if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_COLOR_BIAS)
			{
				(*Game::dx9_device_ptr)->GetPixelShaderConstantF(arg_def->dest, colorbias_constant, 1);
			}


			// *
			// set constants
			if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0)
			{
				float constant[4] = { outscatter_color01[0], outscatter_color01[1], outscatter_color01[2], outscatter_scale01 };
				(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}

			if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_1)
			{
				float constant[4] = { outscatter_color02[0], outscatter_color02[1], outscatter_color02[2], outscatter_scale02 };
				(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}

			if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2)
			{
				float constant[4] = { dn_skydome_scale, dn_cloud_coverage, dn_cloud_thickness, dn_cloud_absorbtion };
				(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}

			if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_3)
			{
				float constant[4] = { dn_star_scale, dn_cloud_step_distance_xz, dn_cloud_steps_after_threshold, dn_cloud_exposure };
				(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}

			if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_4)
			{
				float constant[4] = { dn_cloud_wind_vec[0], dn_cloud_wind_vec[1], dn_cloud_wind_vec[2], dn_cloud_wind_speed };
				(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}

			if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_5)
			{
				float constant[4] = { dn_sky_sun_intensity, dn_sky_rayleigh_coeff[0], dn_sky_rayleigh_coeff[1], dn_sky_rayleigh_coeff[2] };
				(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}

			if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_6)
			{
				float constant[4] = { dn_sky_mie_coeff, dn_sky_rayleigh_scale, dn_sky_mie_scale, dn_sky_mie_scatter_dir };
				(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}
		}
	}


	// *
	// overwrite default sky

	// called from _Renderer::R_SetMaterial
	void DayNightCycle::overwrite_sky_material(Game::switch_material_t* swm)
	{
		if (Dvars::r_dayAndNight && Dvars::r_dayAndNight->current.enabled)
		{
			if (Utils::StartsWith(swm->current_material->info.name, "wc/sky_"))
			{
				swm->technique_type = Game::TECHNIQUE_UNLIT;
				_Renderer::switch_material(swm, "wc/iw3xo_daynight");
			}
		}
	}


	// *
	// set fog

	void r_set_frame_fog(Game::GfxCmdBufInput* input)
	{
		if (input)
		{
			if (!framefog_tweaks)
			{
				Utils::vector::_VectorCopy(input->consts[Game::CONST_SRC_CODE_FOG_COLOR], framefog_color, 4);
			}

			else
			{
				input->consts[Game::CONST_SRC_CODE_FOG_COLOR][0] = framefog_color[0];
				input->consts[Game::CONST_SRC_CODE_FOG_COLOR][1] = framefog_color[1];
				input->consts[Game::CONST_SRC_CODE_FOG_COLOR][2] = framefog_color[2];
				input->consts[Game::CONST_SRC_CODE_FOG_COLOR][3] = framefog_color[3];

				// set the clear color so that the bottom of the skybox matches the fog
				auto r_clear = Game::Dvar_FindVar("r_clear");
				auto r_clearColor = Game::Dvar_FindVar("r_clearColor");

				if (r_clear && r_clearColor)
				{
					Game::Dvar_SetValue(r_clear, 3);	r_clear->modified = true;

					Utils::byte4_pack_rgba(framefog_color, r_clearColor->current.color);
					for (auto i = 0; i < 4; i++)
					{
						r_clearColor->latched.color[i] = r_clearColor->current.color[i];
					}
					r_clearColor->modified = true;
				}
			}
		}
	}

	__declspec(naked) void r_setframefog_stub()
	{
		const static uint32_t rtn_pt = 0x63DBE5;
		__asm
		{
			// stock op's
			fstp    dword ptr[eax + 29Ch];
			
			// eax = GfxCmdBufInput*
			pushad;
			push	eax;
			call	r_set_frame_fog;
			add		esp, 4;
			popad;

			jmp		rtn_pt;
		}
	}


	DayNightCycle::DayNightCycle()
	{ 
		// mid-hook R_SetFrameFog
		Utils::Hook::Nop(0x63DBDF, 6); Utils::Hook(0x63DBDF, r_setframefog_stub, HOOK_JUMP).install()->quick();

		Dvars::r_dayAndNight = Game::Dvar_RegisterBool(
			/* name		*/ "r_dayAndNight",
			/* desc		*/ "enable day and night cycle",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);
	}

	DayNightCycle::~DayNightCycle()
	{ }
}