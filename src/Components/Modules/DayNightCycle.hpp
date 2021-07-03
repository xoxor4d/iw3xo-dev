#pragma once

namespace Components
{
	class DayNightCycle : public Component
	{
	public:
		DayNightCycle();
		~DayNightCycle();
		const char* getName() override { return "DayNightCycle"; };

		struct time_of_day_settings
		{
			float film_brightness;
			float film_contrast;
			float film_desaturation;
			float film_darktint[3];
			float film_lighttint[3];
			float film_interpolation_speed;

			float sun_light;
			float sun_color[3];
			float sun_interpolation_speed;

			bool  tweak_fog;
			float fog_start;
			float fog_density;
			float fog_color[3];
		}; const static int TOD_VAR_AMOUNT = 13;

		static void devgui_tab(Game::gui_menus_t& menu);
		static void set_pixelshader_constants(Game::GfxCmdBufState* state, Game::MaterialShaderArgument* const arg_def);
		static void overwrite_sky_material(Game::switch_material_t* swm);
		
		static void lerp_fogtweaks(const DayNightCycle::time_of_day_settings* lerp_to, const float delta_time);
		static void lerp_suntweaks(const time_of_day_settings* lerp_to, const float delta_time);
		static void lerp_filmtweaks(const time_of_day_settings* lerp_to, const float delta_time);
		static void lerp_time_of_day(const DayNightCycle::time_of_day_settings* lerp_to, const float delta_time);
		static void set_world_time();

	private:
	};
}
