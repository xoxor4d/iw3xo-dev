#include "std_include.hpp"

#define SLIDER_SPEED 0.01f

// show tooltip after x seconds
#define TTDELAY 1.0f 

// tooltip with delay
#define TT(tooltip) if (ImGui::IsItemHoveredDelay(TTDELAY)) { ImGui::SetTooltip(tooltip); }

// spacing dummy
#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 

namespace components
{
	// *
	// internal helper vars

	float		 time_scalar = 1.0f;				// day length scalar
	bool		 time_running = false;				// setup fog, light and filmtweak lerping + sun rotation
	bool		 time_rotates_sun = true;			// automatically rotate the sun

	bool		 framefog_tweaks_overwrite = false;	// TOD uses stock fog
	game::vec4_t framefog_color_stock	= {};		// stock fog
	float		 framefog_start_stock	= 0.0f;
	float		 framefog_density_stock = 0.0f;

	game::vec4_t sundiffuse_constant  = {};			// used to display shader constants
	game::vec4_t sunposition_constant = {};
	game::vec4_t colorbias_constant	  = {};

	// suncolor lerp variable because steps are so small that they get discarded when packing as char[4] color
	float		 tod_suncolor_lerp[3] = {};

	
	// *******************************************************************************
	// actual settings
	
	float		 day_speed_scalar = 0.7f;			// speed at which daytime passes
	float		 night_speed_scalar = 2.2f;			// speed at which nighttime passes
	
	// *
	// default day and night shader constants

	// FILTER_TAP_0 
	game::vec3_t outscatter_color01 = { 0.859f, 0.654f, 0.362f };	// xyz
	float		 outscatter_scale01 = 0.61f;						// w

	// FILTER_TAP_1
	game::vec3_t outscatter_color02 = { 1.0f, 1.00f, 1.00f };		// xyz
	float		 outscatter_scale02 = 15.0f;						// w

	// FILTER_TAP_2
	float		 dn_skydome_scale		= 80.0f; // x
	float		 dn_cloud_coverage		= 0.55f; // y
	float		 dn_cloud_thickness		= 10.0f; // z
	float		 dn_cloud_absorbtion	= 0.36f; // w

	// FILTER_TAP_3
	float		 dn_star_scale				= 0.02f;	// x
	float		 dn_cloud_step_distance_xz	= 150.0f;	// y
	float		 dn_cloud_exposure			= 0.65f;	// w

	// FILTER_TAP_4
	game::vec3_t dn_cloud_wind_vec		= { 0.01f, 0.00f, 0.01f };	// xyz
	float		 dn_cloud_wind_speed	= 1.3f;						// w

	// FILTER_TAP_5
	float		 dn_sky_sun_intensity	= 22.0f;					// x
	game::vec3_t dn_sky_rayleigh_coeff	= { 7.52f, 13.0f, 22.4f };	// yzw

	// FILTER_TAP_6
	float		 dn_sky_mie_coeff		= 7.00f;	// x
	float		 dn_sky_rayleigh_scale	= 1000.0f;	// y
	float		 dn_sky_mie_scale		= 400.0f;	// z
	float		 dn_sky_mie_scatter_dir = 0.99f;	// w

	
	const int	 CLOUD_VAR_AMOUNT = 21;
	
	// amount of variables ^
	// *******************************************************************************

	
	// *
	// default time of day settings

	daynight_cycle::time_of_day_settings tod_night = {
		/*float film_brightness		*/	-0.075f,
		/*float film_contrast		*/	1.06f,
		/*float film_desaturation	*/	0.375f,
		/*float film_darktint[3]	*/	{ 0.413f, 0.638f, 1.008f },
		/*float film_lighttint[3]	*/	{ 0.838f, 0.689f, 0.618f },
		/*float film_interpol_spd	*/	0.2f,

		/*float sun_light			*/	0.6f,
		/*float sun_color[3]		*/	{ 0.400f, 0.400f, 0.700f },
		/*float sun_interpol_spd	*/	0.1f,

		/*bool  tweak_fog;			*/	true,
		/*float fog_start			*/	300.0f,
		/*float fog_density			*/	0.012f,
		/*float fog_color[3]		*/	{ 0.000f, 0.000f, 0.000f }
	};

	daynight_cycle::time_of_day_settings tod_day = {
		/*float film_brightness		*/	0.00f,
		/*float film_contrast		*/	1.0f,
		/*float film_desaturation	*/	0.1f,
		/*float film_darktint[3]	*/	{ 1.000f, 1.000f, 1.000f },
		/*float film_lighttint[3]	*/	{ 1.000f, 1.000f, 1.000f },
		/*float film_interpol_spd	*/	0.05f,

		/*float sun_light			*/	1.5f,
		/*float sun_color[3]		*/	{ 1.000f, 0.840f, 0.750f },
		/*float sun_interpol_spd	*/	0.05f,

		/*bool  tweak_fog;			*/	false,
		/*float fog_start			*/	500.0f,
		/*float fog_density			*/	0.026f,
		/*float fog_color[3]		*/	{ 0.545f, 0.505f, 0.377f }
	};

	daynight_cycle::time_of_day_settings tod_sunset = {
		/*float film_brightness		*/	-0.03f,
		/*float film_contrast		*/	1.155f,
		/*float film_desaturation	*/	0.0f,
		/*float film_darktint[3]	*/	{ 1.197f, 1.008f, 1.004f },
		/*float film_lighttint[3]	*/	{ 1.204f, 1.200f, 0.992f },
		/*float film_interpol_spd	*/	0.1f,

		/*float sun_light			*/	1.3f,
		/*float sun_color[3]		*/	{ 1.000f, 0.800f, 0.900f },
		/*float sun_interpol_spd	*/	0.1f,

		/*bool  tweak_fog;			*/	true,
		/*float fog_start			*/	840.0f,
		/*float fog_density			*/	0.040f,
		/*float fog_color[3]		*/	{ 0.444f, 0.326f, 0.199f }
	};

	// amount of tod's ^
	const int TOD_AMOUNT = 3;


	// *
	// config saving

	void cfg_write_tod_settings(std::ofstream& cfg, const daynight_cycle::time_of_day_settings* tod)
	{
		if (!tod)
		{
			return;
		}

		cfg << "// film_brightness" << std::endl << tod->film_brightness << std::endl << std::endl;
		cfg << "// film_contrast" << std::endl << tod->film_contrast << std::endl << std::endl;
		cfg << "// film_desaturation" << std::endl << tod->film_desaturation << std::endl << std::endl;
		cfg << "// film_darktint" << std::endl << tod->film_darktint[0] << " " << tod->film_darktint[1] << " " << tod->film_darktint[2] << std::endl << std::endl;
		cfg << "// film_lighttint" << std::endl << tod->film_lighttint[0] << " " << tod->film_lighttint[1] << " " << tod->film_lighttint[2] << std::endl << std::endl;
		cfg << "// film_interpolation_speed" << std::endl << tod->film_interpolation_speed << std::endl << std::endl;

		cfg << "// sun_light" << std::endl << tod->sun_light << std::endl << std::endl;
		cfg << "// sun_color" << std::endl << tod->sun_color[0] << " " << tod->sun_color[1] << " " << tod->sun_color[2] << std::endl << std::endl;
		cfg << "// sun_interpolation_speed" << std::endl << tod->sun_interpolation_speed << std::endl << std::endl;

		cfg << "// tweak_fog" << std::endl << tod->tweak_fog << std::endl << std::endl;
		cfg << "// fog_start" << std::endl << tod->fog_start << std::endl << std::endl;
		cfg << "// fog_density" << std::endl << tod->fog_density << std::endl << std::endl;
		cfg << "// fog_color" << std::endl << tod->fog_color[0] << " " << tod->fog_color[1] << " " << tod->fog_color[2] << std::endl << std::endl;
	}

	void save_config(std::string cfg_name)
	{
		if (cfg_name.empty())
		{
			cfg_name = "daynight_unnamed_";
		}

		if (const auto& fs_basepath = game::Dvar_FindVar("fs_basepath"); fs_basepath)
		{
			std::string base_path = fs_basepath->current.string;
						base_path += "\\iw3xo\\daynight\\";

			if (std::filesystem::create_directories(base_path))
			{
				game::Com_PrintMessage(0, "Created directory \"root/iw3xo/daynight\"\n", 0);
			}

			std::ofstream cfg;
			cfg.open((base_path + cfg_name + ".cfg").c_str());

			cfg << "// generated by IW3xo - https://github.com/xoxor4d/iw3xo-dev" << std::endl;
			cfg << "// day and night settings" << std::endl << std::endl;

			cfg << "// day_speed_scalar" << std::endl << day_speed_scalar << std::endl << std::endl;
			cfg << "// night_speed_scalar" << std::endl << night_speed_scalar << std::endl << std::endl;
			
			// FILTER_TAP_0
			cfg << "// outscatter_color01" << std::endl << outscatter_color01[0] << " " << outscatter_color01[1] << " " << outscatter_color01[2] << std::endl << std::endl;
			cfg << "// outscatter_scale01" << std::endl << outscatter_scale01 << std::endl << std::endl;

			// FILTER_TAP_1
			cfg << "// outscatter_color02" << std::endl << outscatter_color02[0] << " " << outscatter_color02[1] << " " << outscatter_color02[2] << std::endl << std::endl;
			cfg << "// outscatter_scale02" << std::endl << outscatter_scale02 << std::endl << std::endl;

			// FILTER_TAP_2
			cfg << "// dn_skydome_scale" << std::endl << dn_skydome_scale << std::endl << std::endl;
			cfg << "// dn_cloud_coverage" << std::endl << dn_cloud_coverage << std::endl << std::endl;
			cfg << "// dn_cloud_thickness" << std::endl << dn_cloud_thickness << std::endl << std::endl;
			cfg << "// dn_cloud_absorbtion" << std::endl << dn_cloud_absorbtion << std::endl << std::endl;

			// FILTER_TAP_3
			cfg << "// dn_star_scale" << std::endl << dn_star_scale << std::endl << std::endl;
			cfg << "// dn_cloud_step_distance_xz" << std::endl << dn_cloud_step_distance_xz << std::endl << std::endl;
			cfg << "// dn_cloud_exposure" << std::endl << dn_cloud_exposure << std::endl << std::endl;

			// FILTER_TAP_4
			cfg << "// dn_cloud_wind_vec" << std::endl << dn_cloud_wind_vec[0] << " " << dn_cloud_wind_vec[1] << " " << dn_cloud_wind_vec[2] << std::endl << std::endl;
			cfg << "// dn_cloud_wind_speed" << std::endl << dn_cloud_wind_speed << std::endl << std::endl;

			// FILTER_TAP_5
			cfg << "// dn_sky_sun_intensity" << std::endl << dn_sky_sun_intensity << std::endl << std::endl;
			cfg << "// dn_sky_rayleigh_coeff" << std::endl << dn_sky_rayleigh_coeff[0] << " " << dn_sky_rayleigh_coeff[1] << " " << dn_sky_rayleigh_coeff[2] << std::endl << std::endl;
			
			// FILTER_TAP_6
			cfg << "// dn_sky_mie_coeff" << std::endl << dn_sky_mie_coeff << std::endl << std::endl;
			cfg << "// dn_sky_rayleigh_scale" << std::endl << dn_sky_rayleigh_scale << std::endl << std::endl;
			cfg << "// dn_sky_mie_scale" << std::endl << dn_sky_mie_scale << std::endl << std::endl;
			cfg << "// dn_sky_mie_scatter_dir" << std::endl << dn_sky_mie_scatter_dir << std::endl << std::endl;

			// time of day settings
			cfg << std::endl << "// *" << std::endl << "// time of day - day" << std::endl << std::endl;
			cfg_write_tod_settings(cfg, &tod_day);

			cfg << std::endl << "// *" << std::endl << "// time of day - sunrise / sunset" << std::endl << std::endl;
			cfg_write_tod_settings(cfg, &tod_sunset);

			cfg << std::endl << "// *" << std::endl << "// time of day - night" << std::endl << std::endl;
			cfg_write_tod_settings(cfg, &tod_night);

			game::Com_PrintMessage(0, utils::va("Saved day and night settings to: %s", (base_path + cfg_name + ".cfg").c_str()), 0);
			cfg.close();
		}
	}


	// *
	// config loading

	template <typename T> void config_set_var(T* var, std::vector<float>& val)
	{
		for (unsigned int i = 0; i < val.size() && i < 4; i++)
		{
			var[i] = val[i];
		}
	}

	void config_set_tod(daynight_cycle::time_of_day_settings* tod, std::vector<std::vector<float>>& v, int& x)
	{
		config_set_var(&tod->film_brightness,	v[x++]);
		config_set_var(&tod->film_contrast,		v[x++]);
		config_set_var(&tod->film_desaturation, v[x++]);
		config_set_var( tod->film_darktint,		v[x++]);
		config_set_var( tod->film_lighttint,	v[x++]);
		config_set_var(&tod->film_interpolation_speed, v[x++]);

		config_set_var(&tod->sun_light, v[x++]);
		config_set_var( tod->sun_color, v[x++]);
		config_set_var(&tod->sun_interpolation_speed, v[x++]);

		config_set_var(&tod->tweak_fog,		v[x++]);
		config_set_var(&tod->fog_start,		v[x++]);
		config_set_var(&tod->fog_density,	v[x++]);
		config_set_var( tod->fog_color,		v[x++]);
	}

	// ouch
	void load_config(const std::string& cfg_name)
	{
		if (cfg_name.empty())
		{
			return;
		}

		if (const auto& fs_basepath = game::Dvar_FindVar("fs_basepath"); 
						fs_basepath)
		{
			std::string base_path = fs_basepath->current.string;
						base_path += "\\iw3xo\\daynight\\";

			// open cfg
			if (std::ifstream cfg((base_path + cfg_name).c_str()); cfg.is_open())
			{
				std::vector<std::vector<float>> v;
				std::string line;

				int i = 0, x = 0;
				
				while (std::getline(cfg, line))
				{
					if (line.starts_with("//") || line.empty())
					{
						continue;
					}

					float value;
					std::stringstream ss(line);

					v.push_back(std::vector<float>());

					while (ss >> value)
					{
						v[i].push_back(value);
					}
					++i;
				}

				if (v.size() != CLOUD_VAR_AMOUNT + (TOD_AMOUNT * daynight_cycle::TOD_VAR_AMOUNT))
				{
					game::Com_PrintMessage(0, utils::va("Unable to load config \"%s\". Variable amount mismatch.", (base_path + cfg_name + ".cfg").c_str()), 0);
					
					cfg.close();
					return;
				}

				config_set_var(&day_speed_scalar,	v[x++]);
				config_set_var(&night_speed_scalar,	v[x++]);

				config_set_var( outscatter_color01, v[x++]);
				config_set_var(&outscatter_scale01, v[x++]);

				config_set_var( outscatter_color02, v[x++]);
				config_set_var(&outscatter_scale02, v[x++]);

				config_set_var(&dn_skydome_scale, v[x++]);
				config_set_var(&dn_cloud_coverage, v[x++]);
				config_set_var(&dn_cloud_thickness, v[x++]);
				config_set_var(&dn_cloud_absorbtion, v[x++]);

				config_set_var(&dn_star_scale, v[x++]);
				config_set_var(&dn_cloud_step_distance_xz, v[x++]);
				config_set_var(&dn_cloud_exposure, v[x++]);
				config_set_var( dn_cloud_wind_vec, v[x++]);

				config_set_var(&dn_cloud_wind_speed, v[x++]);
				config_set_var(&dn_sky_sun_intensity, v[x++]);
				config_set_var( dn_sky_rayleigh_coeff, v[x++]);
				config_set_var(&dn_sky_mie_coeff, v[x++]);

				config_set_var(&dn_sky_rayleigh_scale, v[x++]);
				config_set_var(&dn_sky_mie_scale, v[x++]);
				config_set_var(&dn_sky_mie_scatter_dir, v[x++]);


				config_set_tod(&tod_day, v, x);
				config_set_tod(&tod_sunset, v, x);
				config_set_tod(&tod_night, v, x);

				cfg.close();
			}
		}
	}


	// *
	// *

	// https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/Runtime/Core/Private/Math/UnrealMath.cpp
	void vinterp_to(float* output, int vector_size, const float* current, const float* target, const float delta_time, const float interpolation_speed)
	{
		if (output)
		{
			// If no interp speed, jump to target value
			if (interpolation_speed <= 0.0f)
			{
				utils::vector::copy(target, output, vector_size);
				return;
			}

			for (auto i = 0; i < vector_size && i < 4; i++)
			{
				// distance to reach
				const float distance = target[i] - current[i];

				// If distance is too small, just set the desired location
				if (distance * distance < 1.e-8f)
				{
					output[i] = target[i];
					continue;
				}

				// Delta Move, Clamp so we do not over shoot.
				const float delta_move = distance * std::clamp(delta_time * interpolation_speed, 0.0f, 1.0f);

				output[i] = current[i] + delta_move;
			}
		}
	}

	// https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/Runtime/Core/Private/Math/UnrealMath.cpp
	float finterp_to(const float current, const float target, const float delta_time, const float interpolation_speed)
	{
		// If no interp speed, jump to target value
		if (interpolation_speed <= 0.0f)
		{
			return target;
		}

		// distance to reach
		const float distance = target - current;

		// If distance is too small, just set the desired location
		if (distance * distance < 1.e-8f)
		{
			return target;
		}

		// Delta Move, Clamp so we do not over shoot.
		const float delta_move = distance * std::clamp(delta_time * interpolation_speed, 0.0f, 1.0f);

		return current + delta_move;
	}


	// *
	// fogtweaks over time

	void daynight_cycle::lerp_fogtweaks(const daynight_cycle::time_of_day_settings* lerp_to, const float delta_time)
	{
		if (lerp_to)
		{
			game::dvar_set_value_dirty(dvars::r_fogTweaks, lerp_to->tweak_fog);

			// use default fog if TOD fog is disabled
			if (!lerp_to->tweak_fog)
			{
				// set the lerped framefog constants in "r_set_frame_fog"
				framefog_tweaks_overwrite = true;

				if (lerp_to->film_interpolation_speed <= 0.0f)
				{
					game::dvar_set_value_dirty(dvars::r_fogTweaksStart, framefog_start_stock);
					game::dvar_set_value_dirty(dvars::r_fogTweaksDensity, framefog_density_stock);
					game::dvar_set_value_dirty(dvars::r_fogTweaksColor, framefog_color_stock, 4);

					return;
				}

				game::dvar_set_value_dirty(dvars::r_fogTweaksStart, finterp_to(dvars::r_fogTweaksStart->current.value, framefog_start_stock, delta_time, lerp_to->film_interpolation_speed));
				game::dvar_set_value_dirty(dvars::r_fogTweaksDensity, finterp_to(dvars::r_fogTweaksDensity->current.value, framefog_density_stock, delta_time, lerp_to->film_interpolation_speed));
				
				vinterp_to(dvars::r_fogTweaksColor->current.vector, 3, dvars::r_fogTweaksColor->current.vector, framefog_color_stock, delta_time, lerp_to->film_interpolation_speed);
				utils::vector::copy(dvars::r_fogTweaksColor->current.vector, dvars::r_fogTweaksColor->latched.vector, 3);

				return;
			}


			// if using framefog tweaks
			framefog_tweaks_overwrite = false;

			if (lerp_to->film_interpolation_speed <= 0.0f)
			{
				game::dvar_set_value_dirty(dvars::r_fogTweaksStart, lerp_to->fog_start);
				game::dvar_set_value_dirty(dvars::r_fogTweaksDensity, lerp_to->fog_density);
				game::dvar_set_value_dirty(dvars::r_fogTweaksStart, lerp_to->fog_color, 3);

				return;
			}

			game::dvar_set_value_dirty(dvars::r_fogTweaksStart, finterp_to(dvars::r_fogTweaksStart->current.value, lerp_to->fog_start, delta_time, lerp_to->film_interpolation_speed));
			game::dvar_set_value_dirty(dvars::r_fogTweaksDensity, finterp_to(dvars::r_fogTweaksDensity->current.value, lerp_to->fog_density, delta_time, lerp_to->film_interpolation_speed));
			
			vinterp_to(dvars::r_fogTweaksColor->current.vector, 3, dvars::r_fogTweaksColor->current.vector, lerp_to->fog_color, delta_time, lerp_to->film_interpolation_speed);
			utils::vector::copy(dvars::r_fogTweaksColor->current.vector, dvars::r_fogTweaksColor->latched.vector, 3);
		}
	}


	// *
	// suntweaks over time

	void daynight_cycle::lerp_suntweaks(const daynight_cycle::time_of_day_settings* lerp_to, const float delta_time)
	{
		const auto& r_lightTweakSunColor = game::Dvar_FindVar("r_lightTweakSunColor");
		const auto& r_lightTweakSunLight = game::Dvar_FindVar("r_lightTweakSunLight");

		if (r_lightTweakSunColor && r_lightTweakSunLight)
		{
			if (lerp_to)
			{
				if (lerp_to->film_interpolation_speed <= 0.0f)
				{
					// sunlight
					game::dvar_set_value_dirty(r_lightTweakSunLight, lerp_to->sun_light);		
					r_lightTweakSunLight->modified = true;

					// pack suncolor and modify dvar
					utils::byte3_pack_rgba(lerp_to->sun_color, r_lightTweakSunColor->current.color);

					for (auto i = 0; i < 3; i++)
					{
						r_lightTweakSunColor->latched.color[i] = r_lightTweakSunColor->current.color[i];
					}

					r_lightTweakSunColor->modified = true;

					return;
				}

				// sunlight
				game::dvar_set_value_dirty(r_lightTweakSunLight, finterp_to(r_lightTweakSunLight->current.value, lerp_to->sun_light, delta_time, lerp_to->sun_interpolation_speed));
				r_lightTweakSunLight->modified = true;


				// suncolor
				float temp_suncolor[3] = { 0.0f };
				utils::byte3_unpack_rgba(r_lightTweakSunColor->current.color, temp_suncolor);

				// update tod_suncolor_lerp if != temp_suncolor + epsilon (steps are so small that they get discarded when packing)
				if (!utils::vector::compare_epsilon3(temp_suncolor, tod_suncolor_lerp, 0.05f))
				{
					utils::vector::copy(temp_suncolor, tod_suncolor_lerp, 3);
				}

				vinterp_to(tod_suncolor_lerp, 3, tod_suncolor_lerp, lerp_to->sun_color, delta_time, lerp_to->sun_interpolation_speed);

				// pack suncolor and modify dvar
				utils::byte3_pack_rgba(tod_suncolor_lerp, r_lightTweakSunColor->current.color);

				for (auto i = 0; i < 3; i++) 
				{
					r_lightTweakSunColor->latched.color[i] = r_lightTweakSunColor->current.color[i];
				}

				r_lightTweakSunColor->modified = true;

				
			}
		}
	}


	// *
	// filmtweaks over time

	void daynight_cycle::lerp_filmtweaks(const daynight_cycle::time_of_day_settings* lerp_to, const float delta_time)
	{
		const auto& r_filmTweakBrightness = game::Dvar_FindVar("r_filmTweakBrightness");
		const auto& r_filmTweakContrast = game::Dvar_FindVar("r_filmTweakContrast");
		const auto& r_filmTweakDesaturation = game::Dvar_FindVar("r_filmTweakDesaturation");
		const auto& r_filmTweakDarkTint = game::Dvar_FindVar("r_filmTweakDarkTint");
		const auto& r_filmTweakLightTint = game::Dvar_FindVar("r_filmTweakLightTint");

		if (r_filmTweakBrightness && r_filmTweakContrast && r_filmTweakDesaturation && r_filmTweakDarkTint && r_filmTweakLightTint)
		{
			if (lerp_to)
			{
				if (lerp_to->film_interpolation_speed <= 0.0f)
				{
					game::dvar_set_value_dirty(r_filmTweakBrightness, lerp_to->film_brightness);		r_filmTweakBrightness->modified = true;
					game::dvar_set_value_dirty(r_filmTweakContrast, lerp_to->film_contrast);			r_filmTweakContrast->modified = true;
					game::dvar_set_value_dirty(r_filmTweakDesaturation, lerp_to->film_desaturation);	r_filmTweakDesaturation->modified = true;
					game::dvar_set_value_dirty(r_filmTweakDarkTint, lerp_to->film_darktint, 3);		r_filmTweakDarkTint->modified = true;
					game::dvar_set_value_dirty(r_filmTweakLightTint, lerp_to->film_lighttint, 3);		r_filmTweakLightTint->modified = true;

					return;
				}

				game::dvar_set_value_dirty(r_filmTweakBrightness, finterp_to(r_filmTweakBrightness->current.value, lerp_to->film_brightness, delta_time, lerp_to->film_interpolation_speed));
				r_filmTweakBrightness->modified = true;

				game::dvar_set_value_dirty(r_filmTweakContrast, finterp_to(r_filmTweakContrast->current.value, lerp_to->film_contrast, delta_time, lerp_to->film_interpolation_speed));
				r_filmTweakContrast->modified = true;

				game::dvar_set_value_dirty(r_filmTweakDesaturation, finterp_to(r_filmTweakDesaturation->current.value, lerp_to->film_desaturation, delta_time, lerp_to->film_interpolation_speed));
				r_filmTweakDesaturation->modified = true;

				
				vinterp_to(r_filmTweakDarkTint->current.vector, 3, r_filmTweakDarkTint->current.vector, lerp_to->film_darktint, delta_time, lerp_to->film_interpolation_speed);
				r_filmTweakDarkTint->modified = true;

				vinterp_to(r_filmTweakLightTint->current.vector, 3, r_filmTweakLightTint->current.vector, lerp_to->film_lighttint, delta_time, lerp_to->film_interpolation_speed);
				r_filmTweakDarkTint->modified = true;
			}
		}
	}


	// *
	// lerp over time

	void daynight_cycle::lerp_time_of_day(const daynight_cycle::time_of_day_settings* lerp_to, const float delta_time)
	{
		daynight_cycle::lerp_suntweaks(lerp_to, delta_time);
		daynight_cycle::lerp_filmtweaks(lerp_to, delta_time);
		daynight_cycle::lerp_fogtweaks(lerp_to, delta_time);
	}


	// *
	// called from _client::on_set_cgame_time()

	void daynight_cycle::set_world_time()
	{
		const auto cui = game::clientUI;
		const auto cl = game::clients;

		if (cui && cl)
		{
			if (cui->connectionState == game::CA_ACTIVE)
			{
				
				if (const auto& r_lightTweakSunDirection = game::Dvar_FindVar("r_lightTweakSunDirection"); r_lightTweakSunDirection && time_running)
				{
					const float sun_current_rot = r_lightTweakSunDirection->current.vector[0];
					float delta = static_cast<float>(cl->serverTimeDelta) * 0.0001f * time_scalar;
					const float delta_abs = fabsf(delta);


					// enable filmtweaks
					const auto& r_filmTweakEnable = game::Dvar_FindVar("r_filmTweakEnable");
					const auto& r_filmUseTweaks = game::Dvar_FindVar("r_filmUseTweaks");
					
					if (r_filmTweakEnable && r_filmUseTweaks && (!r_filmTweakEnable->current.enabled || !r_filmUseTweaks->current.enabled))
					{
						game::dvar_set_value_dirty(r_filmTweakEnable, true);	r_filmTweakEnable->modified = true;
						game::dvar_set_value_dirty(r_filmUseTweaks, true);		r_filmUseTweaks->modified = true;
					}


					// disable stock sunflare and darkening effect
					if (const auto& r_drawSun = game::Dvar_FindVar("r_drawSun"); r_drawSun && r_drawSun->current.enabled)
					{
						game::dvar_set_value_dirty(r_drawSun, false);	r_drawSun->modified = true;
					}


					// longer day / sunsets
					delta *= day_speed_scalar;

					// night time (360 equals 0 so start night transition slightly before 360/0)
					if (sun_current_rot >= 355.0f || sun_current_rot >= 0.0f && sun_current_rot <= 175.0f)
					{
						daynight_cycle::lerp_time_of_day(&tod_night, delta_abs);

						if(sun_current_rot >= 10.0f && sun_current_rot <= 175.0f)
						{
							// shorter nights after sunset
							delta *= night_speed_scalar;
						}
					}

					// sunrise
					else if (sun_current_rot <= 200.0f)
					{
						daynight_cycle::lerp_time_of_day(&tod_sunset, delta_abs);
					}

					// day time
					else if (sun_current_rot <= 340.0f)
					{
						daynight_cycle::lerp_time_of_day(&tod_day, delta_abs);
					}

					// sunset
					else
					{
						daynight_cycle::lerp_time_of_day(&tod_sunset, delta_abs);
					}


					// sun position
					if (sun_current_rot >= 360.0f)
					{
						r_lightTweakSunDirection->current.vector[0] -= 360.0f;
						r_lightTweakSunDirection->latched.vector[0] -= 360.0f;
						r_lightTweakSunDirection->modified = true;
					}
					else if (sun_current_rot < 0.0f)
					{
						r_lightTweakSunDirection->current.vector[0] = 359.999f;
						r_lightTweakSunDirection->latched.vector[0] = 359.999f;
						r_lightTweakSunDirection->modified = true;
					}

					// automatically rotate the sun
					if (time_rotates_sun)
					{
						r_lightTweakSunDirection->current.vector[0] += delta;
						r_lightTweakSunDirection->latched.vector[0] += delta;
						r_lightTweakSunDirection->modified = true;
					}
				}
			}
		}
	}


	// *
	// devgui

	std::vector<std::string> devgui_configs;
	
	bool			devgui_loaded_configs = false;
	unsigned int	devgui_selected_config_idx = 0;

	
	void devgui_tod_settings(daynight_cycle::time_of_day_settings* tod)
	{
		ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

		ImGui::DragFloat("Film Brightness",		&tod->film_brightness,	 SLIDER_SPEED, -2.0f, 2.0f, "%.4f");
		ImGui::DragFloat("Film Contrast",			&tod->film_contrast,	 SLIDER_SPEED, -2.0f, 2.0f, "%.4f");
		ImGui::DragFloat("Film Desaturation",		&tod->film_desaturation, SLIDER_SPEED, 0.0f, 1.0f, "%.4f");
		ImGui::ColorEdit3("Film DarkTint",		 tod->film_darktint, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
		ImGui::ColorEdit3("Film LightTint",		 tod->film_darktint, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
		ImGui::DragFloat("Film Interpolation Speed", &tod->film_interpolation_speed, SLIDER_SPEED, 0.0f, 5.0f, "%.4f");

		SPACING(0.0f, 8.0f);

		ImGui::DragFloat("Sun Light", &tod->sun_light, SLIDER_SPEED, 0.0f, 5.0f, "%.4f");
		ImGui::ColorEdit3("Sun Color", tod->sun_color, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
		ImGui::DragFloat("Sun Interpolation Speed", &tod->sun_interpolation_speed, SLIDER_SPEED, 0.0f, 5.0f, "%.4f");

		SPACING(0.0f, 8.0f);

		ImGui::Checkbox("Tweak Fog :: Lerp to default fog if disabled.", &tod->tweak_fog);
		ImGui::ColorEdit4("Fog Color",	 tod->fog_color, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
		ImGui::DragFloat("Fog Start",		&tod->fog_start, 5.0f, -1000.0f, 30000.0f, "%.4f");
		ImGui::DragFloat("Fog Density",	&tod->fog_density, 0.0001f, 0.0f, 1.0f, "%.4f");

		ImGui::Indent(-8.0f); SPACING(0.0f, 4.0f);
	}


	void daynight_cycle::devgui_tab(game::gui_menus_t& menu)
	{
		ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

#if DEBUG
		if (ImGui::Button("Reload xcommon_iw3xo_addon"))
		{
			game::Cbuf_AddText("loadzone xcommon_iw3xo_addon\n", 0);
		}
		SPACING(0.0f, 4.0f);
#endif
		
		if (ImGui::Button("Toggle Day/Night"))
		{
			if (dvars::r_dayAndNight)
			{
				game::dvar_set_value_dirty(dvars::r_dayAndNight, !dvars::r_dayAndNight->current.enabled);
			}

			if (const auto& sm_sunSampleSizeNear = game::Dvar_FindVar("sm_sunSampleSizeNear"); 
							sm_sunSampleSizeNear)
			{
				game::dvar_set_value_dirty(sm_sunSampleSizeNear, 0.55f);
				sm_sunSampleSizeNear->modified = true;
			}
		}

		ImGui::SameLine(); SPACING(0.0f, 0.0f); ImGui::SameLine();
		ImGui::Checkbox("Enable Time of Day Lerping", &time_running);
		ImGui::SameLine(); SPACING(0.0f, 0.0f); ImGui::SameLine();
		ImGui::Checkbox("Enable Auto Sun Rotation", &time_rotates_sun); TT("Automatically rotate the sun. Disable for manual mode using Sun Direction.");

		if (const auto& r_lightTweakSunDirection = game::Dvar_FindVar("r_lightTweakSunDirection"); 
						r_lightTweakSunDirection)
		{
			ImGui::DragFloat("Sun Direction", gui::dvar_get_set<float*>(r_lightTweakSunDirection), 0.25f, -360.0f, 360.0f, "%.2f");
			TT("Current Sun Direction (x). Disable Auto Sun Rotation for manual mode.");
		}
		
		ImGui::DragFloat("Day Length Scalar", &time_scalar, SLIDER_SPEED, 0.0f, 100.0f, "%.4f"); TT("Does influence Time of Day Interpolation Speeds.");
		ImGui::DragFloat("Day Speed Scalar", &day_speed_scalar, SLIDER_SPEED, 0.1f, 50.0f, "%.4f"); TT("Speed up / Slow down daytime.");
		ImGui::DragFloat("Night Speed Scalar", &night_speed_scalar, SLIDER_SPEED, 0.1f, 50.0f, "%.4f"); TT("Speed up / Slow down nighttime.");

		if (const auto& sm_sunSampleSizeNear = game::Dvar_FindVar("sm_sunSampleSizeNear"); 
						sm_sunSampleSizeNear)
		{
			ImGui::DragFloat("Shadow Near Sample", gui::dvar_get_set<float*>(sm_sunSampleSizeNear), 0.25f, 0.0625f, 32.0f, "%.2f");
			TT("sm_sunSampleSizeNear :: Higher values increases shadow draw distance but reduces quality.");
		}
		
		ImGui::Indent(-8.0f); SPACING(0.0f, 4.0f);

		if (ImGui::CollapsingHeader("Cloud Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			ImGui::Text("FilterTap[0]");
			ImGui::ColorEdit3("Cloud Sunset Color", outscatter_color01, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
			ImGui::DragFloat("Extinction Scale", &outscatter_scale01, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");

			SPACING(0.0f, 4.0f);

			ImGui::Text("FilterTap[1]");
			ImGui::DragFloat3("Cloud Day Color", outscatter_color02, SLIDER_SPEED, 0.0f, 4.0f, "%.4f");
			ImGui::DragFloat("Cloud Horizon Fade", &outscatter_scale02, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");

			SPACING(0.0f, 4.0f);

			ImGui::Text("FilterTap[2]");
			ImGui::DragFloat("Skydome Scale", &dn_skydome_scale, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");
			ImGui::DragFloat("Cloud Coverage", &dn_cloud_coverage, SLIDER_SPEED, 0.0f, 10.0f, "%.4f");
			ImGui::DragFloat("Cloud Thickness", &dn_cloud_thickness, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");
			ImGui::DragFloat("Cloud Absorbtion", &dn_cloud_absorbtion, SLIDER_SPEED, 0.0f, 10.0f, "%.4f");

			SPACING(0.0f, 4.0f);

			ImGui::Text("FilterTap[3]");
			ImGui::DragFloat("Star Scale",					&dn_star_scale, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");
			ImGui::DragFloat("Cloud Step Distance XZ",		&dn_cloud_step_distance_xz, SLIDER_SPEED, 0.0f, 500.0f, "%.4f"); TT("Distance from map center at which to draw less detailed clouds.");
			//ImGui::DragFloat("Not used",					&dn_cloud_steps_after_threshold, SLIDER_SPEED, 0.0f, 30.0f, "%.4f");
			ImGui::DragFloat("Cloud Exposure",				&dn_cloud_exposure, SLIDER_SPEED, 0.0f, 10.0f, "%.4f");

			SPACING(0.0f, 4.0f);

			ImGui::Text("FilterTap[4]");
			ImGui::DragFloat3("Cloud Wind Direction", dn_cloud_wind_vec, SLIDER_SPEED, 0.0f, 10.0f, "%.4f");
			ImGui::DragFloat("Cloud Wind Speed",	 &dn_cloud_wind_speed, SLIDER_SPEED, 0.0f, 10.0f, "%.4f");

			SPACING(0.0f, 4.0f);

			ImGui::Text("FilterTap[5]");
			ImGui::DragFloat("Sky Sun Intensity",  &dn_sky_sun_intensity, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");
			ImGui::DragFloat3("Sky Rayleigh Coeff", dn_sky_rayleigh_coeff, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");

			SPACING(0.0f, 4.0f);

			ImGui::Text("FilterTap[6]");
			ImGui::DragFloat("Sky Mie Coeff",		&dn_sky_mie_coeff, SLIDER_SPEED, 0.0f, 100.0f, "%.4f");
			ImGui::DragFloat("Sky Rayleigh Scale",	&dn_sky_rayleigh_scale, SLIDER_SPEED, 0.0f, 2000.0f, "%.4f");
			ImGui::DragFloat("Sky Mie Scale",		&dn_sky_mie_scale, SLIDER_SPEED, 0.0f, 500.0f, "%.4f");
			ImGui::DragFloat("Sky Mie Scatter Dir", &dn_sky_mie_scatter_dir, SLIDER_SPEED, 0.0f, 10.0f, "%.4f");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		if (ImGui::CollapsingHeader("Time of Day Settings"))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			if (ImGui::BeginTabBar("tod_tabs"))
			{
				if (ImGui::BeginTabItem("Day"))
				{
					devgui_tod_settings(&tod_day);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Sunrise/Sunset"))
				{
					devgui_tod_settings(&tod_sunset);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Night"))
				{
					devgui_tod_settings(&tod_night);
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			ImGui::Indent(-8.0f); SPACING(0.0f, 4.0f);
		}


		// dupe of film/lighttweaks tab
		if (ImGui::CollapsingHeader("Filmtweaks", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			const auto& r_filmTweakEnable = game::Dvar_FindVar("r_filmTweakEnable");
			const auto& r_filmUseTweaks = game::Dvar_FindVar("r_filmUseTweaks");

			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			if (r_filmTweakEnable && r_filmUseTweaks)
			{
				ImGui::Checkbox("Enable Filmtweaks", &r_filmTweakEnable->current.enabled);
				ImGui::SameLine();
				ImGui::Checkbox("Use Filmtweaks", &r_filmUseTweaks->current.enabled);

				if (r_filmTweakEnable->current.enabled && r_filmUseTweaks->current.enabled)
				{
					const auto& r_filmTweakInvert = game::Dvar_FindVar("r_filmTweakInvert");
					const auto& r_filmTweakLightTint = game::Dvar_FindVar("r_filmTweakLightTint");
					const auto& r_filmTweakDarkTint = game::Dvar_FindVar("r_filmTweakDarkTint");
					const auto& r_filmTweakDesaturation = game::Dvar_FindVar("r_filmTweakDesaturation");
					const auto& r_filmTweakContrast = game::Dvar_FindVar("r_filmTweakContrast");
					const auto& r_filmTweakBrightness = game::Dvar_FindVar("r_filmTweakBrightness");

					if (r_filmTweakInvert && r_filmTweakLightTint && r_filmTweakDarkTint && r_filmTweakDesaturation && r_filmTweakContrast && r_filmTweakBrightness)
					{
						ImGui::SameLine();
						ImGui::Checkbox("Invert", &r_filmTweakInvert->current.enabled);

						SPACING(0.0f, 4.0f);

						// light and darktint should be capped at 2.0 since hdr allows for much higher values
						ImGui::ColorEdit3("Light Tint", r_filmTweakLightTint->current.vector, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
						ImGui::ColorEdit3("Dark Tint", r_filmTweakDarkTint->current.vector, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
						ImGui::DragFloat("Desaturation", &r_filmTweakDesaturation->current.value, 0.005f, 0.0f, 1.0f);
						ImGui::DragFloat("Contrast", &r_filmTweakContrast->current.value, 0.005f, 0.0f, 4.0f);
						ImGui::DragFloat("Brightness", &r_filmTweakBrightness->current.value, 0.005f, -1.0f, 1.0f);
					}
				}
			}

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		if (ImGui::CollapsingHeader("Lighttweaks", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			const auto& r_lightTweakSunColor = game::Dvar_FindVar("r_lightTweakSunColor");
			const auto& r_lightTweakSunDirection = game::Dvar_FindVar("r_lightTweakSunDirection");
			const auto& r_lightTweakSunLight = game::Dvar_FindVar("r_lightTweakSunLight");

			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			if (r_lightTweakSunColor && r_lightTweakSunDirection && r_lightTweakSunLight)
			{
				// unpack the color and use a swap var
				float sun_color_swap[4] = { 0.0f };
				utils::byte4_unpack_rgba(r_lightTweakSunColor->current.color, sun_color_swap);

				if (ImGui::ColorEdit3("Sun Color", sun_color_swap, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel))
				{
					utils::byte4_pack_rgba(sun_color_swap, r_lightTweakSunColor->current.color);

					for (auto i = 0; i < 4; i++) {
						r_lightTweakSunColor->latched.color[i] = r_lightTweakSunColor->current.color[i];
					}

					r_lightTweakSunColor->modified = true;
				}

				ImGui::DragFloat3("Sun Direction", gui::dvar_get_set<float*>(r_lightTweakSunDirection), 0.25f, -360.0f, 360.0f, "%.2f");
				ImGui::SliderFloat("Sun Light", gui::dvar_get_set<float*>(r_lightTweakSunLight), r_lightTweakSunLight->domain.value.min, r_lightTweakSunLight->domain.value.max, "%.2f");
			}

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		if (ImGui::CollapsingHeader("Fog Tweaks", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			ImGui::Checkbox("Tweak Framefog", gui::dvar_get_set<bool*>(dvars::r_fogTweaks)); TT("r_fogTweaks");
			ImGui::ColorEdit4("Fog Color", gui::dvar_get_set<float*>(dvars::r_fogTweaksColor), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel); TT("r_fogTweaksColor");
			ImGui::DragFloat("Fog Start", gui::dvar_get_set<float*>(dvars::r_fogTweaksStart), 5.0f, -1000.0f, 30000.0f, "%.4f"); TT("r_fogTweaksStart");
			ImGui::DragFloat("Fog Density", gui::dvar_get_set<float*>(dvars::r_fogTweaksDensity), 0.0001f, 0.0f, 1.0f, "%.4f"); TT("r_fogTweaksDensity");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

#if DEBUG
		if (ImGui::CollapsingHeader("Debug Constants", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			ImGui::Text("This section shows actual shader constant values.\n(Only updated when the shader is actually using the constants)");
			SPACING(0.0f, 4.0f);
			
			ImGui::Text("Sun Position: %.2f %.2f %.2f %.2f",
				sunposition_constant[0], sunposition_constant[1], sunposition_constant[2], sunposition_constant[3]);

			ImGui::Text("Sun Diffuse: %.2f %.2f %.2f %.2f",
				sundiffuse_constant[0], sundiffuse_constant[1], sundiffuse_constant[2], sundiffuse_constant[3]);

			ImGui::Text("Filmtweak Colorbias: %.2f %.2f %.2f %.2f",
				colorbias_constant[0], colorbias_constant[1], colorbias_constant[2], colorbias_constant[3]);

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}
#endif

		if (ImGui::CollapsingHeader("Configs"))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			static char cfg_name[64] = "daynight_";
			
			if (ImGui::Button("Save"))
			{
				save_config(cfg_name);
				devgui_loaded_configs = false;
			} TT("Save config -> root/iw3xo/daynight");

			ImGui::SameLine();
			ImGui::PushItemWidth(400.0f);
			ImGui::InputText("Config Name", cfg_name, IM_ARRAYSIZE(cfg_name));
			ImGui::PopItemWidth();
			
			SPACING(0.0f, 4.0f);

			if (ImGui::Button("Load"))
			{
				load_config(devgui_configs[devgui_selected_config_idx]);
			} TT("Load the selected config -> root/iw3xo/daynight");

			ImGui::SameLine();

			if (ImGui::Button("Refresh"))
			{
				devgui_loaded_configs = false;
			} TT("Refresh configs -> root/iw3xo/daynight");

			// load configs once or on reload
			if (!devgui_loaded_configs)
			{
				if (!devgui_configs.empty())
				{
					devgui_configs.clear();
				}

				if (const auto& fs_basepath = game::Dvar_FindVar("fs_basepath"); fs_basepath)
				{
					std::string base_path = fs_basepath->current.string;
								base_path += "\\iw3xo\\daynight\\";

					if(std::filesystem::exists(base_path))
					{
						for (const auto& d : std::filesystem::directory_iterator(base_path))
						{
							if (d.path().extension() == ".cfg")
							{
								auto file = std::filesystem::path(d.path());
								devgui_configs.push_back(file.filename().string());
							}
						}
					}
				}

				devgui_loaded_configs = true;
			}

			if (ImGui::ListBoxHeader("Configs"))
			{
				for (unsigned int n = 0; n < devgui_configs.size(); n++)
				{
					const bool is_selected = (devgui_selected_config_idx == n);

					if (ImGui::Selectable(devgui_configs[n].c_str(), is_selected))
					{
						devgui_selected_config_idx = n;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::ListBoxFooter();
			}

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}
	}


	// *
	// skybox constants

	// called from _Renderer::pixelshader_custom_constants
	void daynight_cycle::set_pixelshader_constants(game::GfxCmdBufState* state, game::MaterialShaderArgument* const arg_def)
	{
		if (state->pass->pixelShader && !utils::q_stricmp(state->pass->pixelShader->name, "iw3xo_daynight"))
		{
			// *
			// get constants for debugging purposes
			if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_SUN_DIFFUSE)
			{
				(*game::dx9_device_ptr)->GetPixelShaderConstantF(arg_def->dest, sundiffuse_constant, 1);
			}

			if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_SUN_POSITION)
			{
				(*game::dx9_device_ptr)->GetPixelShaderConstantF(arg_def->dest, sunposition_constant, 1);
			}

			if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_COLOR_BIAS)
			{
				(*game::dx9_device_ptr)->GetPixelShaderConstantF(arg_def->dest, colorbias_constant, 1);
			}


			// *
			// set constants
			if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0)
			{
				const float constant[4] = { outscatter_color01[0], outscatter_color01[1], outscatter_color01[2], outscatter_scale01 };
				(*game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}

			if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_1)
			{
				const float constant[4] = { outscatter_color02[0], outscatter_color02[1], outscatter_color02[2], outscatter_scale02 };
				(*game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}

			if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2)
			{
				const float constant[4] = { dn_skydome_scale, dn_cloud_coverage, dn_cloud_thickness, dn_cloud_absorbtion };
				(*game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}

			if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_3)
			{
				const float constant[4] = { dn_star_scale, dn_cloud_step_distance_xz, 0.0f, dn_cloud_exposure };
				(*game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}

			if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_4)
			{
				const float constant[4] = { dn_cloud_wind_vec[0], dn_cloud_wind_vec[1], dn_cloud_wind_vec[2], dn_cloud_wind_speed };
				(*game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}

			if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_5)
			{
				const float constant[4] = { dn_sky_sun_intensity, dn_sky_rayleigh_coeff[0], dn_sky_rayleigh_coeff[1], dn_sky_rayleigh_coeff[2] };
				(*game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}

			if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_6)
			{
				const float constant[4] = { dn_sky_mie_coeff, dn_sky_rayleigh_scale, dn_sky_mie_scale, dn_sky_mie_scatter_dir };
				(*game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
			}
		}
	}


	// *
	// overwrite default sky

	// called from _Renderer::R_SetMaterial
	void daynight_cycle::overwrite_sky_material(game::switch_material_t* swm)
	{
		if (dvars::r_dayAndNight && dvars::r_dayAndNight->current.enabled)
		{
			if (utils::starts_with(swm->current_material->info.name, "wc/sky_"))
			{
				swm->technique_type = game::TECHNIQUE_UNLIT;
				_renderer::switch_material(swm, "wc/iw3xo_daynight");
			}
		}
	}


	// TODO:
	// move to _Renderer

	void r_set_frame_fog(game::GfxCmdBufInput* input)
	{
		if (input)
		{
			if (!dvars::r_fogTweaks->current.enabled && !framefog_tweaks_overwrite)
			{
				// save stock settings
				framefog_start_stock = input->data->fogSettings.fogStart;
				framefog_density_stock = input->data->fogSettings.density * 100.0f;
				utils::vector::copy(input->consts[game::CONST_SRC_CODE_FOG_COLOR], framefog_color_stock, 4);

				// set dvars
				game::dvar_set_value_dirty(dvars::r_fogTweaksStart, framefog_start_stock);
				game::dvar_set_value_dirty(dvars::r_fogTweaksDensity, framefog_density_stock);

				utils::vector::copy(framefog_color_stock, dvars::r_fogTweaksColor->current.vector, 4);
				utils::vector::copy(dvars::r_fogTweaksColor->current.vector, dvars::r_fogTweaksColor->latched.vector, 4);
			}

			else
			{
				input->consts[game::CONST_SRC_CODE_FOG_COLOR][0] = dvars::r_fogTweaksColor->current.vector[0];
				input->consts[game::CONST_SRC_CODE_FOG_COLOR][1] = dvars::r_fogTweaksColor->current.vector[1];
				input->consts[game::CONST_SRC_CODE_FOG_COLOR][2] = dvars::r_fogTweaksColor->current.vector[2];
				input->consts[game::CONST_SRC_CODE_FOG_COLOR][3] = dvars::r_fogTweaksColor->current.vector[3];

				input->consts[game::CONST_SRC_CODE_FOG][2] = -(dvars::r_fogTweaksDensity->current.value * 0.01f);
				input->consts[game::CONST_SRC_CODE_FOG][3] = dvars::r_fogTweaksStart->current.value * (dvars::r_fogTweaksDensity->current.value * 0.01f);

				// set the clear color so that the bottom of the skybox matches the fog
				const auto& r_clear = game::Dvar_FindVar("r_clear");
				const auto& r_clearColor = game::Dvar_FindVar("r_clearColor");

				if (r_clear && r_clearColor)
				{
					game::dvar_set_value_dirty(r_clear, 3);	r_clear->modified = true;
					utils::byte4_pack_rgba(dvars::r_fogTweaksColor->current.vector, r_clearColor->current.color);

					for (auto i = 0; i < 4; i++)
					{
						r_clearColor->latched.color[i] = r_clearColor->current.color[i];
					}

					r_clearColor->modified = true;
				}
			}
		}
	}

	__declspec(naked) void r_setframefog_stub()
	{
		const static uint32_t retn_addr = 0x63DBE5;
		__asm
		{
			// stock op's
			fstp    dword ptr[eax + 29Ch];

			pushad;
			push	eax; // GfxCmdBufInput*
			call	r_set_frame_fog;
			add		esp, 4;
			popad;

			jmp		retn_addr;
		}
	}


	daynight_cycle::daynight_cycle()
	{ 
		// TODO: move to _Renderer
		// mid-hook R_SetFrameFog
		
		utils::hook::nop(0x63DBDF, 6); utils::hook(0x63DBDF, r_setframefog_stub, HOOK_JUMP).install()->quick();

		dvars::r_dayAndNight = game::Dvar_RegisterBool(
			/* name		*/ "r_dayAndNight",
			/* desc		*/ "enable day and night cycle",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none);


		// TODO:
		// move to _Renderer

		dvars::r_fogTweaks = game::Dvar_RegisterBool(
			/* name		*/ "r_fogTweaks",
			/* desc		*/ "enable framefog tweaks",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none);

		dvars::r_fogTweaksColor = game::Dvar_RegisterVec4(
			/* name		*/ "r_fogTweaksColor",
			/* desc		*/ "framefog color",
			/* x		*/ 0.0f,
			/* y		*/ 0.0f,
			/* z		*/ 0.0f,
			/* w		*/ 0.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::none);

		dvars::r_fogTweaksStart = game::Dvar_RegisterFloat(
			/* name		*/ "r_fogTweaksStart",
			/* desc		*/ "framefog start",
			/* default	*/ 0.0f,
			/* minVal	*/ -1000.0f,
			/* maxVal	*/ 30000.0f,
			/* flags	*/ game::dvar_flags::none);

		dvars::r_fogTweaksDensity = game::Dvar_RegisterFloat(
			/* name		*/ "r_fogTweaksDensity",
			/* desc		*/ "framefog density",
			/* default	*/ 0.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::none);
	}
}