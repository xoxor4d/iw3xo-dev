#pragma once

namespace components
{
	class rtx final : public component
	{
	public:
		rtx();
		const char* get_name() override { return "rtx"; };

		static bool r_set_material_stub(game::switch_material_t* swm, const game::GfxCmdBufState* state);

		static void skysphere_frame();
		
		static bool skysphere_is_model_valid();
		static void skysphere_update_pos();
		static void skysphere_toggle_vis();
		static void skysphere_change_model(int variant);
		static void skysphere_spawn(int variant = 0);

		static void gui();

		static inline game::vec3_t skysphere_model_origin = {};
		static inline game::vec3_t skysphere_model_rotation = {};
		static inline bool skysphere_auto_rotation = false;
		static inline float skysphere_auto_rotation_speed = 1.0f;

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
		};

		static inline rtx_debug_light rtx_lights[RTX_DEBUGLIGHT_AMOUNT] = {};

	private:
		static const char* skysphere_get_name_for_variant(int variant);
		static inline bool skysphere_spawned = false;
		static inline int skysphere_variant = 0;
		static inline game::gentity_s* skysphere_model = nullptr;
	};
}
