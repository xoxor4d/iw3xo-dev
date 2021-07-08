#pragma once

namespace Components
{
	class Ocean : public Component
	{
	public:
		Ocean();
		~Ocean();
		const char* getName() override { return "Ocean"; };

		static void set_vertexshader_constants(Game::GfxCmdBufState* state, Game::MaterialShaderArgument* const arg_def);
		static void set_pixelshader_constants(Game::GfxCmdBufState* state, Game::MaterialShaderArgument* const arg_def);

		static void devgui_tab(Game::gui_menus_t& menu);
	
	private:
	};
}
