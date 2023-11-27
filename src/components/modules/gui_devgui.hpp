#pragma once

namespace components
{
	class gui_devgui final : public component
	{
	public:
		gui_devgui();
		const char* get_name() override { return "gui_devgui"; };

		static void create_devgui(game::gui_menus_t& menu);

	private:
		static void menu_tab_movement(game::gui_menus_t& menu);
		static void menu_tab_collision(game::gui_menus_t& menu);
		static void menu_tab_shaders(game::gui_menus_t& menu);
		static void menu_tab_visuals(game::gui_menus_t& menu);
		static void menu_tab_settings(game::gui_menus_t& menu);
	};
}
