#pragma once

namespace Components
{
	class gui_devgui final : public Component
	{
	public:
		gui_devgui();
		const char* getName() override { return "gui_devgui"; };

		static void create_devgui(Game::gui_menus_t& menu);
		
	private:
		static void menu_tab_movement(Game::gui_menus_t& menu);
		static void menu_tab_collision(Game::gui_menus_t& menu);
		static void menu_tab_shaders(Game::gui_menus_t& menu);
		static void menu_tab_visuals(Game::gui_menus_t& menu);
		static void menu_tab_settings(Game::gui_menus_t& menu);
	};
}
