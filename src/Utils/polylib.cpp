#include "STDInclude.hpp"

#define MAX_MAP_BOUNDS	65535
#define	ON_EPSILON	0.1f

namespace Utils
{
	namespace polylib 
	{
		// check if point is within bounds
		bool PointWithinBounds(const glm::vec3 &point, const glm::vec3& mins, const glm::vec3& maxs, const float& epsilon)
		{
			for (auto mi = 0; mi < 3; mi++)
			{
				if (point[mi] < mins[mi]) 
				{
					if(glm::distance(point[mi], mins[mi]) > epsilon)
						return false;
				}
			}

			for (auto ma = 0; ma < 3; ma++)
			{
				if (point[ma] > maxs[ma]) 
				{
					if (glm::distance(point[ma], mins[ma]) > epsilon)
						return false;
				}
			}

			return true;
		}

		void WindingPlane(Game::winding_t* w, Game::vec3_t normal, float* dist)
		{
			Game::vec3_t v1, v2;

			VectorSubtract(w->p[1], w->p[0], v1);
			VectorSubtract(w->p[2], w->p[0], v2);

			Utils::vector::_CrossProduct(v2, v1, normal);
			Utils::vector::_VectorNormalize2(normal, normal);

			*dist = DotProduct(w->p[0], normal);
		}

		float WindingArea(Game::winding_t* w)
		{
			int		i;
			float	total = 0;
			Game::vec3_t d1, d2, cross;
			
			for (i = 2; i < w->numpoints; i++)
			{
				VectorSubtract(w->p[i - 1], w->p[0], d1);
				VectorSubtract(w->p[i], w->p[0], d2);

				Utils::vector::_CrossProduct(d1, d2, cross);
				total += 0.5f * VectorLength(cross);
			}

			return total;
		}

		bool CheckWinding(Game::winding_t* w)
		{
			int		i, j;
			float  *p1, *p2;
			float	d, edgedist, area, facedist;
			Game::vec3_t dir, edgenormal, facenormal;

			if (w->numpoints < 3)
			{
				//Com_Error(ERR_DROP, "CheckWinding: %i points", w->numpoints);
				return false;
			}
				
			area = WindingArea(w);
			if (area < 1)
			{
				//Com_Error(ERR_DROP, "CheckWinding: %f area", area);
				return false;
			}
				
			WindingPlane(w, facenormal, &facedist);

			for (i = 0; i < w->numpoints; i++)
			{
				p1 = w->p[i];

				for (j = 0; j < 3; j++)
				{
					if (p1[j] > MAX_MAP_BOUNDS || p1[j] < -MAX_MAP_BOUNDS)
					{
						//Com_Error(ERR_DROP, "CheckFace: BUGUS_RANGE: %f", p1[j]);
						return false;
					}
				}
					
				j = i + 1 == w->numpoints ? 0 : i + 1;

				// check the point is on the face plane
				d = DotProduct(p1, facenormal) - facedist;

				if (d < -ON_EPSILON || d > ON_EPSILON)
				{
					//Com_Error(ERR_DROP, "CheckWinding: point off plane");
					return false;
				}

				// check the edge isnt degenerate
				p2 = w->p[j];
				VectorSubtract(p2, p1, dir);

				if (VectorLength(dir) < ON_EPSILON)
				{
					//Com_Error(ERR_DROP, "CheckWinding: degenerate edge");
					return false;
				}
					

				Utils::vector::_CrossProduct(facenormal, dir, edgenormal);
				Utils::vector::_VectorNormalize2(edgenormal, edgenormal);
				edgedist = DotProduct(p1, edgenormal);
				edgedist += ON_EPSILON;

				// all other points must be on front side
				for (j = 0; j < w->numpoints; j++)
				{
					if (j == i)
					{
						continue;
					}
						
					d = DotProduct(w->p[j], edgenormal);

					if (d > edgedist)
					{
						//Com_Error(ERR_DROP, "CheckWinding: non-convex");
						return false;
					}
				}
			}

			return true;
		}
	}
}