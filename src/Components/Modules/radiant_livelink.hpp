#pragma once

#define SERVER_DEBUG_CMDS true

namespace Components
{
	class radiant_livelink final : public Component
	{
	public:
		radiant_livelink();
		const char* getName() override { return "radiant_livelink"; };

		static void		SV_Init();
		static void		SV_Shutdown();
		static bool		SV_UpdateSocket();
		static void		SV_ReceivePackets();

		static void		send_packet(Game::ServerCommand *cmd);
		static void		cmd_send_camera(const float *origin, const float *angles);

		static void		cmd_process_dvar(Game::SpawnVar* spawnVar);
		static void		cmd_process_camera(Game::SpawnVar *spawnVar);
		static void		Cmd_ProcessBrushSelect(Game::SpawnVar *spawnVar);
		static void		Cmd_ProcessBrushAmount(Game::SpawnVar *spawnVar);
		static void		Cmd_ProcessBrushNum(Game::SpawnVar *spawnVar);
		static void		Cmd_ProcessBrushFaceNormals(Game::SpawnVar *spawnVar, int brushNum);
		static void		Cmd_ProcessBrushFaceCount(Game::SpawnVar *spawnVar, int brushNum);
		static void		Cmd_ProcessBrushFace(Game::SpawnVar *spawnVar, int brushNum);

		// -------
		// Clipmap

		static void		find_dynamic_bmodels();
		static void		draw_debug_brush();

		static void		devgui_tab(Game::gui_menus_t& menu);
	};
}
