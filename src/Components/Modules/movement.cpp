#include "STDInclude.hpp"

#define DAMAGE_NO_KNOCKBACK			0x00000004	// prob. wrong
#define FL_NO_KNOCKBACK				0x00000800	// prob. wrong

#define	ENTITYNUM_NONE		1023

#define	MIN_WALK_NORMAL		0.7f
#define OVERCLIP			1.001f
#define MAX_CLIP_PLANES		5

#define PM_NOCLIP			2
#define PM_UFO				3
#define PM_SPEC				4
#define PM_DEAD				7

#define	SURF_NODAMAGE		0x1
#define SURF_SLICK			0x2

#define PMF_NONE			0x0				// could be ET_NONE?
#define PMF_DUCKED			0x2				// correct
#define PMF_MANTLE			0x4				// correct :: pm_flags 4
#define PMF_LADDER			0x8				// correct :: pm_flags 8
#define PMF_BACKWARDS_JUMP	0x20			// correct :: find correct q3 name
#define PMF_LADDER_DOWN		0x28			// correct
#define	PMF_TIME_KNOCKBACK	0x100			// prob. wrong // in friction 0x80u? == dec 128
#define PMF_JUMP_HELD		0x400
#define PMF_JUMPING			0x4000			// correct
#define PMF_PRONE			0x4001			// correct
#define PMF_SPRINTING		0x8000			// correct
#define PMF_TOOK_DAMAGE		0x10000			// something like that
#define PMF_MELEEING		0x20000			// not changing on melee?
#define PMF_RESPAWNED		0x80000			// ?

#define CPM_PM_JUMP_Z		100.0f
#define CPM_PM_CLIPTIME		200

using namespace utils::vector;

namespace Components
{
	void PM_MT_AirMove(Game::pmove_t *pm, Game::pml_t *pml);
	bool PM_Jump_Check(Game::pmove_t *pm, Game::pml_t *pml);
	void PM_DisableSprint(Game::playerState_s *ps);
	
	#pragma region BG-SLIDEMOVE

	// stock cod4 clipvelocity
	void PM_ClipVelocity(const float *in, const float *normal, float *out)
	{
		const float outA = in[0] * normal[0] + in[1] * normal[1] + in[2] * normal[2];
		const float outB = outA - outA * 0.001000000047497451f;
		const float outC = -outB;

		out[0] = outC * normal[0] + in[0];
		out[1] = outC * normal[1] + in[1];
		out[2] = outC * normal[2] + in[2];
	}

	// Defrag :: clipvelocity
	void PM_Q3_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce)
	{
		float backoff = DotProduct(in, normal);
		if (backoff < 0.0f)
		{
			backoff *= overbounce;
		}
		else
		{
			backoff /= overbounce;
		}

		for (auto i = 0; i < 3; i++)
		{
			out[i] = in[i] - (normal[i] * backoff);
		}
	}

	// Defrag :: Quake 3 slidemove
	bool PM_Q3_SlideMove(Game::pmove_t *pm, Game::pml_t *pml, bool gravity)
	{
		const int NUM_BUMPS = 4;

		vec3_t planes[MAX_CLIP_PLANES];
		vec3_t primal_velocity, clip_velocity, end_velocity, end_clip_velocity;
		vec3_t dir, end, end_pos;

		int	numplanes, bumpcount, i, j, k;
		float time_left, into, d;

		Game::trace_t trace = {};

		VectorCopy(pm->ps->velocity, primal_velocity);

		if (gravity)
		{
			VectorCopy(pm->ps->velocity, end_velocity);
			end_velocity[2] -= static_cast<float>(pm->ps->gravity) * pml->frametime;
			
			pm->ps->velocity[2] = (pm->ps->velocity[2] + end_velocity[2]) * 0.5f;
			primal_velocity[2] = end_velocity[2];

			// slide along the ground plane
			if (pml->groundPlane) 
			{
				PM_Q3_ClipVelocity(pm->ps->velocity, pml->groundTrace.normal, pm->ps->velocity, OVERCLIP);
			}
		}

		time_left = pml->frametime;

		// never turn against the ground plane
		if (pml->groundPlane)
		{
			numplanes = 1;
			VectorCopy(pml->groundTrace.normal, planes[0]);
		}
		else 
		{
			numplanes = 0;
		}
			

		// never turn against original velocity
		_VectorNormalize2(pm->ps->velocity, planes[numplanes]);
		numplanes++;

		for (bumpcount = 0; bumpcount < NUM_BUMPS; bumpcount++)
		{
			// calculate position we are trying to move to
			VectorMA(pm->ps->origin, time_left, pm->ps->velocity, end);

			// see if we can make it there
			Game::PM_playerTrace(pm, &trace, pm->ps->origin, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask);

			// entity is completely trapped in another solid
			if (trace.allsolid)
			{
				pm->ps->velocity[2] = 0.0f;  // don't build up falling damage, but allow sideways acceleration
				
				if (dvars::pm_debug_prints->current.enabled) 
				{
					Game::Com_PrintMessage(0, utils::va("PM_Q3_SlideMove :: ALL SOLID! \n"), 0);
				}
				
				return true;
			}

			// T5? ++++++++++++++++++++++++++++++++++++++
			/*if (trace.startsolid) {
				VectorAdd(pm->ps->origin, trace.normal, pm->ps->origin);
			}*/

			// actually covered some distance
			if (trace.fraction > 0.0f)
			{
				_Vec3Lerp(pm->ps->origin, end, trace.fraction, end_pos);
				VectorCopy(end_pos, pm->ps->origin); // could move "endpos" into origin directly above
			}

			// moved the entire distance
			if (trace.fraction == 1.0f) 
			{
				break;
			} 

			// save entity for contact
			Game::PM_AddTouchEnt(pm, pm->ps->groundEntityNum); // T5 uses Trace_GetEntityHitId(&trace) instead groundEntity

			time_left -= time_left * trace.fraction;

			// this shouldn't really happen
			if (numplanes >= MAX_CLIP_PLANES) // CoD has 8 max planes
			{
				VectorClear(pm->ps->velocity);
				
				if (dvars::pm_debug_prints->current.enabled) 
				{
					Game::Com_PrintMessage(0, utils::va("PM_Q3_SlideMove :: MAX CLIP PLANES! \n"), 0);
				}

				return true;
			}

			// if this is the same plane we hit before, nudge velocity out along it, 
			// which fixes some epsilon issues with non-axial planes
			for (i = 0; i < numplanes; i++)
			{
				if (DotProduct(trace.normal, planes[i]) > 0.99f)
				{
					if (dvars::pm_debug_prints->current.enabled) 
					{
						Game::Com_PrintMessage(0, utils::va("PM_Q3_SlideMove :: same plane, nudge velocity! \n"), 0);
					}
					
					//PM_Q3_ClipVelocity(pm->ps->velocity, trace.normal, pm->ps->velocity, OVERCLIP); // T5 .. maybe remove didint fix stuck
					VectorAdd(trace.normal, pm->ps->velocity, pm->ps->velocity);
					break;
				}
			}

			if (i < numplanes) 
			{
				continue;
			}

			VectorCopy(trace.normal, planes[numplanes]);
			numplanes++;

			//
			// modify velocity so it parallels all of the clip planes
			//

			// find a plane that it enters
			for (i = 0; i < numplanes; i++)
			{
				into = DotProduct(pm->ps->velocity, planes[i]);
				
				if (into >= 0.1f) 
				{
					continue;   // move doesn't interact with the plane
				}
					
				// see how hard we are hitting things
				if (-into > pml->impactSpeed) 
				{
					pml->impactSpeed = -into;
				}

				// slide along the plane
				PM_Q3_ClipVelocity(pm->ps->velocity, planes[i], clip_velocity, OVERCLIP);

				// slide along the plane
				PM_Q3_ClipVelocity(end_velocity, planes[i], end_clip_velocity, OVERCLIP);

				// see if there is a second plane that the new move enters
				for (j = 0; j < numplanes; j++)
				{
					if (j == i) 
					{
						continue;
					}

					if (DotProduct(clip_velocity, planes[j]) >= 0.1f) 
					{
						continue;   // move doesn't interact with the plane
					}

					// try clipping the move to the plane
					PM_Q3_ClipVelocity(clip_velocity,	planes[j], clip_velocity,	OVERCLIP);
					PM_Q3_ClipVelocity(end_clip_velocity, planes[j], end_clip_velocity, OVERCLIP);

					// see if it goes back into the first clip plane
					if (DotProduct(clip_velocity, planes[i]) >= 0) 
					{
						continue;
					}

					// slide the original velocity along the crease
					_CrossProduct(planes[i], planes[j], dir);
					_VectorNormalize(dir);

					d = DotProduct(dir, pm->ps->velocity);
					VectorScale(dir, d, clip_velocity);

					_CrossProduct(planes[i], planes[j], dir);
					_VectorNormalize(dir);

					d = DotProduct(dir, end_velocity);
					VectorScale(dir, d, end_clip_velocity);

					// see if there is a third plane the the new move enters
					for (k = 0; k < numplanes; k++)
					{
						if (k == i || k == j) 
						{
							continue;
						}

						if (DotProduct(clip_velocity, planes[k]) >= 0.1f) 
						{
							continue;   // move doesn't interact with the plane
						}

						// stop dead at a tripple plane interaction
						VectorClear(pm->ps->velocity);

						if (dvars::pm_debug_prints->current.enabled) 
						{
							Game::Com_PrintMessage(0, utils::va("PM_Q3_SlideMove :: STOP DEAD! \n"), 0);
						}

						return true;
					}
				}

				// if we have fixed all interactions, try another move
				VectorCopy(clip_velocity, pm->ps->velocity);
				VectorCopy(end_clip_velocity, end_velocity);
				
				break;
			}
		}

		if (gravity)
		{
			VectorCopy(end_velocity, pm->ps->velocity);
		}

		// don't change velocity if in a timer :: clipping is caused by this
		if (pm->ps->pm_time) 
		{
			VectorCopy(primal_velocity, pm->ps->velocity);
			//Game::Com_PrintMessage(0, utils::va("PM_Q3_SlideMove :: CLIP Timer! \n"), 0);
		}	

		return (bumpcount != 0);
	}

	// Stock ProjectVelocity for exp. bouncing
	void PM_ProjectVelocity(float *normal, float *velIn, float *velOut)
	{
		const float speed_xy = velIn[1] * velIn[1] + velIn[0] * velIn[0];

		if ((normal[2]) < 0.001f || (speed_xy == 0.0f)) 
		{
			VectorCopy(velIn, velOut);
		}

		else
		{
			const float normalized = -(normal[1] * velIn[1] + velIn[0] * normal[0]) / normal[2];
			const float projection = sqrtf(((velIn[2] * velIn[2] + speed_xy) / (speed_xy + normalized * normalized)));
			
			if (projection < 1.0f || normalized < 0.0f || velIn[2] > 0.0f)
			{
				velOut[0] = projection * velIn[0];
				velOut[1] = projection * velIn[1];
				velOut[2] = projection * normalized;
			}
		}
	}

	// Defrag :: StepSlideMove
	void PM_Q3_StepSlideMove(Game::pmove_t *pm, Game::pml_t *pml, bool gravity)
	{
		vec3_t start_o, start_v, endpos;
		vec3_t down_o, down_v;
		vec3_t up, down;

		const auto& jump_stepSize = Game::Dvar_FindVar("jump_stepSize");

		Game::trace_t trace = {};

		if (pm->ps->pm_flags & 8)
		{
			pm->ps->jumpOriginZ = 0.0;
			trace.allsolid = false;
			pm->ps->pm_flags = pm->ps->pm_flags & 0xFFFFBFFF;
		}

		else if (pml->groundPlane) 
		{
			trace.allsolid = true;
		}

		else
		{
			trace.allsolid = false;
			if (pm->ps->pm_flags & 0x4000 && pm->ps->pm_time)
			{
				pm->ps->jumpOriginZ = 0.0f;
				pm->ps->pm_flags = pm->ps->pm_flags & 0xFFFFBFFF;
			}
		}

		VectorCopy(pm->ps->origin, start_o);
		VectorCopy(pm->ps->velocity, start_v);

		if (PM_Q3_SlideMove(pm,pml,gravity) == 0) 
		{
			return;		// we got exactly where we wanted to go first try	
		}

		VectorCopy(start_o, down);
		down[2] -= jump_stepSize->current.value;

		Game::PM_playerTrace(pm, &trace, start_o, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
		VectorSet(up, 0, 0, 1);

		// never step up when you still have up velocity // org
		if (pm->ps->velocity[2] > 0.0f && (trace.fraction == 1.0f || DotProduct(trace.normal, up) < 0.7f)) 
		{
			return;
		}

		VectorCopy(pm->ps->origin, down_o);
		VectorCopy(pm->ps->velocity, down_v);

		VectorCopy(start_o, up);
		up[2] += jump_stepSize->current.value;

		// test the player position if they were a stepheight higher
		Game::PM_playerTrace(pm, &trace, start_o, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask);
		_Vec3Lerp(pm->ps->origin, up, trace.fraction, endpos); // cod4 trace doesn't save the endpos so we calculate it the way q3 does it

		if (trace.allsolid) 
		{
			if (dvars::pm_debug_prints->current.enabled) 
			{
				Game::Com_PrintMessage(0, utils::va("PM_Q3_StepSlideMove :: Bend can't step! \n"), 0);
			}

			return;	// can't step up
		}

		float stepSize = endpos[2] - start_o[2];

		// try slidemove from this position
		VectorCopy(endpos, pm->ps->origin);
		VectorCopy(start_v, pm->ps->velocity);

		PM_Q3_SlideMove(pm,pml,gravity);

		// push down the final amount
		VectorCopy(pm->ps->origin, down);
		down[2] -= stepSize;

		Game::PM_playerTrace(pm, &trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
		_Vec3Lerp(pm->ps->origin, down, trace.fraction, endpos); // cod4 trace doesn't save the endpos so we calculate it the way q3 does it

		if (!trace.allsolid) 
		{
			VectorCopy(endpos, pm->ps->origin);
		}

		if (trace.fraction < 1.0f) 
		{
			// If exp. bouncing
			if (dvars::pm_cpm_useBouncing->current.enabled)
			{
				if (!trace.walkable && trace.normal[2] < 0.30000001f) 
				{ 
					if (dvars::pm_debug_prints->current.enabled) 
					{
						Game::Com_PrintMessage(0, utils::va("PM_Q3_StepSlideMove :: ^1Clip ^7Velocity! \n"), 0);
					}
					//PM_Q3_ClipVelocity(pm->ps->velocity, trace.normal, pm->ps->velocity, OVERCLIP);

					VectorCopy(start_o, pm->ps->velocity);
					VectorCopy(start_v, pm->ps->velocity);

					return;
				}

				PM_ProjectVelocity(trace.normal, pm->ps->velocity, pm->ps->velocity);

				if (dvars::pm_debug_prints->current.enabled) 
				{
					Game::Com_PrintMessage(0, utils::va("PM_Q3_StepSlideMove :: Project Velocity! \n"), 0);
				}
			}

			else 
			{
				// if !inAir // <-------------------------------------------------- needed?
				if (!((pm->ps->velocity[2] > 0.0f) && (trace.fraction == 1.0f || DotProduct(trace.normal, up) < 0.7f))) 
				{
					PM_Q3_ClipVelocity(pm->ps->velocity, trace.normal, pm->ps->velocity, OVERCLIP);
				}
				else 
				{
					VectorCopy(start_v, pm->ps->velocity);
				}

				// this keeps velocity on walkable slopes if fast enough
				//VectorCopy(start_v, pm->ps->velocity);

				//PM_Q3_ClipVelocity(pm->ps->velocity, trace.normal, pm->ps->velocity, OVERCLIP);
			}
		}

		//{
		//	// use the step move
		//	float	delta;

		//	delta = pm->ps->origin[2] - start_o[2];
		//	if (delta > 2) {
		//		if (delta < 7) {
		//			PM_AddEvent(EV_STEP_4);
		//		}
		//		else if (delta < 11) {
		//			PM_AddEvent(EV_STEP_8);
		//		}
		//		else if (delta < 15) {
		//			PM_AddEvent(EV_STEP_12);
		//		}
		//		else {
		//			PM_AddEvent(EV_STEP_16);
		//		}
		//	}
		//	if (pm->debugLevel) {
		//		Com_Printf("%i:stepped\n", c_pmove);
		//	}
		//}

		/*if (pm_debugPrints->current.enabled) {
			Game::Com_PrintMessage(0, utils::va("PM_Q3_StepSlideMove :: stepped! \n"), 0);
		}*/

	}
	
	// Backup StepSlideMove
	
	// ### 1337
	// defrag :: quake 3 stepslidemove - without slope down jump acceleration?
	//void PM_Q3_StepSlideMove(Game::pmove_t *pm, Game::pml_t *pml, bool gravity, bool predictive = false)
	//{
	//	vec3_t		start_o, start_v;
	//	vec3_t		normal, endpos;
	//	vec3_t		step_v, step_vNormal;
	//	vec3_t		up, down;

	//	Game::trace_t   trace;

	//	float		stepSize;
	//	bool		stepped = false; //, inAir;

	//	VectorSet	(normal, 0.0f, 0.0f, 1.0f);

	//	VectorCopy	(pm->ps->origin, start_o);
	//	VectorCopy	(pm->ps->velocity, start_v);

	//	if (pm->ps->pm_flags & 8)
	//	{
	//		pm->ps->jumpOriginZ = 0.0;
	//		trace.allsolid = 0;
	//		pm->ps->pm_flags = pm->ps->pm_flags & 0xFFFFBFFF;
	//	}

	//	else if (pml->groundPlane)
	//	{
	//		trace.allsolid = 1;
	//	}

	//	else
	//	{
	//		trace.allsolid = 0;
	//		if (pm->ps->pm_flags & 0x4000 && pm->ps->pm_time)
	//		{
	//			pm->ps->jumpOriginZ = 0.0;
	//			pm->ps->pm_flags = pm->ps->pm_flags & 0xFFFFBFFF;
	//		}
	//	}

	//	//if (PM_Q3_SlideMove(pm, pml, gravity) == 0)
	//	if (Game::PM_SlideMove(pm, pml, gravity) == 0)
	//	{
	//		VectorCopy	(start_o, down);
	//		VectorMA	(down, -Game::Dvar_FindVar("jump_stepSize")->current.value, normal, down);
	//		
	//		Game::PM_playerTrace(pm, &trace, start_o, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);

	//		// Step down and play anim
	//		if (trace.fraction > 0.01f && trace.fraction < 1.0f && !trace.allsolid && pml->groundPlane != false)
	//		{
	//			//Game::Com_PrintMessage(0, utils::va("PM_Q3_StepSlideMove :: Step down! \n"), 0);
	//			stepped = true;
	//		}
	//	}

	//	else
	//	{
	//		VectorCopy	(start_o, down);
	//		VectorMA	(down, -Game::Dvar_FindVar("jump_stepSize")->current.value, normal, down);
	//
	//		Game::PM_playerTrace(pm, &trace, start_o, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
	//		
	//		// never step up when you still have up velocity
	//		if (DotProduct(trace.normal, pm->ps->velocity) > 0.0f && (trace.fraction == 1.0f || DotProduct(trace.normal, normal) < 0.7f))
	//		{
	//			//if (pm->ps->velocity[2] > 0.0f) // not needed
	//			return;
	//		}

	//		VectorCopy	(start_o, up);
	//		VectorMA	(up, Game::Dvar_FindVar("jump_stepSize")->current.value, normal, up);

	//		// test the player position if they were a stepheight higher
	//		Game::PM_playerTrace(pm, &trace, start_o, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask);
	//		
	//		if (trace.allsolid)
	//		{
	//			Game::Com_PrintMessage(0, utils::va("PM_Q3_StepSlideMove :: Bend can't step! \n"), 0);

	//			return;
	//		}

	//		Vec3Lerp(pm->ps->origin, up, trace.fraction, endpos);

	//		VectorSubtract	(endpos, start_o, step_v);
	//		VectorCopy		(step_v, step_vNormal);
	//		_VectorNormalize(step_vNormal);

	//		stepSize = DotProduct(normal, step_vNormal) * VectorLength(step_v);
	//		
	//		// try slidemove from this position
	//		VectorCopy	(endpos, pm->ps->origin);
	//		VectorCopy	(start_v, pm->ps->velocity);

	//		// Step up and play anim
	//		//if (PM_Q3_SlideMove(pm, pml, gravity) == 0)
	//		if (Game::PM_SlideMove(pm, pml, gravity) == 0)
	//		{
	//			//Game::Com_PrintMessage(0, utils::va("PM_Q3_StepSlideMove :: Step up! \n"), 0);
	//			stepped = true;
	//		}

	//		// push down the final amount
	//		VectorCopy	(pm->ps->origin, down);
	//		VectorMA	(down, -stepSize, normal, down);
	//		
	//		Game::PM_playerTrace(pm, &trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
	//		
	//		Vec3Lerp(pm->ps->origin, down, trace.fraction, endpos);

	//		if (!trace.allsolid)
	//		{
	//			VectorCopy(endpos, pm->ps->origin);
	//		}

	//		if (trace.fraction < 1.0f)
	//		{
	//			// if !inAir // <-------------------------------------------------- needed?
	//			if (!((pm->ps->velocity[2] > 0.0f) && (trace.fraction == 1.0 || DotProduct(trace.normal, up) < 0.7f))) {
	//				PM_Q3_ClipVelocity(pm->ps->velocity, trace.normal, pm->ps->velocity, OVERCLIP);
	//			}

	//			// this keeps velocity on walkable slopes if fast enough
	//			VectorCopy(start_v, pm->ps->velocity); 
	//		}
	//	}

	//	// play anim
	//	if (!predictive && stepped)
	//	{	
	//		//PM_StepEvent(start_o, pm->ps->origin, normal);
	//		//Game::Com_PrintMessage(0, utils::va("PM_Q3_StepSlideMove :: STEP ANIM! \n"), 0);



	//		// or ::

	//		// use the step move
	//		//float	delta;

	//		//delta = pm->ps->origin[2] - start_o[2];
	//		//if (delta > 2) // smooth out transitions
	//		//{
	//		//	if (delta < 7)
	//		//	{
	//		//		PM_AddEvent(EV_STEP_4);
	//		//	}
	//		//	else if (delta < 11)
	//		//	{
	//		//		PM_AddEvent(EV_STEP_8);
	//		//	}
	//		//	else if (delta < 15)
	//		//	{
	//		//		PM_AddEvent(EV_STEP_12);
	//		//	}
	//		//	else
	//		//	{
	//		//		PM_AddEvent(EV_STEP_16);
	//		//	}
	//		//}
	//	}

	//	return;
	//}
	

	#pragma endregion
	#pragma region MOVEMENT-DEFRAG

	// Defrag :: Custom MovementDir used for defrag movement -> https://github.com/ZdrytchX/GPP-1-1/blob/master/src/game/bg_pmove.c#L499
	void PM_Q3_SetMovementDir(Game::pmove_t *pm)
	{
		// don't set dir when proned or crouched
		if (pm->ps->viewHeightCurrent == 11.0f || pm->ps->viewHeightCurrent == 40.0f)
		{
			return;
		}

		if (pm->cmd.forwardmove || pm->cmd.rightmove)
		{
			if (pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0) 
				pm->ps->movementDir = 0; // Forward

			else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0)
				pm->ps->movementDir = 1; // Strafe Forward Left

			else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0)
				pm->ps->movementDir = 2; // Left

			else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0)
				pm->ps->movementDir = 3; // Strafe Backwards Left

			else if (pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0)
				pm->ps->movementDir = 4; // Backward

			else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0)
				pm->ps->movementDir = 5; // Strafe Backwards Right

			else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0)
				pm->ps->movementDir = 6; // Right

			else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0)
				pm->ps->movementDir = 7; // Strafe Forward Right
		}
		else
		{
			// if they aren't actively going directly sideways,
			// change the animation to the diagonal so they

			// don't stop too crooked
			if (pm->ps->movementDir == 2) 
			{
				pm->ps->movementDir = 1;
			}
			else if (pm->ps->movementDir == 6) 
			{
				pm->ps->movementDir = 7;
			}
		}
	}

	// Defrag :: PM_CmdScale -> https://github.com/jangroothuijse/openpromode/blob/master/code/game/bg_pmove.c#L287
	float PM_Q3_CmdScale(Game::playerState_s *ps, Game::usercmd_s *cmd)
	{
		const auto fmove = static_cast<float>(cmd->forwardmove);
		const auto rmove = static_cast<float>(cmd->rightmove);

		float max = abs(fmove);

		if (abs(rmove) > max)
		{
			max = abs(rmove);
		}

		if (max == 0.0f)
		{
			return 0.0f;
		}

		const float total = sqrtf(fmove * fmove + rmove * rmove);
		return static_cast<float>(ps->speed) * max / (127.0f * total);

	}

	// Defrag :: PM_Aircontrol function when we are mid-air -> https://github.com/xzero450/revolution/blob/8d0b37ba438e65e19d5a5e77f5b9c2076b7900bc/game/bg_promode.c#L303
	void PM_Q3_AirControl(Game::pmove_t *pm, Game::pml_t *pml, vec3_t wishdir_b, float wishspeed_b)
	{
		const auto ps = pm->ps;

		//if( pm->cmd.forwardmove == 0.0f || wishspeed_b == 0.0 ) huh ( // if(WishMove.X != 0.f || WishSpeed == 0.f) )
		if ((ps->movementDir && ps->movementDir != 4) || wishspeed_b == 0.0f) 
		{
			return;
		}

		const auto& cpm_airControl = dvars::pm_cpm_airControl->current.value;
		const auto& cpm_airAccelerate = dvars::pm_cpm_airAccelerate->current.value;

		const float zspeed = ps->velocity[2];
		ps->velocity[2] = 0.0f;

		const float speed = _VectorNormalize(ps->velocity);
		const float dot = DotProduct(ps->velocity, wishdir_b);

		float k = 32.0f;
		k *= cpm_airControl * dot * dot * pml->frametime * cpm_airAccelerate;

		if (dot > 0)
		{
			for (auto i = 0; i < 2; i++) 
			{
				ps->velocity[i] = ps->velocity[i] * speed + wishdir_b[i] * k;
			}

			_VectorNormalize(ps->velocity);
		}

		for (auto i = 0; i < 2; i++) 
		{
			ps->velocity[i] *= speed;
		}

		ps->velocity[2] = zspeed;
	}

	// Defrag :: PM_Accelerate when we start our strafejump -> https://github.com/xzero450/revolution/blob/master/game/bg_pmove.c#L221
	void PM_Q3_Accelerate(Game::playerState_s *ps, Game::pml_t *pml, vec3_t wishdir_b, float wishspeed_b, float accel_b)
	{
		const float currentspeed = DotProduct(ps->velocity, wishdir_b);
		const float addspeed = wishspeed_b - currentspeed;
		
		if (addspeed <= 0) 
		{
			return;
		}

		float accelspeed = accel_b * pml->frametime*wishspeed_b;
		
		if (accelspeed > addspeed) 
		{
			accelspeed = addspeed;
		}

		for (auto i = 0; i < 3; i++) 
		{
			ps->velocity[i] += accelspeed * wishdir_b[i];
		}
	}

	// Defrag :: PM_Accelerate for WalkMoveHook Only
	void PM_Q3_Accelerate_Walk(float *wishdir, Game::pml_t *pml, Game::playerState_s *ps, float wishspeed, float accel)
	{
		PM_DisableSprint(ps);

		const float currentspeed = DotProduct(ps->velocity, wishdir);
		const float addspeed = wishspeed - currentspeed;
		
		if (addspeed <= 0)
		{
			//PM_CopyVelocityToLocal(ps);
			return;
		}

		// fix for slow prone, crouch & ads movement
		if (ps->viewHeightCurrent == 40.0f || wishspeed < 100.0f) 
		{
			wishspeed = 100.0f;
		}
		else if (ps->viewHeightCurrent == 11.0f) 
		{
			wishspeed = 80.0f;
		}

		float accelspeed = accel * pml->frametime * wishspeed;
		
		if (accelspeed > addspeed) 
		{
			accelspeed = addspeed;
		}

		for (auto i = 0; i < 3; i++) 
		{
			ps->velocity[i] += accelspeed * wishdir[i];
		}

		//PM_CopyVelocityToLocal(ps);
	}
	
	// Defrag :: Friction
	void PM_Q3_Friction(Game::pmove_t *pm, Game::pml_t *pml)
	{
		const float	friction = Game::Dvar_FindVar("friction")->current.value;

		vec3_t vec;
		VectorCopy(pm->ps->velocity, vec);

		if (pml->walking) 
		{
			vec[2] = 0.0f;	// ignore slope movement
		}

		const float speed = VectorLength(vec);
		if (speed < 1.0f)
		{
			pm->ps->velocity[0] = 0.0f;
			pm->ps->velocity[1] = 0.0f;	// allow sinking underwater

			// FIXME: still have z friction underwater?
			if (pm->ps->pm_type == PM_SPEC) 
			{
				pm->ps->velocity[2] = 0.0f; // no slow-sinking/raising movements
			}

			return;
		}

		float drop = 0.0f;

		// apply ground friction
		if ((pml->walking) && !(pml->groundTrace.surfaceFlags & SURF_SLICK))
		{
			// if getting knocked back, dont enter > no friction - works
			if (!(pm->ps->pm_flags & PMF_TIME_KNOCKBACK))
			{
				const float control = speed < 100.0f ? 100.0f : speed;

				// check if crouch sliding is enabled :: this lets us slide always while crouching?
				/*if (pm->ps->pm_flags & PMF_DUCKED) {
					drop += control * 1.2f * pml->frametime;
				}

				else {*/
					drop += control * friction * pml->frametime;
				//}
			}
		}

		// apply flying friction
		if (pm->ps->pm_type == PM_SPEC) 
		{
			drop += (speed * 5.0f * pml->frametime);
		}

		// scale the velocity
		float newspeed = speed - drop;

		if (newspeed < 0) 
		{
			newspeed = 0;
		}

		newspeed /= speed;

		for (int i = 0; i < 3; i++) 
		{
			pm->ps->velocity[i] *= newspeed;
		}
	}

	#pragma endregion
	#pragma region MOVEMENT-CS
	
	// CS :: Stock ClipVelocity
	void PM_CS_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce)
	{
		// Determine how far along plane to slide based on incoming direction.
		const float backoff = DotProduct(in, normal) * overbounce;

		for (auto i = 0; i < 3; i++)
		{
			out[i] = in[i] - (normal[i] * backoff);
		}

		// iterate once to make sure we aren't still moving through the plane
		const float adjust = DotProduct(out, normal);
		if (adjust < 0.0f)
		{
			vec3_t reduce;
			VectorScale(normal, adjust, reduce);
			VectorSubtract(out, reduce, out);
			//out -= reduce;
		}
	}

	// CS :: Modified TryPlayerMove
	void PM_CS_TryPlayerMove(Game::pmove_t *pm, Game::pml_t *pml)
	{
		const float	SURF_SLOPE_NORMAL = 0.7f;

		vec3_t end;
		Game::trace_t trace = {};

		const auto ps = pm->ps;

		if (VectorLength(ps->velocity) == 0.0) 
		{
			return;
		}

		// Assume we can move all the way from the current origin to the end point.
		VectorMA(ps->origin, pml->frametime, ps->velocity, end);
		Game::PM_playerTrace(pm, &trace, ps->origin, pm->mins, pm->maxs, end, ps->clientNum, pm->tracemask);

		// If we covered the entire distance, we are done and can return.
		if (trace.fraction == 1.0f)
		{
			return;
		}

		// If the plane we hit has a high z component in the normal, then it's probably a floor
		if (trace.normal[2] > SURF_SLOPE_NORMAL) 
		{
			return;
		}

		PM_CS_ClipVelocity(ps->velocity, trace.normal, ps->velocity, 1.0f);
	}

	// CS :: Stock AirAccelerate 
	void PM_CS_AirAccelerateReal(vec3_t wishdir, float wishspeed, Game::playerState_s *ps, Game::pml_t *pml) // we don`t need the default accel value as we use our dvar for that
	{
		float _wishspeed = wishspeed;

		const auto& accel = dvars::pm_cs_airAccelerate->current.value;
		const auto& airspeedcap	= dvars::pm_cs_airspeedCap->current.value;

		if (_wishspeed > airspeedcap) 
		{
			_wishspeed = airspeedcap;
		}

		const float currentspeed = DotProduct(ps->velocity, wishdir);
		const float addspeed = _wishspeed - currentspeed;

		if (addspeed > 0)
		{
			float accelspeed = pml->frametime * accel * wishspeed * 1.0f; // * surfacefriction?
			if (accelspeed > addspeed) 
			{
				accelspeed = addspeed;
			}

			for (auto i = 0; i < 3; i++) 
			{
				ps->velocity[i] += wishdir[i] * accelspeed;
			}
		}
	}
	#pragma endregion
	#pragma region WEAPONS-Q3

	// Rewritten gunRandom function (stock)
	void weapon_gun_random(float *x, float *y)
	{
		float a, b, c;

		a = rand() * 0.000030517578125f;
		b = a * 360.0f;

		a = rand() * 0.000030517578125f;
		b *= 0.01745329238474369f;

		b = cos(b);
		c = sin(b);

		*x = b * a;
		*y = c * a;
	}

	// Rewritten
	int G_GetWeaponIndexForEntity(Game::gentity_s *ent)
	{
		if (!ent->client)
		{
			return ent->s.weapon;
		}

		if (ent->client->ps.eFlags & 0x300 || ent->client->ps.pm_flags & 0x100000) 
		{
			return *&Game::g_entities[157 * ent->client->ps.viewlocked_entNum];
		}

		if (ent->client->ps.weapFlags & 2) 
		{
			return ent->client->ps.offHandIndex;
		}

		return ent->client->ps.weapon;
	}

	// Stock / Defrag
	void Weapon_RocketLauncher_Fire(Game::weaponParms *wp, Game::gentity_s *ent, int weapon_index, float spread, float *gun_vel, [[maybe_unused]] Game::gentity_s *target, float *target_offset)
	{
		vec3_t dir, _dir, kickback;
		float x, y;

		Game::gentity_s *m;

		if (!dvars::pm_cpm_useQuakeDamage || !dvars::pm_cpm_useQuakeDamage->current.enabled)
		{
			const float sprd = tan(spread * 0.017453292f) * 16.0f;
			weapon_gun_random(&x, &y);

			x *= sprd;
			y *= sprd;
		}

		else // no spread if defrag
		{
			x = 0.0f;
			y = 0.0f;
		}

		VectorScale(wp->forward, 16.0f, dir);
		VectorScale(wp->right, x, _dir);
		VectorAdd(dir, _dir, dir);

		VectorScale(wp->up, y, _dir);
		VectorAdd(dir, _dir, dir);

		_VectorNormalize(dir);

		//VectorCopy(wp->muzzleTrace, kickBack);

		//fix rocket delay just a tiny bit .. could be wrong
		/*kickBack[0] = wp->muzzleTrace[0] + (0.025f * (ent->client->ps.velocity[0]));
		kickBack[1] = wp->muzzleTrace[1] + (0.025f * (ent->client->ps.velocity[1]));
		kickBack[2] = wp->muzzleTrace[2] + (0.025f * (ent->client->ps.velocity[2]));*/

		kickback[0] = wp->muzzleTrace[0];
		kickback[1] = wp->muzzleTrace[1];
		kickback[2] = wp->muzzleTrace[2];

		// get current weapon name
		if (!strcmp(wp->weapDef->szInternalName, "q3rocket_mp")) 
		{
			Game::Globals::Q3_LastProjectileWeaponUsed = Game::Q3WeaponNames::Q3_ROCKET;
		}

		else if (!strcmp(wp->weapDef->szInternalName, "q3plasma_mp")) 
		{
			Game::Globals::Q3_LastProjectileWeaponUsed = Game::Q3WeaponNames::Q3_PLASMA;
		}

		else
		{
			Game::Globals::Q3_LastProjectileWeaponUsed = Game::Q3WeaponNames::Q3_NONE;
		}

		m = Game::G_FireRocket(kickback, ent, weapon_index, dir, gun_vel, 0, target_offset);

		if (!dvars::pm_cpm_useQuakeDamage || !dvars::pm_cpm_useQuakeDamage->current.enabled)
		{
			const auto velocity = ent->client->ps.velocity;

			vec3_t vel;
			const vec3_t zero = {};
			
			VectorSubtract(zero, wp->forward, vel);
			
			VectorScale(vel, dvars::pm_rocketJumpHeight->current.value, vel); // stock 64.0f
			VectorAdd(vel, velocity, velocity);
		}
	}

	// Defrag :: G_RadiusDamage - fixes alot of the knockback issues
	bool Q3_RadiusDamage(vec3_t origin, Game::gentity_s *inflictor, Game::gentity_s *attacker, float inner_damage, float fOuterDamage, float radius, float cone_angle_cos, float *cone_direction, Game::gentity_s *ignore_ent, int mod, int self_client)
	{
		float points;
		float dist;
		int	entity_list[1024];

		vec3_t mins, maxs;
		vec3_t v;
		vec3_t dir;

		if (radius < 1) 
		{
			radius = 1;
		}

		for (auto i = 0; i < 3; i++) 
		{
			mins[i] = origin[i] - radius;
			maxs[i] = origin[i] + radius;
		}

		Game::areaParms_t ap = {};
		ap.mins = mins;
		ap.maxs = maxs;
		ap.list = entity_list;
		ap.count = 0;
		ap.maxcount = 1024;
		ap.contentmask = -1;

		Game::CM_AreaEntities(1, &ap);
		//numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

		for (auto e = 0; e < ap.count; e++)
		{
			const auto ent = &Game::scr_g_entities[entity_list[e]];
			if (ent == ignore_ent)
			{
				continue;
			}

			if (!ent->takedamage)
			{
				continue;
			}

			// find the distance from the edge of the bounding box
			for (auto i = 0; i < 3; i++) 
			{
				if (origin[i] < ent->r.absmin[i])
				{
					v[i] = ent->r.absmin[i] - origin[i];
				}
				else if (origin[i] > ent->r.absmax[i]) 
				{
					v[i] = origin[i] - ent->r.absmax[i];
				}
				else
				{
					v[i] = 0;
				}
			}

			dist = VectorLength(v);

			if (dist >= radius) 
			{
				continue;
			}

			points = inner_damage * (1.0f - dist / radius);

			// CanDamageContentMask
			//if ((Game::CanDamage(ent, origin))) 
			//CanDamage(Game::gentity_s *inflictor /*eax*/, float *centerPos /*ecx*/, Game::gentity_s *ent, float coneAngleCos, float *coneDirection, int contentmask)
			if(Game::CanDamage(inflictor, origin, ent, cone_angle_cos, cone_direction, *Game::CanDamageContentMask))
			{

				VectorSubtract(ent->r.currentOrigin, origin, dir);
				// push the center of mass higher than the origin so players
				// get knocked into the air more

				if (ent == inflictor) 
				{
					dir[2] += 24.0f;
				}
				else
				{
					dir[2] += 40.0f;
				}
				
				// this basically calls Q3_DamageClient
				Game::G_Damage(dir, ent, inflictor, attacker, origin, static_cast<int>(points), 5, mod, self_client, 0, 0, 0, 0);
			}
		}

		return false;
	}

	// Defrag :: DamageClient with knockback for rocket and plasma
	void Q3_DamageClient(Game::gentity_s *targ, int weapon, Game::gentity_s *inflictor, Game::gentity_s *attacker, float *dir, float *point, int damage, int dflags, int mod, Game::hitLocation_t hit_loc, int time_offset)
	{
		//Game::Com_PrintMessage(0, utils::va("Q3_DamageClient :: ___dir[]: %.2lf, %.2lf, %.2lf \n", dir[0], dir[1], dir[2]), 0);
		//Game::Com_PrintMessage(0, utils::va("Q3_DamageClient :: _point[]: %.2lf, %.2lf, %.2lf \n", point[0], point[1], point[2]), 0);
		//Game::Com_PrintMessage(0, utils::va("Q3_DamageClient :: damage_org[]: %d \n", damage), 0);

		if (targ->takedamage 
			&& damage > 0 
			&& targ->client->sess.connected == 2
			&& targ->client->ps.pm_type != 7 
			&& !targ->client->noclip 
			&& !targ->client->ufo)
		{
			if (weapon == -1)
			{
				if (inflictor) // (inflictor && !inflictor->client)
				{ 
					weapon = G_GetWeaponIndexForEntity(inflictor);
				}
				else if (attacker) // (attacker && !attacker->client)
				{ 
					weapon = G_GetWeaponIndexForEntity(attacker);
				}
				else 
				{
					weapon = 0;
				}

				if (dir) 
				{
					_VectorNormalize(dir);
				}

				float scale = 1.0f;

				if (Game::Globals::Q3_LastProjectileWeaponUsed == Game::Q3WeaponNames::Q3_PLASMA) 
				{
					scale = 1.0f;
				}

				const int knockback = static_cast<int>( static_cast<float>(damage) * dvars::pm_cpm_damageKnockback->current.value * scale );

				// figure momentum add, even if the damage won't be taken
				if (dir && knockback && targ->client)
				{
					const auto& g_knockback = Game::Dvar_FindVar("g_knockback")->current.value;

					vec3_t kvel;
					const float mass = 200.0f;

					_VectorScale(dir, g_knockback * static_cast<float>(knockback) / mass, kvel);
					VectorAdd(targ->client->ps.velocity, kvel, targ->client->ps.velocity);

					if (dvars::pm_debug_prints->current.enabled) 
					{
						Game::Com_PrintMessage(0, utils::va("Q3_DamageClient :: exp-knockback: %d\n", knockback), 0);
					}

					// set the timer so that the other client can't cancel
					// out the movement immediately
					if (!targ->client->ps.pm_time)
					{
						int t = knockback * 2;
						if (t < 50) 
						{
							t = 50;
						}

						if (t > 200) 
						{
							t = 200;
						}

						targ->client->ps.pm_time = t;
						targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
					}
				}

				// CPM: Custom weapon knockbacks
				//scale = 1;

				//if ((targ == attacker) && (mod == MOD_ROCKET_SPLASH))
				//{
				//	// exception for selfdamage from rocket, rocketjumping remains unchanged
				//}
				//else
				//{
				//	switch (mod)
				//	{
				//	case MOD_GAUNTLET: scale = 0.5; break;
				//	case MOD_ROCKET: scale = 1.2; break;
				//	case MOD_ROCKET_SPLASH: scale = 1.2; break;
				//	case MOD_LIGHTNING: scale = 1.55; break;
				//	case MOD_PLASMA: scale = 1.2; break;
				//	case MOD_PLASMA_SPLASH: scale = 1.3; break;
				//	case MOD_RAILGUN: scale = 0.75; break;
				//	case MOD_BFG: scale = 1.4; break;
				//	case MOD_BFG_SPLASH: scale = 1.4; break;
				//	case MOD_GRENADE_SPLASH: scale = 1.1; break;
				//	default:;
				//	}
				//}

				// always give half damage if hurting self; calculated after knockback, so rocket jumping works
				if (targ == attacker) 
				{
					damage = static_cast<int>( static_cast<float>(damage) * 0.2f ); // default 2, but q3 has more health :p
				}

				if (damage < 1) 
				{
					damage = 1;
				}

				//Game::Com_PrintMessage(0, utils::va("Q3_DamageClient :: damage_mod[]: %d \n", damage), 0);
				Game::Scr_PlayerDamage(dir, targ, inflictor, attacker, damage, dflags, mod, weapon, point, hit_loc, time_offset);
			}
		}
	}

	// Defrag :: G_CalcMuzzlePoints in FireWeapon
	void Q3_CalcMuzzlePoints(Game::weaponParms *wp, Game::gentity_s *ent)
	{
		vec3_t	origin;

		_AngleVectors(ent->client->ps.viewangles, wp->forward, wp->right, wp->up);

		VectorCopy(ent->client->ps.origin, origin);
		origin[2] += ent->client->ps.viewHeightCurrent;
		VectorMA(origin, 14.0f, wp->forward, wp->muzzleTrace);
	}

	#pragma endregion
	#pragma region MOVEMENT-GENERAL

	// Disable Sprinting with dvar
	void PM_DisableSprint(Game::playerState_s *ps)
	{
		if (dvars::pm_disableSprint->current.enabled) 
		{
			ps->sprintState.sprintButtonUpRequired = 1;
		}
	}

	// STOCK/Q3/CS :: Entrypoint - Combined Q3CPM/Source WalkMove ->
	void PM_MT_WalkMove(Game::pmove_t *pm, Game::pml_t *pml)
	{
		// Stock Walkmove if not Defrag
		if (dvars::pm_movementType->current.integer != Game::PM_MTYPE::DEFRAG)
		{
			Game::PM_WalkMove(pm, pml);
			//PM_CopyVelocityToLocal(pm->ps);
			return;
		}

		int		i;
		vec3_t	wishvel;
		vec3_t	wishdir;
		float	wishspeed;
		float	scale;
		float	accelerate;
		float	vel;

		const float	pm_duck_scale = 0.5f;
		const float	pm_prone_scale = 0.25f;
		const float	pm_slick_accelerate = 15.0f;
		const float	pm_accelerate = 15.0f;

		// If Jumped -> AirMove
		if (PM_Jump_Check(pm, pml))
		{
			PM_MT_AirMove(pm, pml);
			return;
		}

		PM_Q3_Friction(pm, pml);

		const float fmove = pm->cmd.forwardmove;
		const float smove = pm->cmd.rightmove;

		Game::usercmd_s cmd = pm->cmd;
		scale = PM_Q3_CmdScale(pm->ps, &cmd);

		// set the movementDir so clients can rotate the legs for strafing
		PM_Q3_SetMovementDir(pm);

		// project moves down to flat plane
		pml->forward[2] = 0;
		pml->right[2]	= 0;

		// project the forward and right directions onto the ground plane
		PM_Q3_ClipVelocity(pml->forward, pml->groundTrace.normal, pml->forward, OVERCLIP);
		PM_Q3_ClipVelocity(pml->right, pml->groundTrace.normal, pml->right, OVERCLIP);
		
		_VectorNormalize(pml->forward);
		_VectorNormalize(pml->right);

		for (i = 0; i < 3; i++) 
		{
			wishvel[i] = pml->forward[i] * fmove + pml->right[i] * smove;
		}

		VectorCopy(wishvel, wishdir);
		wishspeed = _VectorNormalize(wishdir);
		wishspeed *= scale;

		// clamp the speed lower if ducking
		if ((pm->ps->pm_flags & PMF_DUCKED) && (wishspeed > static_cast<float>(pm->ps->speed) * pm_duck_scale)) 
		{
			wishspeed = static_cast<float>(pm->ps->speed) * pm_duck_scale;
		}
		// clamp the speed lower if prone
		else if ((pm->ps->pm_flags & PMF_PRONE) && (wishspeed > static_cast<float>(pm->ps->speed) * pm_prone_scale)) 
		{
			wishspeed = static_cast<float>(pm->ps->speed) * pm_prone_scale;
		}

		// when a player gets hit, he temporarily loses
		// full control, which allows him to be moved a bit
		if ((pml->groundTrace.surfaceFlags & SURF_SLICK) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK) 
		{
			accelerate = pm_slick_accelerate;
		}
		else if (pm->ps->pm_flags & PMF_DUCKED) 
		{
			accelerate = 15.0f; // pm_crouchaccelerate
		}
		else if (pm->ps->pm_flags & PMF_PRONE) 
		{
			accelerate = 10.0f; // pm_proneaccelerate
		}
		else 
		{
			accelerate = pm_accelerate;
		}

		PM_Q3_Accelerate_Walk(wishdir, pml, pm->ps, wishspeed, accelerate);

		if ((pml->groundTrace.surfaceFlags & SURF_SLICK) || (pm->ps->pm_flags & PMF_TIME_KNOCKBACK)) // TIME_KNOCKBACK NOT WORKING
		{ 
			pm->ps->velocity[2] -= static_cast<float>(pm->ps->gravity) * pml->frametime;
		}

		vel = VectorLength(pm->ps->velocity);

		// slide along the ground plane
		PM_Q3_ClipVelocity(pm->ps->velocity, pml->groundTrace.normal, pm->ps->velocity, OVERCLIP);

		// don't decrease velocity when going up or down a slope
		_VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

		// don't do anything if standing still
		if (pm->ps->velocity[0] == 0.0f && pm->ps->velocity[1] == 0.0f) 
		{
			return;
		}

		PM_Q3_StepSlideMove(pm, pml, false);
	}

	// STOCK/Q3/CS :: Entrypoint - Combined Q3CPM/Source AirMove ->
	void PM_MT_AirMove(Game::pmove_t *pm, Game::pml_t *pml)
	{
		// Stock Movement
		if (dvars::pm_movementType->current.integer == Game::PM_MTYPE::STOCK)
		{
			Game::PM_AirMove(pm, pml);
			//PM_CopyVelocityToLocal(ps);

			return;
		}

		const auto ps = pm->ps;

		PM_DisableSprint(ps);

		//////////////////////////////////////////////////////////////////////////////////

		float fmove, smove, wishspeed, scale = 1.0f;
		vec3_t wishvel, wishdir;

		// Defrag Movement
		if (dvars::pm_movementType->current.integer == Game::PM_MTYPE::DEFRAG) 
		{
			//Game::PM_Friction(pm->ps, pml);
			PM_Q3_Friction(pm, pml);
		}

		fmove = pm->cmd.forwardmove;
		smove = pm->cmd.rightmove;

		// Defrag Movement
		if (dvars::pm_movementType->current.integer == Game::PM_MTYPE::DEFRAG)
		{
			scale = PM_Q3_CmdScale(ps, &pm->cmd);
			PM_Q3_SetMovementDir(pm);
		}

		pml->forward[2] = 0.0f;
		pml->right[2] = 0.0f;

		_VectorNormalize(pml->forward);
		_VectorNormalize(pml->right);

		// Determine x and y parts of velocity
		for (auto i = 0; i < 2; i++) 
		{
			wishvel[i] = pml->forward[i] * fmove + pml->right[i] * smove;
		}

		wishvel[2] = 0;					// Zero out z part of velocity
		VectorCopy(wishvel, wishdir);

		wishspeed = _VectorNormalize(wishdir);

		/* --------------------- mType Defrag ------------------------------*/

		if (dvars::pm_movementType->current.integer == Game::PM_MTYPE::DEFRAG)
		{
			wishspeed *= scale;

			float accel;
			const float wishspeed2 = wishspeed;

			const auto& cpm_airstopAccelerate = dvars::pm_cpm_airstopAccelerate->current.value;
			const auto& cpm_strafeAccelerate = dvars::pm_cpm_strafeAccelerate->current.value;

			if (DotProduct(ps->velocity, wishdir) < 0) 
			{
				accel = cpm_airstopAccelerate; // cpm_pm_airstopAccelerate = 2.5
			}
			else 
			{
				accel = 1.0f; // pm_airaccelerate = 1.0
			}

			if (pm->cmd.forwardmove == 0 && pm->cmd.rightmove != 0)
			//if (ps->movementDir == 2 || ps->movementDir == 6)
			{
				if (wishspeed > 30.0f) 
				{
					wishspeed = 30.0f; // cpm_pm_wishspeed = 30;
				}

				accel = cpm_strafeAccelerate; // cpm_pm_strafeaccelerate  = 70
			}

			PM_Q3_Accelerate(ps, pml, wishdir, wishspeed, accel);
			PM_Q3_AirControl(pm, pml, wishdir, wishspeed2);

			if (pml->groundPlane) 
			{
				PM_Q3_ClipVelocity(ps->velocity, pml->groundTrace.normal, ps->velocity, OVERCLIP);
			}

			// Q3 RampSliding and RampJumping
			PM_Q3_StepSlideMove(pm, pml, true);
			
			//Game::PM_SetMovementDir(pm, pml);
			//PM_CopyVelocityToLocal(ps);

			return;
		}

		/* --------------------- mType CS ------------------------------*/

		else
		{
			if (wishspeed != 0 && (wishspeed > 320.0f)) // wishspeed > mv->m_flMaxSpeed // sv_maxspeed = 320; 
			{
				VectorScale(wishvel, 320.0f / wishspeed, wishvel);
				wishspeed = 320.0f;
			}

			PM_CS_AirAccelerateReal(wishdir, wishspeed, ps, pml);

			Game::PM_StepSlideMove(pm, pml, true);
			PM_CS_TryPlayerMove(pm, pml);

			//PM_CopyVelocityToLocal(ps);
		}
	}

	// backup groundtrace
	#if 0
	// ### 1337
	// rewritten pm_groundtrace to match q3cpm :: only active if movementType = defrag :: this enables rampsliding and rampjumping for stairs
	// inverted slopes ceiling, get us stuck or we slide along them?, jumping on a downwards slope doesnt increase our speed on bhop?
	//void PM_Q3_GroundTrace(Game::pmove_t *pm, Game::pml_t *pml)
	//{
	//	// stock GroundTrace if not using defrag movement
	//	if (pm_movementType->current.integer != Game::PM_MTYPE::DEFRAG)
	//	{
	//		Game::PM_GroundTrace_Internal(pm, pml);
	//		return;
	//	}

	//	Game::trace_t		trace;
	//	vec3_t					point;

	//	point[0] = pm->ps->origin[0];
	//	point[1] = pm->ps->origin[1];
	//	point[2] = pm->ps->origin[2] - 0.25f;

	//	//PM_playerTrace(trace_t *results, const float *start, const float *mins, const float *maxs, const float *end, int passEntityNum, int contentMask)
	//	Game::PM_playerTrace(pm, &trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
	//	pml->groundTrace = trace;

	//	// do something corrective if the trace starts in a solid...
	//	if (trace.allsolid) 
	//	{
	//		//bool PM_CorrectAllSolid(Game::pmove_t *pm /*eax*/, Game::pml_t *pml, Game::trace_t *trace); // ASM
	//		if (!Game::PM_CorrectAllSolid(pm, pml, &trace)) {
	//			return;
	//		}
	//	}

	//	// if the trace didn't hit anything, we are in free fall
	//	if (trace.fraction == 1.0) 
	//	{
	//		//void PM_GroundTraceMissed(Game::pmove_t *pm /*eax*/, Game::pml_t *pml); // ASM
	//		Game::PM_GroundTraceMissed(pm, pml);

	//		pml->groundPlane = false;
	//		pml->almostGroundPlane = false;
	//		pml->walking = false;
	//		return;
	//	}

	//	// check if getting thrown off the ground
	//	if (pm->ps->velocity[2] > 0.0f && DotProduct(pm->ps->velocity, trace.normal) > 10) 
	//	{
	//		Game::Com_PrintMessage(0, utils::va("Q3_GroundTrace :: Liftoff! \n"), 0);

	//		// go into jump animation
	//		/*if (pm->cmd.forwardmove >= 0) {
	//			PM_ForceLegsAnim(LEGS_JUMP);
	//			pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
	//		}
	//		else {
	//			PM_ForceLegsAnim(LEGS_JUMPB);
	//			pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
	//		}*/

	//		pm->ps->groundEntityNum = ENTITYNUM_NONE;
	//		pml->groundPlane = false;
	//		pml->almostGroundPlane = false;
	//		pml->walking = false;

	//		return;
	//	}

	//	float jumpVelocity = sqrtf(pm->ps->gravity * (Game::Dvar_FindVar("jump_height")->current.value * 2.0f));

	//	// slopes that are too steep will not be considered onground
	//	if (trace.normal[2] < MIN_WALK_NORMAL || pm->ps->velocity[2] > jumpVelocity)
	//	{
	//		Game::Com_PrintMessage(0, utils::va("Q3_GroundTrace :: To steep! \n"), 0);
	//		
	//		// FIXME: if they can't slide down the slope, let them
	//		// walk (sharp crevices)
	//		pm->ps->groundEntityNum = ENTITYNUM_NONE;
	//		pml->groundPlane = true;
	//		pml->walking = false;

	//		return;
	//	}

	//	pml->groundPlane = true;
	//	pml->walking = true;

	//	if (pm->ps->groundEntityNum == ENTITYNUM_NONE) 
	//	{
	//		// just hit the ground
	//		/*if (pm->debugLevel) {
	//			Com_Printf("%i:Land\n", c_pmove);
	//		}*/

 //			Game::PM_CrashLand(pm, pm->ps, pml);

	//		// don't do landing time if we were just going down a slope
	//		//if (pml->previous_velocity[2] < -200.0f) 
	//		//{
	//		//	// don't allow another jump for a little while
	//		//	pm->ps->pm_flags |= PMF_TIME_LAND;
	//		//	pm->ps->pm_time = 250;
	//		//}
	//	}

	//	switch (trace.hitType)
	//	{
	//		case 2:
	//		case 3:
	//				pm->ps->groundEntityNum = 1022;
	//		case 1:
	//				pm->ps->groundEntityNum = trace.hitId;
	//				break;
	//		default:
	//				pm->ps->groundEntityNum = 1023;
	//				break;
	//	}

	//	// don't reset the z velocity for slopes
	//	/*if (pm_q1rampslip && (!pm_rampjump
	//		|| (pm_rampjump == 2 && pml.previous_velocity[2] < 0)))
	//		pm->ps->velocity[2] = 0;*/

	//	Game::PM_AddTouchEnt(pm, pm->ps->groundEntityNum);
	//}
	#endif

	// rewritten PM_GroundTrace to match q3cpm :: only active if movementType = defrag :: this enables rampsliding and rampjumping for stairs
	void PM_Q3_GroundTrace(Game::pmove_t *pm, Game::pml_t *pml)
	{
		// stock GroundTrace if not using defrag movement
		if (dvars::pm_movementType->current.integer != Game::PM_MTYPE::DEFRAG)
		{
			Game::PM_GroundTrace_Internal(pm, pml);
			return;
		}

		vec3_t point;
		Game::trace_t trace = {};

		point[0] = pm->ps->origin[0];
		point[1] = pm->ps->origin[1];
		point[2] = pm->ps->origin[2] - 0.25f;

		//pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
		Game::PM_playerTrace(pm, &trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
		pml->groundTrace = trace;

		// do something corrective if the trace starts in a solid...
		if ((trace.allsolid) && (!Game::PM_CorrectAllSolid(pm, pml, &trace))) 
		{
			return;
		}

		// if the trace didn't hit anything, we are in free fall
		if (trace.fraction == 1.0f)
		{
			Game::PM_GroundTraceMissed(pm, pml);
			pml->groundPlane = false;
			pml->walking = false;

			return;
		}

		// check if getting thrown off the ground
		if (pm->ps->velocity[2] > 0.0f && DotProduct(pm->ps->velocity, trace.normal) > 10.0f)
		{
			//if (pm->debugLevel)
			//	Com_Printf("%i:kickoff\n", c_pmove);

			// go into jump animation
			if (pm->cmd.forwardmove >= 0)
			{
				//PM_ForceLegsAnim(LEGS_JUMP);
				pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
			}
			else
			{
				//PM_ForceLegsAnim(LEGS_JUMPB);
				pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
			}

			pm->ps->groundEntityNum = ENTITYNUM_NONE;
			pml->groundPlane = false;
			pml->walking = false;

			return;
		}

		// slopes that are too steep will not be considered onground
		if (trace.normal[2] < 0.7f) 
		{
			//if (pm->debugLevel)
			//	Com_Printf("%i:steep\n", c_pmove);

			// FIXME: if they can't slide down the slope, let them
			// walk (sharp crevices)
			pm->ps->groundEntityNum = ENTITYNUM_NONE;
			pml->groundPlane = true;
			pml->walking = false;

			return;
		}

		pml->groundPlane = true;
		pml->walking = true;

		if (pm->ps->groundEntityNum == ENTITYNUM_NONE)
		{
			// just hit the ground
			//if (pm->debugLevel)
			//	Com_Printf("%i:Land\n", c_pmove);

			Game::PM_CrashLand(pm, pm->ps, pml);

			// don't do landing time if we were just going down a slope
			if (pml->previous_velocity[2] < -200)
			{
				// don't allow another jump for a little while
				//pm->ps->pm_flags |= PMF_TIME_LAND;
				//pm->ps->pm_time = 250;
			}
		}

		switch (trace.hitType)
		{
			case Game::TRACE_HITTYPE_ENTITY:
				pm->ps->groundEntityNum = trace.hitId;
				break;

			case Game::TRACE_HITTYPE_DYNENT_MODEL:
			case Game::TRACE_HITTYPE_DYNENT_BRUSH:
				pm->ps->groundEntityNum = 1022;
				break;

			case Game::TRACE_HITTYPE_NONE:
			default:
				pm->ps->groundEntityNum = 1023;
		}

		//pm->ps->groundEntityNum = trace.entityNum;
		//PM_AddTouchEnt(trace.entityNum);
	}
	
	// defines for PmoveSingle_Mid
	int GSCR_NOTIFY_CHECKJUMP_FRAMES = 0;

	// mid-hook @ start of pmovesingle to set / reset variables
	// Not calling PM_AdjustAimSpreadScale for now as it fucks up pml when moving
	void PmoveSingle_mid_hk(Game::pmove_t *pm, Game::pml_t *pml)
	{
		const int number_of_frames_to_wait = 1;

		if (pml->frametime == 0.0f) 
		{
			return;
		}

		Game::Globals::lpmove_pml_frame_time = pml->msec;

		const int frame_rate = (1000 / pml->msec) * number_of_frames_to_wait;

		if (GSCR_NOTIFY_CHECKJUMP_FRAMES < frame_rate) 
		{
			GSCR_NOTIFY_CHECKJUMP_FRAMES++;
		}
		else
		{
			GSCR_NOTIFY_CHECKJUMP_FRAMES = 0;
			Game::Globals::lpmove_check_jump = false;
		}

		// debug print
		//Game::Com_PrintMessage(0, utils::va("%s :: ^3%d\n", Game::Globals::checkJump ? "true" : "false", GSCR_NOTIFY_CHECKJUMP_FRAMES), 0);
	}

	// Rewritten Jump_PushOffLadder for CheckJump
	void Jump_PushOffLadder(Game::playerState_s *ps, Game::pml_t *pml)
	{
		vec3_t flat_forward;
		vec3_t push_off_dir;

		const auto&	jump_ladderPushVel = Game::Dvar_FindVar("jump_ladderPushVel");

		flat_forward[0] = pml->forward[0];
		flat_forward[1] = pml->forward[1];
		flat_forward[2] = 0.0;

		_VectorNormalize(flat_forward);
		float dot = DotProduct(ps->vLadderVec, pml->forward);
		
		if (dot >= 0.0f) 
		{
			VectorCopy(flat_forward, push_off_dir);
		}
		else
		{
			dot = DotProduct(ps->vLadderVec, flat_forward);
			VectorMA(flat_forward, -2.0f * dot, ps->vLadderVec, push_off_dir);
			_VectorNormalize(push_off_dir);
		}

		ps->velocity[0] = jump_ladderPushVel->current.value * push_off_dir[0];
		ps->velocity[1] = jump_ladderPushVel->current.value * push_off_dir[1];
		ps->velocity[2] = ps->velocity[2] * 0.75f;

		ps->pm_flags &= 0xFFFFFFF7;
	}

	// rewrote Jump_Check :: todo :: make it behave like stock if no custom movementtype is set
	bool PM_Jump_Check(Game::pmove_t *pm, Game::pml_t *pml)
	{
		Game::Globals::lpmove_check_jump = false;

		if (pm->ps->pm_flags & 0x80000) 
		{
			return false; // PMF_RESPAWNED? // Stock
		}

		// If Stock movement
		if (dvars::pm_movementType->current.integer == Game::PM_MTYPE::STOCK)
		{
			if (pm->cmd.serverTime - pm->ps->jumpTime < 500) 
			{
				return false; // Stock
			}

		}

		// must wait for jump to be released
		if (pm->ps->pm_flags & PMF_JUMP_HELD) 
		{
			return false; // Stock
		}
			
		if (pm->ps->pm_flags & 4) 
		{
			return false; // Stock
		}

		if (pm->ps->pm_type >= 7) 
		{
			return false; // Stock
		}

		if(pm->ps->viewHeightTarget == 11 || pm->ps->viewHeightTarget == 40)
		{
			return false; // Stock
		}

		if (!(pm->cmd.buttons & PMF_JUMP_HELD)) 
		{
			return false; // Stock
		}

		if (!dvars::pm_bhop_auto->current.enabled) // auto bhop
		{
			if (pm->oldcmd.buttons & PMF_JUMP_HELD) // Stock
			{
				pm->cmd.buttons &= 0xFFFFFBFF;
				return false;
			}
		}

		// Jump_Start
		const auto& jump_height = Game::Dvar_FindVar("jump_height")->current.value;
		float jump_velocity = sqrtf(static_cast<float>(pm->ps->gravity) * (jump_height + jump_height));

		// if stock movement
		if (dvars::pm_movementType->current.integer == Game::PM_MTYPE::STOCK)
		{
			if (pm->ps->pm_flags & 0x4000 && pm->ps->pm_time <= 1800)
			{
				const auto&	jump_slowdownEnable = Game::Dvar_FindVar("jump_slowdownEnable");
				float reduce_friction = 1.0f;

				if (jump_slowdownEnable->current.enabled)
				{
					reduce_friction = static_cast<float>(pm->ps->pm_time) * 1.5f * 0.0005882352706976235f + 1.0f;

					if (pm->ps->pm_time >= 1700)
					{
						reduce_friction = 2.5f;
					}
				}

				jump_velocity /= reduce_friction;
			}

			pm->ps->jumpOriginZ = pm->ps->origin[2];
			pm->ps->jumpTime	= pm->cmd.serverTime;
			
			pm->ps->sprintState.sprintButtonUpRequired = 0;

			const auto& jump_spreadAdd = Game::Dvar_FindVar("jump_spreadAdd");
			pm->ps->aimSpreadScale += jump_spreadAdd->current.value;

			if (pm->ps->aimSpreadScale > 255.0f) 
			{
				pm->ps->aimSpreadScale = 255.0f;
			}
		}

		pml->groundPlane		= false;
		pml->almostGroundPlane	= false;
		pml->walking			= false;

		pm->ps->pm_flags = pm->ps->pm_flags & 0xFFFFFE7F | 0x4000;
		//pm->ps->pm_time = 0;

		pm->ps->groundEntityNum = ENTITYNUM_NONE;

		pm->ps->velocity[2] = jump_velocity; // Q3 JUMP_VELOCITY = 270; cod4 = 250
		pm->ps->jumpOriginZ = pm->ps->origin[2]; // <- what if we enable this for q3 too?

		// old bug?
		pm->ps->pm_time = CPM_PM_CLIPTIME; //clip through walls // corners <- dvar?

		Game::Globals::lpmove_check_jump = true; // used for GScr_Method :: checkJump()

		// Jump_Start :: END

		//Game::Jump_AddSurfaceEvent(ps, pml);

		if (pm->ps->pm_flags & 8) 
		{
			Jump_PushOffLadder(pm->ps, pml); // do we really need this?
		}

		// jump animations :: make that a function
		/*if (pm->cmd.forwardmove < 0)
		{
			if (pm->ps->pm_type < 7)
			{
				_glob = globalScriptData + 100208;
				goto BG_AnimScriptEvent;
			}
		}

		else if (pm->ps->pm_type < 7)
		{
			_glob = globalScriptData + 99692;

		BG_AnimScriptEvent:
			if (*_glob)
			{
				animIndex = BG_FirstValidItem(_glob, pm->ps->clientNum);
				_animIndex = animIndex;
				if (animIndex)
				{
					if (*(animIndex + 124))
					{
						_rand = rand();
						BG_ExecuteCommand(1, pm->ps, (_animIndex + 16 * (_rand % *(_animIndex + 124) + 8)), 0, 1);
					}
				}
			}
		}*/

		return true;
	}

	#pragma endregion
	#pragma region MOVEMENT-ASM

	// CM_IsEdgeWalkable -> return 0 to always bounce on terrain edges
	__declspec(naked) void CM_IsEdgeWalkable_stub()
	{
		__asm
		{
			sbb     edx, edx;	// og
			neg     edx;		// og

			push	eax;
			mov		eax, dvars::pm_terrainEdgeBounces;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			je		edge_bounces;

			// stock
			mov     al, dl;
			retn;

			// terrain edge bounces enabled
		edge_bounces:
			mov		al, 0;
			retn;
		}
	}

	// old way of enabling auto bunnyhop :: cs
	__declspec(naked) void pm_auto_bunnyhop_stub()
	{
		__asm
		{
			push	eax;
			mov		eax, dvars::pm_bhop_auto;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;
			je		auto_hop;

			// auto hop not enabled
			test    eax, 0x400; // overwritten op
			push	0x407DDA;   // jump back to the next op
			retn 

			// auto hop enabled
		auto_hop:
			test    eax, 0x400; // overwritten op
			jz		stock1;
			
			push	0x407DF3;
			retn;

		stock1:
			push 0x407DED;
			retn;
		}
	}

	// disable jump penality in crashland ~ only active if pm_bhop_slowdown = true
	__declspec(naked) void pm_auto_bunnyhop_penality_stub()
	{
		__asm
		{
			push	eax;
			mov		eax, dvars::pm_bhop_slowdown;
			cmp		byte ptr[eax + 12], 0;
			pop		eax;
			je		auto_hop_slowdown;

			// bhop slowdown enabled
			test    edi, edi;				// overwritten op
			fld     dword ptr[esi + 28h];	// overwritten op
			push	0x410318;				// jump back to the next intact op
			retn;

			// auto bhop enabled
		auto_hop_slowdown:
			test    edi, edi;				// overwritten op
											// we dont care about the next 30 bytes (utils::hook::nop(0x410315, 30))
			push	0x410333;				// overjump
			retn;
		}
	}

	// disable call to pm_crashland ~ only active if pm_crashland = true
	__declspec(naked) void pm_crashland_stub()
	{
		const static uint32_t PM_CrashLand_func = 0x40FFB0;
		__asm
		{
			jne		loc_4108ED;		// overwritten op

			push	eax;
			mov		eax, dvars::pm_crashland;
			cmp		byte ptr[eax + 12], 0;
			pop		eax;
			je		disable_crashland;

			// crashland enabled
			push    edx;
			mov     esi, ebp;
			call	PM_CrashLand_func;
			push	0x4108EA;
			retn;

		disable_crashland:
			jne		loc_4108ED;
			push	0x4108ED;		// overjump call to pm_crashland
			retn;

		loc_4108ED:
			push	0x4108ED;
			retn;
		}
	}

	// hook Weapon_RocketLauncher_Fire to implement rocketJumpHeight dvar
	__declspec(naked) void xo_rocketjump_stub()
	{
		const static uint32_t retn_addr = 0x4E9F34;
		__asm
		{
			mov		eax, dvars::pm_rocketJumpHeight;
			fld		[eax + 12];

			jmp		retn_addr;
		}
	}

	// rewrote G_DamageClient ~ only active if pm_cpm_useQuakeDamage = true
	__declspec(naked) void xo_damageclient_stub() 
	{
		__asm
		{
			push	eax;
			mov		eax, dvars::pm_cpm_useQuakeDamage;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;
			jne		stock_damage;

			mov     eax, [esp + 4Ch];	// - timeoffset	// overwritten op
			mov     ecx, [esp + 40h];	// - hitloc		// overwritten op
			mov     edx, [esp + 38h];	// - mod
			push    eax;				// timeOffset
			mov     eax, [esp + 38h];	// - flags
			push    ecx;				// hitloc
			mov     ecx, [esp + 34h];	// - point
			push    edx;				// mod
			mov     edx, [esp + 34h];	// - attacker
			push    eax;				// flags
			mov     eax, [esp + 34h];	// - target
			push    ebp;				// damage
			push    ecx;				// point
			push    esi;				// dir
			push    edx;				// attacker / self
			push    eax;				// inflictor

			push	[esp + 60h];
			push	ebx;

			call	Q3_DamageClient;
			add		esp, 8h;

			push	0x4B55AA;
			retn;

		stock_damage:
			mov     eax, [esp + 4Ch];	// overwritten op
			mov     ecx, [esp + 40h];	// overwritten op
			
			push	0x4B5582;
			retn;
		}
	}

	// does not really fix rocket delay
	__declspec(naked) void pm_fix_rocketdelay_stub() 
	{
		static int memed_server_time = 0; // (c) rekti
		const static uint32_t retn_addr = 0x4C7F6A;
		__asm
		{
			// get servertime
			mov		ecx, dword ptr[0x13EB894];

			// copy real server time into a local clone
			mov		ecx, [ecx];
			mov		memed_server_time, ecx;
			
			// sub serverTime by 10
			sub		memed_server_time, 10;

			// we need the value of memedServerTime in eax
			mov		ecx, memed_server_time;
			mov		[ebp + 10h], ecx;

			jmp		retn_addr;
		}
	}

	// rewrite G_CalcMuzzlePoints in FireWeapon	~ only active if pm_cpm_useQuakeDamage = true
	__declspec(naked) void g_calcmuzzlepoints_stub()
	{
		const static uint32_t G_CalcMuzzlePoints_func = 0x4E9FA0;
		const static uint32_t retn_addr = 0x4EA05C;
		__asm
		{
			push	edx;
			mov		edx, dvars::pm_cpm_useQuakeDamage;
			cmp		byte ptr[edx + 12], 1;
			pop		edx;
			jne		STOCK;

								// ent is pushed already @ ebx
			push	eax;		// push wp
			Call	Q3_CalcMuzzlePoints;
			add		esp, 4h;
			jmp		retn_addr;

		STOCK:
			call	G_CalcMuzzlePoints_func;
			jmp		retn_addr;
		}
	}

	// rewrote Weapon_RocketLauncher_Fire
	__declspec(naked) void weapon_rocketLauncher_fire_stub()
	{
		const static uint32_t retn_addr = 0x4EA176;
		__asm
		{
			push	edi;
			call	Weapon_RocketLauncher_Fire;
			add     esp, 4h;
			jmp		retn_addr;
		}
	}

	// push center of expl. higher in G_RadiusDamage
	//__declspec(naked) void radiusdamage_mid_stub()
	//{
	//	static float centerOfMass = 40.0f;
	//	__asm
	//	{
	//		fadd	centerOfMass

	//		push	0x4B6A08;
	//		retn
	//	}
	//}

	// implementing a func call @ start of pmovesingle to set / reset variables
	__declspec(naked) void pmove_mid_variables_stub() 
	{
		const static uint32_t PM_AdjustAimSpreadScale_func = 0x418870;
		const static uint32_t retn_addr = 0x41471D;
		__asm
		{
			pushad;
			push	edx;
			push	ebx;

			call	PmoveSingle_mid_hk;
			add		esp, 8h;
			popad;

			call	PM_AdjustAimSpreadScale_func;
			jmp		retn_addr;
		}
	}

	// rewritten G_RadiusDamage - Func Call from MissleTrace ~ only active if pm_cpm_useQuakeDamage = true
	__declspec(naked) void radiusdamage1_stub()
	{
		const static uint32_t G_RadiusDamage_func = 0x4B67C0;
		const static uint32_t retn_addr = 0x4C483F;
		__asm
		{
			push	eax;
			mov		eax, dvars::pm_cpm_useQuakeDamage;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;
			jne		STOCK;		// use Stock RadiusDamage if useQuakeDamage = false
								// everything else is pushed already
			push	edi;		// origin

			call	Q3_RadiusDamage;
			add		esp, 4h;
			jmp		retn_addr;

		STOCK:
			call	G_RadiusDamage_func;
			jmp		retn_addr;
		}
	}
	
	// rewritten G_RadiusDamage - Func Call from G_MissleExplode ~ only active if pm_cpm_useQuakeDamage = true
	__declspec(naked) void radiusdamage2_stub()
	{
		const static uint32_t G_RadiusDamage_func = 0x4B67C0;
		const static uint32_t retn_addr = 0x4C4F33;
		__asm
		{
			push	eax;
			mov		eax, dvars::pm_cpm_useQuakeDamage;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;
			jne		STOCK;		// use Stock RadiusDamage if useQuakeDamage = false
								// everything else is pushed already
			push	edi;		// origin

			call	Q3_RadiusDamage;
			add		esp, 4h;
			jmp		retn_addr;

		STOCK:
			call	G_RadiusDamage_func;
			jmp		retn_addr;
		}
	}

	// ability to switch weapons when fire anim is still playing :: can be abused :: not active
	//__declspec(naked) void weapon_checkforchange_stub() 
	//{
	//	const static uint32_t PM_AdjustAimSpreadScale_Func = 0x418870;
	//	__asm
	//	{
	//		push	eax
	//		mov		eax, pm_movementType
	//		cmp		byte ptr[eax + 12], 1
	//		pop		eax
	//		je		disableCheckOrTrue

	//		// stock :: overwritter op's
	//		cmp     dword ptr[esi + 3Ch], 0
	//		jz		disableCheckOrTrue

	//		// stock :: go to next cmp
	//		push	0x418FAE
	//		retn

	//	disableCheckOrTrue :
	//		push	0x418FDF
	//		retn // <--------------------- this can be abused to fire without cooldown ...
	//	}
	//}

#pragma endregion

	movement::movement()
	{
		// -----
		// dvars

		static std::vector <const char*> pm_movementTypeEnum =
		{ 
			"STOCK", 
			"DEFRAG", 
			"CS", 
		};

		dvars::pm_movementType = Game::Dvar_RegisterEnum(
			/* name		*/ "pm_movementType",
			/* desc		*/ "the movementtype to use",
			/* default	*/ 0,
			/* enumSize	*/ pm_movementTypeEnum.size(),
			/* enumData */ pm_movementTypeEnum.data(),
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_cpm_airstopAccelerate = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cpm_airstopAccelerate",
			/* desc		*/ "pm_movementType Defrag :: defines negative player acceleration when the player wants to stop mid-air",
			/* default	*/ 3.0f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 50.0f,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_cpm_strafeAccelerate = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cpm_strafeAccelerate",
			/* desc		*/ "pm_movementType Defrag :: defines player acceleration on initial strafejump",
			/* default	*/ 70.0f,
			/* minVal	*/ 1.0f,
			/* maxVal	*/ 500.0f,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_cpm_airControl = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cpm_airControl",
			/* desc		*/ "pm_movementType Defrag :: defines direction control mid-air",
			/* default	*/ 150.0f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 500.0f,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_cpm_airAccelerate = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cpm_airAccelerate",
			/* desc		*/ "pm_movementType Defrag :: defines player acceleration mid-air",
			/* default	*/ 1.0f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_cpm_useQuakeDamage = Game::Dvar_RegisterBool(
			/* name		*/ "pm_cpm_useQuakeDamage",
			/* desc		*/ "pm_movementType All :: enables quake 3 damange knockback",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_cpm_damageKnockback = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cpm_damageKnockback",
			/* desc		*/ "pm_movementType Defrag :: damage knockback scalar",
			/* default	*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_cpm_useBouncing = Game::Dvar_RegisterBool(
			/* name		*/ "pm_cpm_useBouncing",
			/* desc		*/ "pm_movementType Defrag :: enables experimental bouncing",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_cs_airAccelerate = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cs_airAccelerate",
			/* desc		*/ "pm_movementType CS :: defines player acceleration mid-air",
			/* default	*/ 100.0f,
			/* minVal	*/ 1.0f,
			/* maxVal	*/ 500.0f,
			/* flags	*/ Game::dvar_flags::none);
		
		dvars::pm_cs_airspeedCap = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cs_airspeedCap",
			/* desc		*/ "pm_movementType CS :: maximum speed mid-air",
			/* default	*/ 30.0f,
			/* minVal	*/ 1.0f,
			/* maxVal	*/ 500.0f,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_bhop_auto = Game::Dvar_RegisterBool(
			/* name		*/ "pm_bhop_auto",
			/* desc		*/ "constantly jump when holding space, still has landing slowdown -> disable pm_bhop_slowdown",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_bhop_slowdown = Game::Dvar_RegisterBool(
			/* name		*/ "pm_bhop_slowdown",
			/* desc		*/ "toggle landing slowdown",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::none);

#if 0
		dvars::pm_bhop_rampjump = Game::Dvar_RegisterBool_w(
			/* name		*/ "pm_bhop_rampjump",
			/* desc		*/ "toggle rampjumping",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);
#endif

		dvars::pm_terrainEdgeBounces = Game::Dvar_RegisterBool(
			/* name		*/ "pm_terrainEdgeBounces",
			/* desc		*/ "ability to bounce on terrain edges",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_crashland = Game::Dvar_RegisterBool(
			/* name		*/ "pm_crashland",
			/* desc		*/ "disable this to completely skip any fall related effects",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_disableSprint = Game::Dvar_RegisterBool(
			/* name		*/ "pm_disableSprint",
			/* desc		*/ "disables the ability to sprint",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_rocketJumpHeight = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_rocketJumpHeight",
			/* desc		*/ "pushback amount for rocketjumps using stock movement",
			/* default	*/ 64.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 4096.0f,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_hud_enable = Game::Dvar_RegisterBool(
			/* name		*/ "pm_hud_enable",
			/* desc		*/ "Display current speed and current movement type.",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_hud_x = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_hud_x",
			/* desc		*/ "text offset x",
			/* default	*/ 10.0f,
			/* minVal	*/ -8192.0f,
			/* maxVal	*/ 8192.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_hud_y = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_hud_y",
			/* desc		*/ "text offset y",
			/* default	*/ 300.0f,
			/* minVal	*/ -8192.0f,
			/* maxVal	*/ 8192.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_hud_fontScale = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_hud_fontScale",
			/* desc		*/ "font scale",
			/* default	*/ 0.75f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_hud_fontStyle = Game::Dvar_RegisterInt(
			/* name		*/ "pm_hud_fontStyle",
			/* desc		*/ "font Style",
			/* default	*/ 1,
			/* minVal	*/ 0,
			/* maxVal	*/ 8,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_hud_fontColor = Game::Dvar_RegisterVec4(
			/* name		*/ "pm_hud_fontColor",
			/* desc		*/ "font color",
			/* x		*/ 1.0f,
			/* y		*/ 0.55f,
			/* z		*/ 0.4f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_debug_prints = Game::Dvar_RegisterBool(
			/* name		*/ "pm_debug_prints",
			/* desc		*/ "enable debug prints for movement related functions",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		// --------
		// Commands

		// switch to Cmd_ExecuteSingleCommand() ?

		command::add("pm_preset_stock", [](command::params)
		{
			dvars::pm_movementType->current.integer = Game::PM_MTYPE::STOCK;

			dvars::pm_bhop_auto->current.enabled	 = false;
			dvars::pm_bhop_slowdown->current.enabled = true;
			dvars::pm_disableSprint->current.enabled = false;

			// resets ::
			dvars::pm_cpm_useQuakeDamage->current.enabled = false;
			dvars::pm_rocketJumpHeight->current.value	  = 64.0f;

			Game::Dvar_FindVar("bg_falldamageminheight")->current.value = 128.0f;
			Game::Dvar_FindVar("bg_falldamagemaxheight")->current.value = 300.0f;

			Game::Dvar_FindVar("jump_height")->current.value			= 39.0f;
			Game::Dvar_FindVar("bg_bobMax")->current.value				= 8.0f;
			Game::Dvar_FindVar("friction")->current.value				= 5.5f;
			Game::Dvar_FindVar("g_speed")->current.integer				= 190;
			Game::Dvar_FindVar("jump_slowdownenable")->current.enabled	= true;
		});

		command::add("pm_preset_q3", [](command::params)
		{
			dvars::pm_movementType->current.integer	= Game::PM_MTYPE::DEFRAG;

			dvars::pm_bhop_auto->current.enabled	 = true;
			dvars::pm_bhop_slowdown->current.enabled = false;
			dvars::pm_disableSprint->current.enabled = true;

			dvars::pm_cpm_useQuakeDamage->current.enabled = true;
			dvars::pm_rocketJumpHeight->current.value	  = 0.0;
			
			Game::Dvar_FindVar("bg_falldamageminheight")->current.value = 99998.0f;
			Game::Dvar_FindVar("bg_falldamagemaxheight")->current.value = 99999.0f;
			
			Game::Dvar_FindVar("jump_height")->current.value			= 46.0f;
			Game::Dvar_FindVar("bg_bobMax")->current.value				= 0.0f;
			Game::Dvar_FindVar("friction")->current.value				= 8.0f;
			Game::Dvar_FindVar("g_speed")->current.integer				= 320;
			Game::Dvar_FindVar("jump_slowdownenable")->current.enabled	= false;
			
			
		});

		command::add("pm_preset_cs", [](command::params)
		{
			dvars::pm_movementType->current.integer	= Game::PM_MTYPE::CS;
			
			dvars::pm_bhop_auto->current.enabled	 = true;
			dvars::pm_bhop_slowdown->current.enabled = false;
			dvars::pm_disableSprint->current.enabled = true;

			// resets ::
			dvars::pm_cpm_useQuakeDamage->current.enabled = false;
			dvars::pm_rocketJumpHeight->current.value	  = 64.0f;

			Game::Dvar_FindVar("bg_falldamageminheight")->current.value = 99998.0f;
			Game::Dvar_FindVar("bg_falldamagemaxheight")->current.value = 99999.0f;

			Game::Dvar_FindVar("jump_height")->current.value			= 39.0f;
			Game::Dvar_FindVar("bg_bobMax")->current.value				= 8.0f;
			Game::Dvar_FindVar("friction")->current.value				= 5.5f;
			Game::Dvar_FindVar("g_speed")->current.integer				= 190;
			Game::Dvar_FindVar("jump_slowdownenable")->current.enabled	= false;
		});

		// -----
		// Hooks

		// PM_AdjustAimSpreadScale Call in PmoveSingle								- Used to set / reset variables ( eg. GScr_JumpCheck )
		utils::hook(0x414718, pmove_mid_variables_stub, HOOK_JUMP).install()->quick();

		// PM_AirMove Call in PmoveSingle ( Entrypoint Stock/Defrag/Source )			- Stock movement uses Stock functions
		utils::hook(0x414B87, PM_MT_AirMove, HOOK_CALL).install()->quick();
		
		// PM_AirMove Call in PM_WalkMove												- Stock movement uses Stock functions
		utils::hook(0x40F7F9, PM_MT_AirMove, HOOK_CALL).install()->quick();

		// PM_WalkMove Call in PmoveSingle ( Entrypoint Stock/Defrag/Source )			- Stock movement uses Stock functions
		utils::hook(0x414B7A, PM_MT_WalkMove, HOOK_CALL).install()->quick();

		// enable / disable crashland call
		utils::hook(0x4108E0, pm_crashland_stub, HOOK_JUMP).install()->quick();

		// hook G_DamageClient to add Q3 radius damage knockback
		utils::hook::nop(0x4B557A, 8);
		utils::hook(0x4B557A, xo_damageclient_stub, HOOK_JUMP).install()->quick();

		// enable / disable auto bunnyhop on space
		utils::hook(0x407DD5, pm_auto_bunnyhop_stub, HOOK_JUMP).install()->quick();	// - still needed for cs

		// enable / disable jump landing punishment within PM_CrashLand
		utils::hook(0x410313, pm_auto_bunnyhop_penality_stub, HOOK_JUMP).install()->quick();

		// hook PM_GroundTrace - Top of PmoveSingle ( q3 rampslide )				- using stock func if stock movementtype
		utils::hook(0x414AA9, PM_Q3_GroundTrace, HOOK_CALL).install()->quick();
		
		// hook PM_GroundTrace - Bottom of PmoveSingle ( q3 rampslide )				- using stock func if stock movementtype
		utils::hook(0x414B95, PM_Q3_GroundTrace, HOOK_CALL).install()->quick();

		// hook CM_IsEdgeWalkable to implement terrain edge bounces
		utils::hook::nop(0x4EFCD1, 6);
		utils::hook(0x4EFCD1, CM_IsEdgeWalkable_stub, HOOK_JUMP).install()->quick();

		// implement rocket pre-fire to hopefully fix rocketdelay <- like in Q3
		utils::hook(0x4C7F64, pm_fix_rocketdelay_stub, HOOK_JUMP).install()->quick();
		utils::hook::nop(0x4C7F69, 1);

		// Q3_CalcMuzzlePoints
		utils::hook(0x4EA057, g_calcmuzzlepoints_stub, HOOK_JUMP).install()->quick();

		// Rewrote weapon_rocketLauncher_fire
		utils::hook(0x4EA171, weapon_rocketLauncher_fire_stub, HOOK_JUMP).install()->quick();

		// we get stuck when shooting rockets before jumping?
		// hook G_RadiusDamage to change expl. origin from +24 to +40 @0x4B6A02
		//utils::hook(0x4B6A02, radiusdamage_mid_stub, HOOK_JUMP).install()->quick();
		//utils::hook::nop(0x4B6A07, 1);

		// hook PM_Weapon_CheckForChangeWeapon to disable Timer that stops weapchange when fireAnim is still playing
		//utils::hook(0x418FA8, weapon_checkforchange_stub, HOOK_JUMP).install()->quick();
		//utils::hook::nop(0x418FAD, 1);

		// RadiusDamage in MissleImpact
		utils::hook(0x4C483A, radiusdamage1_stub, HOOK_JUMP).install()->quick();

		// RadiusDamage in G_ExplodeMissle
		utils::hook(0x4C4F2E, radiusdamage2_stub, HOOK_JUMP).install()->quick();
	}
}
