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

using namespace Utils::vector;

namespace Components
{
	void PM_MT_AirMove(Game::pmove_t *pm, Game::pml_t *pml);
	bool PM_Jump_Check(Game::pmove_t *pm, Game::pml_t *pml);
	void PM_DisableSprint(Game::playerState_s *ps);
	
	#pragma region BG-SLIDEMOVE

	// stock cod4 clipvelocity
	void PM_ClipVelocity(const float *in, const float *normal, float *out)
	{
		float outA, outB, outC;

		outA = *in * *normal + in[1] * normal[1] + in[2] * normal[2];
		outB = (float)(outA - (double)outA * 0.001000000047497451);
		outC = -outB;

		*out = *normal * outC + *in;
		out[1] = outC * normal[1] + in[1];
		out[2] = outC * normal[2] + in[2];
	}

	// Defrag :: clipvelocity
	void PM_Q3_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce)
	{
		float backoff;
		float change;
		int   i;

		backoff = DotProduct(in, normal);

		if (backoff < 0.0f)
		{
			backoff *= overbounce;
		}
		else
		{
			backoff /= overbounce;
		}

		for (i = 0; i < 3; i++)
		{
			change = normal[i] * backoff;
			out[i] = in[i] - change;
		}
	}

	// Defrag :: Quake 3 slidemove
	bool PM_Q3_SlideMove(Game::pmove_t *pm, Game::pml_t *pml, bool gravity)
	{
		int			bumpcount, numbumps, numplanes, i, j, k;
		vec3_t		planes[MAX_CLIP_PLANES];
		vec3_t		primal_velocity, clipVelocity, endVelocity, endClipVelocity;
		vec3_t		dir, end, endpos;
		float		time_left, into, d;

		Game::trace_t		trace;
		numbumps = 4;

		VectorCopy(pm->ps->velocity, primal_velocity);

		if (gravity)
		{
			VectorCopy(pm->ps->velocity, endVelocity);
			endVelocity[2] -= pm->ps->gravity * pml->frametime;
			
			pm->ps->velocity[2] = (pm->ps->velocity[2] + endVelocity[2]) * 0.5f;
			primal_velocity[2] = endVelocity[2];

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

		for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
		{
			// calculate position we are trying to move to
			VectorMA(pm->ps->origin, time_left, pm->ps->velocity, end);

			// see if we can make it there
			Game::PM_playerTrace(pm, &trace, pm->ps->origin, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask);

			// entity is completely trapped in another solid
			if (trace.allsolid)
			{
				pm->ps->velocity[2] = 0.0f;  // don't build up falling damage, but allow sideways acceleration
				
				if (Dvars::pm_debug_prints->current.enabled) 
				{
					Game::Com_PrintMessage(0, Utils::VA("PM_Q3_SlideMove :: ALL SOLID! \n"), 0);
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
				_Vec3Lerp(pm->ps->origin, end, trace.fraction, endpos);
				VectorCopy(endpos, pm->ps->origin); // could move "endpos" into origin directly above
			}

			// moved the entire distance
			if (trace.fraction == 1) 
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
				
				if (Dvars::pm_debug_prints->current.enabled) 
				{
					Game::Com_PrintMessage(0, Utils::VA("PM_Q3_SlideMove :: MAX CLIP PLANES! \n"), 0);
				}

				return true;
			}

			// if this is the same plane we hit before, nudge velocity out along it, 
			// which fixes some epsilon issues with non-axial planes
			for (i = 0; i < numplanes; i++)
			{
				if (DotProduct(trace.normal, planes[i]) > 0.99f)
				{
					if (Dvars::pm_debug_prints->current.enabled) 
					{
						Game::Com_PrintMessage(0, Utils::VA("PM_Q3_SlideMove :: same plane, nudge velocity! \n"), 0);
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
				PM_Q3_ClipVelocity(pm->ps->velocity, planes[i], clipVelocity, OVERCLIP);

				// slide along the plane
				PM_Q3_ClipVelocity(endVelocity, planes[i], endClipVelocity, OVERCLIP);

				// see if there is a second plane that the new move enters
				for (j = 0; j < numplanes; j++)
				{
					if (j == i) 
					{
						continue;
					}

					if (DotProduct(clipVelocity, planes[j]) >= 0.1f) 
					{
						continue;   // move doesn't interact with the plane
					}

					// try clipping the move to the plane
					PM_Q3_ClipVelocity(clipVelocity,	planes[j], clipVelocity,	OVERCLIP);
					PM_Q3_ClipVelocity(endClipVelocity, planes[j], endClipVelocity, OVERCLIP);

					// see if it goes back into the first clip plane
					if (DotProduct(clipVelocity, planes[i]) >= 0) 
					{
						continue;
					}

					// slide the original velocity along the crease
					_CrossProduct(planes[i], planes[j], dir);
					_VectorNormalize(dir);

					d = DotProduct(dir, pm->ps->velocity);
					VectorScale(dir, d, clipVelocity);

					_CrossProduct(planes[i], planes[j], dir);
					_VectorNormalize(dir);

					d = DotProduct(dir, endVelocity);
					VectorScale(dir, d, endClipVelocity);

					// see if there is a third plane the the new move enters
					for (k = 0; k < numplanes; k++)
					{
						if (k == i || k == j) 
						{
							continue;
						}

						if (DotProduct(clipVelocity, planes[k]) >= 0.1f) 
						{
							continue;   // move doesn't interact with the plane
						}

						// stop dead at a tripple plane interaction
						VectorClear(pm->ps->velocity);

						if (Dvars::pm_debug_prints->current.enabled) 
						{
							Game::Com_PrintMessage(0, Utils::VA("PM_Q3_SlideMove :: STOP DEAD! \n"), 0);
						}

						return true;
					}
				}

				// if we have fixed all interactions, try another move
				VectorCopy(clipVelocity, pm->ps->velocity);
				VectorCopy(endClipVelocity, endVelocity);
				
				break;
			}
		}

		if (gravity)
		{
			VectorCopy(endVelocity, pm->ps->velocity);
		}

		// don't change velocity if in a timer :: clipping is caused by this
		if (pm->ps->pm_time) 
		{
			VectorCopy(primal_velocity, pm->ps->velocity);
			//Game::Com_PrintMessage(0, Utils::VA("PM_Q3_SlideMove :: CLIP Timer! \n"), 0);
		}	

		return (bumpcount != 0);
	}

	// Stock ProjectVelocity for exp. bouncing
	void PM_ProjectVelocity(float *normal, float *velIn, float *velOut)
	{
		float speedXY, DotNormalXY, normalisedNormalXY, projectionZ, projectionXYZ;

		speedXY = velIn[1] * velIn[1] + velIn[0] * velIn[0];

		if ((normal[2]) < 0.001f || (speedXY == 0.0f)) 
		{
			VectorCopy(velIn, velOut);
		}

		else
		{
			DotNormalXY			= normal[1] * velIn[1] + velIn[0] * normal[0];
			normalisedNormalXY	= -DotNormalXY / normal[2];

			projectionZ			= velIn[2] * velIn[2] + speedXY;
			
			projectionXYZ = sqrtf((projectionZ / (speedXY + normalisedNormalXY * normalisedNormalXY)));
			
			if (projectionXYZ < 1.0f || normalisedNormalXY < 0.0f || velIn[2] > 0.0f)
			{
				velOut[0] = projectionXYZ * velIn[0];
				velOut[1] = projectionXYZ * velIn[1];
				velOut[2] = projectionXYZ * normalisedNormalXY;
			}
		}
	}

	// Defrag :: StepSlideMove
	void PM_Q3_StepSlideMove(Game::pmove_t *pm, Game::pml_t *pml, bool gravity)
	{
		vec3_t		start_o, start_v, endpos;
		vec3_t		down_o, down_v;
		vec3_t		up, down;
		float		stepSize;
		float		STEP_SIZE = Game::Dvar_FindVar("jump_stepSize")->current.value;

		Game::trace_t		trace;

		if (pm->ps->pm_flags & 8)
		{
			pm->ps->jumpOriginZ = 0.0;
			trace.allsolid = 0;
			pm->ps->pm_flags = pm->ps->pm_flags & 0xFFFFBFFF;
		}

		else if (pml->groundPlane) 
		{
			trace.allsolid = 1;
		}

		else
		{
			trace.allsolid = 0;
			if (pm->ps->pm_flags & 0x4000 && pm->ps->pm_time)
			{
				pm->ps->jumpOriginZ = 0.0;
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
		down[2] -= STEP_SIZE;

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
		up[2] += STEP_SIZE;

		// test the player position if they were a stepheight higher
		Game::PM_playerTrace(pm, &trace, start_o, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask);
		_Vec3Lerp(pm->ps->origin, up, trace.fraction, endpos); // cod4 trace doesn't save the endpos so we calculate it the way q3 does it

		if (trace.allsolid) 
		{
			if (Dvars::pm_debug_prints->current.enabled) 
			{
				Game::Com_PrintMessage(0, Utils::VA("PM_Q3_StepSlideMove :: Bend can't step! \n"), 0);
			}

			return;		// can't step up
		}

		stepSize = endpos[2] - start_o[2];

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
			if (Dvars::pm_cpm_useBouncing->current.enabled)
			{
				if (!trace.walkable && trace.normal[2] < 0.30000001) 
				{ 
					if (Dvars::pm_debug_prints->current.enabled) 
					{
						Game::Com_PrintMessage(0, Utils::VA("PM_Q3_StepSlideMove :: ^1Clip ^7Velocity! \n"), 0);
					}
					//PM_Q3_ClipVelocity(pm->ps->velocity, trace.normal, pm->ps->velocity, OVERCLIP);

					VectorCopy(start_o, pm->ps->velocity);
					VectorCopy(start_v, pm->ps->velocity);

					return;
				}

				PM_ProjectVelocity(trace.normal, pm->ps->velocity, pm->ps->velocity);

				if (Dvars::pm_debug_prints->current.enabled) 
				{
					Game::Com_PrintMessage(0, Utils::VA("PM_Q3_StepSlideMove :: Project Velocity! \n"), 0);
				}
			}

			else 
			{
				// if !inAir // <-------------------------------------------------- needed?
				if (!((pm->ps->velocity[2] > 0.0f) && (trace.fraction == 1.0 || DotProduct(trace.normal, up) < 0.7f))) 
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
			Game::Com_PrintMessage(0, Utils::VA("PM_Q3_StepSlideMove :: stepped! \n"), 0);
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
	//			//Game::Com_PrintMessage(0, Utils::VA("PM_Q3_StepSlideMove :: Step down! \n"), 0);
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
	//			Game::Com_PrintMessage(0, Utils::VA("PM_Q3_StepSlideMove :: Bend can't step! \n"), 0);

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
	//			//Game::Com_PrintMessage(0, Utils::VA("PM_Q3_StepSlideMove :: Step up! \n"), 0);
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
	//		//Game::Com_PrintMessage(0, Utils::VA("PM_Q3_StepSlideMove :: STEP ANIM! \n"), 0);



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
	void PM_Q3_SetMovementDir(Game::pmove_t *pm) // curly brackets would make this to large :p
	{
		// dont set Movementdir when proned or crouched
		if (pm->ps->viewHeightCurrent == 11 || pm->ps->viewHeightCurrent == 40)
			return;

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
		int		max;
		float	total;
		float	scale;

		max = abs(cmd->forwardmove);
		
		if (abs(cmd->rightmove) > max)
		{
			max = abs(cmd->rightmove);
		}

		if (!max)
		{
			return 0;
		}

		total = (float)sqrt(cmd->forwardmove * cmd->forwardmove + cmd->rightmove * cmd->rightmove);
		scale = (float)ps->speed * max / (127.0f * total);

		return scale;
	}

	// Defrag :: PM_Aircontrol function when we are mid-air -> https://github.com/xzero450/revolution/blob/8d0b37ba438e65e19d5a5e77f5b9c2076b7900bc/game/bg_promode.c#L303
	void PM_Q3_AirControl(Game::pmove_t *pm, Game::pml_t *pml, vec3_t wishdir_b, float wishspeed_b)
	{
		float	zspeed, speed, dot, k;
		auto	ps = pm->ps;

		//if( pm->cmd.forwardmove == 0.0f || wishspeed_b == 0.0 ) huh ( // if(WishMove.X != 0.f || WishSpeed == 0.f) )
		if ((ps->movementDir && ps->movementDir != 4) || wishspeed_b == 0.0) 
		{
			return;
		}

		auto cpm_airControl = Dvars::pm_cpm_airControl->current.value;
		auto cpm_airAccelerate = Dvars::pm_cpm_airAccelerate->current.value;

		zspeed = ps->velocity[2];
		ps->velocity[2] = 0.0f;

		speed = _VectorNormalize(ps->velocity);
		dot = DotProduct(ps->velocity, wishdir_b);

		k = 32;
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
		float		addspeed, accelspeed, currentspeed;

		currentspeed = DotProduct(ps->velocity, wishdir_b);
		addspeed = wishspeed_b - currentspeed;
		
		if (addspeed <= 0) 
		{
			return;
		}

		accelspeed = accel_b * pml->frametime*wishspeed_b;
		
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
		float	addspeed, accelspeed, currentspeed;

		PM_DisableSprint(ps);

		currentspeed = DotProduct(ps->velocity, wishdir);
		addspeed = wishspeed - currentspeed;
		
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

		accelspeed = accel * pml->frametime * wishspeed;
		
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
		vec3_t	vec;
		float	*vel;
		float	speed, newspeed, control, drop;
		float	friction = Game::Dvar_FindVar("friction")->current.value;

		vel = pm->ps->velocity;
		VectorCopy(vel, vec);

		if (pml->walking) 
		{
			vec[2] = 0;	// ignore slope movement
		}

		speed = VectorLength(vec);

		if (speed < 1.0f)
		{
			vel[0] = 0;
			vel[1] = 0;	// allow sinking underwater

			// FIXME: still have z friction underwater?
			if (pm->ps->pm_type == PM_SPEC) 
			{
				vel[2] = 0.0f; // no slow-sinking/raising movements
			}

			return;
		}

		drop = 0;

		// apply ground friction
		if ((pml->walking) && !(pml->groundTrace.surfaceFlags & SURF_SLICK))
		{
			// if getting knocked back, dont enter > no friction - works
			if (!(pm->ps->pm_flags & PMF_TIME_KNOCKBACK))
			{
				control = speed < 100 ? 100 : speed;

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
			drop += speed * 5 * pml->frametime;
		}

		// scale the velocity
		newspeed = speed - drop;

		if (newspeed < 0) 
		{
			newspeed = 0;
		}

		newspeed /= speed;

		for (int i = 0; i < 3; i++) 
		{
			vel[i] *= newspeed;
		}
	}

	#pragma endregion
	#pragma region MOVEMENT-CS
	
	// CS :: Stock ClipVelocity
	void PM_CS_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce)
	{
		float	backoff, change, angle, adjust;

		angle = normal[2];

		// Determine how far along plane to slide based on incoming direction.
		backoff = DotProduct(in, normal) * overbounce;

		for (auto i = 0; i < 3; i++)
		{
			change = normal[i] * backoff;
			out[i] = in[i] - change;
		}

		// iterate once to make sure we aren't still moving through the plane
		adjust = DotProduct(out, normal);

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
		float	SurfSlope = 0.7f;
		vec3_t	end;

		auto	ps = pm->ps;
		Game::trace_t trace;

		if (VectorLength(ps->velocity) == 0.0) 
		{
			return;
		}

		// Assume we can move all the way from the current origin to the end point.
		VectorMA(ps->origin, pml->frametime, ps->velocity, end);
		Game::PM_playerTrace(pm, &trace, ps->origin, pm->mins, pm->maxs, end, ps->clientNum, pm->tracemask);

		// If we covered the entire distance, we are done and can return.
		if (trace.fraction == 1)
		{
			return;
		}

		// If the plane we hit has a high z component in the normal, then it's probably a floor
		if (trace.normal[2] > SurfSlope) 
		{
			return;
		}

		PM_CS_ClipVelocity(ps->velocity, trace.normal, ps->velocity, 1.0f);
	}

	// CS :: Stock AirAccelerate 
	void PM_CS_AirAccelerateReal(vec3_t wishdir, float wishspeed, Game::playerState_s *ps, Game::pml_t *pml) // we don`t need the default accel value as we use our dvar for that
	{
		float	wishspd = wishspeed, accelspeed, currentspeed, addspeed;

		auto accel			= Dvars::pm_cs_airAccelerate->current.value;
		auto airspeedcap	= Dvars::pm_cs_airspeedCap->current.value;

		if (wishspd > airspeedcap) 
		{
			wishspd = airspeedcap;
		}

		currentspeed = DotProduct(ps->velocity, wishdir);
		addspeed = wishspd - currentspeed;

		if (addspeed > 0)
		{
			accelspeed = pml->frametime * accel * wishspeed * 1.0f; // * surfacefriction?
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
	void Weapon_GunRandom(float *x, float *y)
	{
		float a, b, c;

		a = (float)(rand() * 0.000030517578125);
		b = a * 360.0f;

		a = (float)(rand() * 0.000030517578125);
		b *= 0.01745329238474369;

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
	void Weapon_RocketLauncher_Fire(Game::weaponParms *wp, Game::gentity_s *ent, int weaponIndex, float spread, float *gunVel, Game::gentity_s *target, float *targetOffset)
	{
		vec3_t	dir, _dir, kickBack, _velocity, zero;
		float	x, y, _spread;

		Game::gentity_s *m;

		if (!Dvars::pm_cpm_useQuakeDamage || !Dvars::pm_cpm_useQuakeDamage->current.enabled)
		{
			_spread = tan(spread * 0.017453292f) * 16.0f;
			Weapon_GunRandom(&x, &y);

			x *= _spread;
			y *= _spread;
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

		kickBack[0] = wp->muzzleTrace[0];
		kickBack[1] = wp->muzzleTrace[1];
		kickBack[2] = wp->muzzleTrace[2];

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

		m = Game::G_FireRocket(kickBack, ent, weaponIndex, dir, gunVel, 0, targetOffset);
		//m->damage *= 

		if (!Dvars::pm_cpm_useQuakeDamage || !Dvars::pm_cpm_useQuakeDamage->current.enabled)
		{
			auto velocity = ent->client->ps.velocity;

			_VectorZero(zero);
			VectorSubtract(zero, wp->forward, _velocity);
			
			VectorScale(_velocity, Dvars::pm_rocketJumpHeight->current.value, _velocity); // stock 64.0f
			VectorAdd(_velocity, velocity, velocity);
		}
	}

	// Defrag :: G_RadiusDamage - fixes alot of the knockback issues
	bool Q3_RadiusDamage(vec3_t origin, Game::gentity_s *inflictor, Game::gentity_s *attacker, float fInnerDamage, float fOuterDamage, float radius, float coneAngleCos, float *coneDirection, Game::gentity_s *ignore, int mod, int self_client)
	{
		float		points;
		float		dist;
		int			entityList[1024];
		//int			numListedEntities;
		vec3_t		mins, maxs;
		vec3_t		v;
		vec3_t		dir;
		int			i, e;
		bool		hitClient = false;

		Game::gentity_s	*ent;
		Game::areaParms_t	ap;

		if (radius < 1) 
		{
			radius = 1;
		}

		for (i = 0; i < 3; i++) 
		{
			mins[i] = origin[i] - radius;
			maxs[i] = origin[i] + radius;
		}

		ap.mins = mins;
		ap.maxs = maxs;
		ap.list = entityList;
		ap.count = 0;
		ap.maxcount = 1024;
		ap.contentmask = -1;

		Game::CM_AreaEntities(1, &ap);
		//numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

		for (e = 0; e < ap.count; e++)
		{
			ent = (Game::gentity_s*)&Game::scr_g_entities[entityList[e]]; // fail

			if (ent == ignore) 
			{
				continue;
			}

			if (!ent->takedamage)
			{
				continue;
			}

			// find the distance from the edge of the bounding box
			for (i = 0; i < 3; i++) 
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

			points = fInnerDamage * (1.0f - dist / radius);

			// CanDamageContentMask
			//if ((Game::CanDamage(ent, origin))) 
			//CanDamage(Game::gentity_s *inflictor /*eax*/, float *centerPos /*ecx*/, Game::gentity_s *ent, float coneAngleCos, float *coneDirection, int contentmask)
			if(Game::CanDamage(inflictor, origin, ent, coneAngleCos, coneDirection, *Game::CanDamageContentMask))
			{

				VectorSubtract(ent->r.currentOrigin, origin, dir);
				// push the center of mass higher than the origin so players
				// get knocked into the air more

				if (ent == inflictor) 
				{
					dir[2] += 24;
				}
				else
				{
					dir[2] += 40.0f; //40;
				}
				
				// this basically calls Q3_DamageClient
				Game::G_Damage(dir, ent, inflictor, attacker, origin, (int)points, 5, mod, self_client, 0, 0, 0, 0);
			}
		}

		return hitClient;
	}

	// Defrag :: DamageClient with knockback for rocket and plasma
	void Q3_DamageClient(Game::gentity_s *targ, int weapon, Game::gentity_s *inflictor, Game::gentity_s *attacker, float *dir, float *point, int damage, int dflags, int mod, Game::hitLocation_t hitLoc, int timeOffset)
	{
		int		knockback;
		float	scale = 1.0f;

		//Game::Com_PrintMessage(0, Utils::VA("Q3_DamageClient :: ___dir[]: %.2lf, %.2lf, %.2lf \n", dir[0], dir[1], dir[2]), 0);
		//Game::Com_PrintMessage(0, Utils::VA("Q3_DamageClient :: _point[]: %.2lf, %.2lf, %.2lf \n", point[0], point[1], point[2]), 0);
		//Game::Com_PrintMessage(0, Utils::VA("Q3_DamageClient :: damage_org[]: %d \n", damage), 0);

		if (targ->takedamage && damage > 0 && targ->client->sess.connected == 2
			&& targ->client->ps.pm_type != 7 && !targ->client->noclip && !targ->client->ufo)
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

				if (Game::Globals::Q3_LastProjectileWeaponUsed == Game::Q3WeaponNames::Q3_PLASMA) 
				{
					scale = 1.0f; // hm?
				}

				knockback = damage;
				knockback = (int)(knockback * Dvars::pm_cpm_damageKnockback->current.value * scale);

				// figure momentum add, even if the damage won't be taken
				if (dir && knockback && targ->client)
				{
					vec3_t	kvel;
					float	mass;
					auto	g_knockback = Game::Dvar_FindVar("g_knockback")->current.value;

					mass = 200;

					_VectorScale(dir, g_knockback * (float)knockback / mass, kvel);
					VectorAdd(targ->client->ps.velocity, kvel, targ->client->ps.velocity);

					if (Dvars::pm_debug_prints->current.enabled) 
					{
						Game::Com_PrintMessage(0, Utils::VA("Q3_DamageClient :: exp-knockback: %d\n", knockback), 0);
					}

					// set the timer so that the other client can't cancel
					// out the movement immediately
					if (!targ->client->ps.pm_time)
					{
						int		t;

						t = knockback * 2;
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
					damage = (int)(damage * 0.2f); // default 2, but q3 has more health :p
				}

				if (damage < 1) 
				{
					damage = 1;
				}

				//Game::Com_PrintMessage(0, Utils::VA("Q3_DamageClient :: damage_mod[]: %d \n", damage), 0);
				Game::Scr_PlayerDamage(dir, targ, inflictor, attacker, damage, dflags, mod, weapon, point, hitLoc, timeOffset);
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
		if (Dvars::pm_disableSprint->current.enabled) 
		{
			ps->sprintState.sprintButtonUpRequired = 1;
		}
	}

	// STOCK/Q3/CS :: Entrypoint - Combined Q3CPM/Source WalkMove ->
	void PM_MT_WalkMove(Game::pmove_t *pm, Game::pml_t *pml)
	{
		// Stock Walkmove if not Defrag
		if (Dvars::pm_movementType->current.integer != Game::PM_MTYPE::DEFRAG)
		{
			Game::PM_WalkMove(pm, pml);
			//PM_CopyVelocityToLocal(pm->ps);
			return;
		}

		int			i;
		vec3_t		wishvel;
		float		fmove, smove;
		vec3_t		wishdir;
		float		wishspeed;
		float		scale;
		float		accelerate;
		float		vel;

		Game::usercmd_s	cmd;

		float		pm_duckScale = 0.5f;
		float		pm_proneScale = 0.25f;
		float		pm_slickaccelerate = 15.0f;
		float		pm_accelerate = 15.0f;

		// If Jumped -> AirMove
		if (PM_Jump_Check(pm, pml))
		{
			PM_MT_AirMove(pm, pml);
			return;
		}

		PM_Q3_Friction(pm, pml);

		fmove = pm->cmd.forwardmove;
		smove = pm->cmd.rightmove;

		cmd = pm->cmd;
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
		if ((pm->ps->pm_flags & PMF_DUCKED) && (wishspeed > pm->ps->speed * pm_duckScale)) 
		{
			wishspeed = pm->ps->speed * pm_duckScale;
		}
		// clamp the speed lower if prone
		else if ((pm->ps->pm_flags & PMF_PRONE) && (wishspeed > pm->ps->speed * pm_proneScale)) 
		{
			wishspeed = pm->ps->speed * pm_proneScale;
		}

		// when a player gets hit, he temporarily loses
		// full control, which allows him to be moved a bit
		if ((pml->groundTrace.surfaceFlags & SURF_SLICK) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK) 
		{
			accelerate = pm_slickaccelerate;
		}
		else if (pm->ps->pm_flags & PMF_DUCKED) 
		{
			accelerate = 15.0; // pm_crouchaccelerate
		}
		else if (pm->ps->pm_flags & PMF_PRONE) 
		{
			accelerate = 10.0; // pm_proneaccelerate
		}
		else 
		{
			accelerate = pm_accelerate;
		}

		PM_Q3_Accelerate_Walk(wishdir, pml, pm->ps, wishspeed, accelerate);

		if ((pml->groundTrace.surfaceFlags & SURF_SLICK) || (pm->ps->pm_flags & PMF_TIME_KNOCKBACK)) // TIME_KNOCKBACK NOT WORKING
		{ 
			pm->ps->velocity[2] -= pm->ps->gravity * pml->frametime;
		}

		vel = VectorLength(pm->ps->velocity);

		// slide along the ground plane
		PM_Q3_ClipVelocity(pm->ps->velocity, pml->groundTrace.normal, pm->ps->velocity, OVERCLIP);

		// don't decrease velocity when going up or down a slope
		_VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

		// don't do anything if standing still
		if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) 
		{
			return;
		}

		PM_Q3_StepSlideMove(pm, pml, false);
		//Game::PM_StepSlideMove(pm, pml, false);
	}

	// STOCK/Q3/CS :: Entrypoint - Combined Q3CPM/Source AirMove ->
	void PM_MT_AirMove(Game::pmove_t *pm, Game::pml_t *pml)
	{
		// Stock Movement
		if (Dvars::pm_movementType->current.integer == Game::PM_MTYPE::STOCK)
		{
			Game::PM_AirMove(pm, pml);
			//PM_CopyVelocityToLocal(ps);

			return;
		}

		auto	ps = pm->ps;

		PM_DisableSprint(ps);

		//////////////////////////////////////////////////////////////////////////////////

		float	fmove, smove, wishspeed, scale = 1.0f;
		vec3_t	wishvel, wishdir;

		// Defrag Movement
		if (Dvars::pm_movementType->current.integer == Game::PM_MTYPE::DEFRAG) 
		{
			//Game::PM_Friction(pm->ps, pml);
			PM_Q3_Friction(pm, pml);
		}

		fmove = pm->cmd.forwardmove;
		smove = pm->cmd.rightmove;

		// Defrag Movement
		if (Dvars::pm_movementType->current.integer == Game::PM_MTYPE::DEFRAG)
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

		if (Dvars::pm_movementType->current.integer == Game::PM_MTYPE::DEFRAG)
		{
			wishspeed *= scale;

			float	wishspeed2 = wishspeed, accel;
			auto	cpm_airstopAccelerate = Dvars::pm_cpm_airstopAccelerate->current.value;
			auto	cpm_strafeAccelerate = Dvars::pm_cpm_strafeAccelerate->current.value;

			if (DotProduct(ps->velocity, wishdir) < 0) 
			{
				accel = cpm_airstopAccelerate; // cpm_pm_airstopAccelerate = 2.5
			}
			else 
			{
				accel = 1.0f; // pm_airaccelerate = 1.0
			}

			if (pm->cmd.forwardmove == 0.0f && pm->cmd.rightmove != 0.0f)
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
			if (Dvars::pm_movementType->current.integer == Game::PM_MTYPE::DEFRAG) 
			{
				PM_Q3_StepSlideMove(pm, pml, true);
				//Game::PM_StepSlideMove(pm, pml, true);
			}

			// Stock StepSlide and SlideMove // why is that here?, will never reach that code lol
			else 
			{
				Game::PM_StepSlideMove(pm, pml, 1);
			}

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
	//		Game::Com_PrintMessage(0, Utils::VA("Q3_GroundTrace :: Liftoff! \n"), 0);

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
	//		Game::Com_PrintMessage(0, Utils::VA("Q3_GroundTrace :: To steep! \n"), 0);
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
		if (Dvars::pm_movementType->current.integer != Game::PM_MTYPE::DEFRAG)
		{
			Game::PM_GroundTrace_Internal(pm, pml);
			return;
		}

		vec3_t					point;
		Game::trace_t		trace;

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
		if (trace.fraction == 1.0)
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
			case 1:
				pm->ps->groundEntityNum = trace.hitId;
				break;

			case 2:
			case 3:
				pm->ps->groundEntityNum = 1022;

			default:
				pm->ps->groundEntityNum = 1023;
				break;
		}

		//pm->ps->groundEntityNum = trace.entityNum;
		//PM_AddTouchEnt(trace.entityNum);
	}
	
	// defines for PmoveSingle_Mid
	int GSCR_NOTIFY_CHECKJUMP_FRAMES = 0;

	// mid-hook @ start of pmovesingle to set / reset variables
	// Not calling PM_AdjustAimSpreadScale for now as it fucks up pml when moving
	void PmoveSingle_Mid(Game::pmove_t *pm, Game::pml_t *pml)
	{
		int frameRate = 0;
		int numberOfFramesToWait = 1;

		//pml = 0 when moving wtf?
		if (pml->frametime == 0) 
		{
			return;
		}

		Game::Globals::pmlFrameTime = pml->msec;
		
		if (pml->frametime != 0) 
		{
			frameRate = (1000 / pml->msec) * numberOfFramesToWait;
		}

		if (GSCR_NOTIFY_CHECKJUMP_FRAMES < frameRate) 
		{
			GSCR_NOTIFY_CHECKJUMP_FRAMES++;
		}
		else
		{
			GSCR_NOTIFY_CHECKJUMP_FRAMES = 0;
			Game::Globals::locPmove_checkJump = false;
		}

		// debug print
		//Game::Com_PrintMessage(0, Utils::VA("%s :: ^3%d\n", Game::Globals::checkJump ? "true" : "false", GSCR_NOTIFY_CHECKJUMP_FRAMES), 0);
	}

	// Rewritten Jump_PushOffLadder for CheckJump
	void Jump_PushOffLadder(Game::playerState_s *ps, Game::pml_t *pml)
	{
		vec3_t	flatForward;
		vec3_t	pushOffDir;
		float	dot;

		auto	jump_ladderPushVel = Game::Dvar_FindVar("jump_ladderPushVel");

		//assert(ps->pm_flags & PMF_LADDER);

		ps->velocity[2] = ps->velocity[2] * 0.75f;

		flatForward[0] = pml->forward[0];
		flatForward[1] = pml->forward[1];
		flatForward[2] = 0.0;

		_VectorNormalize(flatForward);
		dot = DotProduct(ps->vLadderVec, pml->forward);
		
		if (dot >= 0.0) 
		{
			VectorCopy(flatForward, pushOffDir);
		}
		else
		{
			dot = DotProduct(ps->vLadderVec, flatForward);
			VectorMA(flatForward, -2.0f * dot, ps->vLadderVec, pushOffDir);
			_VectorNormalize(pushOffDir);
		}

		ps->velocity[0] = jump_ladderPushVel->current.value * pushOffDir[0];
		ps->velocity[1] = jump_ladderPushVel->current.value * pushOffDir[1];
		ps->pm_flags &= 0xFFFFFFF7;
	}

	// rewrote Jump_Check :: todo :: make it behave like stock if no custom movementtype is set
	bool PM_Jump_Check(Game::pmove_t *pm, Game::pml_t *pml)
	{
		auto	jumpHeightDvar = Game::Dvar_FindVar("jump_height")->current.value;
		float	jumpVelocity;

		if (pm->ps->pm_flags & 0x80000) 
		{
			goto RET_FALSE; // PMF_RESPAWNED? // Stock
		}

		// If Stock movement
		if (Dvars::pm_movementType->current.integer == Game::PM_MTYPE::STOCK)
		{
			if (pm->cmd.serverTime - pm->ps->jumpTime < 500) 
			{
				goto RET_FALSE; // Stock
			}

		}

		// must wait for jump to be released
		if (pm->ps->pm_flags & PMF_JUMP_HELD) {
			goto RET_FALSE; // Stock
		}
			
		if (pm->ps->pm_flags & 4) {
			goto RET_FALSE; // Stock
		}

		if (pm->ps->pm_type >= 7) {
			goto RET_FALSE; // Stock
		}

		if(pm->ps->viewHeightTarget == 11 || pm->ps->viewHeightTarget == 40) {
			goto RET_FALSE; // Stock
		}

		if (!(pm->cmd.buttons & PMF_JUMP_HELD)) {
			goto RET_FALSE; // Stock
		}

		if (!Dvars::pm_bhop_auto->current.enabled) // auto bhop
		{
			if (pm->oldcmd.buttons & PMF_JUMP_HELD) // Stock
			{
				pm->cmd.buttons &= 0xFFFFFBFF;
				goto RET_FALSE;
			}
		}

		// Jump_Start
		jumpVelocity = sqrtf(pm->ps->gravity * (jumpHeightDvar + jumpHeightDvar));

		// if stock movement
		if (Dvars::pm_movementType->current.integer == Game::PM_MTYPE::STOCK)
		{
			if (pm->ps->pm_flags & 0x4000 && pm->ps->pm_time <= 1800)
			{
				auto	jump_slowdownEnable = Game::Dvar_FindVar("jump_slowdownEnable");
				float	reduceFriction = 1.0f;

				if (jump_slowdownEnable->current.enabled)
				{
					if (pm->ps->pm_time >= 1700)
					{
						reduceFriction = 2.5f;
					}

					reduceFriction = (float)(pm->ps->pm_time * 1.5f * 0.0005882352706976235 + 1.0f);
				}

				jumpVelocity /= reduceFriction;
			}

			pm->ps->jumpOriginZ = pm->ps->origin[2];
			pm->ps->jumpTime	= pm->cmd.serverTime;
			
			pm->ps->sprintState.sprintButtonUpRequired = 0;

			auto jump_spreadAdd = Game::Dvar_FindVar("jump_spreadAdd");
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

		pm->ps->velocity[2] = jumpVelocity; // Q3 JUMP_VELOCITY = 270; cod4 = 250
		pm->ps->jumpOriginZ = pm->ps->origin[2]; // <- what if we enable this for q3 too?

		// old bug?
		pm->ps->pm_time = CPM_PM_CLIPTIME; //clip through walls // corners <- dvar?

		Game::Globals::locPmove_checkJump = true; // used for GScr_Method :: checkJump()

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

	RET_FALSE:
		Game::Globals::locPmove_checkJump = false;
		return false;
	}

	#pragma endregion
	#pragma region MOVEMENT-ASM

	// old way of enabling auto bunnyhop :: cs
	__declspec(naked) void pm_auto_bunnyhop_stub()
	{
		__asm
		{
			push	eax
			mov		eax, Dvars::pm_bhop_auto
			cmp		byte ptr[eax + 12], 1
			pop		eax
			je		auto_hop

			// auto hop not enabled
			test    eax, 400h // overwritten op
			push	407DDAh   // jump back to the next op
			retn 

			// auto hop enabled
			auto_hop:
			test    eax, 400h // overwritten op
			jz		stock1
			push	407DF3h
			retn

			stock1:
			push 407DEDh
			retn
		}
	}

	// disable jump penality in crashland ~ only active if pm_bhop_slowdown = true
	__declspec(naked) void pm_auto_bunnyhop_penality_stub()
	{
		__asm
		{
			push	eax
			mov		eax, Dvars::pm_bhop_slowdown
			cmp		byte ptr[eax + 12], 0
			pop		eax
			je		auto_hop_slowdown

			// bhop slowdown enabled
			test    edi, edi				// overwritten op
			fld     dword ptr[esi + 28h]	// overwritten op
			push	410318h					// jump back to the next intact op
			retn

			// auto bhop enabled
			auto_hop_slowdown :
			test    edi, edi				// overwritten op
											// we dont care about the next 30 bytes (Utils::Hook::Nop(0x410315, 30))
			push	410333h					// overjump
			retn
		}
	}

	// disable call to pm_crashland ~ only active if pm_crashland = true
	__declspec(naked) void pm_crashland_stub()
	{
		const static uint32_t PM_CrashLand_Func = 0x40FFB0;
		__asm
		{
			jne		loc_4108ED	// overwritten op

			push	eax
			mov		eax, Dvars::pm_crashland
			cmp		byte ptr[eax + 12], 0
			pop		eax
			je		disable_crashland

			// crashland enabled
			push    edx
			mov     esi, ebp
			call	PM_CrashLand_Func
			push	4108EAh
			retn

			disable_crashland :
			jne		loc_4108ED
			push	4108EDh		// overjump call to pm_crashland
			retn

			loc_4108ED :
			push	4108EDh
			retn
		}
	}

	// hook Weapon_RocketLauncher_Fire to implement rocketJumpHeight dvar
	__declspec(naked) void xo_rocketjump_stub()
	{
		const static uint32_t RetAddr = 0x4E9F34;
		__asm
		{
			mov		eax, Dvars::pm_rocketJumpHeight
			fld		[eax + 12]

			jmp		RetAddr
		}
	}

	// rewrote G_DamageClient ~ only active if pm_cpm_useQuakeDamage = true
	__declspec(naked) void xo_damageclient_stub() 
	{
		const static uint32_t RetAddr = 0x4B55AA;
		__asm
		{
			push	eax
			mov		eax, Dvars::pm_cpm_useQuakeDamage
			cmp		byte ptr[eax + 12], 1
			pop		eax
			jne		stock_damage

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

			push	[esp + 60h]
			push	ebx

			Call	Q3_DamageClient
			add		esp, 8h

			push	4B55AAh
			retn

			stock_damage:
			mov     eax, [esp + 4Ch];	// overwritten op
			mov     ecx, [esp + 40h];	// overwritten op
			
			push	4B5582h
			retn
		}
	}

	// does not really fix rocket delay
	__declspec(naked) void pm_fix_rocketdelay_stub() 
	{
		static int memedServerTime = 0; // nice meme rekti

		__asm
		{
			// get servertime
			mov		ecx, dword ptr[0x13EB894];

			// copy real server time into a local clone
			mov		ecx, [ecx];
			mov		memedServerTime, ecx;
			
			// sub serverTime by 10
			sub		memedServerTime, 10;

			// we need the value of memedServerTime in eax
			mov		ecx, memedServerTime;
			mov		[ebp + 10h], ecx

			// jump back
			push	0x4C7F6A;
			retn
		}
	}

	// rewrite G_CalcMuzzlePoints in FireWeapon	~ only active if pm_cpm_useQuakeDamage = true
	__declspec(naked) void g_calcmuzzlepoints_stub()
	{
		const static uint32_t G_CalcMuzzlePoints_Func = 0x4E9FA0;
		const static uint32_t retnPt = 0x4EA05C;
		__asm
		{
			push	edx
			mov		edx, Dvars::pm_cpm_useQuakeDamage
			cmp		byte ptr[edx + 12], 1
			pop		edx
			jne		STOCK

							// ent is pushed already @ ebx
			push	eax		// push wp
			Call	Q3_CalcMuzzlePoints
			add		esp, 4h
			
			jmp		retnPt

			STOCK:
				Call	G_CalcMuzzlePoints_Func
				jmp		retnPt
		}
	}

	// rewrote Weapon_RocketLauncher_Fire
	__declspec(naked) void weapon_rocketLauncher_fire_stub()
	{
		__asm
		{
			push	edi

			Call	Weapon_RocketLauncher_Fire
			add     esp, 4h

			push	0x4EA176;
			retn
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
		const static uint32_t PM_AdjustAimSpreadScale_Func = 0x418870;
		const static uint32_t retnPt = 0x41471D;
		__asm
		{
			pushad
			push	edx
			push	ebx

			Call	PmoveSingle_Mid
			add		esp, 8h
			popad

			Call	PM_AdjustAimSpreadScale_Func
			jmp		retnPt
		}
	}

	// rewritten G_RadiusDamage - Func Call from MissleTrace ~ only active if pm_cpm_useQuakeDamage = true
	__declspec(naked) void radiusdamage1_stub()
	{
		const static uint32_t G_RadiusDamage_Func = 0x4B67C0;
		__asm
		{
			push	eax
			mov		eax, Dvars::pm_cpm_useQuakeDamage
			cmp		byte ptr[eax + 12], 1
			pop		eax
			jne		STOCK	// use Stock RadiusDamage if useQuakeDamage = false

			// everything else is pushed already
			push	edi		// origin

			Call	Q3_RadiusDamage
			add		esp, 4h

			push	0x4C483F;
			retn

		STOCK:
			Call	G_RadiusDamage_Func

			push	0x4C483F;
			retn
		}
	}
	
	// rewritten G_RadiusDamage - Func Call from G_MissleExplode ~ only active if pm_cpm_useQuakeDamage = true
	__declspec(naked) void radiusdamage2_stub()
	{
		const static uint32_t G_RadiusDamage_Func = 0x4B67C0;
		__asm
		{
			push	eax
			mov		eax, Dvars::pm_cpm_useQuakeDamage
			cmp		byte ptr[eax + 12], 1
			pop		eax
			jne		STOCK	// use Stock RadiusDamage if useQuakeDamage = false

			// everything else is pushed already
			push	edi		// origin

			Call	Q3_RadiusDamage
			add		esp, 4h

			push	0x4C4F33;	// ret to "add esp, 28h"
			retn

		STOCK :
			Call	G_RadiusDamage_Func

			push	0x4C4F33; // ret to "add esp, 28h"
			retn
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

	PM_Movement::PM_Movement()
	{
		// -----
		// Dvars

		static std::vector <const char*> pm_movementTypeEnum =
		{ 
			"STOCK", 
			"DEFRAG", 
			"CS", 
		};

		Dvars::pm_movementType = Game::Dvar_RegisterEnum(
			/* name		*/ "pm_movementType",
			/* desc		*/ "the movementtype to use",
			/* default	*/ 0,
			/* enumSize	*/ pm_movementTypeEnum.size(),
			/* enumData */ pm_movementTypeEnum.data(),
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_cpm_airstopAccelerate = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cpm_airstopAccelerate",
			/* desc		*/ "pm_movementType Defrag :: defines negative player acceleration when the player wants to stop mid-air",
			/* default	*/ 3.0f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 50.0f,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_cpm_strafeAccelerate = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cpm_strafeAccelerate",
			/* desc		*/ "pm_movementType Defrag :: defines player acceleration on initial strafejump",
			/* default	*/ 70.0f,
			/* minVal	*/ 1.0f,
			/* maxVal	*/ 500.0f,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_cpm_airControl = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cpm_airControl",
			/* desc		*/ "pm_movementType Defrag :: defines direction control mid-air",
			/* default	*/ 150.0f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 500.0f,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_cpm_airAccelerate = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cpm_airAccelerate",
			/* desc		*/ "pm_movementType Defrag :: defines player acceleration mid-air",
			/* default	*/ 1.0f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_cpm_useQuakeDamage = Game::Dvar_RegisterBool(
			/* name		*/ "pm_cpm_useQuakeDamage",
			/* desc		*/ "pm_movementType All :: enables quake 3 damange knockback",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_cpm_damageKnockback = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cpm_damageKnockback",
			/* desc		*/ "pm_movementType Defrag :: damage knockback scalar",
			/* default	*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_cpm_useBouncing = Game::Dvar_RegisterBool(
			/* name		*/ "pm_cpm_useBouncing",
			/* desc		*/ "pm_movementType Defrag :: enables experimental bouncing",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_cs_airAccelerate = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cs_airAccelerate",
			/* desc		*/ "pm_movementType CS :: defines player acceleration mid-air",
			/* default	*/ 100.0f,
			/* minVal	*/ 1.0f,
			/* maxVal	*/ 500.0f,
			/* flags	*/ Game::dvar_flags::none);
		
		Dvars::pm_cs_airspeedCap = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_cs_airspeedCap",
			/* desc		*/ "pm_movementType CS :: maximum speed mid-air",
			/* default	*/ 30.0f,
			/* minVal	*/ 1.0f,
			/* maxVal	*/ 500.0f,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_bhop_auto = Game::Dvar_RegisterBool(
			/* name		*/ "pm_bhop_auto",
			/* desc		*/ "constantly jump when holding space, still has landing slowdown -> disable pm_bhop_slowdown",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_bhop_slowdown = Game::Dvar_RegisterBool(
			/* name		*/ "pm_bhop_slowdown",
			/* desc		*/ "toggle landing slowdown",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::none);

#if 0
		Dvars::pm_bhop_rampjump = Game::Dvar_RegisterBool_w(
			/* name		*/ "pm_bhop_rampjump",
			/* desc		*/ "toggle rampjumping",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);
#endif

		Dvars::pm_crashland = Game::Dvar_RegisterBool(
			/* name		*/ "pm_crashland",
			/* desc		*/ "disable this to completely skip any fall related effects",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_disableSprint = Game::Dvar_RegisterBool(
			/* name		*/ "pm_disableSprint",
			/* desc		*/ "disables the ability to sprint",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_rocketJumpHeight = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_rocketJumpHeight",
			/* desc		*/ "pushback amount for rocketjumps using stock movement",
			/* default	*/ 64.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 4096.0f,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_hud_enable = Game::Dvar_RegisterBool(
			/* name		*/ "pm_hud_enable",
			/* desc		*/ "Display current speed and current movement type.",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_hud_x = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_hud_x",
			/* desc		*/ "text offset x",
			/* default	*/ 10.0f,
			/* minVal	*/ -8192.0f,
			/* maxVal	*/ 8192.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_hud_y = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_hud_y",
			/* desc		*/ "text offset y",
			/* default	*/ 300.0f,
			/* minVal	*/ -8192.0f,
			/* maxVal	*/ 8192.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_hud_fontScale = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_hud_fontScale",
			/* desc		*/ "font scale",
			/* default	*/ 0.75f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_hud_fontStyle = Game::Dvar_RegisterInt(
			/* name		*/ "pm_hud_fontStyle",
			/* desc		*/ "font Style",
			/* default	*/ 1,
			/* minVal	*/ 0,
			/* maxVal	*/ 8,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_hud_fontColor = Game::Dvar_RegisterVec4(
			/* name		*/ "pm_hud_fontColor",
			/* desc		*/ "font color",
			/* x		*/ 1.0f,
			/* y		*/ 0.55f,
			/* z		*/ 0.4f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

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


		Dvars::pm_debug_prints = Game::Dvar_RegisterBool(
			/* name		*/ "pm_debug_prints",
			/* desc		*/ "enable debug prints for movement related functions",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		// --------
		// Commands

		// switch to Cmd_ExecuteSingleCommand() ?

		Command::Add("pm_preset_stock", [](Command::Params)
		{
			Dvars::pm_movementType->current.integer = Game::PM_MTYPE::STOCK;

			Dvars::pm_bhop_auto->current.enabled	 = false;
			Dvars::pm_bhop_slowdown->current.enabled = true;
			Dvars::pm_disableSprint->current.enabled = false;

			// resets ::
			Dvars::pm_cpm_useQuakeDamage->current.enabled = false;
			Dvars::pm_rocketJumpHeight->current.value	  = 64.0f;

			Game::Dvar_FindVar("bg_falldamageminheight")->current.value = 128.0f;
			Game::Dvar_FindVar("bg_falldamagemaxheight")->current.value = 300.0f;

			Game::Dvar_FindVar("jump_height")->current.value			= 39.0f;
			Game::Dvar_FindVar("bg_bobMax")->current.value				= 8.0f;
			Game::Dvar_FindVar("friction")->current.value				= 5.5f;
			Game::Dvar_FindVar("g_speed")->current.integer				= 190;
			Game::Dvar_FindVar("jump_slowdownenable")->current.enabled	= true;
		});

		Command::Add("pm_preset_q3", [](Command::Params)
		{
			Dvars::pm_movementType->current.integer	= Game::PM_MTYPE::DEFRAG;

			Dvars::pm_bhop_auto->current.enabled	 = true;
			Dvars::pm_bhop_slowdown->current.enabled = false;
			Dvars::pm_disableSprint->current.enabled = true;

			Dvars::pm_cpm_useQuakeDamage->current.enabled = true;
			Dvars::pm_rocketJumpHeight->current.value	  = 0.0;
			
			Game::Dvar_FindVar("bg_falldamageminheight")->current.value = 99998.0f;
			Game::Dvar_FindVar("bg_falldamagemaxheight")->current.value = 99999.0f;
			
			Game::Dvar_FindVar("jump_height")->current.value			= 46.0f;
			Game::Dvar_FindVar("bg_bobMax")->current.value				= 0.0f;
			Game::Dvar_FindVar("friction")->current.value				= 8.0f;
			Game::Dvar_FindVar("g_speed")->current.integer				= 320;
			Game::Dvar_FindVar("jump_slowdownenable")->current.enabled	= false;
			
			
		});

		Command::Add("pm_preset_cs", [](Command::Params)
		{
			Dvars::pm_movementType->current.integer	= Game::PM_MTYPE::CS;
			
			Dvars::pm_bhop_auto->current.enabled	 = true;
			Dvars::pm_bhop_slowdown->current.enabled = false;
			Dvars::pm_disableSprint->current.enabled = true;

			// resets ::
			Dvars::pm_cpm_useQuakeDamage->current.enabled = false;
			Dvars::pm_rocketJumpHeight->current.value	  = 64.0f;

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
		Utils::Hook(0x414718, pmove_mid_variables_stub, HOOK_JUMP).install()->quick();

		// PM_AirMove Call in PmoveSingle ( Entrypoint Stock/Defrag/Source )			- Stock movement uses Stock functions
		Utils::Hook(0x414B87, PM_MT_AirMove, HOOK_CALL).install()->quick();
		
		// PM_AirMove Call in PM_WalkMove												- Stock movement uses Stock functions
		Utils::Hook(0x40F7F9, PM_MT_AirMove, HOOK_CALL).install()->quick();

		// PM_WalkMove Call in PmoveSingle ( Entrypoint Stock/Defrag/Source )			- Stock movement uses Stock functions
		Utils::Hook(0x414B7A, PM_MT_WalkMove, HOOK_CALL).install()->quick();

		// enable / disable crashland call
		Utils::Hook(0x4108E0, pm_crashland_stub, HOOK_JUMP).install()->quick();

		// hook G_DamageClient to add Q3 radius damage knockback
		Utils::Hook::Nop(0x4B557A, 8);
		Utils::Hook(0x4B557A, xo_damageclient_stub, HOOK_JUMP).install()->quick();

		// enable / disable auto bunnyhop on space
		Utils::Hook(0x407DD5, pm_auto_bunnyhop_stub, HOOK_JUMP).install()->quick();	// - still needed for cs

		// enable / disable jump landing punishment within PM_CrashLand
		Utils::Hook(0x410313, pm_auto_bunnyhop_penality_stub, HOOK_JUMP).install()->quick();

		// hook PM_GroundTrace - Top of PmoveSingle ( q3 rampslide )				- using stock func if stock movementtype
		Utils::Hook(0x414AA9, PM_Q3_GroundTrace, HOOK_CALL).install()->quick();
		
		// hook PM_GroundTrace - Bottom of PmoveSingle ( q3 rampslide )				- using stock func if stock movementtype
		Utils::Hook(0x414B95, PM_Q3_GroundTrace, HOOK_CALL).install()->quick();

		// implement rocket pre-fire to hopefully fix rocketdelay <- like in Q3
		Utils::Hook(0x4C7F64, pm_fix_rocketdelay_stub, HOOK_JUMP).install()->quick();
		Utils::Hook::Nop(0x4C7F69, 1);

		// Q3_CalcMuzzlePoints
		Utils::Hook(0x4EA057, g_calcmuzzlepoints_stub, HOOK_JUMP).install()->quick();

		// Rewrote weapon_rocketLauncher_fire
		Utils::Hook(0x4EA171, weapon_rocketLauncher_fire_stub, HOOK_JUMP).install()->quick();

		// we get stuck when shooting rockets before jumping?
		// hook G_RadiusDamage to change expl. origin from +24 to +40 @0x4B6A02
		//Utils::Hook(0x4B6A02, radiusdamage_mid_stub, HOOK_JUMP).install()->quick();
		//Utils::Hook::Nop(0x4B6A07, 1);

		// hook PM_Weapon_CheckForChangeWeapon to disable Timer that stops weapchange when fireAnim is still playing
		//Utils::Hook(0x418FA8, weapon_checkforchange_stub, HOOK_JUMP).install()->quick();
		//Utils::Hook::Nop(0x418FAD, 1);

		// RadiusDamage in MissleImpact
		Utils::Hook(0x4C483A, radiusdamage1_stub, HOOK_JUMP).install()->quick();

		// RadiusDamage in G_ExplodeMissle
		Utils::Hook(0x4C4F2E, radiusdamage2_stub, HOOK_JUMP).install()->quick();
}

	PM_Movement::~PM_Movement()
	{ }
}
