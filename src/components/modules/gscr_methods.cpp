#include "std_include.hpp"

#include "rtx/remix_vars.hpp"

#define SERVERSTATIC_STRUCT_ADDR 0x1CBFC80 // correct, we start at initialized, cba to get real start address
#define svs (*((game::serverStatic_t*)(SERVERSTATIC_STRUCT_ADDR)))

#define SERVER_STRUCT_ADDR 0x17FC7C8 // correct
#define sv (*((game::server_t*)(SERVER_STRUCT_ADDR))) // def. 0x17FC7C8

game::scr_function_t* scr_functions = nullptr;
game::scr_function_t *scr_methods = nullptr; // only for PlayerMethods


namespace components
{
	//bool scr_add_function(const char* cmd_name, xfunction_t function, bool developer) 
	//{
	//	game::scr_function_t* cmd;

	//	// fail if the command already exists
	//	for (cmd = scr_functions; cmd; cmd = cmd->next) 
	//	{
	//		if (!strcmp(cmd_name, cmd->name)) 
	//		{
	//			// allow completion-only commands to be silently doubled
	//			if (function != nullptr)
	//			{
	//				game::Com_PrintMessage(0, utils::va("scr_add_function: %s already defined\n", cmd_name), 0);
	//			}

	//			return false;
	//		}
	//	}

	//	// use a small malloc to avoid zone fragmentation
	//	cmd = static_cast<game::scr_function_t*> (malloc(sizeof(game::scr_function_t) + strlen(cmd_name) + 1));

	//	if (cmd && cmd + 0x1)
	//	{
	//		strcpy((char*)(cmd + 1), cmd_name);
	//		cmd->name = (char*)(cmd + 1);
	//	}
	//	else
	//	{
	//		return false;
	//	}

	//	cmd->function = function;
	//	cmd->developer = developer;
	//	cmd->next = scr_methods;
	//	scr_methods = cmd;

	//	return true;
	//}

	bool add_function(std::string cmd_name, xfunction_t function, bool developer)
	{
		game::scr_function_s* cmd;
		cmd_name = utils::str_to_lower(cmd_name);

		for (cmd = scr_functions; cmd; cmd = cmd->next) 
		{
			if (!strcmp(cmd_name.c_str(), cmd->name)) 
			{
				// allow completion-only commands to be silently doubled
				if (function != nullptr) {
					game::Com_PrintMessage(0, utils::va("Scr_AddFunction: %s already defined\n", cmd_name.c_str()), 0);
				}
				return false;
			}
		}

		// use a small malloc to avoid zone fragmentation
		if (cmd = (game::scr_function_s*)malloc(sizeof(game::scr_function_s) + strlen(cmd_name.c_str()) + 1);
			!cmd)
		{
			game::Com_PrintMessage(0, utils::va("Scr_AddFunction: %s failed to allocate memory\n", cmd_name.c_str()), 0);
			return false;
		}


		strcpy((char*)(cmd + 1), cmd_name.c_str());
		cmd->name = (char*)(cmd + 1);
		cmd->function = function;
		cmd->developer = developer;
		cmd->next = scr_functions;
		scr_functions = cmd;
		return true;
	}

	bool add_method(std::string cmd_name, xfunction_t function, bool developer)
	{
		game::scr_function_t* cmd;
		cmd_name = utils::str_to_lower(cmd_name);

		// fail if the command already exists
		for (cmd = scr_methods; cmd; cmd = cmd->next)
		{
			if (!strcmp(cmd_name.c_str(), cmd->name))
			{
				// allow completion-only commands to be silently doubled
				if (function != nullptr) {
					game::Com_PrintMessage(0, utils::va("Scr_AddMethod: %s already defined\n", cmd_name.c_str()), 0);
				}

				return false;
			}
		}

		// use a small malloc to avoid zone fragmentation
		cmd = static_cast<game::scr_function_t*> (malloc(sizeof(game::scr_function_t) + strlen(cmd_name.c_str()) + 1));

		if (cmd && cmd + 0x1)
		{
			strcpy((char*)(cmd + 1), cmd_name.c_str());
			cmd->name = (char*)(cmd + 1);
		}
		else
		{
			return false;
		}

		cmd->function = function;
		cmd->developer = developer;
		cmd->next = scr_methods;
		scr_methods = cmd;

		return true;
	}

	void scr_error(const char *error)
	{
		*game::scrVarPub_p4 = 1;								// display type?

		if (!*game::scrVarPub)									// if no pointer to error_message
		{
			strncpy(*&game::error_message, error, 1023u);		// copy errorText to location of error_message
			*game::errortype = 0;								// what does that do?
			*game::scrVarPub = (DWORD)(game::error_message);	// scrVarPub pointing to location of error_message
		}

		game::Scr_Error_Internal();
	}

	// *
	// Get playerstate for clientNum
	game::playerState_s *get_playerstate_for_sv_client(int num) 
	{
		return (game::playerState_s *)((BYTE*)sv.gameClients + sv.gameClientSize * (num));
	}


	// -------------------
	// Custom GScr Methods

	// *
	// Method :: wild LNR hiding in the bushes
	void playercmd_lnr(scr_entref_t arg)
	{
		// just print sth to the console for now
		game::Com_PrintMessage(0, utils::va("Scr_AddMethod: ^3LNR is hiding in the bushes!\n"), 0);
	}

	// *
	// method :: SetVelocity
	void playercmd_set_velocity(scr_entref_t arg) 
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else 
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity	= &game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if(*game::scr_numParam != 1) 
		{
			scr_error("Usage: self setVelocity( <Vec3> )\n");
		}

		const auto cl = &svs.clients[entityNum];
		game::Scr_GetVector(0, get_playerstate_for_sv_client(cl - svs.clients)->velocity);
	}

	// *
	// method :: SprintLeftButtonPressed
	void playercmd_sprint_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*game::scr_numParam != 0) 
		{
			scr_error("Usage: self sprintButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		game::Scr_AddInt(game::is_button_pressed(2, cl->lastUsercmd.buttons));
	}

	// *
	// method :: LeanLeftButtonPressed
	void playercmd_lean_left_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*game::scr_numParam != 0) 
		{
			scr_error("Usage: self leanLeftButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		game::Scr_AddInt(game::is_button_pressed(64, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: LeanRightButtonPressed
	void playercmd_lean_right_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*game::scr_numParam != 0) 
		{
			scr_error("Usage: self leanRightButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		game::Scr_AddInt(game::is_button_pressed(128, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: ReloadButtonPressed
	void playercmd_reload_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*game::scr_numParam != 0) 
		{
			scr_error("Usage: self reloadButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		game::Scr_AddInt(game::is_button_pressed(32, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: JumpButtonPressed
	void playercmd_jump_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*game::scr_numParam != 0) 
		{
			scr_error("Usage: self jumpButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		game::Scr_AddInt(game::is_button_pressed(1024, cl->lastUsercmd.buttons));
	}

	// *
	// Method :: ForwardButtonPressed
	void playercmd_forward_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*game::scr_numParam != 0) 
		{
			scr_error("Usage: self forwardButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		game::Scr_AddInt(game::is_button_pressed(127, cl->lastUsercmd.forwardmove));
	}

	// *
	// Method :: BackButtonPressed
	void playercmd_back_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*game::scr_numParam != 0) 
		{
			scr_error("Usage: self backButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		game::Scr_AddInt(game::is_button_pressed(129, cl->lastUsercmd.forwardmove));
	}

	// *
	// Method :: RightButtonPressed
	void playercmd_right_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*game::scr_numParam != 0) 
		{
			scr_error("Usage: self rightButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		game::Scr_AddInt(game::is_button_pressed(127, cl->lastUsercmd.rightmove));
	}

	// *
	// Method :: LeftButtonPressed
	void playercmd_left_button_pressed(scr_entref_t arg)
	{
		std::int32_t entityNum = 0;

		if (HIWORD(arg)) 
		{
			game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*game::scr_numParam != 0) 
		{
			scr_error("Usage: self leftButtonPressed()\n");
		}

		const auto cl = &svs.clients[entityNum];
		game::Scr_AddInt(cl->lastUsercmd.rightmove == 129);
	}

	// *
	// Method :: CheckJump
	void playercmd_check_jump(scr_entref_t arg)
	{
		if (HIWORD(arg)) 
		{
			game::Scr_ObjectError("Not an entity"); // if arg.classnum
		}

		else
		{
			const std::int32_t entityNum = LOWORD(arg); // arg.entnum
			const auto gentity = &game::scr_g_entities[entityNum];

			if (!gentity->client) 
			{
				game::Scr_ObjectError(utils::va("Entity: %i is not a player", entityNum));
			}
		}

		if (*game::scr_numParam != 0) 
		{
			scr_error("Usage: self checkJump()\n");
		}

		game::Scr_AddInt(game::glob::lpmove_check_jump);
	}

	void fn_rtxSetConfig(/*[[maybe_unused]] scr_entref_t arg*/)
	{
		if (*game::scr_numParam < 2) {
			scr_error("Usage: self rtxSetConfig( <\"config name\">, <transition_time>, (opt.) <start_delay> (opt.), <transition_back_delay> )\n");
		}

		/*const auto cl = &svs.clients[entityNum];
		game::Scr_GetVector(0, get_playerstate_for_sv_client(cl - svs.clients)->velocity);*/

		const auto conf_name = game::Scr_GetString(0u);
		const auto transition_time = game::Scr_GetFloat(1u);

		auto start_delay_time = 0.0f;
		if (*game::scr_numParam > 2) {
			start_delay_time = game::Scr_GetFloat(2u);
		}

		auto transition_back_delay = 0.0f;
		if (*game::scr_numParam > 3) {
			transition_back_delay = game::Scr_GetFloat(3u);
		}

		std::string conf_name_str = conf_name;
		conf_name_str += ".conf";

		remix_vars::parse_and_apply_conf_with_lerp(conf_name_str, utils::string_hash64(conf_name_str), remix_vars::EASE_TYPE_CUBIC_IN, transition_time, start_delay_time, transition_back_delay);
	}

	void fn_rtxReloadMapSettings()
	{
		rtx_map_settings::get()->set_settings_for_loaded_map(true);
	}

	void fn_rtxResetVarsToLevel(/*[[maybe_unused]] scr_entref_t arg*/)
	{
		if (*game::scr_numParam < 1) {
			scr_error("Usage: self rtxResetVarsToLevel( <transition_time>, (opt.) <start_delay> (opt.) )\n");
		}

		const auto transition_time = game::Scr_GetFloat(0u);

		auto start_delay_time = 0.0f;
		if (*game::scr_numParam > 1) {
			start_delay_time = game::Scr_GetFloat(1u);
		}

		remix_vars::transition_all_to_level_state(transition_time, start_delay_time);
	}

	//bool g_gscrRtxTimeCyleInProgress = false;

	void fn_rtxTimeCycle()
	{
		if (*game::scr_numParam < 2) {
			scr_error("Usage: rtxTimeCycle( <\"time_str\">, <transition_time>) - time_str = sunrise, day, sunset, night\n");
		}

		const std::string_view time_str = game::Scr_GetString(0u);
		const float transition_time = game::Scr_GetFloat(1u);

		rtx_lights::rtx_debug_lights[0].enable = true;

		// # create options with start settings
		remix_vars::option_s sun_scale = { remix_vars::OPTION_TYPE_FLOAT, {} };
		sun_scale.current.value = rtx_lights::rtx_debug_lights[0].color_scale;
		sun_scale.reset = sun_scale.current;
		sun_scale.reset_level = sun_scale.current;
		sun_scale.not_a_remix_var = true;

		remix_vars::option_s sun_dir = { remix_vars::OPTION_TYPE_VEC3, {} };
		sun_dir.current.vector[0] = rtx_lights::rtx_debug_lights[0].dir[0];
		sun_dir.current.vector[1] = rtx_lights::rtx_debug_lights[0].dir[1];
		sun_dir.current.vector[2] = rtx_lights::rtx_debug_lights[0].dir[2];
		sun_dir.reset = sun_dir.current;
		sun_dir.reset_level = sun_dir.current;
		sun_dir.not_a_remix_var = true; // let interpolation code know that this is not a remix variable

		remix_vars::option_s sun_color = sun_dir; // lazy copy
		sun_color.current.vector[0] = rtx_lights::rtx_debug_lights[0].color[0];
		sun_color.current.vector[1] = rtx_lights::rtx_debug_lights[0].color[1];
		sun_color.current.vector[2] = rtx_lights::rtx_debug_lights[0].color[2];

		remix_vars::option_s sky_rot = sun_color; // lazy copy
		sky_rot.current.vector[0] = rtx_gui::skysphere_model_rotation[0];
		sky_rot.current.vector[1] = rtx_gui::skysphere_model_rotation[1];
		sky_rot.current.vector[2] = rtx_gui::skysphere_model_rotation[2];

		const auto ease_type = remix_vars::EASE_TYPE::EASE_TYPE_LINEAR;

		if (time_str == "sunrise")
		{
			// coming from night
			//remix_vars::set_option(remix_vars::get_option("rtx.skyBrightness"), remix_vars::string_to_option_value(remix_vars::OPTION_TYPE_FLOAT, "0.01"));
			rtx_gui::skysphere_spawn(rtx_gui::SKY::SUNSET);

			if (const auto handle = remix_vars::add_custom_option("#SUN_SCALE", sun_scale); handle)
			{
				remix_vars::option_value goal = { .value = 0.214f };
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_SCALE"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SUN_DIR", sun_dir); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 1.244f; goal.vector[1] = -0.370f; goal.vector[2] = -0.316f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_DIR"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SUN_COLOR", sun_color); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 238.0f / 255.0f; goal.vector[1] = 211.0f / 255.0f; goal.vector[2] = 156.0f / 255.0f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_COLOR"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SKY_ROT", sky_rot); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 0.0f; goal.vector[1] = 0.0f; goal.vector[2] = 0.0f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SKY_ROT"), handle, goal, transition_time, 0, 0, ease_type);
			}

			remix_vars::parse_and_apply_conf_with_lerp("sunset.conf", utils::string_hash64("sunset"), ease_type, transition_time);
		}

		else if(time_str == "day")
		{
			rtx_gui::skysphere_spawn(rtx_gui::SKY::DESERT);

			if (const auto handle = remix_vars::add_custom_option("#SUN_SCALE", sun_scale); handle)
			{
				remix_vars::option_value goal = { .value = 5.0f };
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_SCALE"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SUN_DIR", sun_dir); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 0.206f; goal.vector[1] = -0.242f; goal.vector[2] = -0.479f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_DIR"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SUN_COLOR", sun_color); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 0.933f; goal.vector[1] = 0.87f; goal.vector[2] = 0.60f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_COLOR"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SKY_ROT", sky_rot); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 10.75f; goal.vector[1] = -23.75f; goal.vector[2] = 0.0f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SKY_ROT"), handle, goal, transition_time, 0, 0, ease_type);
			}

			remix_vars::parse_and_apply_conf_with_lerp("daytime.conf", utils::string_hash64("daytime"), ease_type, transition_time);
		}

		else if (time_str == "sunset")
		{
			rtx_gui::skysphere_spawn(rtx_gui::SKY::SUNSET);

			if (const auto handle = remix_vars::add_custom_option("#SUN_SCALE", sun_scale); handle)
			{
				remix_vars::option_value goal = { .value = 1.850f };
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_SCALE"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SUN_DIR", sun_dir); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = -0.279f; goal.vector[1] = -0.316f; goal.vector[2] = -0.121f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_DIR"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SUN_COLOR", sun_color); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 205.0f / 255.0f; goal.vector[1] = 167.0f / 255.0f; goal.vector[2] = 115.0f / 255.0f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_COLOR"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SKY_ROT", sky_rot); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 10.75f; goal.vector[1] = -105.00f; goal.vector[2] = 0.0f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SKY_ROT"), handle, goal, transition_time, 0, 0, ease_type);
			}

			remix_vars::parse_and_apply_conf_with_lerp("sunset.conf", utils::string_hash64("sunset"), ease_type, transition_time);
		}

		else if (time_str == "night")
		{
			rtx_gui::skysphere_spawn(rtx_gui::SKY::NIGHT);

			if (const auto handle = remix_vars::add_custom_option("#SUN_SCALE", sun_scale); handle)
			{
				remix_vars::option_value goal = { .value = 0.016f };
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_SCALE"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SUN_DIR", sun_dir); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 1.721f; goal.vector[1] = -0.544f; goal.vector[2] = -0.726f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_DIR"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SUN_COLOR", sun_color); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 172.0f / 255.0f; goal.vector[1] = 217.0f / 255.0f; goal.vector[2] = 234.0f / 255.0f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_COLOR"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SKY_ROT", sky_rot); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 0.0f; goal.vector[1] = 0.0f; goal.vector[2] = 0.0f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SKY_ROT"), handle, goal, transition_time, 0, 0, ease_type);
			}

			remix_vars::parse_and_apply_conf_with_lerp("night.conf", utils::string_hash64("night"), ease_type, transition_time);
		}

		if (time_str == "overcast")
		{
			rtx_gui::skysphere_spawn(rtx_gui::SKY::OVERCAST);

			if (const auto handle = remix_vars::add_custom_option("#SUN_SCALE", sun_scale); handle)
			{
				remix_vars::option_value goal = { .value = 0.0f };
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_SCALE"), handle, goal, transition_time, 0, 0, ease_type);
			}

			if (const auto handle = remix_vars::add_custom_option("#SKY_ROT", sky_rot); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 0.0f; goal.vector[1] = 0.0f; goal.vector[2] = 0.0f;
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SKY_ROT"), handle, goal, transition_time, 0, 0, ease_type);
			}

			remix_vars::parse_and_apply_conf_with_lerp("overcast.conf", utils::string_hash64("overcast"), ease_type, transition_time);
		}
	}

	/*void fn_rtxIsTimeCycleDone()
	{
		game::Scr_AddInt(g_gscrRtxTimeCyleInProgress);
	}*/

	void fn_rtxTransitionDay()
	{
		const auto ease_type = remix_vars::EASE_TYPE::EASE_TYPE_SIN_OUT;
		const float transition_time = 360.0f; 
		rtx_lights::rtx_debug_lights[0].color_scale = -0.8f; // start negative to ease in more
		rtx_lights::rtx_debug_lights[0].enable = true;
		rtx_lights::rtx_debug_lights[0].dir[0] = 0.676f;
		rtx_lights::rtx_debug_lights[0].dir[1] = 0.182f;
		rtx_lights::rtx_debug_lights[0].dir[2] = -0.07f;

		rtx_gui::skysphere_model_rotation[0] = -45.0f;
		rtx_gui::skysphere_model_rotation[1] = 30.0f;
		rtx_gui::skysphere_model_rotation[2] = 0.0f;
		rtx_gui::skysphere_spawn(rtx_gui::SKY::DESERT);

		rtx_lights::rtx_debug_lights[0].color[0] = 0.78f;
		rtx_lights::rtx_debug_lights[0].color[1] = 0.85f;
		rtx_lights::rtx_debug_lights[0].color[2] = 0.85f;

		remix_vars::set_option(remix_vars::get_option("rtx.skyBrightness"), remix_vars::string_to_option_value(remix_vars::OPTION_TYPE_FLOAT, "0.01"));
#if 1
		remix_vars::parse_and_apply_conf_with_lerp("daytime.conf", utils::string_hash64("daytime"), ease_type, transition_time);
		
		{
			// # create custom option with start settings
			remix_vars::option_s o = { remix_vars::OPTION_TYPE_FLOAT, {} };
			o.current.value = rtx_lights::rtx_debug_lights[0].color_scale;
			o.reset = o.current;
			o.reset_level = o.current;
			o.not_a_remix_var = true; // let interpolation code know that this is not a remix variable

			if (const auto handle = remix_vars::add_custom_option("#SUN_SCALE", o); handle)
			{
				remix_vars::option_value goal = { .value = 5.0 };
				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_SCALE"), handle, goal, transition_time, 0, 0, ease_type);
			}
		}

		{
			// # create custom option with start settings
			remix_vars::option_s o = { remix_vars::OPTION_TYPE_VEC3, {} };
			o.current.vector[0] = rtx_lights::rtx_debug_lights[0].dir[0];
			o.current.vector[1] = rtx_lights::rtx_debug_lights[0].dir[1];
			o.current.vector[2] = rtx_lights::rtx_debug_lights[0].dir[2];
			o.reset = o.current;
			o.reset_level = o.current;
			o.not_a_remix_var = true; // let interpolation code know that this is not a remix variable

			if (const auto handle = remix_vars::add_custom_option("#SUN_DIR", o); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 0.206f;
				goal.vector[1] = -0.242f;
				goal.vector[2] = -0.479f;

				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_DIR"), handle, goal, transition_time, 0, 0, ease_type);
			}
		}

		{
			// # create custom option with start settings
			remix_vars::option_s o = { remix_vars::OPTION_TYPE_VEC3, {} };
			o.current.vector[0] = rtx_lights::rtx_debug_lights[0].color[0];
			o.current.vector[1] = rtx_lights::rtx_debug_lights[0].color[1];
			o.current.vector[2] = rtx_lights::rtx_debug_lights[0].color[2];
			o.reset = o.current;
			o.reset_level = o.current;
			o.not_a_remix_var = true; // let interpolation code know that this is not a remix variable

			if (const auto handle = remix_vars::add_custom_option("#SUN_COLOR", o); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 0.933f;
				goal.vector[1] = 0.87f;
				goal.vector[2] = 0.60f;

				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SUN_COLOR"), handle, goal, transition_time, 0, 0, ease_type);
			}
		}

		{
			// # create custom option with start settings
			remix_vars::option_s o = { remix_vars::OPTION_TYPE_VEC3, {} };
			o.current.vector[0] = rtx_gui::skysphere_model_rotation[0];
			o.current.vector[1] = rtx_gui::skysphere_model_rotation[1];
			o.current.vector[2] = rtx_gui::skysphere_model_rotation[2];
			o.reset = o.current;
			o.reset_level = o.current;
			o.not_a_remix_var = true; // let interpolation code know that this is not a remix variable

			if (const auto handle = remix_vars::add_custom_option("#SKY_ROT", o); handle)
			{
				remix_vars::option_value goal = {};
				goal.vector[0] = 10.75f;
				goal.vector[1] = -23.75f;
				goal.vector[2] = 0.0f;

				remix_vars::get().add_interpolate_entry(utils::string_hash64("#SKY_ROT"), handle, goal, transition_time, 0, 0, ease_type);
			}
		}
#endif
	}

	void fn_rtxIsUnderwater()
	{
		game::Scr_AddInt(g_compmod_water_underwater);
	}

	void playercmd_rtxToggleFlashlight([[maybe_unused]]  scr_entref_t arg)
	{
		rtx_lights::rtx_debug_lights[1].type = D3DLIGHT_SPOT;
		rtx_lights::rtx_debug_lights[1].attach_to_weapon = true;
		rtx_lights::rtx_debug_lights[1].attach_to_head = false;
		rtx_lights::rtx_debug_lights[1].color_scale = 30.0f;
		rtx_lights::rtx_debug_lights[1].inner_angle = 16.0f;
		rtx_lights::rtx_debug_lights[1].outer_angle = 40.0f;
		rtx_lights::rtx_debug_lights[1].enable = !rtx_lights::rtx_debug_lights[1].enable;

		rtx_lights::rtx_debug_lights[2].type = D3DLIGHT_SPOT;
		rtx_lights::rtx_debug_lights[2].attach_to_weapon = true;
		rtx_lights::rtx_debug_lights[2].attach_to_head = false;
		rtx_lights::rtx_debug_lights[2].color_scale = 80.0f;
		rtx_lights::rtx_debug_lights[2].inner_angle = 14.5f;
		rtx_lights::rtx_debug_lights[2].outer_angle = 28.0f;
		rtx_lights::rtx_debug_lights[2].enable = !rtx_lights::rtx_debug_lights[2].enable;

		//game::Cmd_ExecuteSingleCommand(0, 0, "snd_playlocal rtxflashlight\n");
	}

	void fn_rtxEnableDebugLight()
	{
		if (*game::scr_numParam < 2) 
		{
			scr_error("Usage: rtxEnableDebugLight( <light_num>, <on/off (0/1)>)\n");
			return;
		}

		const int index = static_cast<int>(game::Scr_GetFloat(0u));
		const int state = static_cast<int>(game::Scr_GetFloat(1u));

		if (index >= rtx_lights::RTX_DEBUGLIGHT_AMOUNT) 
		{
			scr_error("rtxEnableDebugLight - light_num out of range\n");
			return;
		}

		rtx_lights::rtx_debug_lights[index].enable = state;
	}

	// *
	// Add GScr Methods
	void add_stock_player_methods() 
	{
		add_method("LNR",						(xfunction_t)(playercmd_lnr), 0);
		add_method("setvelocity",				(xfunction_t)(playercmd_set_velocity), 0);
		add_method("sprintButtonPressed",		(xfunction_t)(playercmd_sprint_button_pressed), 0);
		add_method("leanLeftButtonPressed",		(xfunction_t)(playercmd_lean_left_button_pressed), 0);
		add_method("leanRightButtonPressed",		(xfunction_t)(playercmd_lean_right_button_pressed), 0);
		add_method("reloadButtonPressed",		(xfunction_t)(playercmd_reload_button_pressed), 0);
		add_method("jumpButtonPressed",			(xfunction_t)(playercmd_jump_button_pressed), 0);
		add_method("forwardButtonPressed",		(xfunction_t)(playercmd_forward_button_pressed), 0);
		add_method("backButtonPressed",			(xfunction_t)(playercmd_back_button_pressed), 0);
		add_method("rightButtonPressed",			(xfunction_t)(playercmd_right_button_pressed), 0);
		add_method("leftButtonPressed",			(xfunction_t)(playercmd_left_button_pressed), 0);
		add_method("checkJump",					(xfunction_t)(playercmd_check_jump), 0);

		if (game::glob::has_rtx_flag)
		{
			//add_method("rtxSetConfig", (xfunction_t)(playercmd_rtxSetConfig), 0);
			//add_method("rtxResetVarsToLevel", (xfunction_t)(playercmd_rtxResetVarsToLevel), 0);

			add_function("rtxSetConfig", (xfunction_t)fn_rtxSetConfig, false);
			add_function("rtxReloadMapSettings", (xfunction_t)fn_rtxReloadMapSettings, false);
			add_function("rtxResetVarsToLevel", (xfunction_t)fn_rtxResetVarsToLevel, false);
		}

		if (game::glob::has_rtx_comp_flag) 
		{
			add_function("rtxTimeCycle", (xfunction_t)fn_rtxTimeCycle, false);
			add_function("rtxTransitionDay", (xfunction_t)fn_rtxTransitionDay, false);
			add_function("rtxIsUnderwater", (xfunction_t)fn_rtxIsUnderwater, false);
			add_method("rtxToggleFlashlight", (xfunction_t)(playercmd_rtxToggleFlashlight), 0);
			add_function("rtxEnableDebugLight", (xfunction_t)fn_rtxEnableDebugLight, false);
		}
	}

	// *
	// Gets called if no stock player-method has been found
	void* player_get_custom_method(const char **v_functionName)
	{
		game::scr_function_t *cmd;

		for (cmd = scr_methods; cmd != NULL; cmd = cmd->next)
		{
			if (!_stricmp(*v_functionName, cmd->name))
			{
				//*v_developer = cmd->developer;
				*v_functionName = cmd->name;
				return cmd->function;
			}
		}

		return nullptr; // continue searching with the next method types
	}

	// ASM :: Scr_GetMethod :: implement Player_GetCustomMethod
	__declspec(naked) void Scr_GetMethod_stub()
	{
		const static uint32_t Player_GetMethod_Stock_func = 0x4B1FC0;
		const static uint32_t retn_addr = 0x4D8583;
		__asm
		{
			// esi is pushed already
			call	Player_GetMethod_Stock_func;
			add     esp, 4;
			test    eax, eax;

			// Return if we got the Handle for a stock method
			jnz		VALID_METHOD_HANDLE;

			// else look if it is a custom method
			push    esi;
			call	player_get_custom_method;
			add     esp, 4;
			test    eax, eax;
			
			// Return if we got the Handle for a custom method
			jnz		VALID_METHOD_HANDLE;

			// else jump back to the next check in code ( ScriptEnt_GetMethod )
			jmp		retn_addr;

		VALID_METHOD_HANDLE:
			retn;
		}
	}

	xfunction_t Scr_GetFunction(const char** name, int* type)
	{
		/*auto str = std::string_view(*name);
		if (str.contains("rtx"))
		{
			int breakme = 0;
		}*/

		xfunction_t fn = (xfunction_t)Scr_GetFunction_f(name, type);
		auto cmd = scr_functions;

		if (fn) {
			return fn;
		}

		if (!cmd) {
			return nullptr;
		}

		while (strcmp(*name, cmd->name))
		{
			cmd = cmd->next;

			if (!cmd) {
				return nullptr;
			}

		}

		fn = cmd->function;
		*name = cmd->name;
		return fn;
	}

	gscr_methods::gscr_methods()
	{
		// hook Player_GetMethod
		utils::hook(0x4D8577, Scr_GetMethod_stub, HOOK_JUMP).install()->quick();

		MH_CreateHook((LPVOID)0x4D8470, Scr_GetFunction, &(LPVOID&)Scr_GetFunction_f);

		add_stock_player_methods();
	}
}
