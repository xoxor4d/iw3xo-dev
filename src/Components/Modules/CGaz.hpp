#pragma once

namespace components
{
	class cgaz final : public component
	{
	public:
		cgaz();
		const char* get_name() override { return "cgaz"; };

		static void main();
	
	private:
		static void draw_cgaz();

		static void PM_Accelerate(float wishspeed, float accel);
		static void PM_SlickAccelerate(float wishspeed, float accel);

		static float PM_CmdScaleForStance(Game::pmove_t* pm);
		static float PM_GetViewHeightLerp(Game::pmove_t* pm, int from_height, int to_height);
		static float PM_GetViewHeightLerpTime(int iTarget, int bDown);

		static float PM_CmdScale(Game::playerState_s* ps, Game::usercmd_s* cmd);
		static float PM_CmdScale_Walk(Game::pmove_t* pm, Game::usercmd_s* cmd);
		static float PM_DamageScale_Walk(int damage_timer);

		static void PmoveSingle(Game::pmove_t* pm, Game::pml_t* pml);
		static void PM_AirMove(Game::pmove_t* pm, Game::pml_t* pml);
		static void PM_WalkMove(Game::pmove_t* pm, Game::pml_t* pml);
	};
}