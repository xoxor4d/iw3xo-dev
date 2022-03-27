#pragma once

namespace utils
{
	namespace polylib
	{
		bool	is_point_within_bounds(const glm::vec3& point, const glm::vec3& mins, const glm::vec3& maxs, const float& epsilon);

		void	winding_plane(game::winding_t* w, game::vec3_t normal, float* dist);
		float	winding_area(game::winding_t* w);
		bool	check_winding(game::winding_t* w);
		
	}
}