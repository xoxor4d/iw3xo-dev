#include "std_include.hpp"
#include "rtx_api.hpp"

#define CHECK_INIT(ret) if (!bridge.initialized) { game::Com_PrintMessage(0, "BridgeApi not initialized!", 0); return ret; }

namespace components
{
	BRIDGEAPI_ErrorCode rtx_api::init()
	{
		const auto status = bridgeapi_initialize(&bridge);
		game::Com_PrintMessage(0, utils::va("[API] bridgeapi_initialize() : %s ", !status ? "success" : utils::va("error : %d", status)), 0);

		if (bridge.initialized)
		{
			bridge.RegisterDevice();
		}

		return status;
	}

	bool rtx_api::create_sphere_light(uint64_t* in_out_handle, uint64_t initial_hash, const float x, const float y, const float z, const float radiance_r, const float radiance_g, const float radiance_b)
	{
		CHECK_INIT(false);
		if (!in_out_handle)
		{
			return false;
		}

		x86::remixapi_LightInfoSphereEXT s = {};
		{
			s.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT;
			s.position = { x, y, z };
			s.radius = 1.0f;
			s.shaping_hasvalue = FALSE;
			s.shaping_value = {};
		}

		x86::remixapi_LightInfo l = {};
		{
			l.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
			l.hash = *in_out_handle ? *in_out_handle : initial_hash;
			l.radiance = { radiance_r, radiance_g, radiance_b };
		}

		if (*in_out_handle)
		{
			bridge.DestroyLight(*in_out_handle);
		}

		*in_out_handle = bridge.CreateSphereLight(&l, &s);
		//game::Com_PrintMessage(0, utils::va("bridge.CreateLight handle = %d \n", *in_out_handle), 0);

		return true;
	}

	bool rtx_api::destroy_light(uint64_t* in_out_handle)
	{
		CHECK_INIT(false);
		if (!in_out_handle || !*in_out_handle)
		{
			return false;
		}

		bridge.DestroyLight(*in_out_handle);
		*in_out_handle = 0;
		return true;
	}

	rtx_api::rtx_api()
	{
		p_this = this;

		command::add("api_init", "", "calls the x64 process func foo", [this]([[maybe_unused]] command::params parms)
		{
			init();
		});

		command::add("api_set_config_var", "<var> <value>", "RemixApi: sets config variable 'var' to 'value'", [this]([[maybe_unused]] command::params parms)
		{
			CHECK_INIT();
			if (parms.length() <= 2)
			{
				game::Com_PrintMessage(0, "Usage: api_set_config_var <var> <value> :: eg: /api_set_config_var rtx.enableAlphaBlend 0", 0);
				return;
			}

			bridge.SetConfigVariable(parms[1], parms[2]);
			//game::Com_PrintMessage(0, utils::va("Command was: %s %s", parms[1], parms[2]), 0);
		});

		command::add("api_sky_brightness", "value", "RemixApi: sets config variable 'rtx.skyBrightness'", [this]([[maybe_unused]] command::params parms) 
		{
			CHECK_INIT();
			if (parms.length() == 1)
			{
				game::Com_PrintMessage(0, "Usage: api_sky_brightness <brightness value>", 0);
				return;
			}
				
			bridge.SetConfigVariable("rtx.skyBrightness", parms[1]);
		});

		command::add("api_create_light", "[optional:position] <x> <y> <z>  [optional:radiance] <r> <g> <b>", "RemixApi: Create a light with the 'CreateLight' func\neg: api_create_light 0 0 100 500 250 300", [this]([[maybe_unused]] command::params parms)
		{
			create_sphere_light(
				&g_light_handle,
				0x1337,
				parms.length() >= 1 ? utils::try_stof(parms[1]) : 0,
				parms.length() >= 2 ? utils::try_stof(parms[2]) : 200,
				parms.length() >= 3 ? utils::try_stof(parms[3]) : -15,
				parms.length() >= 4 ? utils::try_stof(parms[4]) : 1000,
				parms.length() >= 5 ? utils::try_stof(parms[5]) : 1200,
				parms.length() >= 6 ? utils::try_stof(parms[6]) : 1500);
		});

		command::add("api_create_light2", "[optional:position] <x> <y> <z>  [optional:radiance] <r> <g> <b>", "RemixApi: Create a light with the 'CreateLight' func\neg: api_create_light 0 0 100 500 250 300", [this]([[maybe_unused]] command::params parms)
		{
			create_sphere_light(
				&g_light_handle2,
				0x1338,
				parms.length() >= 1 ? utils::try_stof(parms[1]) : 0,
				parms.length() >= 2 ? utils::try_stof(parms[2]) : 200,
				parms.length() >= 3 ? utils::try_stof(parms[3]) : -15,
				parms.length() >= 4 ? utils::try_stof(parms[4]) : 1000,
				parms.length() >= 5 ? utils::try_stof(parms[5]) : 1200,
				parms.length() >= 6 ? utils::try_stof(parms[6]) : 1500);
		});

		command::add("api_destroy_light", "<handle>", "RemixApi: Destroy light with given handle", [this]([[maybe_unused]] command::params parms)
		{
			if(parms.length() == 1)
			{
				game::Com_PrintMessage(0, "Usage: api_destroy_light <handle>", 0);
				return;
			}

			uint64_t handle = utils::try_stoi(parms[1]);
			game::Com_PrintMessage(0, utils::va("Trying to destroy light with handle: %d \n", handle), 0);
			destroy_light(&handle);
		});

		/*command::add("api_draw_light", "", "calls the x64 process func foo", [this]([[maybe_unused]] command::params parms) 
		{
			CHECK_INIT();
			this->draw_light = !this->draw_light;
			game::Com_PrintMessage(0, utils::va("draw_light = %d", this->draw_light), 0);
		});

		command::add("api_draw_light2", "", "calls the x64 process func foo", [this]([[maybe_unused]] command::params parms)
		{
			CHECK_INIT();
			this->draw_light2 = !this->draw_light2;
			game::Com_PrintMessage(0, utils::va("draw_light2 = %d", this->draw_light2), 0);
		});*/
	}

	rtx_api::~rtx_api()
	{
		
	}
}