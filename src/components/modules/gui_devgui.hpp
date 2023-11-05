#pragma once

namespace components
{
	class gui_devgui final : public component
	{
	public:
		gui_devgui();
		const char* get_name() override { return "gui_devgui"; };

		static void create_devgui(game::gui_menus_t& menu);

		static inline float dev_vec_01[3] = { 00.0f, 0.0f, 600.0 };
		static inline float dev_vec_02[3] = { 0.0f, 2.0f, 400.0 };
		static inline float dev_vec_03[3] = { 0.0f, 0.0f, 400.0 };
		static inline float dev_vec_04[3] = { 1.0f, 1.0f, 1.0 };

	private:
		static void menu_tab_movement(game::gui_menus_t& menu);
		static void menu_tab_collision(game::gui_menus_t& menu);
		static void menu_tab_shaders(game::gui_menus_t& menu);
		static void menu_tab_visuals(game::gui_menus_t& menu);
		static void menu_tab_settings(game::gui_menus_t& menu);
	};
}
