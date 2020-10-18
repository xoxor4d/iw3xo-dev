// Port from mDd client Proxymod (https://github.com/Jelvan1/cgame_proxymod/blob/master/src/cg_cgaz.c)
#include "STDInclude.hpp"

#define PM_NORMAL				0x0
#define PM_NORMAL_LINKED		0x1
#define PM_NOCLIP				0x2
#define PM_UFO					0x3
#define PM_SPEC					0x4
#define PM_INTERMISSION			0x5
#define PM_LASTSTAND			0x6
#define PM_DEAD					0x7
#define PM_DEAD_LINKED			0x8

#define SURF_NODAMAGE			0x1
#define SURF_SLICK				0x2
#define SURF_SKY				0x4
#define SURF_LADDER				0x8
#define SURF_NOIMPACT			0x10
#define SURF_NOMARKS			0x20
#define SURF_NODRAW				0x80
#define SURF_NOPENETRATE		0x100
#define SURF_NOLIGHTMAP			0x400
#define SURF_NOSTEPS			0x2000
#define SURF_NONSOLID			0x4000
#define SURF_NODLIGHT			0x20000
#define SURF_NOCASTSHADOW		0x40000
#define SURF_MANTLEON			0x2000000
#define SURF_MANTLEOVER			0x4000000
#define SURF_PORTAL				0x80000000
#define SURF_BARK				0x100000
#define SURF_BRICK				0x200000
#define SURF_CARPET				0x300000
#define SURF_CLOTH				0x400000
#define SURF_CONCRETE			0x500000
#define SURF_DIRT				0x600000
#define SURF_FLESH				0x700000
#define SURF_FOLIAGE			0x800000
#define SURF_GLASS				0x900000
#define SURF_GRASS				0xA00000
#define SURF_GRAVEL				0xB00000
#define SURF_ICE				0xC00000
#define SURF_METAL				0xD00000
#define SURF_MUD				0xE00000
#define SURF_PAPER				0xF00000
#define SURF_PLASTER			0x1000000
#define SURF_ROCK				0x1100000
#define SURF_SAND				0x1200000
#define SURF_SNOW				0x1300000
#define SURF_WATER				0x1400000
#define SURF_WOOD				0x1500000
#define SURF_ASPHALT			0x1600000
#define SURF_CERAMIC			0x1700000
#define SURF_PLASTIC			0x1800000
#define SURF_RUBBER				0x1900000
#define SURF_CUSHION			0x1A00000
#define SURF_FRUIT				0x1B00000
#define SURF_PAINTEDMETAL		0x1C00000

#define PMF_PRONE				0x1
#define PMF_FOLLOW				0x2 // spectate following another player
#define PMF_MANTLE				0x4
#define PMF_LADDER				0x8
#define PMF_BACKWARDS_RUN		0x20
#define PMF_LEAN				0x40
#define PMF_TIME_KNOCKBACK		0x100
#define PMF_RESPAWNED			0x400
#define PMF_JUMPING				0x4000
#define PMF_SPRINTING			0x8000
#define PMF_TOOK_DAMAGE			0x10000
#define PMF_VEHICLE_ATTACHED	0x100000

// movement parameters
#define pm_accelerate			9.0f
#define pm_crouch_accelerate	12.0f
#define pm_prone_accelerate		19.0f
#define pm_airaccelerate		1.0f

namespace Components
{
	typedef struct
	{
		float g_squared; // 0 when not on slick.
		float v_squared;
		float vf_squared;
		float a_squared;

		float v;
		float vf;
		float a;

		float wishspeed;
	} state_t;

	typedef struct
	{
		float d_min;
		float d_opt;
		float d_max_cos;
		float d_max;

		float d_vel;

		Utils::vector::vec2_t wishvel;

		Game::pmove_t		pm;
		Game::pml_t         pml;
	} cgaz_t;

	static cgaz_t s;

	void CGaz::CG_DrawCGaz()
	{
		float const yaw = atan2f(s.wishvel[1], s.wishvel[0]) - s.d_vel;

		Game::CG_FillAngleYaw(-s.d_min, +s.d_min, yaw, Dvars::mdd_cgaz_yh->current.vector[0], Dvars::mdd_cgaz_yh->current.vector[1], Dvars::mdd_cgaz_rgbaNoAccel->current.vector);

		Game::CG_FillAngleYaw(+s.d_min, +s.d_opt, yaw, Dvars::mdd_cgaz_yh->current.vector[0], Dvars::mdd_cgaz_yh->current.vector[1], Dvars::mdd_cgaz_rgbaPartialAccel->current.vector);
		Game::CG_FillAngleYaw(-s.d_opt, -s.d_min, yaw, Dvars::mdd_cgaz_yh->current.vector[0], Dvars::mdd_cgaz_yh->current.vector[1], Dvars::mdd_cgaz_rgbaPartialAccel->current.vector);

		Game::CG_FillAngleYaw(+s.d_opt, +s.d_max_cos, yaw, Dvars::mdd_cgaz_yh->current.vector[0], Dvars::mdd_cgaz_yh->current.vector[1], Dvars::mdd_cgaz_rgbaFullAccel->current.vector);
		Game::CG_FillAngleYaw(-s.d_max_cos, -s.d_opt, yaw, Dvars::mdd_cgaz_yh->current.vector[0], Dvars::mdd_cgaz_yh->current.vector[1], Dvars::mdd_cgaz_rgbaFullAccel->current.vector);

		Game::CG_FillAngleYaw(+s.d_max_cos, +s.d_max, yaw, Dvars::mdd_cgaz_yh->current.vector[0], Dvars::mdd_cgaz_yh->current.vector[1], Dvars::mdd_cgaz_rgbaTurnZone->current.vector);
		Game::CG_FillAngleYaw(-s.d_max, -s.d_max_cos, yaw, Dvars::mdd_cgaz_yh->current.vector[0], Dvars::mdd_cgaz_yh->current.vector[1], Dvars::mdd_cgaz_rgbaTurnZone->current.vector);
	}

	static float update_d_min(state_t const* state)
	{
		float const num_squared = state->wishspeed * state->wishspeed - state->v_squared + state->vf_squared + state->g_squared;
		float const num = sqrtf(num_squared);

		return num >= state->vf ? 0 : acosf(num / state->vf);
	}

	static float update_d_opt(state_t const* state)
	{
		float const num = state->wishspeed - state->a;

		return num >= state->vf ? 0 : acosf(num / state->vf);
	}

	static float update_d_max_cos(state_t const* state, float d_opt)
	{
		float const num = sqrtf(state->v_squared - state->g_squared) - state->vf;
		float d_max_cos = num >= state->a ? 0 : acosf(num / state->a);
		
		if (d_max_cos < d_opt)
		{
			d_max_cos = d_opt;
		}

		return d_max_cos;
	}

	static float update_d_max(state_t const* state, float d_max_cos)
	{
		float const num = state->v_squared - state->vf_squared - state->a_squared - state->g_squared;
		float const den = 2 * state->a * state->vf;
		
		if (num >= den)
		{
			return 0;
		}
		else if (-num >= den)
		{
			return M_PI;
		}

		float d_max = acosf(num / den);
		
		if (d_max < d_max_cos)
		{
			d_max = d_max_cos;
		}

		return d_max;
	}

	static void update_d(float wishspeed, float accel, float slickGravity)
	{
		state_t state;
		state.g_squared = slickGravity * slickGravity;
		state.v_squared = Utils::vector::_VectorLengthSquared2(s.pml.previous_velocity);
		state.vf_squared = Utils::vector::_VectorLengthSquared2(s.pm.ps->velocity);
		state.wishspeed = wishspeed;
		state.a = accel * state.wishspeed * s.pml.frametime;
		state.a_squared = state.a * state.a;
		
		if ((Dvars::mdd_cgaz_ground && !Dvars::mdd_cgaz_ground->current.enabled) || state.v_squared - state.vf_squared >= 2 * state.a * state.wishspeed - state.a_squared)
		{
			state.v_squared = state.vf_squared;
		}

		state.v = sqrtf(state.v_squared);
		state.vf = sqrtf(state.vf_squared);

		s.d_min = update_d_min(&state);
		s.d_opt = update_d_opt(&state);
		s.d_max_cos = update_d_max_cos(&state, s.d_opt);
		s.d_max = update_d_max(&state, s.d_max_cos);

		s.d_vel = atan2f(s.pm.ps->velocity[1], s.pm.ps->velocity[0]);
	}

	/*
	==============
	PM_Accelerate

	Handles user intended acceleration
	==============
	*/
	void CGaz::PM_Accelerate(float wishspeed, float accel)
	{
		update_d(wishspeed, accel, 0);
	}

	void CGaz::PM_SlickAccelerate(float wishspeed, float accel)
	{
		update_d(wishspeed, accel, s.pm.ps->gravity * s.pml.frametime);
	}

	/*
	==============
	PM_CmdScale

	Returns the scale factor to apply to cmd movements
	This allows the clients to use axial -127 to 127 values for all directions
	without getting a sqrt(2) distortion in speed.
	==============
	*/
	float CGaz::PM_CmdScale(Game::playerState_s *ps, Game::usercmd_s *cmd)
	{
		signed int max;
		float total, scale;
		const auto player_spectateSpeedScale = Game::Dvar_FindVar("player_spectateSpeedScale")->current.value;

		max = abs(cmd->forwardmove);
		
		if (abs(cmd->rightmove) > max)
		{
			max = abs(cmd->rightmove);
		}

		if (!max)
		{
			return 0.0f;
		}

		total = sqrtf((float)(cmd->rightmove * cmd->rightmove + cmd->forwardmove * cmd->forwardmove));
		scale = (float)ps->speed * (float)max / (total * 127.0f);
		
		if (ps->pm_flags & PMF_LEAN || 0.0f != ps->leanf)
		{
			scale *= 0.4f;
		}
		
		if (ps->pm_type == PM_NOCLIP)
		{
			scale *= 3.0f;
		}

		if (ps->pm_type == PM_UFO)
		{
			scale *= 6.0f;
		}
		
		if (ps->pm_type == PM_SPEC)
		{
			scale *= player_spectateSpeedScale;
		}

		return scale;
	}

	/*
	===================
	PM_AirMove

	===================
	*/
	void CGaz::PM_AirMove(Game::pmove_t *pm, Game::pml_t *pml)
	{
		Game::PM_Friction(pm->ps, pml);

		const float scale = CGaz::PM_CmdScale(pm->ps, &pm->cmd);

		// project moves down to flat plane
		pml->forward[2] = 0;
		pml->right[2] = 0;

		Utils::vector::_VectorNormalize(pml->forward);
		Utils::vector::_VectorNormalize(pml->right);

		for (uint8_t i = 0; i < 2; ++i)
		{
			s.wishvel[i] = pm->cmd.forwardmove * pml->forward[i] + pm->cmd.rightmove * pml->right[i];
		}

		float const wishspeed = scale * VectorLength2(s.wishvel);

		CGaz::PM_Accelerate(wishspeed, pm_airaccelerate);
	}

	float CGaz::PM_DamageScale_Walk(int damage_timer)
	{
		const auto player_dmgtimer_maxTime = Game::Dvar_FindVar("player_dmgtimer_maxTime")->current.value;
		const auto player_dmgtimer_minScale = Game::Dvar_FindVar("player_dmgtimer_minScale")->current.value;

		if (!damage_timer || player_dmgtimer_maxTime == 0.0f)
		{
			return 1.0f;
		}

		return ((-player_dmgtimer_minScale / player_dmgtimer_maxTime) * damage_timer + 1.0f);
	}

	float CGaz::PM_GetViewHeightLerpTime(int iTarget, int bDown)
	{
		if (iTarget == 11)
		{
			return 400.0f;
		}

		if (iTarget != 40)
		{
			return 200.0f;
		}
		
		if (bDown)
		{
			return 200.0f;
		}

		return 400.0f;
	}

	float CGaz::PM_GetViewHeightLerp(Game::pmove_t* pm, int iFromHeight, int iToHeight)
	{
		float fLerpFrac;

		if (!pm->ps->viewHeightLerpTime)
		{
			return 0.0f;
		}

		if (iFromHeight != -1 && iToHeight != -1
			&& (iToHeight != pm->ps->viewHeightLerpTarget || iToHeight == 40 
				&& (iFromHeight != 11 || pm->ps->viewHeightLerpDown) 
				&& (iFromHeight != 60 || !pm->ps->viewHeightLerpDown)))
		{
			return 0.0f;
		}

		fLerpFrac = (float)(pm->cmd.serverTime - pm->ps->viewHeightLerpTime) / CGaz::PM_GetViewHeightLerpTime(pm->ps->viewHeightLerpTarget, pm->ps->viewHeightLerpDown);
		if (fLerpFrac >= 0.0f)
		{
			if (fLerpFrac > 1.0f)
			{
				fLerpFrac = 1.0f;
			}
		}
		else
		{
			fLerpFrac = 0.0f;
		}

		return fLerpFrac;
	}

	float CGaz::PM_CmdScaleForStance(Game::pmove_t* pm)
	{
		float lerpFrac;

		lerpFrac = CGaz::PM_GetViewHeightLerp(pm, 40, 11);

		if (lerpFrac != 0.0f)
		{
			return 0.15000001f * lerpFrac + (1.0f - lerpFrac) * 0.64999998f;
		}
			
		lerpFrac = CGaz::PM_GetViewHeightLerp(pm, 11, 40);

		if (lerpFrac != 0.0f)
		{
			return 0.64999998f * lerpFrac + (1.0f - lerpFrac) * 0.15000001f;
		}

		if (pm->ps->viewHeightTarget == 11)
		{
			return 0.15000001f;
		}

		if (pm->ps->viewHeightTarget == 22 || pm->ps->viewHeightTarget == 40)
		{
			return 0.64999998f;
		}

		return 1.0f;
	}

	float CGaz::PM_CmdScale_Walk(Game::pmove_t* pm, Game::usercmd_s* cmd)
	{
		float total, speed, scale;

		bool isProne = pm->ps->pm_flags & PMF_PRONE && pm->ps->fWeaponPosFrac > 0.0f;

		total = sqrtf((float)(cmd->rightmove * cmd->rightmove + cmd->forwardmove * cmd->forwardmove));

		if (cmd->forwardmove >= 0)
		{
			speed = fabs((float)cmd->forwardmove);
		}
		else
		{
			speed = fabs((float)cmd->forwardmove * Game::Dvar_FindVar("player_backSpeedScale")->current.value);
		}

		if (speed - fabs((float)cmd->rightmove * Game::Dvar_FindVar("player_strafeSpeedScale")->current.value) < 0.0f)
		{
			speed = fabs((float)cmd->rightmove * Game::Dvar_FindVar("player_strafeSpeedScale")->current.value);
		}

		if (speed == 0.0f)
		{
			return 0.0f;
		}

		scale = ((float)pm->ps->speed * speed) / (127.0f * total);

		if (pm->ps->pm_flags & PMF_LEAN || pm->ps->leanf != 0.0 || isProne)
		{
			scale *= 0.40000001f;
		}

		if (pm->ps->pm_flags & PMF_SPRINTING && pm->ps->viewHeightTarget == 60)
		{
			scale *= Game::Dvar_FindVar("player_sprintSpeedScale")->current.value;
		}

		if (pm->ps->pm_type == PM_NOCLIP)
		{
			scale *= 3.0f;
		}
		else if (pm->ps->pm_type == PM_UFO)
		{
			scale *= 6.0f;
		}
		else
		{
			scale *= CGaz::PM_CmdScaleForStance(pm);
		}

		auto weapon = Game::BG_WeaponNames[pm->ps->weapon];

		if (!pm->ps->weapon || weapon->moveSpeedScale <= 0.0f || pm->ps->pm_flags & PMF_LEAN || isProne)
		{
			if (pm->ps->weapon && weapon->adsMoveSpeedScale > 0.0f) {
				scale = scale * weapon->adsMoveSpeedScale;
			}
		}
		else
		{
			scale = scale * weapon->moveSpeedScale;
		}

		// skip shellshock stuff

		return scale * pm->ps->moveSpeedScaleMultiplier;
	}

	/*
	===================
	PM_WalkMove

	===================
	*/
	void CGaz::PM_WalkMove(Game::pmove_t *pm, Game::pml_t *pml)
	{
		float accelerate;

		if (Game::Jump_Check(pm, pml))
		{
			// jumped away
			CGaz::PM_AirMove(pm, pml);
			return;
		}

		Game::PM_Friction(pm->ps, pml);

		float scale = CGaz::PM_CmdScale_Walk(pm, &pm->cmd);
		float dmg_scale = CGaz::PM_DamageScale_Walk(pm->ps->damageTimer) * scale;

		// project moves down to flat plane
		pml->forward[2] = 0;
		pml->right[2] = 0;

		Utils::vector::_VectorNormalize(pml->forward);
		Utils::vector::_VectorNormalize(pml->right);

		for (uint8_t i = 0; i < 2; ++i)
		{
			s.wishvel[i] = pm->cmd.forwardmove * pml->forward[i] + pm->cmd.rightmove * pml->right[i];
		}

		float wishspeed = dmg_scale * VectorLength2(s.wishvel);

		// when a player gets hit, they temporarily lose
		// full control, which allows them to be moved a bit
		if (pml->groundTrace.surfaceFlags & SURF_SLICK || pm->ps->pm_flags & PMF_TIME_KNOCKBACK)
		{
			CGaz::PM_SlickAccelerate(wishspeed, pm_airaccelerate);
		}
		else
		{
			if (pm->ps->viewHeightTarget == 11)
			{
				accelerate = pm_prone_accelerate;
			}
			else if (pm->ps->viewHeightTarget == 40)
			{
				accelerate = pm_crouch_accelerate;
			}
			else
			{
				accelerate = pm_accelerate;
			}

			CGaz::PM_Accelerate(wishspeed, accelerate);
		}
	}

	void CGaz::PmoveSingle(Game::pmove_t* pm, Game::pml_t* pml)
	{
		// clear all pmove local vars
		memset(pml, 0, sizeof(pml));

		// save old velocity for crashlanding
		VectorCopy(pm->ps->velocity, pml->previous_velocity);

		Utils::vector::_AngleVectors(pm->ps->viewangles, pml->forward, pml->right, pml->up);

		pml->frametime = Game::cgs->frametime / 1000.f;

		// Use default key combination when no user input
		if (!pm->cmd.forwardmove && !pm->cmd.rightmove)
		{
			pm->cmd.forwardmove = 127;
		}

		// set groundentity
		Game::PM_GroundTrace_Internal(pm, pml);

		if (pml->walking)
		{
			// walking on ground
			CGaz::PM_WalkMove(pm, pml);
		}
		else
		{
			// airborne
			CGaz::PM_AirMove(pm, pml);
		}

		CGaz::CG_DrawCGaz();
	}

	void CGaz::main()
	{
		if ((Dvars::mdd_cgaz && !Dvars::mdd_cgaz->current.enabled) || (Game::cgs && Game::cgs->demoType))
		{
			return;
		}

		memcpy(&s.pm, &*Game::pmove, sizeof(Game::pmove_t));

		if (!(s.pm.ps->otherFlags & PMF_FOLLOW) && (Utils::vector::_VectorLengthSquared2(s.pm.ps->velocity) >= Dvars::mdd_cgaz_speed->current.integer * Dvars::mdd_cgaz_speed->current.integer))
		{
			CGaz::PmoveSingle(&s.pm, &s.pml);
		}
	}

	CGaz::CGaz()
	{
		Dvars::mdd_cgaz = Game::Dvar_RegisterBool(
			/* name		*/ "mdd_cgaz",
			/* desc		*/ "Display mDd CampingGaz HUD",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_cgaz_ground = Game::Dvar_RegisterBool(
			/* name		*/ "mdd_cgaz_ground",
			/* desc		*/ "show true ground zones",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_cgaz_speed = Game::Dvar_RegisterInt(
			/* name		*/ "mdd_cgaz_speed",
			/* desc		*/ "min speed value for draw CGaz",
			/* default	*/ 1,
			/* minVal	*/ 1,
			/* maxVal	*/ 1000,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_cgaz_yh = Game::Dvar_RegisterVec2(
			/* name		*/ "mdd_cgaz_yh",
			/* desc		*/ "mdd_cgaz position on screen 'Y' and cgaz thickness 'H'",
			/* y		*/ 180.0f,
			/* h		*/ 12.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 640.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_cgaz_rgbaNoAccel = Game::Dvar_RegisterVec4(
			/* name		*/ "mdd_cgaz_rgbaNoAccel",
			/* desc		*/ "color for no accel zone",
			/* x		*/ 0.25f,
			/* y		*/ 0.25f,
			/* z		*/ 0.25f,
			/* w		*/ 0.5f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_cgaz_rgbaPartialAccel = Game::Dvar_RegisterVec4(
			/* name		*/ "mdd_cgaz_rgbaPartialAccel",
			/* desc		*/ "color for partial accel zone",
			/* x		*/ 0.0f,
			/* y		*/ 1.0f,
			/* z		*/ 0.0f,
			/* w		*/ 0.5f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_cgaz_rgbaFullAccel = Game::Dvar_RegisterVec4(
			/* name		*/ "mdd_cgaz_rgbaFullAccel",
			/* desc		*/ "color for full accel zone",
			/* x		*/ 0.0f,
			/* y		*/ 0.25f,
			/* z		*/ 0.25f,
			/* w		*/ 0.5f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mdd_cgaz_rgbaTurnZone = Game::Dvar_RegisterVec4(
			/* name		*/ "mdd_cgaz_rgbaTurnZone",
			/* desc		*/ "color for turn zone",
			/* x		*/ 1.0f,
			/* y		*/ 1.0f,
			/* z		*/ 0.0f,
			/* w		*/ 0.5f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);
	}

	CGaz::~CGaz()
	{ }
}