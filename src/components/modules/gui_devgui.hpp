#pragma once

namespace components
{
	class gui_devgui final : public component
	{
	public:
		gui_devgui();
		const char* get_name() override { return "gui_devgui"; };

		static void create_devgui(game::gui_menus_t& menu);

		static inline bool  rtx_spawn_light = false;
		static inline float rtx_debug_light_origin[3] = { 0.0f, 0.0f, 400.0 };
		static inline float rtx_debug_light_range = 600.0f;
		static inline float rtx_debug_light_color[3] = { 1.0f, 1.0f, 1.0 };

	private:
		static void menu_tab_movement(game::gui_menus_t& menu);
		static void menu_tab_collision(game::gui_menus_t& menu);
		static void menu_tab_shaders(game::gui_menus_t& menu);
		static void menu_tab_visuals(game::gui_menus_t& menu);
		static void menu_tab_settings(game::gui_menus_t& menu);
	};
}
