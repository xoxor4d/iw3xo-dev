#include "STDInclude.hpp"

namespace Components
{
	void _Client::OnDisconnect()
	{ 
		if (Components::active.Mvm)
		{
			const auto& cl_avidemo = Game::Dvar_FindVar("cl_avidemo");

			if (cl_avidemo && cl_avidemo->current.integer)
			{
				Game::Dvar_SetValue(cl_avidemo, 0);
			}

			if (Dvars::cl_avidemo_streams && Dvars::cl_avidemo_streams->current.integer)
			{
				Game::Dvar_SetValue(Dvars::cl_avidemo_streams, 0);
			}
		}

		Game::Globals::mainmenu_fadeDone = false;
		Game::Globals::loaded_MainMenu = false;

		if (Components::active.Gui)
		{
			GET_GGUI.menus[Game::GUI_MENUS::CHANGELOG].menustate = false;
		}
    }

	__declspec(naked) void on_disconnect_stub()
	{
		const static uint32_t rtnPt = 0x4696D5;
		__asm
		{
            // stock op's
            xor     eax, eax;
            cmp     esi, 5;

            pushad;
            call	_Client::OnDisconnect;
            popad;
			
			jmp		rtnPt;
		}
	}

	// --------

	_Client::_Client()
	{ 
		// CL_Disconnect, random spot (mvm hooks on first op)
		Utils::Hook(0x4696D0, on_disconnect_stub, HOOK_JUMP).install()->quick();
	}

	_Client::~_Client()
	{ }
}