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

		static bool create_sphere_light(uint64_t* in_out_handle, const x86::remixapi_LightInfo* l, const x86::remixapi_LightInfoSphereEXT* s);
		static bool create_rect_light(uint64_t* in_out_handle, const x86::remixapi_LightInfo* l, const x86::remixapi_LightInfoRectEXT* r);
		static bool create_disk_light(uint64_t* in_out_handle, const x86::remixapi_LightInfo* l, const x86::remixapi_LightInfoDiskEXT* r);
		static bool destroy_light(uint64_t* in_out_handle);


		static inline bridgeapi_Interface bridge = {};

		static inline uint64_t g_light_handle = 0;
		static inline uint64_t g_light_handle2 = 0;
	};
}
