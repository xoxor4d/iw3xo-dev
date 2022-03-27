#pragma once

namespace components
{
	class gui_devgui final : public component
	{
	public:
		gui_devgui();
		const char* get_name() override { return "gui_devgui"; };

		static void create_devgui(Game::gui_menus_t& menu);
		
	private:
		static void menu_tab_movement(Game::gui_menus_t& menu);
		static void menu_tab_collision(Game::gui_menus_t& menu);
		static void menu_tab_shaders(Game::gui_menus_t& menu);
		static void menu_tab_visuals(Game::gui_menus_t& menu);
		static void menu_tab_settings(Game::gui_menus_t& menu);
	};
}
