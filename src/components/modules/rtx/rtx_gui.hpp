#pragma once

namespace components
{
	class rtx_gui final : public component
	{
	public:
		rtx_gui();
		const char* get_name() override { return "rtx_gui"; };

		static void gui();

		static void skysphere_frame();
		static bool skysphere_is_model_valid();
		static void skysphere_update_pos();
		static void skysphere_toggle_vis();
		static void skysphere_change_model(int variant);
		static void skysphere_spawn(int variant = 0);

		static inline game::vec3_t skysphere_model_origin = {};
		static inline game::vec3_t skysphere_model_rotation = {};
		static inline bool skysphere_auto_rotation = false;
		static inline float skysphere_auto_rotation_speed = 1.0f;
		static inline float skysphere_scale = 1000.0f;

		static inline int d3d_alpha_blend = 4;

	private:
		static const char* skysphere_get_name_for_variant(int variant);
		static inline bool skysphere_spawned = false;
		static inline int skysphere_variant = 0;
		static inline game::gentity_s* skysphere_model = nullptr;

		static void map_materials_update();
		static inline std::string map_materials;
	};
}
