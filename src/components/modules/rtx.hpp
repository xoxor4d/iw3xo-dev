#pragma once

namespace components
{
	class rtx final : public component
	{
	public:
		rtx();
		const char* get_name() override { return "rtx"; }

		static inline auto loc_disable_world_culling = 0u;
		static inline auto loc_disable_entity_culling = 0u;

		static inline std::vector rtx_disable_world_culling_enum = { "default", "less", "all", "all-but-models" };

		static void r_set_3d();
		static void setup_dvars_rtx();
		static bool r_set_material_stub(game::switch_material_t* swm, const game::GfxCmdBufState* state);
		static void player_origin_model();

	private:
		static inline bool axis_spawned = false;
		static inline game::gentity_s* axis_model = nullptr;
	};
}
