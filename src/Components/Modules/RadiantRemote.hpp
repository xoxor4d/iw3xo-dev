#pragma once

#define SERVER_DEBUG_CMDS true

namespace Components
{
	class RadiantRemote : public Component
	{
	public:
		RadiantRemote();
		~RadiantRemote();

		const char* getName() override { return "RadiantRemote"; };

		static void		SV_Init();
		static void		SV_Shutdown();
		static bool		SV_UpdateSocket();
		static void		SV_ReceivePackets();

		static void		SV_SendPacket(Game::ServerCommand *Command);
		static void		Cmd_SendCamera(const float *origin, const float *angles);

		static void		Cmd_ProcessDvar(Game::SpawnVar* spawnVar);
		static void		Cmd_ProcessCamera(Game::SpawnVar *spawnVar);
		static void		Cmd_ProcessBrushSelect(Game::SpawnVar *spawnVar);
		static void		Cmd_ProcessBrushAmount(Game::SpawnVar *spawnVar);
		static void		Cmd_ProcessBrushNum(Game::SpawnVar *spawnVar);
		static void		Cmd_ProcessBrushFaceNormals(Game::SpawnVar *spawnVar, int brushNum);
		static void		Cmd_ProcessBrushFaceCount(Game::SpawnVar *spawnVar, int brushNum);
		static void		Cmd_ProcessBrushFace(Game::SpawnVar *spawnVar, int brushNum);

		// -------
		// Clipmap

		static void		CM_FindDynamicBrushModels();
		static void		RadiantDebugBrush();

		static void		devgui_tab(Game::gui_menus_t& menu);

	private:
	};
}
