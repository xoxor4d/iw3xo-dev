#pragma once

#ifndef RENDERING_TEST
namespace Components
{
	class RB_DrawCollision : public Component
	{
	public:
		RB_DrawCollision();
		~RB_DrawCollision();
		const char* getName() override { return "RB_DrawCollision"; };

		static void		RB_ShowCollision(Game::GfxViewParms *viewParms);

	private:
	};
}
#endif
