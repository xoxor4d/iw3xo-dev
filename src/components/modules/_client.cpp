#include "std_include.hpp"

namespace components
{
	void on_disconnect()
	{ 
		/*if (Components::active.Mvm)
		{
			const auto& cl_avidemo = Game::Dvar_FindVar("cl_avidemo");

			if (cl_avidemo && cl_avidemo->current.integer)
			{
				Game::Dvar_SetValue(cl_avidemo, 0);
			}

			if (dvars::cl_avidemo_streams && dvars::cl_avidemo_streams->current.integer)
			{
				Game::Dvar_SetValue(dvars::cl_avidemo_streams, 0);
			}
		}*/

		game::glob::mainmenu_fade_done = false;
		game::glob::loaded_main_menu = false;

		if (components::active.gui)
		{
			GET_GGUI.menus[game::GUI_MENUS::CHANGELOG].menustate = false;
		}
    }

	__declspec(naked) void on_disconnect_stub()
	{
		const static uint32_t retn_addr = 0x4696D5;
		__asm
		{
            // stock op's
            xor     eax, eax;
            cmp     esi, 5;

            pushad;
            call	on_disconnect;
            popad;
			
			jmp		retn_addr;
		}
	}

	// --------

	// actually after time was set
	void on_set_cgame_time()
	{
		if (components::active.daynight_cycle)
		{
			if (dvars::r_dayAndNight && dvars::r_dayAndNight->current.enabled)
			{
				daynight_cycle::set_world_time();
			}
		}
	}

	__declspec(naked) void on_set_cgame_time_stub()
	{
		const static uint32_t CL_SetCGameTime_func = 0x45C440;
		const static uint32_t retn_addr = 0x46C9FB;
		__asm
		{
			// stock op's
			call	CL_SetCGameTime_func;

			pushad;
			call	on_set_cgame_time;
			popad;

			jmp		retn_addr;
		}
	}

	_client::_client()
	{ 
		// CL_Disconnect, random spot (mvm hooks on first op)
		utils::hook(0x4696D0, on_disconnect_stub, HOOK_JUMP).install()->quick();

		// CL_SetCGameTime, called every client frame
		utils::hook(0x46C9F6, on_set_cgame_time_stub, HOOK_JUMP).install()->quick();
	}
}