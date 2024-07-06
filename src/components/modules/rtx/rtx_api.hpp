#pragma once
#include <bridge_c.h>

namespace components
{
	class rtx_api final : public component
	{
	public:
		rtx_api();
		~rtx_api();
		const char* get_name() override { return "rtx_api"; }

		static inline rtx_api* p_this = nullptr;
		static rtx_api* get() { return p_this; }

		static BRIDGEAPI_ErrorCode init();
		static bool create_sphere_light(uint64_t* in_out_handle, uint64_t initial_hash, float x = 0.0f, float y = 0.0f, float z = 0.0f, float radiance_r = 500.0f, float radiance_g = 500.0f, float radiance_b = 500.0f);
		static bool destroy_light(uint64_t* in_out_handle);

		bool created_device = false;
		bool draw_light = false;
		bool draw_light2 = false;

		static inline bridgeapi_Interface bridge = {};

		static inline x86::remixapi_LightHandle g_scene_light = nullptr;
		static inline uint64_t g_light_handle = 0;
		static inline uint64_t g_light_handle2 = 0;
	};
}
