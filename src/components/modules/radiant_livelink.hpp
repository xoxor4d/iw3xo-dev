#pragma once

#define SERVER_DEBUG_CMDS true

namespace components
{
	class radiant_livelink final : public component
	{
	public:
		radiant_livelink();
		const char* get_name() override { return "radiant_livelink"; };

		static void		init();
		static void		shutdown_livelink();
		static bool		update_socket();
		static void		receive_packets();

		static void		send_packet(game::radiant_server_command_s *cmd);
		static void		cmd_send_camera(const float *origin, const float *angles);

		static void		cmd_process_dvar(game::SpawnVar* spawn_var);
		static void		cmd_process_camera(game::SpawnVar *spawn_var);
		static void		cmd_process_brush_select(game::SpawnVar *spawn_var);
		static void		cmd_process_brush_amount(game::SpawnVar *spawn_var);
		static void		cmd_process_brush_num(game::SpawnVar *spawnVar);
		static void		cmd_process_brush_face_normals(game::SpawnVar *spawn_var, int brush_num);
		static void		cmd_process_brush_face_count(game::SpawnVar *spawnVar, int brush_num);
		static void		cmd_process_brush_face(game::SpawnVar *spawnVar, int brushNum);

		// -------
		// Clipmap

		static void		find_dynamic_bmodels();
		static void		draw_debug_brush();

		static void		devgui_tab(game::gui_menus_t& menu);
	};
}
