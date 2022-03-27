#include "STDInclude.hpp"

namespace components
{
	// ---------------
	// Movement traces

	Game::dbgLinesDelaySync_t sync_debug_lines = Game::dbgLinesDelaySync_t();
	Game::dbgLines_t line_trace_origin = Game::dbgLines_t();
	Game::dbgLines_t line_trace_velocity = Game::dbgLines_t();

	// add renderer command
	void trace_velocity(Game::pmove_t* pm, bool on_ground, int frame_time)
	{
		const int trace_type = dvars::pm_debug_traceVelocity->current.integer;

		// return when not in sync or not using realtime tracing
		if (trace_type != 3 && !_debug::debug_lines_sync(&sync_debug_lines)) 
		{
			return;
		}

		// only trace when not on the ground
		if (trace_type == 2 && on_ground) 
		{
			return;
		}

		const glm::vec3 velocity = glm::to_vec3(pm->ps->velocity);
		if (velocity.x + velocity.y + velocity.z == 0.0f) 
		{
			return;
		}

		glm::vec4 color = glm::to_vec4(dvars::pm_debug_lineColor->current.vector);
		color.r = glm::clamp(abs(velocity.x * 0.25f) * 0.01f, 0.0f, 1.0f);
		color.g = glm::clamp(abs(velocity.y * 0.25f) * 0.01f, 0.0f, 1.0f);
		color.b = glm::clamp(abs(velocity.z * 0.25f) * 0.01f, 0.0f, 1.0f);

		line_trace_velocity.ptFrom = glm::to_vec3(pm->ps->origin);
		
		// realtime trace
		if (trace_type == 3)
		{ 
			line_trace_velocity.ptTo = line_trace_velocity.ptFrom + velocity;
			_debug::add_debug_line_client(line_trace_velocity.ptFrom, line_trace_velocity.ptTo, glm::to_vec4(dvars::pm_debug_lineColor->current.vector), dvars::pm_debug_lineDepth->current.enabled, frame_time);
		}

		else // trace with duration + scale down the length of velocity
		{ 
			line_trace_velocity.ptTo = line_trace_velocity.ptFrom + (velocity * 0.125f);
			_debug::add_debug_line_client(line_trace_velocity.ptFrom, line_trace_velocity.ptTo, color, dvars::pm_debug_lineDepth->current.enabled, dvars::pm_debug_lineDuration->current.integer);
		}
	}

	// add renderer command
	void trace_origin(Game::pmove_t* pm, bool on_ground)
	{
		// return when not in sync
		if (!_debug::debug_lines_sync(&sync_debug_lines)) 
		{
			return;
		}

		line_trace_origin.ptTo = glm::to_vec3(pm->ps->origin);

		// only trace when not on the ground but keep the last pos active
		if (dvars::pm_debug_traceOrigin->current.integer == 2 && on_ground) 
		{
			line_trace_origin.ptFrom = line_trace_origin.ptTo;
			return;
		}
		
		// do not add lines if the player did not move
		if (line_trace_origin.ptFrom == line_trace_origin.ptTo) 
		{
			return;
		}

		_debug::add_debug_line_client(line_trace_origin.ptFrom, line_trace_origin.ptTo, glm::to_vec4(dvars::pm_debug_lineColor->current.vector), dvars::pm_debug_lineDepth->current.enabled, dvars::pm_debug_lineDuration->current.integer);
		line_trace_origin.ptFrom = line_trace_origin.ptTo;
	}

	// x = forward, y = left, angle in degrees
	glm::vec3 circle_get_coord_for_angle(const glm::vec3& center, const float radius, const float angle)
	{
		// sin / cos uses radians ...
		const float radians = utils::vector::deg_to_rad(angle + 90);

		glm::vec3 coord;
		coord.x = radius * sinf(radians) + center.x;
		coord.y = radius * cosf(radians) + center.y;
		coord.z = center.z;

		return coord;
	}

	// render directly
	void draw_world_axis()
	{
		const auto origin = glm::to_vec3(Game::cgs->lastVieworg);
		const auto radius = dvars::pm_debug_drawAxis_radius->current.value;
		const auto height = dvars::pm_debug_drawAxis_height->current.value;
		const auto center = glm::vec3(origin.x, origin.y, height + origin.z);

		// fps zones (mode 2)
		if (dvars::pm_debug_drawAxis->current.integer == 2)
		{
			const int numZones = 16;
			Game::circleZone_t zones[numZones];

			// N ~ +x
			// W ~ +y
			// starting at -15° for first 125 zone (top)
			// 3 angles per zone (outer left, outer center, outer right)

			const std::vector<float> zoneAngles
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
				Game::Dvar_SetValue(dvars::pm_debug_drawAxis, 0);
				Game::Com_PrintMessage(0, "^1-> pm_debug_drawAxis ^7:: invalid circle zones.\n", 0);

				return;
			}

			for (auto z = 0; z < numZones; z++)
			{
				float* zoneColor;

				switch (z)
				{
				case 0: case 4: case 8: case 12:
					zoneColor = dvars::pm_debug_drawAxis_col125->current.vector;
					break;

				case 1: case 3: case 5: case 7: case 9: case 11: case 13: case 15:
					zoneColor = dvars::pm_debug_drawAxis_col250->current.vector;
					break;

				default:
					zoneColor = dvars::pm_debug_drawAxis_col333->current.vector;
				}

				// poly vertices ccw rotation starting at the center of the circle
				zones[z].zoneCoords[0] = center;
				zones[z].zoneCoords[1] = circle_get_coord_for_angle(center, radius, zoneAngles[z * 3 + 2]);
				zones[z].zoneCoords[2] = circle_get_coord_for_angle(center, radius, zoneAngles[z * 3 + 1]);
				zones[z].zoneCoords[3] = circle_get_coord_for_angle(center, radius, zoneAngles[z * 3 + 0]);

				_debug::draw_poly(
					/* numpts	*/ 4,
					/* points	*/ (float(*)[3]) & zones[z].zoneCoords,
					/* pcolor	*/ zoneColor,
					/* lit		*/ false,
					/* outlines */ false,
					/* lcolor	*/ nullptr,
					/* depth	*/ dvars::pm_debug_lineDepth->current.enabled,
					/* 2 sided	*/ false);
			}
		}

		Game::dbgLines_t lines[2];

		lines[0].ptFrom = origin - glm::vec3(radius, 0.0f, -height);
		lines[0].ptTo	= origin + glm::vec3(radius, 0.0f, height);

		lines[1].ptFrom = origin - glm::vec3(0.0f, radius, -height);
		lines[1].ptTo	= origin + glm::vec3(0.0f, radius, height);

		_debug::add_and_draw_debug_lines(
			/* linecount */ 2, 
			/* dbglines	 */ lines, 
			/* color	 */ dvars::pm_debug_lineColor->current.vector, 
			/* depth	 */ dvars::pm_debug_lineDepth->current.enabled,
			/* linewidth */ dvars::pm_debug_lineWidth->current.integer);
	}

	// Called from _Debug (render thread)
	void _pmove::draw_debug()
	{
		// draw axial "compass"
		if (dvars::pm_debug_drawAxis->current.integer != 0)
		{
			draw_world_axis();
		}
	}

	// -------------------------------------------------------------------------

	// Proxy Function for "PM_UpdateSprint" :: Grab and save local playerState variables
	// DO NOT USE FOR FUNCTIONS THAT SHOULD ONLY RUN ONCE PER FRAME ..
	void copy_local_playerstate_vars(Game::pmove_t* pm)
	{
		// Grab velocity for speedhud
		Game::Globals::lpmove_velocity = glm::vec3(pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);

		// Grab player origin and angles
		Game::Globals::lpmove_angles = glm::vec3(pm->ps->viewangles[0], pm->ps->viewangles[1], pm->ps->viewangles[2]);
		Game::Globals::lpmove_origin = glm::vec3(pm->ps->origin[0], pm->ps->origin[1], pm->ps->origin[2]);

		// Calculate camera origin
		Game::Globals::lpmove_camera_origin = Game::Globals::lpmove_origin + glm::vec3(0.0f, 0.0f, pm->ps->viewHeightCurrent);

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
	void pmove_client(Game::pmove_t *pm)
	{
		// remove me later
		radiant_livelink::receive_packets();

		const int server_time = pm->cmd.serverTime;
		const int command_time = pm->ps->commandTime;
		
		// if radiant cam-sync is active and using either game->radiant or game->radiant & radiant->game
		const bool radiant_camera_active = Game::Globals::cgs_addons.radiant_livelink_connected && (dvars::radiant_syncCamera->current.integer == 2 || dvars::radiant_syncCamera->current.integer == 3);
		int refresh_rate_ms = 0, current_diff = 0;

		if (components::active.radiant_livelink && radiant_camera_active)
		{
			// *
			// calculate client frametime

			// wait atleast 1 frame
			if (!Game::Globals::cgs_addons.radiant_camera_client_time_old)
			{
				Game::Globals::cgs_addons.radiant_camera_client_time_old = command_time;
			}
			else
			{
				current_diff = (command_time - Game::Globals::cgs_addons.radiant_camera_client_time_old);

				// add the previous frametime to our counter
				Game::Globals::cgs_addons.radiant_camera_client_msec_passed += current_diff;

				// save the current time for the next frame
				Game::Globals::cgs_addons.radiant_camera_client_time_old = command_time;
			}

			int div = 16; // assume 60hz

			if (const auto	refresh_rate = Game::Dvar_FindVar("r_displayRefresh"); 
							refresh_rate)
			{
				div = utils::extract_first_integer_from_string(refresh_rate->domain.enumeration.strings[refresh_rate->current.integer]);
			}

			// we need the screen refreshrate to limit the packet-rate
			refresh_rate_ms = 1000 / div;
		}
		
		// stock pmove
		if (server_time >= command_time)
		{
			if (server_time > command_time + 1000) 
			{
				pm->ps->commandTime = server_time - 1000;
			}

			pm->numtouch = 0;

			if (pm->ps->commandTime != server_time)
			{
				while (true)
				{
					int msec = server_time - pm->ps->commandTime;

					if (msec > 66) 
					{
						msec = 66;
					}

					pm->cmd.serverTime = msec + pm->ps->commandTime;
					Game::PmoveSingle(pm);

					// -------------------------
					// pmove debug tracelines
					
					if (dvars::pm_debug_traceOrigin->current.enabled || dvars::pm_debug_traceVelocity->current.enabled)
					{
						// init and create 1 timer for all debug lines, timer is true for 1 frame
						_debug::debug_lines_init_sync(&sync_debug_lines);

						bool on_ground = false;

						if (pm->ps->groundEntityNum == 1022) 
						{
							on_ground = true;
						} 

						// trace player origin
						if (dvars::pm_debug_traceOrigin->current.integer)
						{
							trace_origin(pm, on_ground);
						}

						// trace player velocity
						if (dvars::pm_debug_traceVelocity->current.integer)
						{
							trace_velocity(pm, on_ground, msec);
						}
					}

					// -------------------------
					// game->radiant cam control

					if (components::active.radiant_livelink && radiant_camera_active)
					{
						const float temp_camera_origin[3] =
						{
							pm->ps->origin[0],
							pm->ps->origin[1],
							pm->ps->origin[2] + pm->ps->viewHeightCurrent
						};

						/*if (!Game::Globals::cgsAddons.radiant_camera_origin_old)
						{
							memcpy(&Game::Globals::cgsAddons.radiant_camera_origin_old, temp_camera_origin, sizeof(int[3]));
						}*/

						// compare last frames origin/viewpos with the current origin/viewpos
						if (!utils::vector::compare_int3(pm->cmd.angles, pm->oldcmd.angles) || !utils::vector::compare3(temp_camera_origin, Game::Globals::cgs_addons.radiant_camera_origin_old))
						{
							memcpy(&Game::Globals::cgs_addons.radiant_camera_origin_old, temp_camera_origin, sizeof(int[3]));

							const float temp_camera_angles[3] =
							{
								-pm->ps->viewangles[0],
								pm->ps->viewangles[1],
								pm->ps->viewangles[2]
							};

							// always if client ms/frame is greater then ms/screen-refreshrate (no packet-rate capping needed)
							// OR if client ms/frame is smaller then ms/screen-refreshrate and we did not cap the packet-rate yet
							if (current_diff >= refresh_rate_ms || Game::Globals::cgs_addons.radiant_camera_client_msec_passed <= refresh_rate_ms && !Game::Globals::cgs_addons.radiant_camera_capped_packets)
							{
								Game::Globals::cgs_addons.radiant_camera_capped_packets = true;
								radiant_livelink::cmd_send_camera(temp_camera_origin, temp_camera_angles);
							}

							// allow a new packet if client passed the ms/screen-refreshrate
							else if (Game::Globals::cgs_addons.radiant_camera_client_msec_passed >= refresh_rate_ms)
							{
								Game::Globals::cgs_addons.radiant_camera_client_msec_passed = 0;
								Game::Globals::cgs_addons.radiant_camera_capped_packets = false;
							}
						}
					}

					// -----------
					// stock pmove

					memcpy(&pm->oldcmd, &pm->cmd, sizeof(pm->oldcmd));

					if (pm->ps->commandTime == server_time) 
					{
						break;
					}
				}
			}
		}
	}

	// pmove used for prediction / spectator (when connected to a server)
	void pmove_spec_predict(Game::pmove_t *pm)
	{
		const int server_time = pm->cmd.serverTime;
		const int command_time = pm->ps->commandTime;

		if (server_time >= command_time)
		{
			if (server_time > command_time + 1000) 
			{
				pm->ps->commandTime = server_time - 1000;
			}

			pm->numtouch = 0;
			if (pm->ps->commandTime != server_time)
			{
				while (true)
				{
					int msec = server_time - pm->ps->commandTime;
					if (msec > 66) 
					{
						msec = 66;
					}

					pm->cmd.serverTime = msec + pm->ps->commandTime;
					Game::PmoveSingle(pm);

					memcpy(&pm->oldcmd, &pm->cmd, sizeof(pm->oldcmd));

					if (pm->ps->commandTime == server_time) 
					{
						break;
					}
				}
			}
		}
	}

	_pmove::_pmove()
	{
		// hook Pmove :x
		utils::hook(0x4476A3, pmove_spec_predict, HOOK_CALL).install()->quick(); // CG_PredictPlayerState_Internal // Pmove_SpecPredict
		utils::hook(0x4A7DA6, pmove_spec_predict, HOOK_CALL).install()->quick(); // SpectatorThink // Pmove_SpecPredict
		utils::hook(0x4A8AD3, pmove_client, HOOK_CALL).install()->quick(); // ClientThink_real

		//// hook PM_UpdateSprint Call in PmoveSingle (PM_TYPE 0) (DEAD)?					- Used to grab local playerstate variables
		utils::hook(0x41482A, copy_local_playerstate_vars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (PM_FLAGS 4) (SPECTATOR)?
		utils::hook(0x4148D0, copy_local_playerstate_vars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (PM_FLAGS 2) (NOCLIP)
		utils::hook(0x414936, copy_local_playerstate_vars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (PM_FLAGS 3) (UFO)
		utils::hook(0x414995, copy_local_playerstate_vars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (PM_FLAGS 5)
		utils::hook(0x4149D9, copy_local_playerstate_vars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (PM_FLAGS 8) (FREEZE DEAD)?
		utils::hook(0x414A29, copy_local_playerstate_vars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (PM_FLAGS NOT 4)
		utils::hook(0x414A85, copy_local_playerstate_vars, HOOK_CALL).install()->quick();

		//// PM_UpdateSprint Call in PmoveSingle (MANTLE)
		utils::hook(0x414AF3, copy_local_playerstate_vars, HOOK_CALL).install()->quick();

		// -----------------------------------------------------------------

		dvars::pm_debug_traceOrigin = Game::Dvar_RegisterInt(
			/* name		*/ "pm_debug_traceOrigin",
			/* desc		*/ "trace the players origin.\n0: disabled\n1: trace and display for duration\n2: trace and display for duration when not on the ground",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 2,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_debug_traceVelocity = Game::Dvar_RegisterInt(
			/* name		*/ "pm_debug_traceVelocity",
			/* desc		*/ "trace the players velocity.\n0: disabled\n1: trace and display for duration\n2: trace and display for duration when not on the ground\n3: trace in realtime",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 3,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_debug_lineDepth = Game::Dvar_RegisterBool(
			/* name		*/ "pm_debug_lineDepth",
			/* desc		*/ "if the trace should perform a depth check",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_debug_lineColor = Game::Dvar_RegisterVec4(
			/* name		*/ "pm_debug_lineColor",
			/* desc		*/ "color of debug lines for movement related functions",
			/* x		*/ 1.0f,
			/* y		*/ 0.3f,
			/* z		*/ 0.4f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_debug_lineDuration = Game::Dvar_RegisterInt(
			/* name		*/ "pm_debug_lineDuration",
			/* desc		*/ "how long the trace persists",
			/* default	*/ 500,
			/* minVal	*/ 0,
			/* maxVal	*/ 5000,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_debug_lineWidth = Game::Dvar_RegisterInt(
			/* name		*/ "pm_debug_lineWidth",
			/* desc		*/ "with of debug lines",
			/* default	*/ 1,
			/* minVal	*/ 1,
			/* maxVal	*/ 20,
			/* flags	*/ Game::dvar_flags::saved);


		dvars::pm_debug_drawAxis = Game::Dvar_RegisterInt(
			/* name		*/ "pm_debug_drawAxis",
			/* desc		*/ "draw axial information\n1: draw world axial lines only\n2: draw additional fps zones (default colors >> Green 125; Cyan 250; Blue 333)",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 2,
			/* flags	*/ Game::dvar_flags::none);

		dvars::pm_debug_drawAxis_col125 = Game::Dvar_RegisterVec4(
			/* name		*/ "pm_debug_drawAxis_col125",
			/* desc		*/ "color of 125 fps zone",
			/* x		*/ 0.4f,
			/* y		*/ 1.0f,
			/* z		*/ 0.3f,
			/* w		*/ 0.6f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_debug_drawAxis_col250 = Game::Dvar_RegisterVec4(
			/* name		*/ "pm_debug_drawAxis_col250",
			/* desc		*/ "color of 250 fps zone",
			/* x		*/ 0.32f,
			/* y		*/ 1.00f,
			/* z		*/ 0.99f,
			/* w		*/ 0.6f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_debug_drawAxis_col333 = Game::Dvar_RegisterVec4(
			/* name		*/ "pm_debug_drawAxis_col333",
			/* desc		*/ "color of 333 fps zone",
			/* x		*/ 0.17f,
			/* y		*/ 0.36f,
			/* z		*/ 1.00f,
			/* w		*/ 0.6f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_debug_drawAxis_radius = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_debug_drawAxis_radius",
			/* desc		*/ "radius of axial lines / zones",
			/* default	*/ 40.0f,
			/* minVal	*/ -1000.0f,
			/* maxVal	*/ 1000.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::pm_debug_drawAxis_height = Game::Dvar_RegisterFloat(
			/* name		*/ "pm_debug_drawAxis_height",
			/* desc		*/ "player origin + height",
			/* default	*/ 30.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 200.0f,
			/* flags	*/ Game::dvar_flags::saved);
	}
}