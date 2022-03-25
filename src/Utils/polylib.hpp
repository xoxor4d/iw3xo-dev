#pragma once

namespace utils
{
	namespace polylib
	{
		bool	PointWithinBounds(const glm::vec3& point, const glm::vec3& mins, const glm::vec3& maxs, const float& epsilon);

		void	WindingPlane(Game::winding_t* w, Game::vec3_t normal, float* dist);
		float	WindingArea(Game::winding_t* w);
		bool	CheckWinding(Game::winding_t* w);
		
	}
}