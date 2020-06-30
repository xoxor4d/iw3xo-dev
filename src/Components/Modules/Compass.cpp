// Port from mDd client Proxymod (https://github.com/Jelvan1/cgame_proxymod/blob/master/src/compass.c)
#include "STDInclude.hpp"

namespace Components
{
	typedef struct
	{
		Game::playerState_s pm_ps;
	} compass_t;

	static compass_t s;

	void Compass::draw_compass()
	{
		if (!Dvars::mdd_compass->current.enabled) return;

		s.pm_ps = Game::cgs->predictedPlayerState;
		// draw quadrants
		float const yaw = Utils::vector::_DegreesToRadians(s.pm_ps.viewangles[YAW]);
		Game::CG_FillAngleYaw(0, M_PI / 2, yaw, Dvars::mdd_compass_yh->current.vector[0], Dvars::mdd_compass_yh->current.vector[1], Dvars::mdd_compass_quadrant_rgbas0->current.vector);
		Game::CG_FillAngleYaw(M_PI / 2, M_PI, yaw, Dvars::mdd_compass_yh->current.vector[0], Dvars::mdd_compass_yh->current.vector[1], Dvars::mdd_compass_quadrant_rgbas1->current.vector);
		Game::CG_FillAngleYaw(-M_PI / 2, -M_PI, yaw, Dvars::mdd_compass_yh->current.vector[0], Dvars::mdd_compass_yh->current.vector[1], Dvars::mdd_compass_quadrant_rgbas2->current.vector);
		Game::CG_FillAngleYaw(0, -M_PI / 2, yaw, Dvars::mdd_compass_yh->current.vector[0], Dvars::mdd_compass_yh->current.vector[1], Dvars::mdd_compass_quadrant_rgbas3->current.vector);
		// draw ticks
		{
			float const y = Dvars::mdd_compass_yh->current.vector[0] + Dvars::mdd_compass_yh->current.vector[1] / 2;
			float const w = 1;
			float const h = Dvars::mdd_compass_yh->current.vector[1] / 2;
			Game::CG_DrawLineYaw(0, yaw, y, w, h, Dvars::mdd_compass_ticks_rgba->current.vector);
			Game::CG_DrawLineYaw(M_PI / 2, yaw, y, w, h, Dvars::mdd_compass_ticks_rgba->current.vector);
			Game::CG_DrawLineYaw(M_PI, yaw, y, w, h, Dvars::mdd_compass_ticks_rgba->current.vector);
			Game::CG_DrawLineYaw(-M_PI / 2, yaw, y, w, h, Dvars::mdd_compass_ticks_rgba->current.vector);
		}
		{
			float const y = Dvars::mdd_compass_yh->current.vector[0] + 3 * Dvars::mdd_compass_yh->current.vector[1] / 4;
			float const w = 1;
			float const h = Dvars::mdd_compass_yh->current.vector[1] / 4;
			Game::CG_DrawLineYaw(M_PI / 4, yaw, y, w, h, Dvars::mdd_compass_ticks_rgba->current.vector);
			Game::CG_DrawLineYaw(3 * M_PI / 4, yaw, y, w, h, Dvars::mdd_compass_ticks_rgba->current.vector);
			Game::CG_DrawLineYaw(-M_PI / 4, yaw, y, w, h, Dvars::mdd_compass_ticks_rgba->current.vector);
			Game::CG_DrawLineYaw(-3 * M_PI / 4, yaw, y, w, h, Dvars::mdd_compass_ticks_rgba->current.vector);
		}
	}

	Compass::Compass()
	{
		Dvars::mdd_compass = Game::Dvar_RegisterBool(
			/* name		*/ "mdd_compass",
			/* desc		*/ "Display mDd world compass",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_compass_yh = Game::Dvar_RegisterVec2(
			/* name		*/ "mdd_compass_yh",
			/* desc		*/ "mdd_compass position on screen 'Y' and compass thickness 'H'",
			/* y		*/ 192.0f,
			/* h		*/ 12.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 640.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_compass_quadrant_rgbas0 = Game::Dvar_RegisterVec4(
			/* name		*/ "mdd_compass_quadrant_rgbas0",
			/* desc		*/ "color 0° to 90° quadrant",
			/* x		*/ 1.0f,
			/* y		*/ 1.0f,
			/* z		*/ 0.0f,
			/* w		*/ 0.25f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_compass_quadrant_rgbas1 = Game::Dvar_RegisterVec4(
			/* name		*/ "mdd_compass_quadrant_rgbas1",
			/* desc		*/ "color 90° to 180° quadrant",
			/* x		*/ 0.0f,
			/* y		*/ 1.0f,
			/* z		*/ 0.0f,
			/* w		*/ 0.25f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_compass_quadrant_rgbas2 = Game::Dvar_RegisterVec4(
			/* name		*/ "mdd_compass_quadrant_rgbas2",
			/* desc		*/ "color 180° to 270° quadrant",
			/* x		*/ 0.0f,
			/* y		*/ 0.0f,
			/* z		*/ 1.0f,
			/* w		*/ 0.25f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_compass_quadrant_rgbas3 = Game::Dvar_RegisterVec4(
			/* name		*/ "mdd_compass_quadrant_rgbas3",
			/* desc		*/ "color 270° to 360° quadrant",
			/* x		*/ 1.0f,
			/* y		*/ 0.0f,
			/* z		*/ 1.0f,
			/* w		*/ 0.25f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_compass_ticks_rgba = Game::Dvar_RegisterVec4(
			/* name		*/ "mdd_compass_ticks_rgba",
			/* desc		*/ "color for ticks",
			/* x		*/ 1.0f,
			/* y		*/ 1.0f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);
	}

	Compass::~Compass()
	{ }
}