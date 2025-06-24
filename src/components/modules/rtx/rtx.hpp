#pragma once

namespace components
{
	class rtx final : public component
	{
	public:
		rtx();
		const char* get_name() override { return "rtx"; }

		static inline auto loc_culling_tweak_mins = 0u;
		static inline auto loc_culling_tweak_maxs = 0u;
		static inline auto loc_culling_tweak_frustum = 0u;
		static inline auto loc_culling_tweak_smodel = 0u;

		static constexpr auto FRUSTUM_PLANE_OFFSETS_COUNT = 7;
		static inline float m_frustum_plane_offsets[FRUSTUM_PLANE_OFFSETS_COUNT] = { 5000.0f, 5000.0f, 5000.0f, 5000.0f, 5000.0f, 5000.0f, 5000.0f };

		// old
		static inline bool OLD_CULLING_ACTIVE = false;
		static inline auto loc_disable_world_culling = 0u;
		static inline auto loc_disable_entity_culling = 0u;


		static inline std::uint16_t textureOverrideCount = 0;
		static inline game::GfxTextureOverride textureOverrides[256] = {};


		static inline std::vector rtx_disable_world_culling_enum = { "default", "less", "all", "all-but-models" };

		static void r_set_3d();
		static bool r_set_material_stub(game::switch_material_t* swm, const game::GfxCmdBufState* state);
		static void rb_show_tess(game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, const float* center, const char* name, const float* color, game::DebugGlobals* manual_debug_glob = nullptr);
		static void player_origin_model();

		static void force_dvars_on_frame();
		static void set_dvars_defaults_on_init();
		static void set_dvars_defaults_on_mapload();

		static void on_map_load();
		static void on_map_shutdown();

		static void on_device_creation();

		static void sky_material_update(std::string_view buffer, bool use_dvar = false);

	private:
		static inline bool axis_spawned = false;
		static inline game::gentity_s* axis_model = nullptr;

		static inline std::vector<std::string> sky_material_addons;
	};
}
