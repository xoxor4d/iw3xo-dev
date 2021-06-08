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
		if (Dvars::xo_shaderdbg_matrix && Dvars::xo_shaderdbg_matrix->current.enabled)
		{
			Game::DrawTextWithEngine(
				/* x	*/ 10.0f,
				/* y	*/ 200.0f,
				/* scaX */ 0.8f,
				/* scaY */ 0.8f,
				/* font */ _CG::GetFontForStyle(Dvars::pm_hud_fontStyle->current.integer),
				/* colr */ Dvars::pm_hud_fontColor->current.vector,
				/* txt	*/ Utils::VA("viewMatrix:\n%.2lf %.2lf %.2lf %.2lf\n%.2lf %.2lf %.2lf %.2lf\n%.2lf %.2lf %.2lf %.2lf\n%.2lf %.2lf %.2lf %.2lf\n",
					Game::Globals::viewMatrix.m[0][0], Game::Globals::viewMatrix.m[0][1], Game::Globals::viewMatrix.m[0][2], Game::Globals::viewMatrix.m[0][3],
					Game::Globals::viewMatrix.m[1][0], Game::Globals::viewMatrix.m[1][1], Game::Globals::viewMatrix.m[1][2], Game::Globals::viewMatrix.m[1][3],
					Game::Globals::viewMatrix.m[2][0], Game::Globals::viewMatrix.m[2][1], Game::Globals::viewMatrix.m[1][2], Game::Globals::viewMatrix.m[2][3],
					Game::Globals::viewMatrix.m[3][0], Game::Globals::viewMatrix.m[3][1], Game::Globals::viewMatrix.m[1][2], Game::Globals::viewMatrix.m[3][3]));

			Game::DrawTextWithEngine(
				/* x	*/ 10.0f,
				/* y	*/ 310.0f,
				/* scaX */ 0.8f,
				/* scaY */ 0.8f,
				/* font */ _CG::GetFontForStyle(Dvars::pm_hud_fontStyle->current.integer),
				/* colr */ Dvars::pm_hud_fontColor->current.vector,
				/* txt	*/ Utils::VA("projectionMatrix:\n%.2lf %.2lf %.2lf %.2lf\n%.2lf %.2lf %.2lf %.2lf\n%.2lf %.2lf %.2lf %.2lf\n%.2lf %.2lf %.2lf %.2lf\n",
					Game::Globals::projectionMatrix.m[0][0], Game::Globals::projectionMatrix.m[0][1], Game::Globals::projectionMatrix.m[0][2], Game::Globals::projectionMatrix.m[0][3],
					Game::Globals::projectionMatrix.m[1][0], Game::Globals::projectionMatrix.m[1][1], Game::Globals::projectionMatrix.m[1][2], Game::Globals::projectionMatrix.m[1][3],
					Game::Globals::projectionMatrix.m[2][0], Game::Globals::projectionMatrix.m[2][1], Game::Globals::projectionMatrix.m[1][2], Game::Globals::projectionMatrix.m[2][3],
					Game::Globals::projectionMatrix.m[3][0], Game::Globals::projectionMatrix.m[3][1], Game::Globals::projectionMatrix.m[1][2], Game::Globals::projectionMatrix.m[3][3]));

			Game::DrawTextWithEngine(
				/* x	*/ 10.0f,
				/* y	*/ 420.0f,
				/* scaX */ 0.8f,
				/* scaY */ 0.8f,
				/* font */ _CG::GetFontForStyle(Dvars::pm_hud_fontStyle->current.integer),
				/* colr */ Dvars::pm_hud_fontColor->current.vector,
				/* txt	*/ Utils::VA("viewProjectionMatrix:\n%.2lf %.2lf %.2lf %.2lf\n%.2lf %.2lf %.2lf %.2lf\n%.2lf %.2lf %.2lf %.2lf\n%.2lf %.2lf %.2lf %.2lf\n",
					Game::Globals::viewProjectionMatrix.m[0][0], Game::Globals::viewProjectionMatrix.m[0][1], Game::Globals::viewProjectionMatrix.m[0][2], Game::Globals::viewProjectionMatrix.m[0][3],
					Game::Globals::viewProjectionMatrix.m[1][0], Game::Globals::viewProjectionMatrix.m[1][1], Game::Globals::viewProjectionMatrix.m[1][2], Game::Globals::viewProjectionMatrix.m[1][3],
					Game::Globals::viewProjectionMatrix.m[2][0], Game::Globals::viewProjectionMatrix.m[2][1], Game::Globals::viewProjectionMatrix.m[1][2], Game::Globals::viewProjectionMatrix.m[2][3],
					Game::Globals::viewProjectionMatrix.m[3][0], Game::Globals::viewProjectionMatrix.m[3][1], Game::Globals::viewProjectionMatrix.m[1][2], Game::Globals::viewProjectionMatrix.m[3][3]));

			Game::DrawTextWithEngine(
				/* x	*/ 10.0f,
				/* y	*/ 530.0f,
				/* scaX */ 0.8f,
				/* scaY */ 0.8f,
				/* font */ _CG::GetFontForStyle(Dvars::pm_hud_fontStyle->current.integer),
				/* colr */ Dvars::pm_hud_fontColor->current.vector,
				/* txt	*/ Utils::VA("inverseViewProjectionMatrix:\n%.2lf %.2lf %.2lf %.2lf\n%.2lf %.2lf %.2lf %.2lf\n%.2lf %.2lf %.2lf %.2lf\n%.2lf %.2lf %.2lf %.2lf\n",
					Game::Globals::inverseViewProjectionMatrix.m[0][0], Game::Globals::inverseViewProjectionMatrix.m[0][1], Game::Globals::inverseViewProjectionMatrix.m[0][2], Game::Globals::inverseViewProjectionMatrix.m[0][3],
					Game::Globals::inverseViewProjectionMatrix.m[1][0], Game::Globals::inverseViewProjectionMatrix.m[1][1], Game::Globals::inverseViewProjectionMatrix.m[1][2], Game::Globals::inverseViewProjectionMatrix.m[1][3],
					Game::Globals::inverseViewProjectionMatrix.m[2][0], Game::Globals::inverseViewProjectionMatrix.m[2][1], Game::Globals::inverseViewProjectionMatrix.m[1][2], Game::Globals::inverseViewProjectionMatrix.m[2][3],
					Game::Globals::inverseViewProjectionMatrix.m[3][0], Game::Globals::inverseViewProjectionMatrix.m[3][1], Game::Globals::inverseViewProjectionMatrix.m[1][2], Game::Globals::inverseViewProjectionMatrix.m[3][3]));

			// GfxCodeMatrices matrices;
			/*memcpy(&Game::Globals::viewMatrix, &Game::gfxCmdBufSourceState->input.data->viewParms->viewMatrix, sizeof(Game::GfxMatrix));
			memcpy(&Game::Globals::projectionMatrix, &Game::gfxCmdBufSourceState->input.data->viewParms->projectionMatrix, sizeof(Game::GfxMatrix));
			memcpy(&Game::Globals::viewProjectionMatrix, &Game::gfxCmdBufSourceState->input.data->viewParms->viewProjectionMatrix, sizeof(Game::GfxMatrix));
			memcpy(&Game::Globals::inverseViewProjectionMatrix, &Game::gfxCmdBufSourceState->input.data->viewParms->inverseViewProjectionMatrix, sizeof(Game::GfxMatrix));*/
		}

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
	}

	_CG::~_CG()
	{ }
}