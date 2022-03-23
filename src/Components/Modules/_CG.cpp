#include "STDInclude.hpp"

namespace Components
{
	// *
	// return font string for style
	const char* _CG::GetFontForStyle(int fontStyle)
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
			const char* movementType = Dvars::pm_movementType->current.integer == Game::PM_MTYPE::STOCK ? "Stock"
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
					/* colr */ Dvars::r_drawCollision_hud_fontColor->current.vector,
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
					/* colr */ Dvars::r_drawCollision_hud_fontColor->current.vector,
					/* txt	*/ Utils::VA("Total amount of Brushes used for calculations: %d \n"
						"Total amount of Polygons drawn: %d", Game::Globals::dbgColl_drawnBrushAmount, Game::Globals::dbgColl_drawnPlanesAmount));
			}
		}

		// Pmove Origin Hud
		if (Dvars::pm_origin_hud && Dvars::pm_origin_hud->current.enabled)
		{
			float x = Game::cgs->predictedPlayerState.origin[0];
			float y = Game::cgs->predictedPlayerState.origin[1];
			float z = Game::cgs->predictedPlayerState.origin[2];

			float angle = Game::cgs->refdefViewAngles[YAW];

			Game::DrawTextWithEngine(
				/* x	*/ Dvars::pm_origin_hud_x->current.value,
				/* y	*/ Dvars::pm_origin_hud_y->current.value,
				/* scaX */ Dvars::pm_origin_hud_fontScale->current.value,
				/* scaY */ Dvars::pm_origin_hud_fontScale->current.value,
				/* font */ _CG::GetFontForStyle(Dvars::pm_origin_hud_fontStyle->current.integer),
				/* colr */ Dvars::pm_origin_hud_fontColor->current.vector,
				/* txt	*/ Utils::VA("(X: %f, Y: %f, Z: %.3lf) YAW: %f", x, y, z, angle));
		}

		// mDd proxymod compass
		if (Components::active.Compass)
		{
			Compass::main();
		}

		// mDd proxymod CGaz
		if (Components::active.CGaz)
		{
			CGaz::main();
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

		// -----
		// Dvars

		Dvars::pm_origin_hud = Game::Dvar_RegisterBool(
			/* name		*/ "pm_origin_hud",
			/* desc		*/ "Display current player's position.",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_origin_hud_x = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_origin_hud_x",
			/* desc		*/ "text offset x",
			/* default	*/ 10.0f,
			/* minVal	*/ -1000.0f,
			/* maxVal	*/ 1000.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_origin_hud_y = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_origin_hud_y",
			/* desc		*/ "text offset y",
			/* default	*/ 230.0f,
			/* minVal	*/ -1000.0f,
			/* maxVal	*/ 1000.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_origin_hud_fontScale = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_origin_hud_fontScale",
			/* desc		*/ "font scale",
			/* default	*/ 0.75f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_origin_hud_fontStyle = Game::Dvar_RegisterInt(
			/* name		*/ "pm_origin_hud_fontStyle",
			/* desc		*/ "font Style",
			/* default	*/ 1,
			/* minVal	*/ 0,
			/* maxVal	*/ 8,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_origin_hud_fontColor = Game::Dvar_RegisterVec4(
			/* name		*/ "pm_origin_hud_fontColor",
			/* desc		*/ "font color",
			/* x		*/ 1.0f,
			/* y		*/ 0.55f,
			/* z		*/ 0.4f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);
	}

	_CG::~_CG()
	{ }
}