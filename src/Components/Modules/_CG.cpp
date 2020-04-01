#include "STDInclude.hpp"

namespace Components
{
	// *
	// return font string for style
	char *_CG::GetFontForStyle(int fontStyle)
	{
		return	fontStyle == 0 ? FONT_SMALL_DEV : fontStyle == 1 ? FONT_BIG_DEV		: fontStyle == 2 ? FONT_CONSOLE :
				fontStyle == 3 ? FONT_BIG		: fontStyle == 4 ? FONT_SMALL		: fontStyle == 5 ? FONT_BOLD :
				fontStyle == 6 ? FONT_NORMAL	: fontStyle == 7 ? FONT_EXTRA_BIG	: FONT_OBJECTIVE;
	}

	// *
	// custom huds
	void CG_DrawCustomHud()
	{
		// defaults
		char* font		= FONT_CONSOLE;
		float color[4]	= { 1.0f, 0.6f, 0.6f, 1.0f };

		// Pmove Speed Hud
		if (Dvars::pm_hud_enable != nullptr && Dvars::pm_hud_enable->current.enabled)
		{
			char* movementType	= Dvars::pm_movementType->current.integer == Game::PM_MTYPE::STOCK ? "Stock" 
								: Dvars::pm_movementType->current.integer == Game::PM_MTYPE::DEFRAG ? "Defrag" : "CS-Surf";

			float xySpeed = sqrtf(Game::Globals::locPmove_playerVelocity.x * Game::Globals::locPmove_playerVelocity.x
								+ Game::Globals::locPmove_playerVelocity.y * Game::Globals::locPmove_playerVelocity.y);

			Game::DrawTextWithEngine(
				/*  x  */ Dvars::pm_hud_x->current.value,
				/*  y  */ Dvars::pm_hud_y->current.value,
				/* scX */ Dvars::pm_hud_fontScale->current.value,
				/* scY */ Dvars::pm_hud_fontScale->current.value,
				/* fot */ _CG::GetFontForStyle(Dvars::pm_hud_fontStyle->current.integer),
				/* col */ color,
				/* txt */ Utils::VA("Current Mode: %s\nCurrent Speed: %.2lf", movementType, xySpeed));
		}

		// Debug Collision Hud
		if (Dvars::r_drawCollision_hud != nullptr && Dvars::r_drawCollision_hud->current.enabled)
		{
			if (Dvars::r_drawCollision != nullptr && Dvars::r_drawCollision->current.integer != 0)
			{
				Game::DrawTextWithEngine(
					/*  x  */ Dvars::r_drawCollision_hud_x->current.value,
					/*  y  */ Dvars::r_drawCollision_hud_y->current.value,
					/* scX */ Dvars::r_drawCollision_hud_fontScale->current.value,
					/* scY */ Dvars::r_drawCollision_hud_fontScale->current.value,
					/* fot */ _CG::GetFontForStyle(Dvars::r_drawCollision_hud_fontStyle->current.integer),
					/* col */ color,
					/* txt */ Utils::VA("Total amount of Brushes used for calculations: %d \n"
										"Total amount of Polygons drawn: %d", Game::Globals::drawnBrushAmount, Game::Globals::drawnPlanesAmount));
			}
		}
	}

	// Hook call to CG_Drawcrosshair in CG_Draw2D ~ only active if cg_draw2d
	__declspec(naked) void cg_drawcustomhud_stub()
	{
		const static uint32_t CG_DrawCrosshair = 0x4311A0;
		__asm
		{
			pushad
			call	CG_DrawCustomHud
			popad

			jmp		CG_DrawCrosshair
			retn
		}
	}

	_CG::_CG()
	{
		// Hook call to CG_Drawcrosshair in CG_Draw2D ~ only active if cg_draw2d
		Utils::Hook(0x42F6B5, cg_drawcustomhud_stub, HOOK_CALL).install()->quick();
	}

	_CG::~_CG()
	{ }
}