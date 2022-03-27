#include "std_include.hpp"

namespace components
{
	// *
	// return font string for style
	const char* _cg::get_font_for_style(int font_style)
	{
		return	font_style == 0 ? FONT_SMALL_DEV : font_style == 1 ? FONT_BIG_DEV		: font_style == 2 ? FONT_CONSOLE :
				font_style == 3 ? FONT_BIG		 : font_style == 4 ? FONT_SMALL			: font_style == 5 ? FONT_BOLD :
				font_style == 6 ? FONT_NORMAL	 : font_style == 7 ? FONT_EXTRA_BIG		: FONT_OBJECTIVE;
	}

	// *
	// custom huds
	void draw_custom_hud()
	{
		// pmove speed hud
		if (dvars::pm_hud_enable && dvars::pm_hud_enable->current.enabled)
		{
			const char* movementType = dvars::pm_movementType->current.integer == game::PM_MTYPE::STOCK ? "Stock"
				: dvars::pm_movementType->current.integer == game::PM_MTYPE::DEFRAG ? "Defrag" : "CS-Surf";

			const float xy_speed = sqrtf(game::glob::lpmove_velocity.x * game::glob::lpmove_velocity.x
				+ game::glob::lpmove_velocity.y * game::glob::lpmove_velocity.y);

			game::draw_text_with_engine(
				/* x	*/ dvars::pm_hud_x->current.value,
				/* y	*/ dvars::pm_hud_y->current.value,
				/* scaX */ dvars::pm_hud_fontScale->current.value,
				/* scaY */ dvars::pm_hud_fontScale->current.value,
				/* font */ _cg::get_font_for_style(dvars::pm_hud_fontStyle->current.integer),
				/* colr */ dvars::pm_hud_fontColor->current.vector,
				/* txt	*/ utils::va("Current Mode: %s\nCurrent Speed: %.2lf", movementType, xy_speed));
		}

		// Debug Collision Hud
		if (dvars::r_drawCollision_hud && dvars::r_drawCollision_hud->current.enabled)
		{
			if (dvars::mapexport_selectionMode && dvars::mapexport_selectionMode->current.integer != 0)
			{
				game::draw_text_with_engine(
					/* x	*/ dvars::r_drawCollision_hud_position->current.vector[0],
					/* y	*/ dvars::r_drawCollision_hud_position->current.vector[1],
					/* scaX */ dvars::r_drawCollision_hud_fontScale->current.value,
					/* scaY */ dvars::r_drawCollision_hud_fontScale->current.value,
					/* font */ _cg::get_font_for_style(dvars::r_drawCollision_hud_fontStyle->current.integer),
					/* colr */ dvars::r_drawCollision_hud_fontColor->current.vector,
					/* txt	*/ "Selection Mode Active! Highlighting of brushes disabled.\n[Selection Mode Commands]\n-- mapexport_selectionAdd\n-- mapexport_selectionClear");
			}

			else if (dvars::r_drawCollision && dvars::r_drawCollision->current.integer != 0)
			{
				game::draw_text_with_engine(
					/* x	*/ dvars::r_drawCollision_hud_position->current.vector[0],
					/* y	*/ dvars::r_drawCollision_hud_position->current.vector[1],
					/* scaX */ dvars::r_drawCollision_hud_fontScale->current.value,
					/* scaY */ dvars::r_drawCollision_hud_fontScale->current.value,
					/* font */ _cg::get_font_for_style(dvars::r_drawCollision_hud_fontStyle->current.integer),
					/* colr */ dvars::r_drawCollision_hud_fontColor->current.vector,
					/* txt	*/ utils::va("Total amount of Brushes used for calculations: %d \n"
						"Total amount of Polygons drawn: %d", game::glob::debug_collision_rendered_brush_amount, game::glob::debug_collision_rendered_planes_amount));
			}
		}

		// Pmove Origin Hud
		if (dvars::pm_origin_hud && dvars::pm_origin_hud->current.enabled)
		{
			game::draw_text_with_engine(
				/* x	*/ dvars::pm_origin_hud_x->current.value,
				/* y	*/ dvars::pm_origin_hud_y->current.value,
				/* scaX */ dvars::pm_origin_hud_fontScale->current.value,
				/* scaY */ dvars::pm_origin_hud_fontScale->current.value,
				/* font */ _cg::get_font_for_style(dvars::pm_origin_hud_fontStyle->current.integer),
				/* colr */ dvars::pm_origin_hud_fontColor->current.vector,
				/* txt	*/ utils::va("(X: %.2f, Y: %.2f, Z: %.2f) YAW: %.2f", 
					game::cgs->predictedPlayerState.origin[0], game::cgs->predictedPlayerState.origin[1], game::cgs->predictedPlayerState.origin[2], game::cgs->refdefViewAngles[YAW]));
		}

		// mDd proxymod compass
		if (components::active.compass)
		{
			compass::main();
		}

		// mDd proxymod CGaz
		if (components::active.cgaz)
		{
			cgaz::main();
		}
		
	}

	// hk call to CG_Drawcrosshair in CG_Draw2D ~ only active if cg_draw2d
	__declspec(naked) void draw_custom_hud_stub()
	{
		const static uint32_t func_addr = 0x4311A0;
		const static uint32_t retn_addr = 0x42F6BD;
		__asm
		{
			call	func_addr;
			add		esp, 4;

			pushad;
			call	draw_custom_hud;
			popad;

			jmp		retn_addr;
		}
	}

	_cg::_cg()
	{
		// hook call to CG_Drawcrosshair in CG_Draw2D ~ only active if cg_draw2d
		utils::hook(0x42F6B5, draw_custom_hud_stub, HOOK_JUMP).install()->quick();

		// -----
		// dvars

		dvars::pm_origin_hud = game::Dvar_RegisterBool(
			/* name		*/ "pm_origin_hud",
			/* desc		*/ "Display current player's position.",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);

		dvars::pm_origin_hud_x = game::Dvar_RegisterFloat(
			/* name		*/ "pm_origin_hud_x",
			/* desc		*/ "text offset x",
			/* default	*/ 10.0f,
			/* minVal	*/ -1000.0f,
			/* maxVal	*/ 1000.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::pm_origin_hud_y = game::Dvar_RegisterFloat(
			/* name		*/ "pm_origin_hud_y",
			/* desc		*/ "text offset y",
			/* default	*/ 230.0f,
			/* minVal	*/ -1000.0f,
			/* maxVal	*/ 1000.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::pm_origin_hud_fontScale = game::Dvar_RegisterFloat(
			/* name		*/ "pm_origin_hud_fontScale",
			/* desc		*/ "font scale",
			/* default	*/ 0.75f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::pm_origin_hud_fontStyle = game::Dvar_RegisterInt(
			/* name		*/ "pm_origin_hud_fontStyle",
			/* desc		*/ "font Style",
			/* default	*/ 1,
			/* minVal	*/ 0,
			/* maxVal	*/ 8,
			/* flags	*/ game::dvar_flags::saved);

		dvars::pm_origin_hud_fontColor = game::Dvar_RegisterVec4(
			/* name		*/ "pm_origin_hud_fontColor",
			/* desc		*/ "font color",
			/* x		*/ 1.0f,
			/* y		*/ 0.55f,
			/* z		*/ 0.4f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);
	}
}