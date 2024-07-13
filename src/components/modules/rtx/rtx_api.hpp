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

		static bool destroy_mesh(uint64_t* in_out_handle);
		static bool create_cod4_mesh(uint64_t* in_out_handle, const char* model_name, uint64_t* material = nullptr);

		static bool create_sphere_light(uint64_t* in_out_handle, const x86::remixapi_LightInfo* l, const x86::remixapi_LightInfoSphereEXT* s);
		static bool create_rect_light(uint64_t* in_out_handle, const x86::remixapi_LightInfo* l, const x86::remixapi_LightInfoRectEXT* r);
		static bool create_disk_light(uint64_t* in_out_handle, const x86::remixapi_LightInfo* l, const x86::remixapi_LightInfoDiskEXT* d);
		static bool create_cylinder_light(uint64_t* in_out_handle, const x86::remixapi_LightInfo* l, const x86::remixapi_LightInfoCylinderEXT* cy);
		static bool create_distant_light(uint64_t* in_out_handle, const x86::remixapi_LightInfo* l, const x86::remixapi_LightInfoDistantEXT* d);
		static bool destroy_light(uint64_t* in_out_handle);

		static void create_quad(x86::remixapi_HardcodedVertex* v_out, uint32_t* i_out, const float scale);
		static void to_remix_transform(x86::remixapi_Transform* transform, game::vec3_t position, game::vec3_t rotation, game::vec3_t scale);

		static inline bridgeapi_Interface bridge = {};

		static inline uint64_t g_light_handle = 0;
		static inline uint64_t g_light_handle2 = 0;
	};
}
