#pragma once

namespace Components
{
	class Devgui : public Component
	{
	public:
		Devgui();
		~Devgui();
		const char* getName() override { return "Devgui"; };

		static void imgui_init();
		static void reset();
		static void reset_mouse();
		static void render_loop();
		static void load_settings();
		static void save_settings();

		static void update_hWnd(void* hwnd);
		static void toggle(bool onCommand);

		static void begin_frame();
		static void end_frame();

		static void create_devgui(bool* p_open);
		static void menu_tab_settings();
		static void menu_tab_movement();
		static void menu_tab_shaders();
		static void menu_tab_collision();
		static void menu_tab_radiant();

	private:
	};
}
