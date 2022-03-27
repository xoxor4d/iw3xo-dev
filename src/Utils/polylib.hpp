#pragma once

namespace utils
{
	namespace polylib
	{
		bool	is_point_within_bounds(const glm::vec3& point, const glm::vec3& mins, const glm::vec3& maxs, const float& epsilon);

		void	winding_plane(Game::winding_t* w, Game::vec3_t normal, float* dist);
		float	winding_area(Game::winding_t* w);
		bool	check_winding(Game::winding_t* w);
		
	}
}