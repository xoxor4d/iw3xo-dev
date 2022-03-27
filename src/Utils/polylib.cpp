#include "STDInclude.hpp"

#define MAX_MAP_BOUNDS	65535
#define	ON_EPSILON	0.1f

namespace utils
{
	namespace polylib 
	{
		// check if point is within bounds
		bool is_point_within_bounds(const glm::vec3& point, const glm::vec3& mins, const glm::vec3& maxs, const float& epsilon)
		{
			for (auto mi = 0; mi < 3; mi++)
			{
				if (point[mi] < mins[mi]) 
				{
					if(glm::distance(point[mi], mins[mi]) > epsilon)
					{
						return false;
					}
				}
			}

			for (auto ma = 0; ma < 3; ma++)
			{
				if (point[ma] > maxs[ma]) 
				{
					if (glm::distance(point[ma], mins[ma]) > epsilon)
					{
						return false;
					}
				}
			}

			return true;
		}

		void winding_plane(Game::winding_t* w, Game::vec3_t normal, float* dist)
		{
			Game::vec3_t v1, v2;

			m_vector_subtract(w->p[1], w->p[0], v1);
			m_vector_subtract(w->p[2], w->p[0], v2);

			utils::vector::cross3(v2, v1, normal);
			utils::vector::normalize_to(normal, normal);

			*dist = m_dot_product(w->p[0], normal);
		}

		float winding_area(Game::winding_t* w)
		{
			float total = 0.0f;
			Game::vec3_t d1, d2, cross;
			
			for (auto i = 2; i < w->numpoints; i++)
			{
				m_vector_subtract(w->p[i - 1], w->p[0], d1);
				m_vector_subtract(w->p[i], w->p[0], d2);

				utils::vector::cross3(d1, d2, cross);
				total += 0.5f * m_vector_length(cross);
			}

			return total;
		}

		bool check_winding(Game::winding_t* w)
		{
			int	i, j;
			Game::vec3_t dir, edge_normal, face_normal;

			if (w->numpoints < 3)
			{
				//Com_Error(ERR_DROP, "CheckWinding: %i points", w->numpoints);
				return false;
			}

			if (winding_area(w) < 1)
			{
				//Com_Error(ERR_DROP, "CheckWinding: %f area", area);
				return false;
			}

			float face_dist;
			winding_plane(w, face_normal, &face_dist);

			for (i = 0; i < w->numpoints; i++)
			{
				const auto p1 = w->p[i];

				for (j = 0; j < 3; j++)
				{
					if (p1[j] > MAX_MAP_BOUNDS || p1[j] < -MAX_MAP_BOUNDS)
					{
						return false;
					}
				}
					
				j = i + 1 == w->numpoints ? 0 : i + 1;

				// check the point is on the face plane
				float d = m_dot_product(p1, face_normal) - face_dist;

				if (d < -ON_EPSILON || d > ON_EPSILON)
				{
					return false;
				}

				// check the edge isnt degenerate
				const auto p2 = w->p[j];
				m_vector_subtract(p2, p1, dir);

				if (m_vector_length(dir) < ON_EPSILON)
				{
					return false;
				}
					

				utils::vector::cross3(face_normal, dir, edge_normal);
				utils::vector::normalize_to(edge_normal, edge_normal);
				float edge_dist = m_dot_product(p1, edge_normal);
				edge_dist += ON_EPSILON;

				// all other points must be on front side
				for (j = 0; j < w->numpoints; j++)
				{
					if (j == i)
					{
						continue;
					}
						
					d = m_dot_product(w->p[j], edge_normal);

					if (d > edge_dist)
					{
						return false;
					}
				}
			}

			return true;
		}
	}
}