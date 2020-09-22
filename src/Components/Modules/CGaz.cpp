// Port from mDd client Proxymod (https://github.com/Jelvan1/cgame_proxymod/blob/master/src/cg_cgaz.c)
#include "STDInclude.hpp"

#define PM_NOCLIP			2
#define PM_UFO				3
#define PM_SPEC				4

#define SURF_SLICK			0x2

#define PMF_PRONE			0x1
#define	PMF_TIME_KNOCKBACK	0x100
#define PMF_SPRINTING		0x8000
#define PMF_TOOK_DAMAGE		0x10000

#define PMF_FOLLOW			2				// spectate following another player

// movement parameters
#define pm_accelerate			9.f
#define pm_crouch_accelerate	12.f
#define pm_prone_accelerate		19.f
#define pm_airaccelerate		1.f

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

	static void PmoveSingle(Game::pmove_t *pm, Game::pml_t *pml);
	static void PM_AirMove(Game::pmove_t *pm, Game::pml_t *pml);
	static void PM_WalkMove(Game::pmove_t *pm, Game::pml_t *pml);

	static void CG_DrawCGaz();

	void CGaz::draw_cgaz()
	{
		if (!Dvars::mdd_cgaz->current.enabled || Game::cgs->demoType) return;

		s.pm = *Game::pmove;

		if (!(s.pm.ps->otherFlags & PMF_FOLLOW) && (Utils::vector::_VectorLengthSquared2(s.pm.ps->velocity) >= Dvars::mdd_cgaz_speed->current.integer * Dvars::mdd_cgaz_speed->current.integer))
			PmoveSingle(&s.pm, &s.pml);
	}

	static void PmoveSingle(Game::pmove_t *pm, Game::pml_t *pml)
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
			PM_WalkMove(pm, pml);
		}
		else
		{
			// airborne
			PM_AirMove(pm, pml);
		}
		CG_DrawCGaz();
	}

	static void CG_DrawCGaz()
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
		float       d_max_cos = num >= state->a ? 0 : acosf(num / state->a);
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
		if (!Dvars::mdd_cgaz_ground->current.enabled || state.v_squared - state.vf_squared >= 2 * state.a * state.wishspeed - state.a_squared)
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
	static void PM_Accelerate(float wishspeed, float accel)
	{
		update_d(wishspeed, accel, 0);
	}

	static void PM_SlickAccelerate(float wishspeed, float accel)
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
	float PM_CmdScale(Game::playerState_s *ps, Game::usercmd_s *cmd)
	{
		signed int max;
		double total;
		double scale;
		double player_spectateSpeedScale = Game::Dvar_FindVar("player_spectateSpeedScale")->current.value;

		max = abs(cmd->forwardmove);
		if (abs(cmd->rightmove) > max)
		{
			max = abs(cmd->rightmove);
		}
		if (!max)
		{
			return 0.0;
		}

		total = sqrt(cmd->forwardmove * (signed int)cmd->forwardmove + cmd->rightmove * (signed int)cmd->rightmove);
		scale = ps->speed * max / (total * 127.0);
		if (ps->pm_flags & 0x40 || 0.0 != ps->leanf)
			scale *= 0.4000000059604645;
		if (ps->pm_type == PM_NOCLIP)
			scale *= 3.0;
		if (ps->pm_type == PM_UFO)
			scale *= 6.0;
		if (ps->pm_type == PM_SPEC)
			scale *= player_spectateSpeedScale;
		return (float)scale;
	}

	/*
	===================
	PM_AirMove

	===================
	*/
	static void PM_AirMove(Game::pmove_t *pm, Game::pml_t *pml)
	{
		Game::PM_Friction(pm->ps, pml);

		float const scale = PM_CmdScale(pm->ps, &pm->cmd);

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

		PM_Accelerate(wishspeed, pm_airaccelerate);
	}

	double sub_410A10(Game::pmove_t* pm, int a2, int a3)
	{
		double v9, v10;

		auto ps = pm->ps;

		if (!ps->viewHeightLerpTime)
			return 0.0;

		if (a2 != ps->viewHeightLerpTarget || a2 == 0x28 && (a3 != 0xB || ps->viewHeightLerpDown))
			return 0.0;

		if (ps->viewHeightLerpTarget == 0xB)
		{
			v9 = 400.0;
		}
		else if (ps->viewHeightLerpTarget == 0x28)
		{
			v9 = ps->viewHeightLerpDown != 0 ? 200.0 : 400.0;
		}
		else
		{
			v9 = 200.0;
		}

		v10 = (pm->cmd.serverTime - ps->viewHeightLerpTime) / v9;

		if (v10 >= 0.0)
		{
			if (v10 <= 1.0)
				return v10;
		}
		return 0.0;
	}

	double PM_CmdScaleForStance(Game::pmove_t* pm)
	{
		double v10, v11, result;

		auto ps = pm->ps;

		v10 = sub_410A10(pm, 0xB, 0x28);
		if (v10 != 0.0)
		{
			return ((1.0 - v10) * 0.6499999761581421 + v10 * 0.1500000059604645);
		}

		if (ps->viewHeightLerpTime)
		{
			if (ps->viewHeightLerpTarget == 0x28 && !ps->viewHeightLerpDown)
			{
				v11 = (pm->cmd.serverTime - ps->viewHeightLerpTime) / 400.0;
				if (v11 >= 0.0)
				{
					if (v11 > 1.0)
					{
						return ((1.0 - 1.0) * 0.1500000059604645 + 1.0 * 0.6499999761581421);
					}
					if (v11 != 0.0)
					{
						return ((1.0 - v11) * 0.1500000059604645 + v11 * 0.6499999761581421);
					}
				}
			}
		}
		result = 1.0;
		if (ps->viewHeightTarget == 0x16 || ps->viewHeightTarget == 0x28)
			return 0.64999998;
		if (ps->viewHeightTarget == 0xB)
			result = 0.15000001;
		return result;
	}

	/*
	==============
	PM_CmdScale_Walk

	Returns the scale factor to apply to cmd movements when on ground
	==============
	*/
	float PM_CmdScale_Walk(Game::pmove_t* pm)
	{
		double v8;
		char v25;
		double result;

		auto player_backSpeedScale = Game::Dvar_FindVar("player_backSpeedScale")->current.value;
		auto player_strafeSpeedScale = Game::Dvar_FindVar("player_strafeSpeedScale")->current.value;
		auto player_sprintSpeedScale = Game::Dvar_FindVar("player_sprintSpeedScale")->current.value;

		auto ps = pm->ps;

		if (!(ps->pm_flags & PMF_PRONE) || (v25 = 1, ps->fWeaponPosFrac <= 0.0))
			v25 = 0;

		char fmove = pm->cmd.forwardmove;
		char smove = pm->cmd.rightmove;

		double total = sqrt(fmove * fmove + smove * smove);

		v8 = fmove >= 0 ? fmove : player_backSpeedScale * fmove;

		double max = abs(v8);
		if (abs(player_strafeSpeedScale * smove) > max)
		{
			max = abs(player_strafeSpeedScale * smove);
		}
		if (!max)
		{
			return 0.0;
		}

		int** dword_736db8 = reinterpret_cast<int**>(0x736DB8);
		byte* byte_732654 = reinterpret_cast<byte*>(0x732654);

		double scale = ps->speed * max / (total * 127.0);

		if (ps->pm_flags & 0x40 || 0.0 != ps->leanf || v25)
			scale *= 0.4000000059604645;

		if (ps->pm_flags & PMF_SPRINTING)
			scale *= player_sprintSpeedScale;

		if (ps->pm_type == PM_NOCLIP)
		{
			scale *= 3.0;
		}
		else if (ps->pm_type == PM_UFO)
		{
			scale *= 6.0;
		}
		else
		{
			scale *= PM_CmdScaleForStance(pm);
		}

		if (!ps->weapon || 0.0 >= *((float*)&*dword_736db8[ps->weapon] + 0x106) || ps->pm_flags & 0x40 || v25)
		{
			if (ps->weapon && 0.0 < *((float*)&*dword_736db8[ps->weapon] + 0x107))
				scale *= *((float*)&*dword_736db8[ps->weapon] + 0x107);
		}
		else
		{
			scale *= *((float*)&*dword_736db8[ps->weapon] + 0x106);
		}

		if (ps->pm_flags & PMF_TOOK_DAMAGE)
		{
			if (byte_732654[0x268 * ps->shellshockIndex])
				scale *= 0.4000000059604645;
		}
		result = ps->moveSpeedScaleMultiplier * scale;

		return (float)result;
	}

	float PM_DamageScale_Walk(int damage_timer)
	{
		float player_dmgtimer_maxTime = Game::Dvar_FindVar("player_dmgtimer_maxTime")->current.value;
		float player_dmgtimer_minScale = Game::Dvar_FindVar("player_dmgtimer_minScale")->current.value;

		if (!damage_timer || player_dmgtimer_maxTime == 0.0f)
			return 1.0f;

		return ((-player_dmgtimer_minScale / player_dmgtimer_maxTime) * damage_timer + 1.0f);
	}

	/*
	===================
	PM_WalkMove

	===================
	*/
	static void PM_WalkMove(Game::pmove_t *pm, Game::pml_t *pml)
	{
		if (Game::Jump_Check(pm, pml))
		{
			// jumped away
			PM_AirMove(pm, pml);
			return;
		}

		Game::PM_Friction(pm->ps, pml);

		float const scale = PM_CmdScale_Walk(pm);
		float const dmg_scale = PM_DamageScale_Walk(pm->ps->damageTimer) * scale;

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

		float accelerate;
		// when a player gets hit, they temporarily lose
		// full control, which allows them to be moved a bit
		if (pml->groundTrace.surfaceFlags & SURF_SLICK || pm->ps->pm_flags & PMF_TIME_KNOCKBACK)
		{
			PM_SlickAccelerate(wishspeed, pm_airaccelerate);
		}
		else
		{
			if (pm->ps->viewHeightTarget == 0xB)
			{
				accelerate = pm_prone_accelerate;
			}
			else if (pm->ps->viewHeightTarget == 0x28)
			{
				accelerate = pm_crouch_accelerate;
			}
			else
			{
				accelerate = pm_accelerate;
			}
			PM_Accelerate(wishspeed, accelerate);
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