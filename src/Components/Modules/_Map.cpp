#include "STDInclude.hpp"

namespace Components
{
	void _Map::OnLoad()
	{
		memset(&Game::Globals::cgsAddons, 0, sizeof(Game::cgsAddon));

		if (Components::active.RadiantRemote)
		{
			memset(&Game::Globals::dynBrushModels, 0, sizeof(Game::dynBrushModelsArray_t));
			memset(&Game::Globals::rad_savedBrushes, 0, sizeof(Game::savedRadiantBrushes));

			RadiantRemote::CM_FindDynamicBrushModels();
		}

		if (Components::active.RB_DrawCollision)
		{
			Game::Globals::dbgColl_initialized = false;
		}
	}

	void _Map::OnUnload()
	{
		if (Components::active.RadiantRemote)
		{
			RadiantRemote::SV_Shutdown();
		}
	}

	// --------

	__declspec(naked) void sv_spawnserver_stub()
	{
		const static uint32_t retnPt = 0x52F8A7;

		// overwritten op's
		__asm	add     esp, 8
		__asm and esi, 0FFFFFFF0h

		// huh
		__asm	Call	_Map::OnLoad
		__asm	jmp		retnPt
	}

	__declspec(naked) void com_shutdowninternal_stub()
	{
		const static uint32_t Com_Restart_Jmp = 0x5004C0;

		__asm	Call	_Map::OnUnload
		__asm	jmp		Com_Restart_Jmp
	}

	// --------

	_Map::_Map()
	{ 
		// On map load
		Utils::Hook::Nop(0x52F8A1, 6);  Utils::Hook(0x52F8A1, sv_spawnserver_stub, HOOK_JUMP).install()->quick(); // after SV_InitGameProgs before G_RunFrame

		// On map unload
		Utils::Hook(0x4FCDF8, com_shutdowninternal_stub, HOOK_JUMP).install()->quick(); // before Com_Restart
	}

	_Map::~_Map()
	{ }
}