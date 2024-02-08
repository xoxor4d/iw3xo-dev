#pragma once

namespace components
{
	// show tooltip after x seconds
	#define TTDELAY 1.0f 

	// tooltip with delay
	#define TT(tooltip) if (ImGui::IsItemHoveredDelay(TTDELAY)) { ImGui::SetTooltip(tooltip); }
	
	// spacing dummy
	#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 
	
	// seperator with spacing
	#define SEPERATORV(spacing) SPACING(0.0f, spacing); ImGui::Separator(); SPACING(0.0f, spacing); 
	
	// execute a single command
	#define CMDEXEC(command) game::Cmd_ExecuteSingleCommand(0, 0, command)

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
