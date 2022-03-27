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

		static float PM_CmdScaleForStance(game::pmove_t* pm);
		static float PM_GetViewHeightLerp(game::pmove_t* pm, int from_height, int to_height);
		static float PM_GetViewHeightLerpTime(int iTarget, int bDown);

		static float PM_CmdScale(game::playerState_s* ps, game::usercmd_s* cmd);
		static float PM_CmdScale_Walk(game::pmove_t* pm, game::usercmd_s* cmd);
		static float PM_DamageScale_Walk(int damage_timer);

		static void PmoveSingle(game::pmove_t* pm, game::pml_t* pml);
		static void PM_AirMove(game::pmove_t* pm, game::pml_t* pml);
		static void PM_WalkMove(game::pmove_t* pm, game::pml_t* pml);
	};
}