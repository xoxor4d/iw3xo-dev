#pragma once

namespace components
{
	class rtx final : public component
	{
	public:
		rtx();
		const char* get_name() override { return "rtx"; };

		static void r_set_3d();
		static void setup_dvars_rtx();
		static bool r_set_material_stub(game::switch_material_t* swm, const game::GfxCmdBufState* state);
	};
}
