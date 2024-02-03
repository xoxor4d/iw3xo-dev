#pragma once

namespace components
{
	class rtx_lights final : public component
	{
	public:
		rtx_lights();
		const char* get_name() override { return "rtx_lights"; };

		static void spawn_light();

		static constexpr auto RTX_DEBUGLIGHT_AMOUNT = 16;

		struct rtx_debug_light
		{
			bool enable = false;
			bool attach = false;
			bool virgin = true;
			int disable_hack = 10;

			D3DLIGHTTYPE type = D3DLIGHT_POINT;
			game::vec3_t origin = {};
			game::vec3_t color = { 1.0f, 1.0f, 1.0f };
			float color_scale = 1.0f;
			float range = 500.0f;

			// spot
			game::vec3_t dir = { 0.0f, 1.0f, 0.0f };
			game::vec3_t dir_offset = {};
			float inner_angle = 20.0f;
			float outer_angle = 45.0f;
		};

		static inline rtx_debug_light rtx_debug_lights[RTX_DEBUGLIGHT_AMOUNT] = {};
	};
}
