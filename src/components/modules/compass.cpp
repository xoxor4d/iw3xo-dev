// Port from mDd client Proxymod (https://github.com/Jelvan1/cgame_proxymod/blob/master/src/compass.c)
#include "std_include.hpp"

namespace components
{
	typedef struct
	{
		game::playerState_s pm_ps;
	} compass_t;

	static compass_t s;

	void compass::main()
	{
		if (dvars::mdd_compass && !dvars::mdd_compass->current.enabled)
		{
			return;
		}

		s.pm_ps = game::cgs->predictedPlayerState;

		const float yaw = utils::vector::deg_to_rad(s.pm_ps.viewangles[YAW]);
		float y, w, h;

		// draw quadrants
		game::CG_FillAngleYaw(0, M_PI / 2.0f, yaw, dvars::mdd_compass_yh->current.vector[0], dvars::mdd_compass_yh->current.vector[1], dvars::mdd_compass_quadrant_rgbas0->current.vector);
		game::CG_FillAngleYaw(M_PI / 2.0f, M_PI, yaw, dvars::mdd_compass_yh->current.vector[0], dvars::mdd_compass_yh->current.vector[1], dvars::mdd_compass_quadrant_rgbas1->current.vector);
		game::CG_FillAngleYaw(-M_PI / 2.0f, -M_PI, yaw, dvars::mdd_compass_yh->current.vector[0], dvars::mdd_compass_yh->current.vector[1], dvars::mdd_compass_quadrant_rgbas2->current.vector);
		game::CG_FillAngleYaw(0, -M_PI / 2.0f, yaw, dvars::mdd_compass_yh->current.vector[0], dvars::mdd_compass_yh->current.vector[1], dvars::mdd_compass_quadrant_rgbas3->current.vector);
		
		// draw ticks
		y = dvars::mdd_compass_yh->current.vector[0] + dvars::mdd_compass_yh->current.vector[1] / 2.0f;
		w = 1.0f;
		h = dvars::mdd_compass_yh->current.vector[1] / 2.0f;

		game::CG_DrawLineYaw(0, yaw, y, w, h, dvars::mdd_compass_ticks_rgba->current.vector);
		game::CG_DrawLineYaw(M_PI / 2.0f, yaw, y, w, h, dvars::mdd_compass_ticks_rgba->current.vector);
		game::CG_DrawLineYaw(M_PI, yaw, y, w, h, dvars::mdd_compass_ticks_rgba->current.vector);
		game::CG_DrawLineYaw(-M_PI / 2.0f, yaw, y, w, h, dvars::mdd_compass_ticks_rgba->current.vector);
		 
		// --
		y = dvars::mdd_compass_yh->current.vector[0] + 3.0f * dvars::mdd_compass_yh->current.vector[1] / 4.0f;
		w = 1.0f;
		h = dvars::mdd_compass_yh->current.vector[1] / 4.0f;

		game::CG_DrawLineYaw(M_PI / 4.0f, yaw, y, w, h, dvars::mdd_compass_ticks_rgba->current.vector);
		game::CG_DrawLineYaw(3.0f * M_PI / 4.0f, yaw, y, w, h, dvars::mdd_compass_ticks_rgba->current.vector);
		game::CG_DrawLineYaw(-M_PI / 4.0f, yaw, y, w, h, dvars::mdd_compass_ticks_rgba->current.vector);
		game::CG_DrawLineYaw(-3.0f * M_PI / 4.0f, yaw, y, w, h, dvars::mdd_compass_ticks_rgba->current.vector);

	}

	compass::compass()
	{
		dvars::mdd_compass = game::Dvar_RegisterBool(
			/* name		*/ "mdd_compass",
			/* desc		*/ "Display mDd world compass",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);

		dvars::mdd_compass_yh = game::Dvar_RegisterVec2(
			/* name		*/ "mdd_compass_yh",
			/* desc		*/ "mdd_compass position on screen 'Y' and compass thickness 'H'",
			/* y		*/ 192.0f,
			/* h		*/ 12.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 640.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::mdd_compass_quadrant_rgbas0 = game::Dvar_RegisterVec4(
			/* name		*/ "mdd_compass_quadrant_rgbas0",
			/* desc		*/ "color 0 to 90 deg quadrant",
			/* x		*/ 1.0f,
			/* y		*/ 1.0f,
			/* z		*/ 0.0f,
			/* w		*/ 0.25f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::mdd_compass_quadrant_rgbas1 = game::Dvar_RegisterVec4(
			/* name		*/ "mdd_compass_quadrant_rgbas1",
			/* desc		*/ "color 90 to 180 deg quadrant",
			/* x		*/ 0.0f,
			/* y		*/ 1.0f,
			/* z		*/ 0.0f,
			/* w		*/ 0.25f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::mdd_compass_quadrant_rgbas2 = game::Dvar_RegisterVec4(
			/* name		*/ "mdd_compass_quadrant_rgbas2",
			/* desc		*/ "color 180 to 270 deg quadrant",
			/* x		*/ 0.0f,
			/* y		*/ 0.0f,
			/* z		*/ 1.0f,
			/* w		*/ 0.25f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::mdd_compass_quadrant_rgbas3 = game::Dvar_RegisterVec4(
			/* name		*/ "mdd_compass_quadrant_rgbas3",
			/* desc		*/ "color 270 to 360 deg quadrant",
			/* x		*/ 1.0f,
			/* y		*/ 0.0f,
			/* z		*/ 1.0f,
			/* w		*/ 0.25f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::mdd_compass_ticks_rgba = game::Dvar_RegisterVec4(
			/* name		*/ "mdd_compass_ticks_rgba",
			/* desc		*/ "color for ticks",
			/* x		*/ 1.0f,
			/* y		*/ 1.0f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);
	}
}