#include "STDInclude.hpp"

namespace Components
{
	// ---------------
	// Movement traces

	Game::dbgLinesDelaySync_t syncDbgLines = Game::dbgLinesDelaySync_t();
	Game::dbgLines_t lineTrace_Origin = Game::dbgLines_t();
	Game::dbgLines_t lineTrace_Velocity = Game::dbgLines_t();

	// add renderer command
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

	// add renderer command
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

	// x = forward, y = left, angle in degrees
	glm::vec3 PM_GetCircleCoordForAngle(const glm::vec3& center, const float radius, const float angle)
	{
		glm::vec3 coord;

		// sin / cos uses radians ...
		float radians = Utils::vector::_DegreesToRadians(angle + 90);

		coord.x = radius * sinf(radians) + center.x;
		coord.y = radius * cosf(radians) + center.y;
		coord.z = center.z;

		return coord;
	}

	// render directly
	void PM_DrawWorldAxis()
	{
		auto _origin = glm::toVec3(Game::cgs->lastVieworg);
		auto _radius = Dvars::pm_debug_drawAxis_radius->current.value;
		auto _height = Dvars::pm_debug_drawAxis_height->current.value;
		auto _center = glm::vec3(_origin.x, _origin.y, _height + _origin.z);

		// fps zones (mode 2)
		if (Dvars::pm_debug_drawAxis->current.integer == 2)
		{
			const int numZones = 16;
			Game::circleZone_t zones[numZones];

			// N ~ +x
			// W ~ +y
			// starting at -15° for first 125 zone (top)
			// 3 angles per zone (outer left, outer center, outer right)

			std::vector<float> zoneAngles
			{
				// 125 top
				-15.0f, 0.0f, 15.0f,

				// 250
				15.0f, 22.5f, 30.0f,

				// 333
				30.0f, 45.0f, 60.0f,

				// 250
				60.0f, 67.5f, 75.0f,

				// 125 right
				75.0f, 90.0f, 105.0f,

				// 250
				105.0f, 112.5f, 120.0f,

				// 333
				120.0f, 135.0f, 150.0f,

				// 250
				150.0f, 157.5f, 165.0f,

				// 125 bottom
				165.0f, 180.0f, 195.0f,

				// 250
				195.0f, 202.5f, 210.0f,

				// 333
				210.0f, 225.0f, 240.0f,

				// 250
				240.0f, 247.5f, 255.0f,

				// 125 left
				255.0f, 270.0f, 285.0f,

				// 250
				285.0f, 292.5f, 300.0f,

				// 333
				300.0f, 315.0f, 330.0f,

				// 250
				330.0f, 337.5f, 345.0f,

				// 125 top ...
			};

			if (static_cast<int>(zoneAngles.size()) < numZones * 3)
			{
				Game::Dvar_SetValue(Dvars::pm_debug_drawAxis, 0);
				Game::Com_PrintMessage(0, "^1-> pm_debug_drawAxis ^7:: invalid circle zones.\n", 0);

				return;
			}

			for (auto z = 0; z < numZones; z++)
			{
				float* zoneColor;

				switch (z)
				{
				case 0: case 4: case 8: case 12:
					zoneColor = Dvars::pm_debug_drawAxis_col125->current.vector;
					break;

				case 1: case 3: case 5: case 7: case 9: case 11: case 13: case 15:
					zoneColor = Dvars::pm_debug_drawAxis_col250->current.vector;
					break;

				default:
					zoneColor = Dvars::pm_debug_drawAxis_col333->current.vector;
				}

				// poly vertices ccw rotation starting at the center of the circle
				zones[z].zoneCoords[0] = _center;
				zones[z].zoneCoords[1] = PM_GetCircleCoordForAngle(_center, _radius, zoneAngles[z * 3 + 2]);
				zones[z].zoneCoords[2] = PM_GetCircleCoordForAngle(_center, _radius, zoneAngles[z * 3 + 1]);
				zones[z].zoneCoords[3] = PM_GetCircleCoordForAngle(_center, _radius, zoneAngles[z * 3 + 0]);

				_Debug::RB_DrawPoly(
					/* numpts	*/ 4,
					/* points	*/ (float(*)[3]) & zones[z].zoneCoords,
					/* pcolor	*/ zoneColor,
					/* lit		*/ false,
					/* outlines */ false,
					/* lcolor	*/ nullptr,
					/* depth	*/ Dvars::pm_debug_lineDepth->current.enabled,
					/* 2 sided	*/ false);
			}
		}

		Game::dbgLines_t lines[2];

		lines[0].ptFrom = _origin - glm::vec3(_radius, 0.0f, -_height);
		lines[0].ptTo = _origin + glm::vec3(_radius, 0.0f, _height);

		lines[1].ptFrom = _origin - glm::vec3(0.0f, _radius, -_height);
		lines[1].ptTo = _origin + glm::vec3(0.0f, _radius, _height);

		_Debug::RB_AddAndDrawDebugLines(
			/* linecount */ 2, 
			/* dbglines	 */ lines, 
			/* color	 */ Dvars::pm_debug_lineColor->current.vector, 
			/* depth	 */ Dvars::pm_debug_lineDepth->current.enabled,
			/* linewidth */ Dvars::pm_debug_lineWidth->current.integer);
	}

	// Called from _Debug (render thread)
	void _Pmove::PM_DrawDebug()
	{
		// draw axial "compass"
		if (Dvars::pm_debug_drawAxis->current.integer != 0)
		{
			PM_DrawWorldAxis();
		}
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
					
					if (Dvars::pm_debug_traceOrigin->current.enabled || Dvars::pm_debug_traceVelocity->current.enabled || Dvars::pm_debug_drawAxis->current.enabled)
					{
						// init and create 1 timer for all debug lines, timer is true for 1 frame
						_Debug::DebugLines_InitSync(&syncDbgLines);

						bool onGround;

						if (pm->ps->groundEntityNum == 1022) 
						{
							onGround = true;
						} 
						else 
						{
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

						// draw axial "compass"
						/*if (Dvars::pm_debug_drawAxis->current.enabled)
						{
							PM_DrawWorldAxis(pm, msec);
						}*/
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
			/* desc		*/ "trace the players origin.\n0: disabled\n1: trace and display for duration\n2: trace and display for duration when not on the ground",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 3,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_debug_traceVelocity = Game::Dvar_RegisterInt(
			/* name		*/ "pm_debug_traceVelocity",
			/* desc		*/ "trace the players velocity.\n0: disabled\n1: trace and display for duration\n2: trace and display for duration when not on the ground\n3: trace in realtime",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 3,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_debug_lineDepth = Game::Dvar_RegisterBool(
			/* name		*/ "pm_debug_lineDepth",
			/* desc		*/ "if the trace should perform a depth check",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_debug_lineColor = Game::Dvar_RegisterVec4(
			/* name		*/ "pm_debug_lineColor",
			/* desc		*/ "color of debug lines for movement related functions",
			/* x		*/ 1.0f,
			/* y		*/ 0.3f,
			/* z		*/ 0.4f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_debug_lineDuration = Game::Dvar_RegisterInt(
			/* name		*/ "pm_debug_lineDuration",
			/* desc		*/ "how long the trace persists",
			/* default	*/ 500,
			/* minVal	*/ 0,
			/* maxVal	*/ 5000,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_debug_lineWidth = Game::Dvar_RegisterInt(
			/* name		*/ "pm_debug_lineWidth",
			/* desc		*/ "with of debug lines",
			/* default	*/ 1,
			/* minVal	*/ 1,
			/* maxVal	*/ 20,
			/* flags	*/ Game::dvar_flags::saved);


		Dvars::pm_debug_drawAxis = Game::Dvar_RegisterInt(
			/* name		*/ "pm_debug_drawAxis",
			/* desc		*/ "draw axial information\n1: draw world axial lines only\n2: draw additional fps zones (default colors >> Green 125; Cyan 250; Blue 333)",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 2,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::pm_debug_drawAxis_col125 = Game::Dvar_RegisterVec4(
			/* name		*/ "pm_debug_drawAxis_col125",
			/* desc		*/ "color of 125 fps zone",
			/* x		*/ 0.4f,
			/* y		*/ 1.0f,
			/* z		*/ 0.3f,
			/* w		*/ 0.6f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_debug_drawAxis_col250 = Game::Dvar_RegisterVec4(
			/* name		*/ "pm_debug_drawAxis_col250",
			/* desc		*/ "color of 250 fps zone",
			/* x		*/ 0.32f,
			/* y		*/ 1.00f,
			/* z		*/ 0.99f,
			/* w		*/ 0.6f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_debug_drawAxis_col333 = Game::Dvar_RegisterVec4(
			/* name		*/ "pm_debug_drawAxis_col333",
			/* desc		*/ "color of 333 fps zone",
			/* x		*/ 0.17f,
			/* y		*/ 0.36f,
			/* z		*/ 1.00f,
			/* w		*/ 0.6f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_debug_drawAxis_radius = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_debug_drawAxis_radius",
			/* desc		*/ "radius of axial lines / zones",
			/* default	*/ 40.0f,
			/* minVal	*/ -1000.0f,
			/* maxVal	*/ 1000.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::pm_debug_drawAxis_height = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_debug_drawAxis_height",
			/* desc		*/ "player origin + height",
			/* default	*/ 30.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 200.0f,
			/* flags	*/ Game::dvar_flags::saved);
	}

	_Pmove::~_Pmove()
	{ }
}