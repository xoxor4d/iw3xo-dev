#pragma once

namespace components
{
	class ocean final : public component
	{
	public:
		ocean() = default;
		const char* get_name() override { return "ocean"; };

		static void set_vertexshader_constants(game::GfxCmdBufState* state, game::MaterialShaderArgument* const arg_def);
		static void set_pixelshader_constants(game::GfxCmdBufState* state, game::MaterialShaderArgument* const arg_def);

		static void devgui_tab(game::gui_menus_t& menu);
	};
}
