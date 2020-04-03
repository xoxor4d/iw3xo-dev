#include "STDInclude.hpp"

namespace Components
{
	// ---------------
	// Movement traces

	Game::dbgLinesDelaySync_t syncDbgLines = Game::dbgLinesDelaySync_t();
	Game::dbgLines_t lineTrace_Origin = Game::dbgLines_t();
	Game::dbgLines_t lineTrace_Velocity = Game::dbgLines_t();

	void PM_TraceVelocity(Game::pmove_t* pm, bool onGround, int frameTime)
	{
		int traceType = Dvars::pm_debug_traceVelocity->current.integer;

		// return when not in sync or not using realtime tracing
		if (traceType != 3 && !_Debug::DebugLines_Sync(&syncDbgLines)) 
		{
				return;
		}

		// only trace when not on the ground
		if (traceType == 2 && onGround) 
		{
			return;
		}

		glm::vec3 pVelocity = glm::toVec3(pm->ps->velocity);
		if (pVelocity.x + pVelocity.y + pVelocity.z == 0.0f) 
		{
			return;
		}

		glm::vec4 color = glm::toVec4(Dvars::pm_debug_lineColor->current.vector);
		float alpha = color.a;

		color.r = glm::clamp(abs(pVelocity.x * 0.25f) * 0.01f, 0.0f, 1.0f);
		color.g = glm::clamp(abs(pVelocity.y * 0.25f) * 0.01f, 0.0f, 1.0f);
		color.b = glm::clamp(abs(pVelocity.z * 0.25f) * 0.01f, 0.0f, 1.0f);
		color.a = alpha;

		lineTrace_Velocity.ptFrom = glm::toVec3(pm->ps->origin);
		
		// realtime trace
		if (traceType == 3)
		{ 
			lineTrace_Velocity.ptTo = lineTrace_Velocity.ptFrom + pVelocity;
			_Debug::AddDebugLineClient(lineTrace_Velocity.ptFrom, lineTrace_Velocity.ptTo, glm::toVec4(Dvars::pm_debug_lineColor->current.vector), Dvars::pm_debug_lineDepth->current.enabled, frameTime);
		}

		else // trace with duration + scale down the length of velocity
		{ 
			lineTrace_Velocity.ptTo = lineTrace_Velocity.ptFrom + (pVelocity * 0.125f);
			_Debug::AddDebugLineClient(lineTrace_Velocity.ptFrom, lineTrace_Velocity.ptTo, color, Dvars::pm_debug_lineDepth->current.enabled, Dvars::pm_debug_lineDuration->current.integer);
		}
	}

	void PM_TraceOrigin(Game::pmove_t* pm, bool onGround)
	{
		// return when not in sync
		if (!_Debug::DebugLines_Sync(&syncDbgLines)) 
		{
			return;
		}

		lineTrace_Origin.ptTo = glm::toVec3(pm->ps->origin);

		// only trace when not on the ground but keep the last pos active
		if (Dvars::pm_debug_traceOrigin->current.integer == 2 && onGround) 
		{
			lineTrace_Origin.ptFrom = lineTrace_Origin.ptTo;
			return;
		}
		
		// do not add lines if the player did not move
		if (lineTrace_Origin.ptFrom == lineTrace_Origin.ptTo) 
		{
			return;
		}

		_Debug::AddDebugLineClient(lineTrace_Origin.ptFrom, lineTrace_Origin.ptTo, glm::toVec4(Dvars::pm_debug_lineColor->current.vector), Dvars::pm_debug_lineDepth->current.enabled, Dvars::pm_debug_lineDuration->current.integer);
		lineTrace_Origin.ptFrom = lineTrace_Origin.ptTo;
	}

	// -------------------------------------------------------------------------

	// Proxy Function for "PM_UpdateSprint" :: Grab and save local playerState variables
	// DO NOT USE FOR FUNCTIONS THAT SHOULD ONLY RUN ONCE PER FRAME ..
	void PM_CopyLocalPlayerstateVars(Game::pmove_t* pm)
	{
		// Grab velocity for speedhud
		Game::Globals::locPmove_playerVelocity = glm::vec3(pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);

		// Grab player origin and angles
		Game::Globals::locPmove_playerAngles = glm::vec3(pm->ps->viewangles[0], pm->ps->viewangles[1], pm->ps->viewangles[2]);
		Game::Globals::locPmove_playerOrigin = glm::vec3(pm->ps->origin[0], pm->ps->origin[1], pm->ps->origin[2]);

		// Calculate camera origin
		Game::Globals::locPmove_cameraOrigin = Game::Globals::locPmove_playerOrigin + glm::vec3(0.0f, 0.0f, pm->ps->viewHeightCurrent);

		// not really needed and prob. inacurate because this function gets called 6 times
		// ----- Grab servertime / frametime stuff ------
		Game::Globals::serverTimeOld = Game::Globals::serverTime;
		Game::Globals::serverTime = pm->cmd.serverTime;

		// -> calculate frametime
		Game::Globals::serverFrameTime = Game::Globals::serverTime - Game::Globals::serverTimeOld;

		// call stock function
		Game::PM_UpdateSprint(pm);
	}


	// -----
	// Pmove

	// runs when the client = the server
	void Pmove_Client(Game::pmove_t *pm)
	{
		// remove me later
		RadiantRemote::SV_ReceivePackets();

		int serverTime = pm->cmd.serverTime;
		int commandTime = pm->ps->commandTime;
		
		// if radiant cam-sync is active and using either game->radiant or game->radiant & radiant->game
		bool radiantcam_active = Game::Globals::cgsAddons.radiantLiveConnected && (Dvars::radiant_syncCamera->current.integer == 2 || Dvars::radiant_syncCamera->current.integer == 3);
		int refreshRateMs = 0, currentDiff = 0;

		if (Components::active.RadiantRemote && radiantcam_active)
		{
			// *
			// calculate client frametime

			// wait atleast 1 frame
			if (!Game::Globals::cgsAddons.radiantCamClientTimeOld)
			{
				Game::Globals::cgsAddons.radiantCamClientTimeOld = commandTime;
			}
			else
			{
				currentDiff = (commandTime - Game::Globals::cgsAddons.radiantCamClientTimeOld);

				// add the previous frametime to our counter
				Game::Globals::cgsAddons.radiantCamClientMsecPassed += currentDiff;

				// save the current time for the next frame
				Game::Globals::cgsAddons.radiantCamClientTimeOld = commandTime;
			}

			// we need the screen refreshrate to limit the packet-rate
			refreshRateMs = 1000 / Utils::extractFirstIntegerFromString(*Game::Dvar_FindVar("r_displayRefresh")->domain.enumeration.strings);
		}
		
		// stock pmove
		if (serverTime >= commandTime)
		{
			if (serverTime > commandTime + 1000) 
			{
				pm->ps->commandTime = serverTime - 1000;
			}

			pm->numtouch = 0;

			if (pm->ps->commandTime != serverTime)
			{
				while (1)
				{
					int msec = serverTime - pm->ps->commandTime;

					if (msec > 66) 
					{
						msec = 66;
					}

					pm->cmd.serverTime = msec + pm->ps->commandTime;
					Game::PmoveSingle(pm);

					// -------------------------
					// pmove debug tracelines
					
					if (Dvars::pm_debug_traceOrigin->current.enabled || Dvars::pm_debug_traceVelocity->current.enabled)
					{
						// init and create 1 timer for all debug lines, timer is true for 1 frame
						_Debug::DebugLines_InitSync(&syncDbgLines);

						bool onGround;

						if (pm->ps->groundEntityNum == 1022) {
							onGround = true;
						} else {
							onGround = false;
						}

						// trace player origin
						if (Dvars::pm_debug_traceOrigin->current.enabled)
						{
							PM_TraceOrigin(pm, onGround);
						}

						// trace player velocity
						if (Dvars::pm_debug_traceVelocity->current.enabled)
						{
							PM_TraceVelocity(pm, onGround, msec);
						}
					}

					// -------------------------
					// game->radiant cam control

					if (Components::active.RadiantRemote && radiantcam_active)
					{
						float temp_camOrigin[3] =
						{
							pm->ps->origin[0],
							pm->ps->origin[1],
							pm->ps->origin[2] + pm->ps->viewHeightCurrent
						};

						if (!Game::Globals::cgsAddons.radiantCamOriginOld)
						{
							memcpy(&Game::Globals::cgsAddons.radiantCamOriginOld, temp_camOrigin, sizeof(int[3]));
						}

						// compare last frames origin/viewpos with the current origin/viewpos
						if (!Utils::vector::_VectorCompareInt(pm->cmd.angles, pm->oldcmd.angles) || !Utils::vector::_VectorCompare(temp_camOrigin, Game::Globals::cgsAddons.radiantCamOriginOld))
						{
							memcpy(&Game::Globals::cgsAddons.radiantCamOriginOld, temp_camOrigin, sizeof(int[3]));

							float temp_camAngles[3] =
							{
								-pm->ps->viewangles[0],
								pm->ps->viewangles[1],
								pm->ps->viewangles[2]
							};

							// always if client ms/frame is greater then ms/screen-refreshrate (no packet-rate capping needed)
							// OR if client ms/frame is smaller then ms/screen-refreshrate and we did not cap the packet-rate yet
							if (currentDiff >= refreshRateMs || Game::Globals::cgsAddons.radiantCamClientMsecPassed <= refreshRateMs && !Game::Globals::cgsAddons.radiantCamCappedPackets)
							{
								Game::Globals::cgsAddons.radiantCamCappedPackets = true;
								RadiantRemote::Cmd_SendCamera(temp_camOrigin, temp_camAngles);
							}

							// allow a new packet if client passed the ms/screen-refreshrate
							else if (Game::Globals::cgsAddons.radiantCamClientMsecPassed >= refreshRateMs)
							{
								Game::Globals::cgsAddons.radiantCamClientMsecPassed = 0;
								Game::Globals::cgsAddons.radiantCamCappedPackets = false;
							}
						}
					}

					// -----------
					// stock pmove

					memcpy(&pm->oldcmd, &pm->cmd, sizeof(pm->oldcmd));

					if (pm->ps->commandTime == serverTime) 
					{
						break;
					}
				}
			}
		}
	}

	// pmove used for prediction / spectator (when connected to a server)
	void Pmove_SpecPredict(Game::pmove_t *pm)
	{
		int serverTime = pm->cmd.serverTime;
		int commandTime = pm->ps->commandTime;

		if (serverTime >= commandTime)
		{
			if (serverTime > commandTime + 1000) 
			{
				pm->ps->commandTime = serverTime - 1000;
			}

			pm->numtouch = 0;
			if (pm->ps->commandTime != serverTime)
			{
				while (1)
				{
					int msec = serverTime - pm->ps->commandTime;

					if (msec > 66) {
						msec = 66;
					}

					pm->cmd.serverTime = msec + pm->ps->commandTime;
					Game::PmoveSingle(pm);

					memcpy(&pm->oldcmd, &pm->cmd, sizeof(pm->oldcmd));

					if (pm->ps->commandTime == serverTime) 
					{
						break;
					}
				}
			}
		}
	}

	_Pmove::_Pmove()
	{
		//// Hook Pmove :x
		Utils::Hook(0x4476A3, Pmove_SpecPredict, HOOK_CALL).install()->quick(); // CG_PredictPlayerState_Internal // Pmove_SpecPredict
		Utils::Hook(0x4A7DA6, Pmove_SpecPredict, HOOK_CALL).install()->quick(); // SpectatorThink // Pmove_SpecPredict
		Utils::Hook(0x4A8AD3, Pmove_Client, HOOK_CALL).install()->quick(); // ClientThink_real

		//// Hook PM_UpdateSprint Call in PmoveSingle (PM_TYPE 0) (DEAD)?					- Used to grab local playerstate variables
		Utils::Hook(0x41482A, PM_CopyLocalPlayerstateVars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (PM_FLAGS 4) (SPECTATOR)?
		Utils::Hook(0x4148D0, PM_CopyLocalPlayerstateVars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (PM_FLAGS 2) (NOCLIP)
		Utils::Hook(0x414936, PM_CopyLocalPlayerstateVars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (PM_FLAGS 3) (UFO)
		Utils::Hook(0x414995, PM_CopyLocalPlayerstateVars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (PM_FLAGS 5)
		Utils::Hook(0x4149D9, PM_CopyLocalPlayerstateVars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (PM_FLAGS 8) (FREEZE DEAD)?
		Utils::Hook(0x414A29, PM_CopyLocalPlayerstateVars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (PM_FLAGS NOT 4)
		Utils::Hook(0x414A85, PM_CopyLocalPlayerstateVars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (MANTLE)
		Utils::Hook(0x414AF3, PM_CopyLocalPlayerstateVars, HOOK_CALL).install()->quick();

		// -----------------------------------------------------------------

		Dvars::pm_debug_traceOrigin = Game::Dvar_RegisterInt(
			/* name		*/ "pm_debug_traceOrigin",
			/* desc		*/ "movement :: trace the players origin.\n0: disabled\n1: trace and display for duration\n2: trace and display for duration when not on the ground",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 3,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_debug_traceVelocity = Game::Dvar_RegisterInt(
			/* name		*/ "pm_debug_traceVelocity",
			/* desc		*/ "movement :: trace the players velocity.\n0: disabled\n1: trace and display for duration\n2: trace and display for duration when not on the ground\n3: trace in realtime",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 3,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_debug_lineDepth = Game::Dvar_RegisterBool(
			/* name		*/ "pm_debug_lineDepth",
			/* desc		*/ "movement :: if the trace should perform a depth check",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_debug_lineColor = Game::Dvar_RegisterVec4(
			/* name		*/ "pm_debug_lineColor",
			/* desc		*/ "movement :: color of debug lines for movement related functions",
			/* x		*/ 1.0f,
			/* y		*/ 0.3f,
			/* z		*/ 0.4f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_debug_lineDuration = Game::Dvar_RegisterInt(
			/* name		*/ "pm_debug_lineDuration",
			/* desc		*/ "movement :: how long the trace persists",
			/* default	*/ 500,
			/* minVal	*/ 0,
			/* maxVal	*/ 5000,
			/* flags	*/ Game::dvar_flags::none);
	}

	_Pmove::~_Pmove()
	{ }
}