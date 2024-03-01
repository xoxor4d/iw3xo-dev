#include "std_include.hpp"

namespace components
{
	rtx_map_settings* rtx_map_settings::p_this = nullptr;
	rtx_map_settings* rtx_map_settings::get() { return p_this; }

	void rtx_map_settings::set_settings_for_loaded_map(bool reload_settings)
	{
		if ((m_settings.empty() || reload_settings) && !rtx_map_settings::load_settings())
		{
			return;
		}

		if (game::rgp->world && game::rgp->world->name)
		{
			std::string map_name = game::rgp->world->name;
			utils::replace_all(map_name, std::string("maps/mp/"), "");	// if mp map
			utils::replace_all(map_name, std::string("maps/"), "");		// if sp map
			utils::replace_all(map_name, std::string(".d3dbsp"), "");

			bool found = false;
			for (const auto& s : m_settings)
			{
				if (s.mapname == map_name)
				{
					m_max_distance = s.max_distance;
					m_color = s.m_color;

					rtx_gui::skysphere_spawn(s.skybox);

					found = true;
					break;
				}
			}

			if (!found)
			{
				m_max_distance = 5000.0f;
				m_color.packed = D3DCOLOR_XRGB(200, 200, 220);

				if (!flags::has_flag("no_default_sky") && !rtx_gui::skysphere_is_valid())
				{
					rtx_gui::skysphere_spawn(5); // always spawn sunset
				}
			}
		}
	}

	constexpr auto INI_MAPNAME_ARG = 0;
	constexpr auto INI_SKYBOX_ARG = 1;
	constexpr auto INI_FOG_MAX_ARG = 2;
	constexpr auto INI_FOG_COLOR_ARG_BEGIN = 3;

	bool rtx_map_settings::load_settings()
	{
		m_settings.clear();
		m_settings.reserve(32);

		std::ifstream file;
		if (utils::fs::open_file_homepath("iw3xo\\rtx", "map_settings.ini", false, file))
		{
			std::string input;
			std::vector<std::string> args;

			// read line by line
			while (std::getline(file, input))
			{
				// ignore comment
				if (utils::starts_with(input, "//"))
				{
					continue;
				}

				// split string on ','
				args = utils::split(input, ',');

				if (args.size() == INI_FOG_COLOR_ARG_BEGIN+3) // fog colors rgb are last
				{
					const DWORD color = D3DCOLOR_XRGB(
						utils::try_stoi(args[INI_FOG_COLOR_ARG_BEGIN+0], 255), 
						utils::try_stoi(args[INI_FOG_COLOR_ARG_BEGIN+1], 255), 
						utils::try_stoi(args[INI_FOG_COLOR_ARG_BEGIN+2], 255));

					m_settings.push_back(
						{
							args[INI_MAPNAME_ARG],
							utils::try_stoi(args[INI_SKYBOX_ARG], 0),
							utils::try_stof(args[INI_FOG_MAX_ARG], 5000.0f),
							color
						});
				}
			}

			file.close();
			return true;
		}

		return false;
	}

	rtx_map_settings::rtx_map_settings()
	{
		command::add("mapsettings_update", [this](command::params)
		{
			rtx_map_settings::set_settings_for_loaded_map(true);
		});
	}
}