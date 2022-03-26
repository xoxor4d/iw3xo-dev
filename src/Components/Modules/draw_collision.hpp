#pragma once

namespace components
{
	class draw_collision final : public component
	{
	public:
		draw_collision();
		const char* getName() override { return "draw_collision"; };

		static void	debug_collision_frame(Game::GfxViewParms *viewParms);
	};
}
