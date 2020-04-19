#include "STDInclude.hpp"

namespace Components
{
	// *
	// return font string for style
	char* _CG::GetFontForStyle(int fontStyle)
	{
		return	fontStyle == 0 ? FONT_SMALL_DEV : fontStyle == 1 ? FONT_BIG_DEV		: fontStyle == 2 ? FONT_CONSOLE :
				fontStyle == 3 ? FONT_BIG		: fontStyle == 4 ? FONT_SMALL		: fontStyle == 5 ? FONT_BOLD :
				fontStyle == 6 ? FONT_NORMAL	: fontStyle == 7 ? FONT_EXTRA_BIG	: FONT_OBJECTIVE;
	}

	// *
	// custom huds
	void CG_DrawCustomHud()
	{
		// Pmove Speed Hud
		if (Dvars::pm_hud_enable && Dvars::pm_hud_enable->current.enabled)
		{
			char* movementType	= Dvars::pm_movementType->current.integer == Game::PM_MTYPE::STOCK ? "Stock" 
								: Dvars::pm_movementType->current.integer == Game::PM_MTYPE::DEFRAG ? "Defrag" : "CS-Surf";

			float xySpeed = sqrtf(Game::Globals::locPmove_playerVelocity.x * Game::Globals::locPmove_playerVelocity.x
								+ Game::Globals::locPmove_playerVelocity.y * Game::Globals::locPmove_playerVelocity.y);

			Game::DrawTextWithEngine(
				/* x	*/ Dvars::pm_hud_x->current.value,
				/* y	*/ Dvars::pm_hud_y->current.value,
				/* scaX */ Dvars::pm_hud_fontScale->current.value,
				/* scaY */ Dvars::pm_hud_fontScale->current.value,
				/* font */ _CG::GetFontForStyle(Dvars::pm_hud_fontStyle->current.integer),
				/* colr */ Dvars::pm_hud_fontColor->current.vector,
				/* txt	*/ Utils::VA("Current Mode: %s\nCurrent Speed: %.2lf", movementType, xySpeed));
		}

		// Debug Collision Hud
		if (Dvars::r_drawCollision_hud && Dvars::r_drawCollision_hud->current.enabled)
		{
			if (Dvars::mapexport_selectionMode && Dvars::mapexport_selectionMode->current.integer != 0)
			{
				Game::DrawTextWithEngine(
					/* x	*/ Dvars::r_drawCollision_hud_position->current.vector[0],
					/* y	*/ Dvars::r_drawCollision_hud_position->current.vector[1],
					/* scaX */ Dvars::r_drawCollision_hud_fontScale->current.value,
					/* scaY */ Dvars::r_drawCollision_hud_fontScale->current.value,
					/* font */ _CG::GetFontForStyle(Dvars::r_drawCollision_hud_fontStyle->current.integer),
					/* colr */ Dvars::pm_hud_fontColor->current.vector,
					/* txt	*/ "Selection Mode Active! Highlighting of brushes disabled.\n[Selection Mode Commands]\n-- mapexport_selectionAdd\n-- mapexport_selectionClear");
			}

			else if (Dvars::r_drawCollision && Dvars::r_drawCollision->current.integer != 0)
			{
				Game::DrawTextWithEngine(
					/* x	*/ Dvars::r_drawCollision_hud_position->current.vector[0],
					/* y	*/ Dvars::r_drawCollision_hud_position->current.vector[1],
					/* scaX */ Dvars::r_drawCollision_hud_fontScale->current.value,
					/* scaY */ Dvars::r_drawCollision_hud_fontScale->current.value,
					/* font */ _CG::GetFontForStyle(Dvars::r_drawCollision_hud_fontStyle->current.integer),
					/* colr */ Dvars::pm_hud_fontColor->current.vector,
					/* txt	*/ Utils::VA("Total amount of Brushes used for calculations: %d \n"
										 "Total amount of Polygons drawn: %d", Game::Globals::dbgColl_drawnBrushAmount, Game::Globals::dbgColl_drawnPlanesAmount));
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