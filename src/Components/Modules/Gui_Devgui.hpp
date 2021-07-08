#pragma once

namespace Components
{
	class Gui_Devgui : public Component
	{
	public:
		Gui_Devgui();
		~Gui_Devgui();
		const char* getName() override { return "Gui_Devgui"; };

		static void create_devgui(Game::gui_menus_t& menu);
		
	private:
		static void menu_tab_movement(Game::gui_menus_t& menu);
		static void menu_tab_collision(Game::gui_menus_t& menu);
		static void menu_tab_shaders(Game::gui_menus_t& menu);
		static void menu_tab_radiant(Game::gui_menus_t& menu);
		static void menu_tab_visuals(Game::gui_menus_t& menu);
		static void menu_tab_settings(Game::gui_menus_t& menu);
	};
}
