#include "STDInclude.hpp"

#define CM_CONTENTS_SOLID       0x1
#define CM_CONTENTS_CLIPSHOT    0x2000      // weapon clip
#define CM_CONTENTS_DETAIL      0x8000000   // detail

#define CM_MAX_BRUSHPOINTS_FROM_INTERSECTIONS 128

std::chrono::time_point<std::chrono::steady_clock> mapexport_timestamp_start_;
std::chrono::time_point<std::chrono::steady_clock> mapexport_timestamp_brushgen_start_;

bool  mapexport_cmd_ = false;
bool  mapexport_in_progress_ = false;
bool  mapexport_selection_add_;
int	  mapexport_current_brush_index_ = 0;
float mapexport_quad_eps_ = 0.0f;

std::ofstream mapexport_mapfile_;
std::ofstream mapexport_mapfile_addon_;
utils::entities	mapexport_entities_;
Game::boundingbox_s mapexport_selectionbox_;

const char *map_clipmap_name_ = "";

// last origin we sorted brushes at
glm::vec3 last_sorting_origin_ = glm::vec3(0.0f);

// winding pool
struct windingpool_t
{
	int numpoints;
	float p[256][4][3];
};

windingpool_t winding_pool_ = {};

// dynamic dvar strings
//std::string g_dvarMaterialList_str;

// material lists
std::vector<Game::dmaterial_t*> map_material_list_;
std::vector<std::string> map_material_list_duplicates_;
std::vector<std::string> map_material_list_single_;

// brush lists
std::vector<Game::cbrush_t*> map_brush_list_;
std::vector<Game::cbrush_t*> map_brush_list_for_index_filtering_;
std::vector<Game::brushmodel_entity_s> map_brushmodel_list_;

// used to calculate the delta between sv_fps and client fps
int SvFramerateToRendertime_Counter = 0;
int SvFramerateToRendertime_CurrentDelta = 0;

// view-frustum planes
const Game::vec4_t frustumSidePlanes[5] = 
{
	{-1.0f,  0.0f,  0.0f,  1.0f}, // left?
	{ 1.0f,  0.0f,  0.0f,  1.0f}, // right?
	{ 0.0f, -1.0f,  0.0f,  1.0f}, // bottom?
	{ 0.0f,  1.0f,  0.0f,  1.0f}, // top?
	{ 0.0f,  0.0f,  1.0f,  1.0f}  // back?
};

namespace components
{
	// scale strings with distance + zoom when looking directly at them
	float scale_string_lookat(Game::GfxViewParms *view_parms, const glm::vec3 origin)
	{
		glm::vec3 delta = glm::vec3(origin.x, origin.y, origin.z) - glm::vec3(view_parms->origin[0], view_parms->origin[1], view_parms->origin[2]);

		float scale = utils::vector::normalize3_glm(delta);
		const float dot = glm::dot(delta, glm::vec3(view_parms->axis[0][0], view_parms->axis[0][1], view_parms->axis[0][2]));

		scale = (dot - 0.995f) * scale;

		if (scale < 1.0f) 
		{
			scale = 1.0f;
		}

		return scale;
	}

	// show brush index numbers (used for r_drawCollision_brushIndexFilter) ---- adding strings will cause flicker or duplicates because renderthread runs at diff. framerate?
	void draw_brush_index_numbers(Game::GfxViewParms *view_parms, int brush_index, const glm::vec3 origin, [[maybe_unused]] int sorted_brush_index, [[maybe_unused]] int max_string_amount)
	{
		// we need the viewAxis to scale strings, calling Set3D populates our struct
		Game::R_Set3D();

		const glm::vec4 color = glm::vec4(1.0f);

		// not propper but will do for now
		_debug::add_debug_string_client(
			origin, 
			color, 
			scale_string_lookat(view_parms, origin), 
			utils::va("Index: %d", brush_index), 
			2); 
	}

	// some kind of wizardry
	void set_plane_signbits(Game::cplane_s *out)
	{
		// for fast box on planeside test
		char bits = 0;

		for (auto j = 0; j < 3; j++) 
		{
			if (out->normal[j] < 0.0f) 
			{
				bits |= 1 << j;
			}
		}

		out->signbits = bits;
	}

	// build view frustum (R_SetDpvsPlaneSides)
	void set_dpvs_plane_sides(Game::DpvsPlane *plane)
	{
		plane->side[0] = static_cast<char>(plane->coeffs[0]) <= 0 ? 0 : 12;
		plane->side[1] = static_cast<char>(plane->coeffs[1]) <= 0 ? 4 : 16;
		plane->side[2] = static_cast<char>(plane->coeffs[2]) <= 0 ? 8 : 20;
	}

	// build view frustum (R_FrustumClipPlanes)
	void frustum_clip_planes(const Game::GfxMatrix *view_proj_mtx, const float(*side_planes)[4], const int side_plane_count, Game::DpvsPlane *frustum_planes)
	{
		for (auto planeIndex = 0; planeIndex < side_plane_count; ++planeIndex)
		{
			for (auto term = 0; term < 4; ++term) 
			{
				frustum_planes[planeIndex].coeffs[term] = ((((*side_planes)[4 * planeIndex + 0] * view_proj_mtx->m[term][0])
														 + ((*side_planes)[4 * planeIndex + 1] * view_proj_mtx->m[term][1]))
														 + ((*side_planes)[4 * planeIndex + 2] * view_proj_mtx->m[term][2]))
														 + ((*side_planes)[4 * planeIndex + 3] * view_proj_mtx->m[term][3]);
			}

			const float length = utils::vector::length3(frustum_planes[planeIndex].coeffs);

			if (length <= 0.0f) 
			{
				Game::Com_PrintMessage(0, utils::va("^frustum_clip_planes L#%d ^7:: length <= 0 \n", __LINE__), 0);
			}

			Game::DpvsPlane* plane = &frustum_planes[planeIndex];

			plane->coeffs[0] = (1.0f / length) * plane->coeffs[0];
			plane->coeffs[1] = (1.0f / length) * plane->coeffs[1];
			plane->coeffs[2] = (1.0f / length) * plane->coeffs[2];
			plane->coeffs[3] = (1.0f / length) * plane->coeffs[3];

			set_dpvs_plane_sides(&frustum_planes[planeIndex]);
		}
	}

	// build view frustum
	void build_frustum_planes(Game::GfxViewParms *view_parms, Game::cplane_s *frustum_planes)
	{
		if (!view_parms) 
		{
			Game::Com_Error(0, COM_ERROR_MSG);
			return;
		}

		if (!frustum_planes) 
		{
			Game::Com_Error(0, COM_ERROR_MSG);
			return;
		}

		Game::DpvsPlane dpvs_frustum_planes[5];
		frustum_clip_planes(&view_parms->viewProjectionMatrix, frustumSidePlanes, 5, dpvs_frustum_planes);
		
		for (auto plane_index = 0u; plane_index < 5; ++plane_index)
		{
			Game::cplane_s* cplane = &frustum_planes[plane_index];
			const auto dpvs_plane = &dpvs_frustum_planes[plane_index];
			
			cplane->normal[0] = dpvs_plane->coeffs[0];
			cplane->normal[1] = dpvs_plane->coeffs[1];
			cplane->normal[2] = dpvs_plane->coeffs[2];
			cplane->dist	  = dpvs_plane->coeffs[3];
			
			frustum_planes[plane_index].dist = frustum_planes[plane_index].dist * -1.0f;

			char frustum_type;

			if (frustum_planes[plane_index].normal[0] == 1.0f) 
			{ 
				frustum_type = 0; 
			}
			else 
			{
				if (frustum_planes[plane_index].normal[1] == 1.0f) 
				{ 
					frustum_type = 1; 
				}
				else 
				{
					if (frustum_planes[plane_index].normal[2] == 1.0f) 
					{ 
						frustum_type = 2; 
					}
					else 
					{ 
						frustum_type = 3; 
					}	
				}
			}

			frustum_planes[plane_index].type = frustum_type;
			set_plane_signbits(&frustum_planes[plane_index]);
		}
	}

	// create plane for intersection (CM_GetPlaneVec4Form)
	void get_plane_vec4(const Game::cbrushside_t* sides, const Game::axialPlane_t* axial_planes, const int index, float* expanded_plane)
	{
		if (index >= 6)
		{
			if (!sides ) 
			{
				Game::Com_Error(0, COM_ERROR_MSG);
				return;
			}

			expanded_plane[0] = sides[index - 6].plane->normal[0];
			expanded_plane[1] = sides[index - 6].plane->normal[1];
			expanded_plane[2] = sides[index - 6].plane->normal[2];
			expanded_plane[3] = sides[index - 6].plane->dist;
		}
		else
		{
			glm::set_float3(expanded_plane, axial_planes[index].plane);
			expanded_plane[3] = axial_planes[index].dist;
		}
	}

	// intersect 3 planes
	int intersect_planes(const float *plane0, const float *plane1, const float *plane2, float *xyz)
	{
		float determinant;

		determinant = (((plane1[1] * plane2[2]) - (plane2[1] * plane1[2])) * plane0[0])
					+ (((plane2[1] * plane0[2]) - (plane0[1] * plane2[2])) * plane1[0])
					+ (((plane0[1] * plane1[2]) - (plane1[1] * plane0[2])) * plane2[0]);

		if (fabs(determinant) < 0.001f)
		{
			return 0;
		}

		determinant = 1.0f / determinant;

		xyz[0] = ((((plane1[1] * plane2[2]) - (plane2[1] * plane1[2])) * plane0[3])
				+ (((plane2[1] * plane0[2]) - (plane0[1] * plane2[2])) * plane1[3])
				+ (((plane0[1] * plane1[2]) - (plane1[1] * plane0[2])) * plane2[3])) * determinant;

		xyz[1] = ((((plane1[2] * plane2[0]) - (plane2[2] * plane1[0])) * plane0[3])
				+ (((plane2[2] * plane0[0]) - (plane0[2] * plane2[0])) * plane1[3])
				+ (((plane0[2] * plane1[0]) - (plane1[2] * plane0[0])) * plane2[3])) * determinant;

		xyz[2] = ((((plane1[0] * plane2[1]) - (plane2[0] * plane1[1])) * plane0[3])
				+ (((plane2[0] * plane0[1]) - (plane0[0] * plane2[1])) * plane1[3])
				+ (((plane0[0] * plane1[1]) - (plane1[0] * plane0[1])) * plane2[3])) * determinant;

		return 1;
	}

	// cod4map
	bool is_on_grid(const float *snapped, const float *xyz)
	{
		return xyz[0] == snapped[0] && xyz[1] == snapped[1] && xyz[2] == snapped[2];
	}

	// snap points to grid. might prod. some issues
	void snap_point_to_intersecting_planes(const float *plane0, const float *plane1, const float *plane2, float *xyz, float snap_grid, const float snap_epsilon)
	{
		float snapped[3], current_plane[4];

		snap_grid = 1.0f / snap_grid;

		// cod4map
		for (auto axis = 0; axis < 3; ++axis)
		{
			const float rounded = round(xyz[axis] * snap_grid) / snap_grid;
			const float delta	= fabs(rounded - xyz[axis]);

			if (snap_epsilon <= delta)
			{
				snapped[axis] = xyz[axis];
			}
			else
			{
				snapped[axis] = rounded;
			}
		}

		if (!is_on_grid(snapped, xyz))
		{
			float max_snap_error = 0.0f;
			float max_base_error = snap_epsilon;

			for (auto plane_index = 0; plane_index < 3; ++plane_index)
			{
				if (plane_index == 0)
				{
					memcpy(&current_plane, plane0, sizeof(current_plane));
				}
				else if (plane_index == 1)
				{
					memcpy(&current_plane, plane1, sizeof(current_plane));
				}
				else if (plane_index == 2)
				{
					memcpy(&current_plane, plane2, sizeof(current_plane));
				}

				const float snap_error = log((current_plane[0] * snapped[0] + current_plane[1] * snapped[1] + current_plane[2] * snapped[2]) - current_plane[3]);
				if (snap_error > max_snap_error)
				{
					max_snap_error = snap_error;
				}
					
				const float base_error = log((current_plane[0] * xyz[0] + current_plane[1] * xyz[1] + current_plane[2] * xyz[2]) - current_plane[3]);
				if (base_error > max_base_error) 
				{
					max_base_error = base_error;
				}
			}

			if (max_base_error > max_snap_error)
			{
				xyz[0] = snapped[0];
				xyz[1] = snapped[1];
				xyz[2] = snapped[2];
			}
		}
	}

	// add valid vertices from 3 plane intersections (CM_AddSimpleBrushPoint)
	int add_simple_brush_point(const Game::cbrush_t* brush, const Game::axialPlane_t* axial_planes, const __int16* side_indices, const float* xyz, int pt_count, Game::ShowCollisionBrushPt* brush_pts)
	{
		if (!brush)
		{
			Game::Com_Error(0, utils::va("CM_AddSimpleBrushPoint L#%d :: brush \n", __LINE__));
			return 0;
		}

		if (!brush_pts)
		{
			Game::Com_Error(0, utils::va("CM_AddSimpleBrushPoint L#%d :: brushPts \n", __LINE__));
			return 0;
		}

		for (auto side_index = 0u; side_index < 6; ++side_index)
		{
			if(( (axial_planes[side_index].plane.x * xyz[0] + axial_planes[side_index].plane.y * xyz[1] + axial_planes[side_index].plane.z * xyz[2])
				- axial_planes[side_index].dist) > 0.1f)
			{
				return pt_count;
			}
		}

		for (auto side_index = 0u; side_index < brush->numsides; ++side_index)
		{
			const auto plane = brush->sides[side_index].plane;

			if (   plane != brush->sides[side_indices[0] - 6].plane
				&& plane != brush->sides[side_indices[1] - 6].plane
				&& plane != brush->sides[side_indices[2] - 6].plane
				&& ((plane->normal[0] * xyz[0]) + (plane->normal[1] * xyz[1]) + (plane->normal[2] * xyz[2]) - plane->dist) > 0.1f)
			{
				return pt_count;
			}
		}

		if (pt_count > CM_MAX_BRUSHPOINTS_FROM_INTERSECTIONS - 2) // T5: 1024
		{
			Game::Com_PrintMessage(0, utils::va("add_simple_brush_point :: More than %i points from plane intersections on %i-sided brush\n", pt_count, brush->numsides), 0);
			return pt_count;
		}

		brush_pts[pt_count].xyz[0] = xyz[0];
		brush_pts[pt_count].xyz[1] = xyz[1];
		brush_pts[pt_count].xyz[2] = xyz[2];

		brush_pts[pt_count].sideIndex[0] = side_indices[0];
		brush_pts[pt_count].sideIndex[1] = side_indices[1];
		brush_pts[pt_count].sideIndex[2] = side_indices[2];

		return pt_count + 1;
	}

	// intersect 3 planes (for all planes) to reconstruct vertices (CM_ForEachBrushPlaneIntersection)
	int for_each_brush_plane_intersection(const Game::cbrush_t* brush, const Game::axialPlane_t* axial_planes, Game::ShowCollisionBrushPt* brush_pts)
	{
		if (!brush) 
		{
			Game::Com_Error(0, utils::va("for_each_brush_plane_intersection L#%d :: brush \n", __LINE__));
			return 0;
		}

		if (!brush_pts) 
		{
			Game::Com_Error(0, utils::va("for_each_brush_plane_intersection L#%d :: brushPts \n", __LINE__));
			return 0;
		}

		int pt_count = 0;
		const int side_count = brush->numsides + 6;
		std::int16_t side_index[3] = {};
		float expanded_plane[3][4] = {};

		// first loop should only get the axial planes till brush->numsides < 3
		for (side_index[0] = 0; side_index[0] < side_count - 2; ++side_index[0])
		{
			// sideIndex[0]-[5] are axial planes only; move the current plane into expandedPlane[0]
			get_plane_vec4(brush->sides, axial_planes, side_index[0], (float *)expanded_plane);

			// get a plane 1 plane ahead of our first plane
			for (side_index[1] = side_index[0] + 1; side_index[1] < side_count - 1; ++side_index[1])
			{
				// check if we're using an axial plane and 2 different planes
				if (side_index[0] < 6 || side_index[1] < 6 || brush->sides[side_index[0] - 6].plane != brush->sides[side_index[1] - 6].plane)
				{
					// move the current plane into expandedPlane[1]
					get_plane_vec4(brush->sides, axial_planes, side_index[1], expanded_plane[1]);

					// get a plane 1 plane ahead of our second plane
					for (side_index[2] = side_index[1] + 1; side_index[2] < side_count - 0; ++side_index[2])
					{
						// check if we use axial planes or atleast 3 different sides
						if ((   side_index[0] < 6 || side_index[2] < 6 || brush->sides[side_index[0] - 6].plane != brush->sides[side_index[2] - 6].plane)
							&& (side_index[1] < 6 || side_index[2] < 6 || brush->sides[side_index[1] - 6].plane != brush->sides[side_index[2] - 6].plane))
						{
							// move the current plane into expandedPlane[2]
							get_plane_vec4(brush->sides, axial_planes, side_index[2], expanded_plane[2]);

							// intersect the 3 planes
							float xyz[3];

							if (intersect_planes(expanded_plane[0], expanded_plane[1], expanded_plane[2], xyz))
							{
								// snap our verts in xyz onto the grid
								snap_point_to_intersecting_planes(expanded_plane[0], expanded_plane[1], expanded_plane[2], xyz, 0.25f, 0.0099999998f);

								// if the planes intersected, put verts into brushPts and increase our pointCount
								pt_count = add_simple_brush_point(brush, axial_planes, side_index, xyz, pt_count, brush_pts);

								if (pt_count >= CM_MAX_BRUSHPOINTS_FROM_INTERSECTIONS - 1)
								{
									return 0;
								}
							}
						}
					}
				}
			}
		}

		return pt_count;
	}

	// check for float precision errors and check if a point lies within an epsilon (VecNCompareCustomEpsilon)
	bool vec_compare_custom_epsilon(const glm::vec3* xyz_list, const int xyz_index, const float* v1, const float epsilon, const int coord_count)
	{
		for (auto i = 0; i < coord_count; ++i)
		{
			if (((xyz_list[xyz_index][i] - v1[i]) * (xyz_list[xyz_index][i] - v1[i])) > (epsilon * epsilon))
			{
				return false;
			}
		}

		return true;
	}
	
	// check if point exists (CM_PointInList)
	int point_exists_in_list(const float* point, const glm::vec3* xyz_list, const int xyz_count)
	{
		for (auto xyz_index = 0; xyz_index < xyz_count; ++xyz_index)
		{
			if (vec_compare_custom_epsilon(xyz_list, xyz_index, point, 0.1f, 3)) // larger epsilon decreases quality
			{
				return 1;
			}
		}

		return 0;
	}

	// create a list of vertex points (CM_GetXyzList)
	int get_xyz_list(const unsigned int side_index, const Game::ShowCollisionBrushPt* pts, const int pt_count, glm::vec3* xyz_list, const int xyz_limit)
	{
		int count = 0;

		if (!pts) 
		{
			Game::Com_Error(0, COM_ERROR_MSG);
			return 0;
		}

		for (auto index = 0; index < pt_count; ++index)
		{
			if ((  side_index == static_cast<std::uint16_t>( pts[index].sideIndex[0] ) 
				|| side_index == static_cast<std::uint16_t>( pts[index].sideIndex[1] ) 
				|| side_index == static_cast<std::uint16_t>( pts[index].sideIndex[2] ))
				&& !point_exists_in_list(pts[index].xyz, xyz_list, count))
			{
				if (count == xyz_limit) 
				{
					Game::Com_PrintMessage(0, utils::va("^1get_xyz_list L#%d ^7:: Winding point limit (%i) exceeded on brush face \n", __LINE__, xyz_limit), 0);
					return 0;
				}
#if DEBUG
				if (dvars::r_drawCollision_brushDebug->current.enabled)
				{
					Game::Com_PrintMessage(0, utils::va("^4get_xyz_list L#%d ^7:: Adding X:^2 %.2lf ^7Y:^2 %.2lf ^7Z:^2 %.2lf ^7 \n", __LINE__, xyz_list[count][0], xyz_list[count][1], xyz_list[count][2]), 0);
				}
#endif
				xyz_list[count] = glm::to_vec3(pts[index].xyz);
				++count;
			}
		}

#if DEBUG
		if (dvars::r_drawCollision_brushDebug->current.enabled)
		{
			Game::Com_PrintMessage(0, utils::va("^1get_xyz_list L#%d ^7:: Total XYZCOUNT: %d \n", __LINE__, count), 0);
		}
#endif

		return count;
	}

	// pick the major axis (CM_PickProjectionAxes)
	void pick_projection_axes(const float *normal, int *i, int *j)
	{
		int k = 0;

		if (fabs(normal[1]) > fabs(normal[0])) 
		{
			k = 1;
		}

		if (fabs(normal[2]) > fabs(normal[k]))
		{
			k = 2;
		}

		*i = ~k & 1;
		*j = ~k & 2;
	}
	
	// cross product (CM_SignedAreaForPointsProjected)
	float signed_area_for_points_projected(const float* pt0, const glm::vec3& pt1, const float* pt2, const int i, const int j)
	{
		return (pt2[j] - pt1[j]) * pt0[i] + (pt0[j] - pt2[j]) * pt1[i] + (pt1[j] - pt0[j]) * pt2[i];
	}

	// add a point that intersected behind another plane that still is within the bounding box? (CM_AddColinearExteriorPointToWindingProjected)
	void add_colinear_exterior_point_to_winding(Game::winding_t* w, const glm::vec3& pt, int i, int j, int index0, int index1)
	{
		float delta; int axis; 

#if DEBUG
		if (w->p[index0][i] == w->p[index1][i] && w->p[index0][j] == w->p[index1][j])
		{
			Game::Com_PrintMessage(0, utils::va("^1add_colinear_exterior_point_to_winding L#%d ^7:: w->p[%d][%d] %.2lf == w->p[%d][%d] %.2lf && w->p[%d][%d] %.2lf == w->p[%d][%d] %.2lf \n", 
												__LINE__, index0, i, w->p[index0][i], index1, i, w->p[index1][i], index0, j, w->p[index0][j], index1, j, w->p[index1][j]), 0);
		}
#endif

		if (fabs(float(uint32_t(w->p[index1][i] - w->p[index0][i]))) < fabs(float(uint32_t(w->p[index1][j] - w->p[index0][j]))))
		{
			axis = j;
			delta = w->p[index1][j] - w->p[index0][j];
		}
		else
		{
			axis = i;
			delta = w->p[index1][i] - w->p[index0][i];
		}

		if (delta <= 0.0f)
		{

#if DEBUG
			if (w->p[index0][axis] <= w->p[index1][axis])
			{
				Game::Com_PrintMessage(0, utils::va("^1add_colinear_exterior_point_to_winding L#%d ^7:: w->p[%d][%d] %.2lf <= w->p[%d][%d] %.2lf \n", 
													__LINE__, index0, axis, w->p[index0][axis], index1, axis, w->p[index1][axis]), 0);
			}
#endif

			if (pt[axis] <= w->p[index0][axis]) 
			{
				if (w->p[index1][axis] > pt[axis])
				{
					glm::set_float3(w->p[index1], pt);
				}
			}
			else
			{
				glm::set_float3(w->p[index0], pt);
			}
		}

		else
		{

#if DEBUG
			if (w->p[index1][axis] <= w->p[index0][axis])
			{
				Game::Com_PrintMessage(0, utils::va("^1add_colinear_exterior_point_to_winding L#%d ^7:: w->p[%d][%d] %.2lf < w->p[%d][%d] %.2lf \n", 
													__LINE__, index1, axis, w->p[index1][axis], index0, axis, w->p[index0][axis]), 0);
			}
#endif

			if (w->p[index0][axis] <= pt[axis]) 
			{
				if (pt[axis] > w->p[index1][axis])
				{
					glm::set_float3(w->p[index1], pt);
				}
			}
			else
			{
				glm::set_float3(w->p[index0], pt);
			}
		}
	}

	// Source :: PolyFromPlane || Q3 :: RemoveColinearPoints? (CM_AddExteriorPointToWindingProjected)
	void add_exterior_point_to_winding(Game::winding_t* w, const glm::vec3& pt, int i, int j)
	{
		int best_index = -1;
		float best_signed_area = FLT_MAX;

		int index_prev = w->numpoints - 1;

		for (auto index = 0; index < w->numpoints; ++index)
		{
			const float signed_area = signed_area_for_points_projected(w->p[index_prev], pt, w->p[index], i, j);

			if (best_signed_area > signed_area)
			{
				best_signed_area = signed_area;
				best_index = index;
			}

			index_prev = index;
		}

#if DEBUG
		if (best_index < 0)
		{
			Game::Com_PrintMessage(0, utils::va("^1CM_AddExteriorPointToWindingProjected L#%d ^7:: bestIndex < 0 \n", __LINE__), 0);
		}
#endif

		if (best_signed_area < -0.001f)
		{
			memmove((char *)w->p[best_index + 1], (char *)w->p[best_index], 12 * (w->numpoints - best_index));
			
			glm::set_float3(w->p[best_index], pt);
			++w->numpoints;
		}

		else if (best_signed_area <= 0.001f) 
		{
			add_colinear_exterior_point_to_winding(w, pt, i, j, (best_index + w->numpoints - 1) % w->numpoints, best_index);
		}
	}

	// create a triangle to check the winding order (CM_RepresentativeTriangleFromWinding)
	float representative_triangle_from_winding(const Game::winding_t *w, const float *normal, int *i0, int *i1, int *i2)
	{
		float area_best = 0.0f;
		float va[3], vb[3], vc[3];
		
		*i0 = 0;
		*i1 = 1;
		*i2 = 2;

		for (auto k = 2; k < w->numpoints; ++k)
		{
			for (auto j = 1; j < k; ++j)
			{
				vb[0] = w->p[k][0] - w->p[j][0];	
				vb[1] = w->p[k][1] - w->p[j][1];	
				vb[2] = w->p[k][2] - w->p[j][2];

				for (auto i = 0; i < j; ++i)
				{
					va[0] = w->p[i][0] - w->p[j][0];	
					va[1] = w->p[i][1] - w->p[j][1];	
					va[2] = w->p[i][2] - w->p[j][2];

					utils::vector::cross3(va, vb, vc);
					const float test_against = fabs(((vc[0] * normal[0]) + (vc[1] * normal[1])) + (vc[2] * normal[2]));

					if(test_against > 0.0f)
					{
						area_best = test_against;
						*i0 = i;
						*i1 = j;
						*i2 = k;
					}
				}
			}
		}

		return area_best;
	}

	// create a plane from points
	bool plane_from_points(float *plane, const float *v0, const float *v1, const float *v2)
	{
		float v2_v0[3], v1_v0[3];
		v1_v0[0] = v1[0] - v0[0];
		v1_v0[1] = v1[1] - v0[1];
		v1_v0[2] = v1[2] - v0[2];
		v2_v0[0] = v2[0] - v0[0];
		v2_v0[1] = v2[1] - v0[1];
		v2_v0[2] = v2[2] - v0[2];

		utils::vector::cross3(v2_v0, v1_v0, plane);
		const float length_sqr = ((plane[0] * plane[0]) + (plane[1] * plane[1])) + (plane[2] * plane[2]);

		if (length_sqr < 2.0f)
		{
			if (length_sqr == 0.0f)
			{
				return false;
			}

			if(utils::vector::length_squared3(v2_v0) * utils::vector::length_squared3(v1_v0) * 0.0000010000001f >= length_sqr)
			{
				v1_v0[0] = v2[0] - v1[0];
				v1_v0[1] = v2[1] - v1[1];
				v1_v0[2] = v2[2] - v1[2];
				v2_v0[0] = v0[0] - v1[0];
				v2_v0[1] = v0[1] - v1[1];
				v2_v0[2] = v0[2] - v1[2];

				utils::vector::cross3(v2_v0, v1_v0, plane);

				if (utils::vector::length_squared3(v2_v0) * utils::vector::length_squared3(v1_v0) * 0.0000010000001f >= length_sqr)
				{
					return false;
				}
			}
		}

		const float length = sqrt(length_sqr);
		plane[0] = plane[0] / length;
		plane[1] = plane[1] / length;
		plane[2] = plane[2] / length;
		plane[3] = ((v0[0] * plane[0]) + (v0[1] * plane[1])) + (v0[2] * plane[2]);
		
		return true;
	}

	// reverse clock-wise windings (CM_ReverseWinding)
	void reverse_winding(Game::winding_t *w)
	{
		for (auto i = 0; i < w->numpoints / 2; ++i)
		{
			const float s_winding[3] =
			{
				w->p[i][0], w->p[i][1], w->p[i][2]
			};

			w->p[i][0] = w->p[w->numpoints - 1 - i][0];
			w->p[i][1] = w->p[w->numpoints - 1 - i][1];
			w->p[i][2] = w->p[w->numpoints - 1 - i][2];

			w->p[w->numpoints - 1 - i][0] = s_winding[0];
			w->p[w->numpoints - 1 - i][1] = s_winding[1];
			w->p[w->numpoints - 1 - i][2] = s_winding[2];
		}
	}

	// Map Export - CM_BuildBrushWindingForSide
	bool mapexport_build_winding_for_side(Game::winding_t *winding, const float *plane_normal, const unsigned int side_index, Game::ShowCollisionBrushPt *pts, int pt_count, Game::map_brushSide_t *brush_side)
	{
		int i, i0, i1, i2, j;

		Game::vec4_t plane;
		utils::vector::zero4(plane);

		if (!winding)
		{
			Game::Com_Error(0, COM_ERROR_MSG);
			return false;
		}

		if (!plane_normal)
		{
			Game::Com_Error(0, COM_ERROR_MSG);
			return false;
		}

		if (!pts)
		{
			Game::Com_Error(0, COM_ERROR_MSG);
			return false;
		}

		glm::vec3 xyz_list[1024];
		const int xyzCount = get_xyz_list(side_index, pts, pt_count, xyz_list, 1024);

		if (xyzCount < 3) 
		{
			return false;
		}

		pick_projection_axes(plane_normal, &i, &j);

		glm::set_float3(winding->p[0], xyz_list[0]);
		glm::set_float3(winding->p[1], xyz_list[1]);

		winding->numpoints = 2;

		for (auto k = 2; k < xyzCount; ++k) 
		{
			add_exterior_point_to_winding(winding, xyz_list[k], i, j);
		}

		if (representative_triangle_from_winding(winding, plane_normal, &i0, &i1, &i2) < 0.001f) 
		{
			return false;
		}

		plane_from_points(&*plane, winding->p[i0], winding->p[i1], winding->p[i2]);

		if (utils::vector::dot3(plane, plane_normal) < 0.0f)
		{
			reverse_winding(winding);
		}

		// huh
		Game::winding_t *w = winding;

		for (auto _i = 0; _i < 3; _i++)
		{
			for (auto _j = 0; _j < 3; _j++)
			{ 
				if (fabs(w->p[_i][_j]) < dvars::mapexport_brushEpsilon1->current.value)
				{
					w->p[_i][_j] = 0;
				}
				else if (fabs((int)w->p[_i][_j] - w->p[_i][_j]) < dvars::mapexport_brushEpsilon2->current.value)
				{
					w->p[_i][_j] = (float)(int)w->p[_i][_j];
				}
			}
		}

		// *
		// create the brushside

		// plane 0
		for (auto idx = 0; idx < 3; idx++)
		{
			brush_side->brushPlane[0].point[idx] = w->p[0][idx];
		}

		// plane 1
		for (auto idx = 0; idx < 3; idx++) 
		{
			brush_side->brushPlane[1].point[idx] = w->p[1][idx];
		}

		// plane 2
		for (auto idx = 0; idx < 3; idx++)
		{
			brush_side->brushPlane[2].point[idx] = w->p[2][idx];
		}

		/*if (!utils::polylib::CheckWinding(w))
		{
			Game::Com_PrintMessage(0, "removed degenerate brushside.\n", 0);
			return false;
		}*/
		
		return true;
	}

	// build winding (poly) for side (CM_BuildBrushWindingForSide)
	bool build_brush_winding_for_side(Game::winding_t* winding, const float* plane_normal, const int side_index, Game::ShowCollisionBrushPt* pts, int pt_count)
	{
		int i, i0, i1, i2, j;
		Game::vec4_t plane; utils::vector::zero4(plane);

		if (!winding) 
		{
			Game::Com_Error(0, utils::va("CM_BuildBrushWindingForSide L#%d :: winding \n", __LINE__));
			return false;
		}

		if (!plane_normal) 
		{
			Game::Com_Error(0, COM_ERROR_MSG);
			return false;
		}

		if (!pts) 
		{
			Game::Com_Error(0, COM_ERROR_MSG);
			return false;
		}

		// create a list of vertex points
		glm::vec3 xyz_list[1024];
		const int xyzCount = get_xyz_list(side_index, pts, pt_count, xyz_list, 1024);

		// we need atleast a triangle to create a poly
		if (xyzCount < 3) 
		{
			return false;
		}

		// direction of camera plane
		const glm::vec3 camera_direction_to_plane = xyz_list[0] - Game::Globals::lpmove_camera_origin;

		// dot product between line from camera to the plane and the normal
		// if dot > 0 then the plane is facing away from the camera (dot = 1 = plane is facing the same way as the camera; dot = -1 = plane looking directly towards the camera)
		if (glm::dot( glm::vec3(plane_normal[0], plane_normal[1], plane_normal[2]), camera_direction_to_plane ) > 0.0f && !dvars::r_drawCollision_polyFace->current.enabled) 
		{
			return false;
		}
		
		// find the major axis
		pick_projection_axes(plane_normal, &i, &j);

		glm::set_float3(winding->p[0], xyz_list[0]);
		glm::set_float3(winding->p[1], xyz_list[1]);

		winding->numpoints = 2;

		for (auto k = 2; k < xyzCount; ++k) 
		{
			add_exterior_point_to_winding(winding, xyz_list[k], i, j);
		}

		// build a triangle of our winding points so we can check if the winding is clock-wise
		if (representative_triangle_from_winding(winding, plane_normal, &i0, &i1, &i2) < 0.001f) 
		{	
			// do nothing if it is counter clock-wise
			return false;
		}

		// *
		// winding is clock-wise ..

		// create a temp plane
		plane_from_points(&*plane, winding->p[i0], winding->p[i1], winding->p[i2]);

		// if our winding has a clock-wise winding, reverse it
		if (utils::vector::dot3(plane, plane_normal) > 0.0f)
		{
			reverse_winding(winding);
		}

		return true;
	}

	// Allocates a single brushside
	Game::map_brushSide_t *alloc_brush_side()
	{
		auto brush_side = static_cast<Game::map_brushSide_t*>(malloc(sizeof(Game::map_brushSide_t)));

		if (brush_side)
		{
			memset(brush_side, 0, sizeof(Game::map_brushSide_t));
			return brush_side;
		}
		
		Game::Com_Error(0, COM_ERROR_MSG);
		return nullptr;
	}

	bool is_brush_side_within_bounds(const Game::map_brushSide_t* brush_side, const glm::vec3& mins, const glm::vec3& maxs)
	{
		if (!brush_side)
		{
			return false;
		}

		for (auto plane = 0; plane < 3; plane++)
		{
			if (!utils::polylib::is_point_within_bounds(glm::to_vec3(brush_side->brushPlane[plane].point), mins, maxs, 0.25f))
			{
				return false;
			}
		}

		return true;
	}
	
	// rebuild and draw brush from bounding box and dynamic sides (CM_ShowSingleBrushCollision)
	void draw_single_brush_collision(Game::cbrush_t *brush, const float *color, [[maybe_unused]] int brush_index, bool enable_export = true)
	{
		if (!brush) 
		{
			Game::Com_Error(0, COM_ERROR_MSG);
			return;
		}

		if (!color) 
		{
			Game::Com_Error(0, COM_ERROR_MSG);
			return;
		}

		Game::ShowCollisionBrushPt brush_pts[CM_MAX_BRUSHPOINTS_FROM_INTERSECTIONS]; // T5: 1024 .. wtf

		// Create static sides (CM_BuildAxialPlanes)
		Game::axialPlane_t axial_planes[6];
		axial_planes[0].plane = glm::vec3(-1.0f, 0.0f, 0.0f);
		axial_planes[0].dist = -brush->mins[0];

		axial_planes[1].plane = glm::vec3(1.0f, 0.0f, 0.0f);
		axial_planes[1].dist = brush->maxs[0];

		axial_planes[2].plane = glm::vec3(0.0f, -1.0f, 0.0f);
		axial_planes[2].dist = -brush->mins[1];

		axial_planes[3].plane = glm::vec3(0.0f, 1.0f, 0.0f);
		axial_planes[3].dist = brush->maxs[1];

		axial_planes[4].plane = glm::vec3(0.0f, 0.0f, -1.0f);
		axial_planes[4].dist = -brush->mins[2];

		axial_planes[5].plane = glm::vec3(0.0f, 0.0f, 1.0f);
		axial_planes[5].dist = brush->maxs[2];

		// intersect all planes, 3 at a time, to to reconstruct face windings 
		const int pt_count = for_each_brush_plane_intersection(brush, axial_planes, brush_pts);

		// we need atleast 4 valid points
		if (pt_count >= 4)
		{
			// list of brushsides we are going to create within "CM_BuildBrushWindingForSideMapExport"
			std::vector<Game::map_brushSide_t*> map_brush;

			const auto poly_lit = dvars::r_drawCollision_polyLit->current.enabled;
			const auto poly_outlines = dvars::r_drawCollision->current.integer == 3 ? true : false;
			const auto poly_linecolor = dvars::r_drawCollision_lineColor->current.vector;
			const auto poly_depth = dvars::r_drawCollision_polyDepth->current.enabled;
			const auto poly_face = dvars::r_drawCollision_polyFace->current.enabled;

			// -------------------------------
			// brushside [0]-[5] (axialPlanes)

			for (auto side_index = 0u; side_index < 6; ++side_index)
			{
				float plane_normal[3];
				glm::set_float3(plane_normal, axial_planes[side_index].plane);

				// build winding for the current brushside and check if it is visible (culling)
				if (build_brush_winding_for_side((Game::winding_t*)&winding_pool_, plane_normal, side_index, brush_pts, pt_count))
				{
					if (dvars::r_drawCollision->current.integer == 1)
					{
						_debug::add_and_draw_debug_lines(winding_pool_.numpoints, (float(*)[3])&winding_pool_.p, dvars::r_drawCollision_lineColor->current.vector);
					}
					else
					{
						_debug::draw_poly(
							/* numPts	*/ winding_pool_.numpoints,
							/* points	*/ (float(*)[3])&winding_pool_.p, 
							/* pColor	*/ color, 
							/* pLit		*/ poly_lit,
							/* pOutline */ poly_outlines,
							/* pLineCol	*/ poly_linecolor,
							/* pDepth	*/ poly_depth,
							/* pFace	*/ poly_face);
					}
				
					Game::Globals::debug_collision_rendered_planes_counter++;
				}

				// create brushsides from brush bounds (side [0]-[5])
				if (mapexport_in_progress_ && enable_export)
				{
					// allocate a brushside
					Game::map_brushSide_t *brush_side = alloc_brush_side();

					// create a brushside from windings
					if (mapexport_build_winding_for_side((Game::winding_t*)&winding_pool_, plane_normal, side_index, brush_pts, pt_count, brush_side))
					{
						// brushside is valid
						map_brush.push_back(brush_side);
					}

					else
					{
						// not a valid brushside so free it
						free(brush_side);
					}
				}
			}

			// ---------------------------------
			// brushside [6] and up (additional)

			for (auto side_index = 6u; side_index < brush->numsides + 6; ++side_index)
			{
				if (build_brush_winding_for_side((Game::winding_t*)&winding_pool_, brush->sides[side_index - 6].plane->normal, side_index, brush_pts, pt_count))
				{
					if (dvars::r_drawCollision->current.integer == 1)
					{
						_debug::add_and_draw_debug_lines(winding_pool_.numpoints, (float(*)[3])&winding_pool_.p, dvars::r_drawCollision_lineColor->current.vector);
					}
					else
					{
						_debug::draw_poly(
							/* numPts	*/ winding_pool_.numpoints,
							/* points	*/ (float(*)[3])&winding_pool_.p,
							/* pColor	*/ color,
							/* pLit		*/ poly_lit,
							/* pOutline */ poly_outlines,
							/* pLineCol	*/ poly_linecolor,
							/* pDepth	*/ poly_depth,
							/* pFace	*/ poly_face);
					}

					Game::Globals::debug_collision_rendered_planes_counter++;
				}

				// create brushsides from cm->brushes->sides (side [6] and up)
				if (mapexport_in_progress_ && enable_export)
				{
					// allocate a brushside
					Game::map_brushSide_t *brush_side = alloc_brush_side();

					// create a brushside from windings
					if (mapexport_build_winding_for_side((Game::winding_t*)&winding_pool_, brush->sides[side_index - 6].plane->normal, side_index, brush_pts, pt_count, brush_side))
					{
						// brushside is valid
						map_brush.push_back(brush_side);
					}

					else
					{
						// not a valid brushside so free it
						free(brush_side);
					}
				}
			}

			// if we are exporting the map
			if (mapexport_in_progress_ && enable_export)
			{
				bool dirty_hack_5_sides = false;

				// we need atleast 6 valid brushsides
				if (map_brush.size() < 6)
				{
					if (map_brush.size() == 5 && dvars::mapexport_brush5Sides && dvars::mapexport_brush5Sides->current.enabled)
					{
						dirty_hack_5_sides = true;
					}
					else
					{
						return;
					}
				}

				const glm::vec3 brush_mins = glm::to_vec3(brush->mins);
				const glm::vec3 brush_maxs = glm::to_vec3(brush->maxs);

				// check brushes defined by more then their axialplanes
				if(map_brush.size() > 6)
				{
					if (glm::distance(brush_mins, brush_maxs) < dvars::mapexport_brushMinSize->current.value)
					{
						return;
					}
				}

				for (const auto& side : map_brush)
				{
					if (!is_brush_side_within_bounds(side, brush_mins, brush_maxs))
					{
						return;
					}
				}

				// swap brushsides (bottom, top, right, back, left, front)
				if (!dirty_hack_5_sides)
				{
					std::swap(map_brush[0], map_brush[5]);
				}
					
				std::swap(map_brush[3], map_brush[4]);
				std::swap(map_brush[1], map_brush[3]);
				std::swap(map_brush[0], map_brush[1]);

				// * 
				// do not export brushmodels as normal brushes
				// write brushside strings to g_mapBrushModelList instead

				if (brush->isSubmodel)
				{
					// clear any existing sides
					map_brushmodel_list_[brush->cmSubmodelIndex].brush_sides.clear();
				}
				else
				{
					// start brush
					mapexport_mapfile_ << utils::va("// brush %d\n{", mapexport_current_brush_index_) << std::endl;
					mapexport_mapfile_ << "layer \"000_Global/Brushes\"" << std::endl;

					// global brush exporting index count
					mapexport_current_brush_index_++;

					// write brush contents
					if (brush->contents & CM_CONTENTS_DETAIL)
					{
						mapexport_mapfile_ << "contents detail;" <<  std::endl;
					}
					else if (brush->contents & CM_CONTENTS_CLIPSHOT)
					{
						mapexport_mapfile_ << "contents weaponClip;" << std::endl;
					}
				}

				// print brush sides and material info
				for (auto bs = 0u; bs < map_brush.size(); bs++)
				{
					std::string brush_side_str = utils::va(" ( %d %d %d ) ( %d %d %d ) ( %d %d %d ) ",
						(int)map_brush[bs]->brushPlane[0].point[0], (int)map_brush[bs]->brushPlane[0].point[1], (int)map_brush[bs]->brushPlane[0].point[2],
						(int)map_brush[bs]->brushPlane[1].point[0], (int)map_brush[bs]->brushPlane[1].point[1], (int)map_brush[bs]->brushPlane[1].point[2],
						(int)map_brush[bs]->brushPlane[2].point[0], (int)map_brush[bs]->brushPlane[2].point[1], (int)map_brush[bs]->brushPlane[2].point[2]);

					if (!brush->isSubmodel)
					{
						mapexport_mapfile_ << brush_side_str.c_str();
					}

					// get material index for the current brush side
					int material_side_index = 0;

					// for the 6 brush sides created from axialplanes (brush bounds)
					if (bs < 6)
					{
						// get material (brush->axialMaterialNum[array][index]) for the current brush side
						// mapping axialnum order to .map brush side order
						switch (bs)
						{
						case 0: // bottom
							material_side_index = static_cast<int>( brush->axialMaterialNum[0][2] );
							break;
						case 1: // top
							material_side_index = static_cast<int>( brush->axialMaterialNum[1][2] );
							break;
						case 2: // right
							material_side_index = static_cast<int>( brush->axialMaterialNum[0][1] );
							break;
						case 3: // back
							material_side_index = static_cast<int>( brush->axialMaterialNum[1][0] );
							break;
						case 4: // left
							material_side_index = static_cast<int>( brush->axialMaterialNum[1][1] );
							break;

						case 5: // front
							material_side_index = static_cast<int>( brush->axialMaterialNum[0][0] );
							break;
						}
					}

					// we have atleast 1 additional brush side
					else
					{
						if (!dirty_hack_5_sides)
						{
							// additional brush sides start at index 0
							material_side_index = static_cast<int>( brush->sides[bs - 6].materialNum );
						}
					}

					// *
					// Material Dimensions

					// default values if we fail to find the correct texture size
					int texture_width = 128;
					int texture_height = 128;

					// texture size scalar (depends on texture quality settings)
					float tex_scalar = 1.0f;

					
					if (const auto& r_picmip = Game::Dvar_FindVar("r_picmip"); 
									r_picmip)
					{
						switch (r_picmip->current.integer)
						{
						case 0: // extra
							tex_scalar = 0.25f;
							break;

						case 1: // high
							tex_scalar = 0.5f;
							break;

						case 2: // normal
							tex_scalar = 1.0f;
							break;

						case 3:
							tex_scalar = 2.0f;
							break;

						default:
							tex_scalar = 1.0f;
						}
					}

					// get the world material and its size
					std::string material_name_for_brushside = Game::cm->materials[material_side_index].material;
					std::string material_for_side = "wc/"s + material_name_for_brushside;

					
					if (const auto	material = Game::Material_RegisterHandle(material_for_side.c_str(), 3); 
									material)
					{
						for (auto tex = 0; tex < material->textureCount; tex++)
						{
							// 0x2 = color, 0x5 = normal, 0x8 = spec
							if (material->textureTable[tex].u.image->semantic == 0x2)
							{
								texture_width = static_cast<int>((material->textureTable[tex].u.image->width * tex_scalar)); // loaded texture sizes vary (texture quality settings)
								texture_height = static_cast<int>((material->textureTable[tex].u.image->height * tex_scalar)); // so we need to use a scalar to get a 0.25 stretch in radiant

								break;
							}
						}
					}

					if (!brush->isSubmodel)
					{
						// materialname, width, height, xpos, ypos, rotation, ?, lightmapMat, lmap_sampleSizeX, lmap_sampleSizeY, lmap_xpos, lmap_ypos, lmap_rotation, ?
						mapexport_mapfile_ << utils::va("%s %d %d 0 0 0 0 lightmap_gray 16384 16384 0 0 0 0\n", material_name_for_brushside.c_str(), texture_width, texture_height);
					}
					else
					{
						map_brushmodel_list_[brush->cmSubmodelIndex].brush_sides.push_back(brush_side_str + utils::va("%s %d %d 0 0 0 0 lightmap_gray 16384 16384 0 0 0 0\n", material_name_for_brushside.c_str(), texture_width, texture_height));
					}
					
				}

				if (!brush->isSubmodel)
				{
					// end brush
					mapexport_mapfile_ << "}" << std::endl;
				}
			}
		}
	}

	// Brush View Culling
	int box_on_plane_side(const float *emins, const float *emaxs, const Game::cplane_s *p)
	{
		// fast axial cases
		if (p->type < 3)
		{
			if (p->dist <= emins[static_cast<std::uint16_t>(p->type)])
			{
				return 1;
			}

			if (p->dist >= emaxs[static_cast<std::uint16_t>(p->type)])
			{
				return 2;
			}
			
			return 3;
		}

		float dist1, dist2;

		// general case
		switch (p->signbits)
		{
		case 0:
			dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
			dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
			break;

		case 1:
			dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
			dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
			break;

		case 2:
			dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
			dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
			break;

		case 3:
			dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
			dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
			break;

		case 4:
			dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
			dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
			break;

		case 5:
			dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
			dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
			break;

		case 6:
			dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
			dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
			break;

		case 7:
			dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
			dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
			break;

		default:
			dist1 = dist2 = 0; // shut up compiler
			break;
		}

		int sides = 0;

		if (dist1 >= p->dist) 
		{ 
			sides = 1; 
		}

		if (dist2 < p->dist) 
		{ 
			sides |= 2; 
		}

		return sides;
	}

	// check if any side of a brush is within the view frustum (CM_BrushInView)
	bool brush_in_view(Game::cbrush_t *brush, Game::cplane_s *frustum_planes, unsigned int frustum_plane_count)
	{
		if (!frustum_planes) 
		{ 
			Game::Com_Error(0, COM_ERROR_MSG);
			return false;
		}

		for (auto i = 0u; i < frustum_plane_count; ++i)
		{
			if (!(box_on_plane_side(brush->mins, brush->maxs, &frustum_planes[i]) & 1))
			{
				return false;
			}
		}

		return true;
	}

	// check if the material selected via dvar equals the current brush material or one of its sides if the first side uses caulk
	bool is_valid_brush_material_selection(const Game::cbrush_t* brush, int material_num_from_dvar)
	{
		// we can also filter materials by substrings with "r_drawCollision_materialInclude" ( "clip_player" returns true when using "clip" )
		std::string include_string;

		// workaround till we get dvar strings to register - actually, cba. to implement that now
		switch (dvars::r_drawCollision_materialInclude->current.integer)
		{
		case 1:
			include_string = "clip";
			break;

		case 2:
			include_string = "mantle";
			break;

		case 3:
			include_string = "trigger";
			break;

		case 4:
			include_string = "all"; 
			return true; // no need to check any material so return true

		case 5:
			include_string = "all-no-tools";
			break;
		case 6:
			include_string = "all-no-tools-clip";
			break;
		}

		// check if we are within array bounds
		if (static_cast<size_t>(brush->axialMaterialNum[0][0]) >= map_material_list_duplicates_.size())
		{
			return false;
		}

		if (static_cast<size_t>(material_num_from_dvar) >= map_material_list_single_.size())
		{
			return false;
		}

		// bridge dupe materials list with the cleaned list
		const std::string material_name_from_duplicates = map_material_list_duplicates_[(int)brush->axialMaterialNum[0][0]];
		const std::string material_name_from_single = map_material_list_single_[material_num_from_dvar];
		
		// instantly found our material
		if (material_name_from_duplicates == material_name_from_single) 
		{
			return true;
		}

		// if filter is not empty
		if (!include_string.empty())
		{
			// if we found a matching substring
			if (material_name_from_duplicates.contains(include_string))
			{
				return true;
			}

			// draw all materials (mainly for map exporting)
			if (include_string == "all") 
			{
				return true;
			}

			// draw all materials without info volumes
			if (include_string == "all-no-tools") 
			{
				if(material_name_from_duplicates.contains("portal"))
					return false;

				if (material_name_from_duplicates.contains("hint"))
					return false;

				if (material_name_from_duplicates.contains("volume"))
					return false;

				if (material_name_from_duplicates.contains("mantle"))
					return false;

				if (material_name_from_duplicates.contains("trigger"))
					return false;

				if (material_name_from_duplicates.contains("sky"))
					return false;

				// if no excluded materials were found, return true
				return true;
			}

			if (include_string == "all-no-tools-clip")
			{
				if (material_name_from_duplicates.contains("portal"))
					return false;

				if (material_name_from_duplicates.contains("hint"))
					return false;

				if (material_name_from_duplicates.contains("volume"))
					return false;

				if (material_name_from_duplicates.contains("mantle"))
					return false;

				if (material_name_from_duplicates.contains("trigger"))
					return false;

				if (material_name_from_duplicates.contains("sky"))
					return false;

				if (material_name_from_duplicates.contains("clip"))
					return false;

				// if no excluded materials were found, return true
				return true;
			}
		}

		// if the brush has caulk as its first side, check the other sides for our material/substring
		else if (material_name_from_duplicates == "caulk")
		{
			int currentMatArray;

			// check the first and second material array
			for (currentMatArray = 0; currentMatArray < 2; currentMatArray++)
			{
				// check its 3 elements
				for (int matIndex = 0; matIndex < 3; matIndex++)
				{
					// if one of the sides uses our material
					if (map_material_list_duplicates_[(int)brush->axialMaterialNum[currentMatArray][matIndex]] == material_name_from_single) 
					{
						return true;
					}

					// if we have a filter set
					if (dvars::r_drawCollision_materialInclude->current.integer != 0)
					{
						// if one of the sides matches our substring
						if (map_material_list_duplicates_[(int)brush->axialMaterialNum[currentMatArray][matIndex]].find(include_string) != std::string::npos) 
						{
							return true;
						}
					}
				}
			}
		}

		// brush does not contain our material/substring
		return false;
	}

	// resets a selection bounding box
	void sbox_reset(Game::boundingbox_s* sbox)
	{
		memset(sbox, 0, sizeof(Game::boundingbox_s));

		sbox->mins = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		sbox->maxs = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		sbox->was_reset = true;
	}

	// get mins/maxs from points (CM_BoundsFromPoints)
	void bounds_from_points(const int& num_points, const glm::vec3* points, glm::vec3& mins, glm::vec3& maxs)
	{
		mins = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		maxs = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (auto pt = 0; pt < num_points; pt++)
		{
			for (auto axis = 0; axis < 3; axis++)
			{
				// mins :: find the closest point on each axis
				if (mins[axis] > points[pt][axis])
					mins[axis] = points[pt][axis];

				// maxs :: find the furthest point on each axis
				if (maxs[axis] < points[pt][axis])
					maxs[axis] = points[pt][axis];
			}
		}
	}

	// calculate brush midpoint
	glm::vec3 get_brush_midpoint(const Game::cbrush_t* brush, bool xyz_midpoint = false)
	{
		if (!xyz_midpoint)
		{
			return (glm::vec3(brush->mins[0], brush->mins[1], 0.0f) 
				  + glm::vec3(brush->maxs[0], brush->maxs[1], 0.0f)) * 0.5f;
		}

		return (glm::vec3(brush->mins[0], brush->mins[1], brush->mins[2]) 
			  + glm::vec3(brush->maxs[0], brush->maxs[1], brush->maxs[2])) * 0.5f;
		
	}

	// check if a brushes midpoint is within a selection box
	bool sbox_is_brush_inside(const Game::cbrush_t* brush, const Game::boundingbox_s* sbox)
	{
		if (brush && sbox->is_box_valid)
		{
			return utils::polylib::is_point_within_bounds(get_brush_midpoint(brush, true), sbox->mins, sbox->maxs, 0.25f);
		}
		
		return false;
	}

	// check if a triangles midpoint is within a selection box
	bool sbox_is_triangle_inside(const Game::map_patchTris_t* tris, const Game::boundingbox_s* sbox)
	{
		for (auto t = 0; t < 3; t++)
		{
			if(!utils::polylib::is_point_within_bounds(glm::to_vec3(tris->coords[t].xyz), sbox->mins, sbox->maxs, 0.25f))
			{
				return false;
			}
		}

		return true;
	}

	// once per map after r_drawcollision was turned on
	// :: create "map_material_list_duplicates_" and write all materials without \0
	// :: create "map_material_list_single_" and add all material from map_material_list_duplicates_ without creating duplicates
	// :: create a dvar description from "map_material_list_single_"
	// :: compare axialMaterialNum with "map_material_list_duplicates_" to get the original material name
	// :: compare "map_material_list_duplicates_" material name with the choosen dvar material name
	// :: draw the brush if they match
	void init_once_per_map()
	{
		// assign a color to each brush (modified cbrush_t struct) so we use persistent colors even when sorting brushes
		int color_counter = 0;

		// only once per map
		if(!Game::Globals::debug_collision_initialized || utils::q_stricmp(map_clipmap_name_, Game::cm->name))
		{
			Game::Com_PrintMessage(0, "[Debug Collision] : initializing ...\n", 0);

			// list of all brushmodels within the current map mapped to their respective brushes in cm->brushes

			const char* mapents_ptr = Game::cm->mapEnts->entityString;

			if(_map::mpsp_is_sp_map)
			{
				mapents_ptr = _map::mpsp_mapents_original;
			}

			utils::entities mapEnts(mapents_ptr);
			map_brushmodel_list_ = mapEnts.get_brushmodels();

			Game::Com_PrintMessage(0, utils::va("|-> found %d submodels\n", static_cast<int>(map_brushmodel_list_.size())), 0);

			// hacky string dvar
			const char* debug_collision_rendered_planes_amount = "r_drawCollision_brushIndexFilter";
			Game::Dvar_RegisterString_hacky(debug_collision_rendered_planes_amount, "null", "Specifies which brushes to draw. Ignores all other filters and will disable brush sorting.\nInput example: \"101 99 2\" :: \"^1null^7\" disables this filter");

			// assign hacky dvar to the global dvar
			dvars::r_drawCollision_brushIndexFilter = Game::Dvar_FindVar(debug_collision_rendered_planes_amount);

#if DEBUG
			// reduce default visible brush count on debug builds
			Game::Dvar_SetValue(dvars::r_drawCollision_brushAmount, 1);
#endif

			// cap brushAmount
			dvars::r_drawCollision_brushAmount->domain.integer.max = Game::cm->numBrushes - 1;

			// reset brush distance filter
			Game::Dvar_SetValue(dvars::r_drawCollision_brushDist, 800.0f);

			// clear global vectors
			map_brush_list_.clear();
			map_material_list_.clear();
			map_material_list_duplicates_.clear();
			map_material_list_single_.clear();
			Game::Globals::r_drawCollision_materialList_string.clear();

			// init/reset the selection box
			sbox_reset(&mapexport_selectionbox_);

			// create a dmaterial_t vector
			const std::uint32_t clipmap_material_index = Game::cm->numMaterials;
			std::vector<Game::dmaterial_t*> map_materials(clipmap_material_index);

			// assign clipmap material pointers
			for (auto num = 0u; num < clipmap_material_index; num++) 
			{
				map_materials[num] = &Game::cm->materials[num];
			}

			// create a cbrush_t vector
			std::vector<Game::cbrush_t*> brush_list(Game::cm->numBrushes);

			// assign clipMap brush pointers
			for (auto num = 0u; num < Game::cm->numBrushes; num++)
			{
				brush_list[num] = &Game::cm->brushes[num];
				brush_list[num]->colorCounter = (short)color_counter++;
				brush_list[num]->cmBrushIndex = (short)num;

				color_counter %= 8;
			}

			// set globals
			Game::Globals::debug_collision_initialized = true;
			map_clipmap_name_ = Game::cm->name;
			map_material_list_ = map_materials;
			map_brush_list_ = brush_list;

			// create a string vector of all material (contains duplicates)
			for (auto num = 0u; num < map_material_list_.size(); num++)
			{
				std::string materialName = utils::convert_to_string(map_material_list_[num]->material, sizeof(map_material_list_[num]->material));
				materialName.erase(std::remove(materialName.begin(), materialName.end(), '\0'), materialName.end());

				map_material_list_duplicates_.push_back(materialName);
			}

			// create a string vector without duplicate materials (used for dvar description)
			for (auto num = 0u; num < map_material_list_duplicates_.size(); num++)
			{
				std::string materialName = map_material_list_duplicates_[num];

				// if the vector is empty or the materialName does not exist within the array
				if (map_material_list_single_.empty() || std::ranges::find(map_material_list_single_.begin(), map_material_list_single_.end(), materialName) == map_material_list_single_.end())
				{
					map_material_list_single_.push_back(materialName);
				}
			}

			// create the dvar string
			const auto single_material_count = map_material_list_single_.size();

			for (auto num = 0u; num < single_material_count; num++) 
			{
				// do not print "empty" materials
				if (!utils::starts_with(map_material_list_single_[num], "*"))
				{
					Game::Globals::r_drawCollision_materialList_string += std::to_string(num) + ": " + map_material_list_single_[num] + "\n";
				}
			}

			Game::Com_PrintMessage(0, utils::va("|-> found %d materials\n", static_cast<int>(single_material_count)), 0);

			// set material dvar back to 0, update description and max value
			Game::Dvar_SetValue(dvars::r_drawCollision_material, 0);
			dvars::r_drawCollision_material->domain.integer.max = static_cast<int>( single_material_count - 1 );

			// print the material list to the large console if the list has more then 20 materials
			if (single_material_count > 20)
			{
				dvars::r_drawCollision_material->description = "Too many materials to show here! Use ^1\"r_drawCollision_materialList\" ^7to print a list of all materials.\nOnly works with ^1\"r_drawCollision\" ^7enabled!";
			}
			else
			{
				dvars::r_drawCollision_material->description = Game::Globals::r_drawCollision_materialList_string.c_str();
			}

			Game::Com_PrintMessage(0, "|-> done\n", 0);
		}
	}

	// sort globBrushList depending on distance from brush to camera
	void sort_brush_list_by_distance(bool far_to_near = true, bool use_current_brushes_drawn_list = false, bool always_update = false)
	{
		if (!use_current_brushes_drawn_list)
		{
			// sort brushes by distance from brush midpoint(xy) to camera origin :: only sort if the players origin has changed
			if (last_sorting_origin_[0] != Game::Globals::lpmove_origin[0] || last_sorting_origin_[1] != Game::Globals::lpmove_origin[1] || always_update)
			{
				// sort from far to near (used for rendering)
				if (far_to_near)
				{
					std::ranges::sort(map_brush_list_.begin(), map_brush_list_.end(), [](Game::cbrush_t *brush1, Game::cbrush_t *brush2)
					{
						const glm::vec3 b1_mid = get_brush_midpoint(brush1, false);
						const glm::vec3 b2_mid = get_brush_midpoint(brush2, false);

						return glm::distance(b1_mid, Game::Globals::lpmove_origin) > glm::distance(b2_mid, Game::Globals::lpmove_origin);
					});
				}

				// sort from near to far (get closest brush)
				else
				{
					std::ranges::sort(map_brush_list_.begin(), map_brush_list_.end(), [](Game::cbrush_t *brush1, Game::cbrush_t *brush2)
					{
						const glm::vec3 b1_mid = get_brush_midpoint(brush1, true);
						const glm::vec3 b2_mid = get_brush_midpoint(brush2, true);

						return glm::distance(b1_mid, Game::Globals::lpmove_origin) < glm::distance(b2_mid, Game::Globals::lpmove_origin);
					});
				}

				last_sorting_origin_ = Game::Globals::lpmove_origin;
			}
		}

		// use drawn brushes list if not empty
		else if(!map_brush_list_for_index_filtering_.empty())
		{
			// sort brushes by distance from brush midpoint(xy) to camera origin :: only sort if the players origin has changed
			if (last_sorting_origin_[0] != Game::Globals::lpmove_origin[0] || last_sorting_origin_[1] != Game::Globals::lpmove_origin[1] || always_update)
			{
				// sort from far to near (used for rendering)
				if (far_to_near)
				{
					std::ranges::sort(map_brush_list_for_index_filtering_.begin(), map_brush_list_for_index_filtering_.end(), [](Game::cbrush_t *brush1, Game::cbrush_t *brush2)
					{
						const glm::vec3 b1_mid = get_brush_midpoint(brush1, false);
						const glm::vec3 b2_mid = get_brush_midpoint(brush2, false);

						return glm::distance(b1_mid, Game::Globals::lpmove_origin) > glm::distance(b2_mid, Game::Globals::lpmove_origin);
					});
				}

				// sort from near to far (get closest brush)
				else
				{
					std::ranges::sort(map_brush_list_for_index_filtering_.begin(), map_brush_list_for_index_filtering_.end(), [](Game::cbrush_t *brush1, Game::cbrush_t *brush2)
					{
						const glm::vec3 b1_mid = get_brush_midpoint(brush1, true);
						const glm::vec3 b2_mid = get_brush_midpoint(brush2, true);

						return glm::distance(b1_mid, Game::Globals::lpmove_origin) < glm::distance(b2_mid, Game::Globals::lpmove_origin);
					});
				}

				last_sorting_origin_ = Game::Globals::lpmove_origin;
			}
		}
	}

	// color brushes depending on their index within cm->brushes (CM_GetShowCollisionColor)
	void get_collision_color(float *color, const int counter)
	{
		if (!color) 
		{
			Game::Com_Error(0, COM_ERROR_MSG);
			return;
		}

		if (counter & 1)
		{
			color[0] = 1.0f;
		}
		else
		{
			color[0] = 0.0f;
		}

		if (counter & 2)
		{
			color[1] = 1.0f;
		}
		else
		{
			color[1] = 0.0f;
		}

		if (counter & 4)
		{
			color[2] = 1.0f;
		}
		else
		{
			color[2] = 0.0f;
		}

		color[3] = dvars::r_drawCollision_polyAlpha->current.value;
	}
	
	// check if 2 triangles share an edge
	bool patch_tri_shares_edge(const Game::map_patchTris_t *ptri1, const Game::map_patchTris_t *ptri2)
	{
		int matched_points = 0;

		// for each edge of triangle 1
		for (auto edge_tri1 = 0; edge_tri1 < 3; edge_tri1++)
		{
			// for each edge of triangle 2
			for (auto edge_tri2 = 0; edge_tri2 < 3; edge_tri2++)
			{
				matched_points += utils::vector::compare3(ptri1->coords[edge_tri1].xyz, ptri2->coords[edge_tri2].xyz);
			}

			// shouldnt happen
			if (matched_points > 2)
			{
				Game::Com_PrintMessage(0, "^1[MAP-EXPORT]: ^7 PatchTriangle_SharesEdge matchedPoints > 2", 0);
			}

			if (matched_points == 2)
			{
				return true;
			}
		}

		return false;
	}

	// return true if clock wise
	bool patch_tri_clockwise_winding(const float *pt0, const float *pt1, const float *pt2)
	{
		const float k = (pt1[1] - pt0[1]) * (pt2[0] - pt1[0]) - (pt1[0] - pt0[0]) * (pt2[1] - pt1[1]);
		return k > 0.001f;
	}

	// not in map format order
	void patch_tri_from_incides(Game::map_patchTris_t *ptris, const unsigned short *incides)
	{
		bool found_first = false, found_second = false, found_third = false, second_iteration = false;

		// find the first match
		for (auto gfx_vert = 0u; gfx_vert < Game::_gfxWorld->vertexCount; gfx_vert++)
		{
			// try to match our first clipmap vertex to a gfxworld vertex
			if (!found_first && utils::vector::compare3(Game::cm->verts[incides[0]], Game::_gfxWorld->vd.vertices[gfx_vert].xyz))
			{
				// found the first corrosponding gfx vertex :: copy gfx vertex data to our temp vertex
				memcpy(&ptris->coords[0], &Game::_gfxWorld->vd.vertices[gfx_vert], sizeof(Game::GfxWorldVertex));

				// go back 6 verts in the gfxworld vertex array and start searching for our second point
				// :: assuming that our verts are close to each other
				if (gfx_vert - 6 > 0) 
				{
					gfx_vert -= 6; 
				}

				else 
				{
					gfx_vert = 0; 
				}

				// do not match first vert again
				found_first = true;
			}

			// try to match our second clipmap vertex to a gfxworld vertex
			if (!found_second && utils::vector::compare3(Game::cm->verts[incides[1]], Game::_gfxWorld->vd.vertices[gfx_vert].xyz))
			{
				// found the second corrosponding gfx vertex :: copy gfx vertex data to our temp vertex
				memcpy(&ptris->coords[1], &Game::_gfxWorld->vd.vertices[gfx_vert], sizeof(Game::GfxWorldVertex));

				// go back 6 verts in the gfxworld vertex array
				// :: assuming that our verts are close to each other
				if (gfx_vert - 6 > 0)
				{
					gfx_vert -= 6;
				}

				else
				{
					gfx_vert = 0;
				}

				// do not match second vert again
				found_second = true;
			}

			// try to match our third clipmap vertex to a gfxworld vertex
			if (!found_third && utils::vector::compare3(Game::cm->verts[incides[2]], Game::_gfxWorld->vd.vertices[gfx_vert].xyz))
			{
				// found the third corrosponding gfx vertex :: copy gfx vertex data to our temp vertex
				memcpy(&ptris->coords[2], &Game::_gfxWorld->vd.vertices[gfx_vert], sizeof(Game::GfxWorldVertex));

				// go back 6 verts in the gfxworld vertex array
				// :: assuming that our verts are close to each other
				if (gfx_vert - 6 > 0)
				{
					gfx_vert -= 6;
				}

				else
				{
					gfx_vert = 0;
				}

				// do not match third vert again
				found_third = true;
			}

			if (found_first && found_second && found_third)
			{
				break;
			}

			// if we did not match all 3 and looped all of the gfxworld vertices
			if (gfx_vert + 1 > Game::_gfxWorld->vertexCount && !second_iteration)
			{
				// check the whole array once again (as we only gone back 6 verts each time we found a corrosponding vert)
				gfx_vert = 0;

				// break out of the loop if we failed to match all 3 verts within the second iteration
				second_iteration = true;
			}
		}

		// if we failed to match all points, write clipmap vert data
		if (!found_first)
		{
			ptris->coords[0].xyz[0] = Game::cm->verts[incides[0]][0];
			ptris->coords[0].xyz[1] = Game::cm->verts[incides[0]][1];
			ptris->coords[0].xyz[2] = Game::cm->verts[incides[0]][2];
		}

		if (!found_second)
		{
			ptris->coords[1].xyz[0] = Game::cm->verts[incides[1]][0];
			ptris->coords[1].xyz[1] = Game::cm->verts[incides[1]][1];
			ptris->coords[1].xyz[2] = Game::cm->verts[incides[1]][2];
		}

		if (!found_third)
		{
			ptris->coords[2].xyz[0] = Game::cm->verts[incides[2]][0];
			ptris->coords[2].xyz[1] = Game::cm->verts[incides[2]][1];
			ptris->coords[2].xyz[2] = Game::cm->verts[incides[2]][2];
		}
	}

	// Allocates a single patch triangle
	Game::map_patchTris_t *alloc_patch_triangle()
	{
		if (auto tri = static_cast<Game::map_patchTris_t*>( malloc(sizeof(Game::map_patchTris_t)) ); 
				 tri)
		{
			memset(tri, 0, sizeof(Game::map_patchTris_t));
			return tri;
		}

		Game::Com_Error(0, COM_ERROR_MSG);
		return nullptr;
	}

	// Allocates a single patch quad
	Game::map_patchQuads_t *alloc_patch_quad()
	{
		if (auto quad = static_cast<Game::map_patchQuads_t*>(malloc(sizeof(Game::map_patchQuads_t)));
				 quad)
		{
			memset(quad, 0, sizeof(Game::map_patchQuads_t));
			return quad;
		}

		Game::Com_Error(0, COM_ERROR_MSG);
		return nullptr;
	}

	// coord to check = xy; bounds = xywh
	bool is_coordinate_within_bounds_2d(const float *coord, const float *bounds)
	{
		// if pt to right of left border
		if (coord[0] >= bounds[0]) // bounds x
		{
			// if pt to left of right border
			if (coord[0] <= (bounds[0] + bounds[2])) // bounds x + w
			{
				// if pt above bottom border
				if (coord[1] >= bounds[1]) // bounds y
				{
					// if pt below top border
					if (coord[1] <= (bounds[1] + bounds[3])) // bounds y + h
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	// quad in map format order (tiangles need to share an edge or it will fail)
	bool single_patch_quad_from_triangle(Game::map_patchQuads_t *quad, const Game::map_patchTris_t *ptri1, const Game::map_patchTris_t *ptri2, const bool check_skew = false)
	{
		// temp quad points
		std::vector v_coord(3, 0.0f);
		std::vector v_quad_pts(4, v_coord);

		int unique_pt_count = 0;

		// for each coord of triangle 1
		for (auto t1 = 0; t1 < 3; t1++)
		{
			int shared_pt_count = 0;

			// for each coord of triangle 2
			for (auto t2 = 0; t2 < 3; t2++)
			{
				// check if the current coord from tri 1 is unique and not part of tri 2
				if (utils::vector::compare3(ptri1->coords[t1].xyz, ptri2->coords[t2].xyz))
				{
					// found shared point
					shared_pt_count++;
					break;
				}
			}

			// check if we found the unique coord of triangle 1
			if (!shared_pt_count)
			{
				// add unique point that isnt on the shared edge
				v_quad_pts[0][0] = ptri1->coords[t1].xyz[0];
				v_quad_pts[0][1] = ptri1->coords[t1].xyz[1];
				v_quad_pts[0][2] = ptri1->coords[t1].xyz[2];
				
				unique_pt_count++;
				break;
			}
		}

		// if we found the unique point
		if (unique_pt_count)
		{
			// we should only have 1 point from the first tiangle in our list, otherwise, throw an error?
			if (unique_pt_count > 1)
			{
				Game::Com_PrintMessage(0, "^1[MAP-EXPORT]: ^7 PatchQuad_SingleFromTris t_quadPts.size() > 1", 0);
				return false;
			}

			for (auto secondTriCoords = 0; secondTriCoords < 3; secondTriCoords++)
			{
				// add point that isnt on the shared edge
				v_quad_pts[secondTriCoords + 1][0] = ptri2->coords[secondTriCoords].xyz[0];
				v_quad_pts[secondTriCoords + 1][1] = ptri2->coords[secondTriCoords].xyz[1];
				v_quad_pts[secondTriCoords + 1][2] = ptri2->coords[secondTriCoords].xyz[2];
			}

			// unpack the normal .. might be needed for texture info later down the line
			Game::vec3_t normal; 
			utils::vector::unpack_unit_vec3(ptri2->coords[1].normal, normal);

			// sort x accending
			std::ranges::sort(v_quad_pts.begin(), v_quad_pts.end());

			// x was sorted, now sort by y
			std::ranges::sort(v_quad_pts.begin(), v_quad_pts.end());

			// always check clockwise ordering?
			float pt0[3], pt1[3], pt2[3], pt3[3];

			// why did i do that
			memcpy(&pt0, &v_quad_pts[0][0], sizeof(float[3]));
			memcpy(&pt1, &v_quad_pts[1][0], sizeof(float[3]));
			memcpy(&pt2, &v_quad_pts[2][0], sizeof(float[3]));
			memcpy(&pt3, &v_quad_pts[3][0], sizeof(float[3]));

			if(!patch_tri_clockwise_winding(pt0, pt1, pt2))
			{
				// triangle is counter clockwise, discard
				return false;
			}

			if (check_skew)
			{
				/*Game::vec3_t dirVec1, dirVec2; 
				utils::vector::_VectorZero(dirVec1); utils::vector::_VectorZero(dirVec2);
				
				utils::vector::_VectorSubtract(pt0, pt1, dirVec1);
				utils::vector::_VectorSubtract(pt2, pt1, dirVec2);

				utils::vector::_VectorNormalize(dirVec1);
				utils::vector::_VectorNormalize(dirVec2);

				float dot = utils::vector::_DotProduct(dirVec1, dirVec2);
				float angle = acosf(dot) * 180.0f / 3.141592f;*/

				//if(angle > 70.0f || angle < 10.0f )
				//	return false;

				// triangle can be in clockwise order and still be using a point of a neighboring triangle 
				// project a quad from the 3 coords >> skip the triangle if the 4th point is not within the bounds

				// horizontal check from left to right
				const float pt_to_check[2] = 
				{
					pt3[0], pt3[1]
				};
				
				const float bounds[4] = 
				{ 
					/*x*/ pt0[0], 
					/*y*/ pt0[1], 
					/*w*/ fabs(pt2[0] - pt0[0]) + mapexport_quad_eps_,
					/*h*/ fabs(pt1[1] - pt0[1]) + mapexport_quad_eps_
				};

				// discard point if not within bounds 
				if (!is_coordinate_within_bounds_2d(pt_to_check, bounds)) 
				{
					return false;
				}

				// vertical check from top to bottom
				const float pt_to_check_2[2] = 
				{
					pt2[0], pt2[1]
				};

				const float bounds2[4] =
				{
					/*x*/ pt0[0],
					/*y*/ pt0[1],
					/*w*/ fabs(pt3[0] - pt0[0]) + mapexport_quad_eps_,
					/*h*/ fabs(pt1[1] - pt0[1]) + mapexport_quad_eps_
				};
				
				// discard point if not within bounds 
				if (!is_coordinate_within_bounds_2d(pt_to_check_2, bounds2)) 
				{
					return false;
				}
			}

			// build the quad from the 4 sorted coordinates
			for (auto q = 0; q < 4; q++)
			{
				for (auto qi = 0; qi < 3; qi++)
				{
					quad->coords[q][qi] = v_quad_pts[q][qi];
				}
			}

			return true;
		}

		return false;
	}

	// handle filters
	void brush_filters(int &filter_brush_amount, bool &filter_brush_index, bool &filter_brush_sorting, bool &filter_brush_selection)
	{
		const int total_brush_amount = Game::cm->numBrushes;

		// brush bounding box filter
		if (dvars::mapexport_selectionMode && dvars::mapexport_selectionMode->current.integer != 0)
		{
			if(dvars::r_drawCollision_brushAmount && dvars::r_drawCollision_brushAmount->current.integer != total_brush_amount)
			{
				Game::Dvar_SetValue(dvars::r_drawCollision_brushAmount, total_brush_amount);
			}

			if (dvars::r_drawCollision_brushSorting && dvars::r_drawCollision_brushSorting->current.integer != 0)
			{
				Game::Dvar_SetValue(dvars::r_drawCollision_brushSorting, 0);
			}

			if (dvars::r_drawCollision_brushIndexFilter && (std::string)dvars::r_drawCollision_brushIndexFilter->current.string != "null")
			{
				Game::Dvar_SetString("null", dvars::r_drawCollision_brushIndexFilter);
			}

			filter_brush_amount = total_brush_amount;
			filter_brush_selection = true;

			return;
		}

		// brush index filter & none of the above
		if(dvars::r_drawCollision_brushIndexFilter && (std::string)dvars::r_drawCollision_brushIndexFilter->current.string != "null")
		{
			if (dvars::r_drawCollision_brushAmount && dvars::r_drawCollision_brushAmount->current.integer != total_brush_amount)
			{
				Game::Dvar_SetValue(dvars::r_drawCollision_brushAmount, total_brush_amount);
			}

			if (dvars::r_drawCollision_brushSorting && dvars::r_drawCollision_brushSorting->current.integer != 0)
			{
				Game::Dvar_SetValue(dvars::r_drawCollision_brushSorting, 0);
			}

			filter_brush_amount = total_brush_amount;
			filter_brush_index = true;

			return;
		}

		// brush sorting & none of the above
		if(dvars::r_drawCollision_brushSorting && dvars::r_drawCollision_brushSorting->current.integer != 0)
		{
			filter_brush_sorting = true;
		}

		// brush amount filter & none of the above
		if (dvars::r_drawCollision_brushAmount && dvars::r_drawCollision_brushAmount->current.integer != 0 && dvars::r_drawCollision_brushAmount->current.integer <= total_brush_amount)
		{
			filter_brush_amount = dvars::r_drawCollision_brushAmount->current.integer;
			return;
		}

		if (dvars::r_drawCollision_brushAmount && dvars::r_drawCollision_brushAmount->current.integer != total_brush_amount)
		{
			Game::Dvar_SetValue(dvars::r_drawCollision_brushAmount, total_brush_amount);
		}

		filter_brush_amount = total_brush_amount;
		
	}

	// handle brush dvar commands
	void handle_debug_collision_commands()
	{
		// cmd :: print material list to console
		if (dvars::r_drawCollision_materialList && dvars::r_drawCollision_materialList->current.enabled)
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, "clear\n");

			// add spaces to the console so we can scroll the mini console
			Game::Com_PrintMessage(0, utils::va(" \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n"), 0);

			// reset the dvar and print our material list
			Game::Dvar_SetValue(dvars::r_drawCollision_materialList, false);
			Game::Com_PrintMessage(0, utils::va("%s\n", Game::Globals::r_drawCollision_materialList_string.c_str()), 0);
		}
	}

	// create a bounding box to select things that should be exported
	void sbox_frame(bool is_active)
	{
		if (is_active)
		{
			const float sbox_color[4] = { 0.0f, 1.0f, 0.0f, 0.6f };
			mapexport_selectionbox_.was_reset = false;

			if (!mapexport_selectionbox_.is_box_valid)
			{
				// we need atleast 2 valid points to define the selection box
				if (mapexport_selectionbox_.num_points < 2)
				{
					if(mapexport_selection_add_)
					{
						mapexport_selectionbox_.points[mapexport_selectionbox_.num_points] = Game::Globals::lpmove_origin;
						mapexport_selectionbox_.num_points++;

						mapexport_selection_add_ = false;
					}

					// use the players origin as a temporary second point till a second point was defined
					if (mapexport_selectionbox_.num_points == 1)
					{
						mapexport_selectionbox_.points[1] = Game::Globals::lpmove_origin;

						bounds_from_points(2, mapexport_selectionbox_.points, mapexport_selectionbox_.mins, mapexport_selectionbox_.maxs);
						glm::set_float3(mapexport_selectionbox_.box.mins, mapexport_selectionbox_.mins);
						glm::set_float3(mapexport_selectionbox_.box.maxs, mapexport_selectionbox_.maxs);

						draw_single_brush_collision(&mapexport_selectionbox_.box, sbox_color, 0, false);
					}
				}
				else
				{
					// calculate mins and maxs for our selection box once
					if (mapexport_selectionbox_.num_points == 2 && !mapexport_selectionbox_.is_box_valid)
					{
						bounds_from_points(mapexport_selectionbox_.num_points, mapexport_selectionbox_.points, mapexport_selectionbox_.mins, mapexport_selectionbox_.maxs);
						glm::set_float3(mapexport_selectionbox_.box.mins, mapexport_selectionbox_.mins);
						glm::set_float3(mapexport_selectionbox_.box.maxs, mapexport_selectionbox_.maxs);

						mapexport_selectionbox_.is_box_valid = true;
					}
				}
			}
			else
			{
				draw_single_brush_collision(&mapexport_selectionbox_.box, sbox_color, 0, false);
			}
		}
		else
		{
			if (!mapexport_selectionbox_.was_reset)
			{
				sbox_reset(&mapexport_selectionbox_);
			}
		}
	}

	// main logic for brush drawing (CM_ShowBrushCollision)
	void draw_brush_collision(Game::GfxViewParms *viewParms, Game::cplane_s *frustumPlanes, int frustumPlaneCount)
	{
		int last_drawn_brush_amount = 0, filter_brush_amount = 0;
		bool filter_brush_index = false, filter_brush_sorting = false, filter_brush_selection = false;

		Game::cbrush_t *brush;
		
		if (!frustumPlanes) 
		{
			Game::Com_Error(0, COM_ERROR_MSG);
			return;
		}

		// One time init per map when r_drawcollison was enabled
		init_once_per_map();

		// Handle filter dvars
		brush_filters(filter_brush_amount, filter_brush_index, filter_brush_sorting, filter_brush_selection);

		// Handle commands
		handle_debug_collision_commands();

		// *
		// Map Export
		
		mapexport_in_progress_ = false;		// reset after exporting
		mapexport_current_brush_index_ = 0;	// brush index for .map file brushes
		
		// cmd :: export current map
		if (mapexport_cmd_)
		{
			// reset the the command bool
			mapexport_cmd_ = false;

			// let our code know that we are about to export a map
			mapexport_in_progress_ = true;

			// map file name
			std::string map_name = Game::cm->name;
			utils::replace_all(map_name, std::string("maps/mp/"), std::string(""));

			// if sp map
			utils::replace_all(map_name, std::string("maps/"), std::string(""));

			utils::replace_all(map_name, std::string(".d3dbsp"), std::string(".map"));

			// export to root/map_export
			std::string base_path = Game::Dvar_FindVar("fs_basepath")->current.string;
						base_path += "\\iw3xo\\map_export\\";

			std::string file_path = base_path + map_name;

			Game::Com_PrintMessage(0, "\n------------------------------------------------------\n", 0);
			mapexport_timestamp_start_ = utils::clock_start_timer_print(utils::va("[MAP-EXPORT]: Starting to export %s to %s ...\n", Game::cm->name, file_path.c_str()));

			// create directory root/map_export if it doesnt exist
			// client is only able to export to a sub-directory of "menu_export"

			if (std::filesystem::create_directories(base_path))
			{
				Game::Com_PrintMessage(0, "|- Created directory \"root/iw3xo/map_export\"\n", 0);
			}

			// steam to .map file
			mapexport_mapfile_.open(file_path.c_str());

			// build entity list
			char* mapents_ptr = Game::cm->mapEnts->entityString;

			if (_map::mpsp_is_sp_map)
			{
				mapents_ptr = _map::mpsp_mapents_original;
			}

			mapexport_entities_ = utils::entities(mapents_ptr);
			Game::Com_PrintMessage(0, "|- Writing header and world entity ...\n\n", 0);

			// write header
			mapexport_mapfile_ <<	"iwmap 4\n"
									"\"000_Global\" flags expanded  active\n"
									"\"000_Global/Brushes\" flags\n"
									"\"000_Global/SingleQuads\" flags\n"
									"\"000_Global/Triangles\" flags\n"
									"\"000_Global/Models\" flags\n"
									"\"The Map\" flags" << std::endl; // header

			// write worldspawn
			mapexport_mapfile_ <<	"// entity 0\n"
									"{\n"
									+ mapexport_entities_.build_worldspawn();

			// Use debug collision methods to create our brushes ...
			mapexport_timestamp_brushgen_start_ = utils::clock_start_timer_print("[MAP-EXPORT]: Creating brushes ...\n");
		}

		// Handle box selection
		sbox_frame(filter_brush_selection);

		// do not draw brushes when using the selection box
		if (!mapexport_in_progress_ && filter_brush_selection)
		{
			return;
		}

		// --------

		const bool brush_index_visible = dvars::r_drawCollision_brushIndexVisible && dvars::r_drawCollision_brushIndexVisible->current.enabled;

		if (filter_brush_sorting)
		{
			// sort brushes far to near
			if (dvars::r_drawCollision_brushSorting->current.integer == 1)
			{
				sort_brush_list_by_distance();
			}
			else // sort brushes near to far
			{
				sort_brush_list_by_distance(false);
			}

			// disable sorting if sorted brushList is empty .. should not happen
			if (map_brush_list_.empty())
			{
				Game::Dvar_SetValue(dvars::r_drawCollision_brushSorting, 0);
				filter_brush_sorting = false;
			}
		}

		// clear drawn brushes vector
		map_brush_list_for_index_filtering_.clear();

		// reset hud element
		Game::Globals::debug_collision_rendered_planes_counter = 0;

		int BRUSH_INDEX, BRUSH_COUNT;
		std::vector<int> filter_brush_indices;

		// brush index filtering
		if (filter_brush_index)
		{
			utils::extract_integer_words(dvars::r_drawCollision_brushIndexFilter->current.string, filter_brush_indices, true);
			BRUSH_COUNT = static_cast<int>(filter_brush_indices.size());
		}

		// sorted / unsorted
		else
		{
			BRUSH_COUNT = Game::cm->numBrushes;
		}

		for (BRUSH_INDEX = 0; BRUSH_INDEX < BRUSH_COUNT; ++BRUSH_INDEX)
		{
			int brushIndex;

			// break after we drew the amount of brushes we set (works best with brushSorting)
			if (last_drawn_brush_amount >= filter_brush_amount)
			{
				break;
			}

			// brush index filtering
			if (filter_brush_index)
			{
				brushIndex = filter_brush_indices[BRUSH_INDEX];

				// check if the index is within bounds
				if (brushIndex < 0 || brushIndex >= Game::cm->numBrushes)
				{
					// find and remove the <out of bounds> index
					filter_brush_indices.erase(std::remove(filter_brush_indices.begin(), filter_brush_indices.end(), brushIndex), filter_brush_indices.end());

					// vector to string
					std::string indices_to_string;
					for (auto num = 0u; num < filter_brush_indices.size(); num++)
					{
						indices_to_string += std::to_string(filter_brush_indices[num]) + " ";
					}

					Game::Dvar_SetString(indices_to_string.c_str(), dvars::r_drawCollision_brushIndexFilter);
					Game::Com_PrintMessage(0, utils::va("^1-> r_drawCollision_brushIndexFilter ^7:: found and removed <out of bounds> index: %d \n", brushIndex), 0);

					return;
				}
			}

			// sorted / unsorted
			else
			{
				brushIndex = BRUSH_INDEX;
			}

			if (filter_brush_sorting)
			{
				brush = map_brush_list_[brushIndex];
			} 
			else 
			{
				brush = &Game::cm->brushes[brushIndex];
			}

			// if brush is part of a submodel, translate brushmodel bounds by the submodel origin
			if (brush->isSubmodel)
			{
				Game::cbrush_t dupe = {};
				memcpy(&dupe, brush, sizeof(Game::cbrush_t));

				utils::vector::add3(map_brushmodel_list_[dupe.cmSubmodelIndex].cm_submodel_origin, dupe.mins, dupe.mins);
				utils::vector::add3(map_brushmodel_list_[dupe.cmSubmodelIndex].cm_submodel_origin, dupe.maxs, dupe.maxs);

				brush = &dupe;
			}

			// when not exporting a map
			if (!mapexport_in_progress_)
			{
				// always cull if not exporting
				if (!brush_in_view(brush, frustumPlanes, frustumPlaneCount)) 
				{
					continue;
				}

				// disable material filter when using index filter
				if (!filter_brush_index)
				{
					// check if its a material we selected otherwise
					if (!is_valid_brush_material_selection(brush, dvars::r_drawCollision_material->current.integer)) 
					{
						continue;
					}
				}
			}

			// on exporting
			else
			{
				// exclude current brush if its not part of the selection box (when using selectionMode)
				if (filter_brush_selection && !sbox_is_brush_inside(brush, &mapexport_selectionbox_)) 
				{
					continue;
				}

				// skip material check if using index filtering or selectionMode
				if (!filter_brush_index && !filter_brush_selection)
				{
					// check if its a material we selected otherwise
					if (!is_valid_brush_material_selection(brush, dvars::r_drawCollision_material->current.integer)) 
					{
						continue;
					}
				}
			}

			// always use the brush index within clipmap->brushes to define its color
			float brush_color[4] = {};
			get_collision_color(brush_color, brush->colorCounter);

			// draw the current brush
			draw_single_brush_collision(brush, brush_color, brushIndex);

			if (brush_index_visible)
			{
				map_brush_list_for_index_filtering_.push_back(brush);
			}

			last_drawn_brush_amount++;	
		}

		// *
		// draw brush indices as 3D text (only when: unsorted brushes / index filtering)

		if (brush_index_visible && !map_brush_list_for_index_filtering_.empty())
		{
			// sort brushes from near to far
			sort_brush_list_by_distance(false, true, true);

			// should not happen
			if (map_brush_list_for_index_filtering_.empty())
			{
				Game::Dvar_SetValue(dvars::r_drawCollision_brushIndexVisible, false);
				Game::Com_PrintMessage(0, utils::va("^1draw_brush_collision L#%d ^7:: GlobalBrushList was empty. Disabled r_drawCollision_brushIndexVisible! \n", __LINE__), 0);
			}

			// maximum amount of brush indices to draw
			std::uint32_t max_debug_prints = 64;

			// only draw as many indices as the map has brushes
			if (map_brush_list_for_index_filtering_.size() < max_debug_prints)
			{
				max_debug_prints = map_brush_list_for_index_filtering_.size();
			}

			// draw strings near - far
			for (auto p = 0u; p < max_debug_prints; ++p)
			{
				// get distance-sorted brush
				brush = map_brush_list_for_index_filtering_[p];

				// get midpoint of brush bounds (xyz)
				glm::vec3 printOrigin = get_brush_midpoint(brush, true);

				// draw original brush index in the middle of the collision poly
				draw_brush_index_numbers(viewParms, brush->cmBrushIndex, printOrigin, p, max_debug_prints);
			}
		}
		else
		{
			// if not drawing debug strings
			SvFramerateToRendertime_Counter = 0;
		}

		// *
		// Map Export

		if (mapexport_in_progress_)
		{
			const char* brush_str = utils::va("|- Building (%d) brushes took", mapexport_current_brush_index_);
			std::string timefmt = brush_str + " took (%.4f)\n\n"s;

			utils::clock_end_timer_print_seconds(mapexport_timestamp_brushgen_start_, timefmt.c_str());

			// *
			// Create Tris

			std::vector<Game::map_patchTris_t*> single_triangles;
			bool mapexport_needs_triangles = false;

			// only generate triangles if exporting triangles/quads or both
			if (dvars::mapexport_writeQuads->current.enabled || dvars::mapexport_writeTriangles->current.enabled)
			{
				mapexport_needs_triangles = true;

				auto export_time_createTrisStart = utils::clock_start_timer_print("[MAP-EXPORT]: Building triangles ...\n");

				for (auto tri = 0; tri < Game::cm->triCount; tri++)
				{
					// get incides that define the triangle
					unsigned short tri_indices1[3] =
					{
						Game::cm->triIndices[tri * 3 + 0],
						Game::cm->triIndices[tri * 3 + 1],
						Game::cm->triIndices[tri * 3 + 2]
					};

					Game::map_patchTris_t *ptri = alloc_patch_triangle();
					ptri->triIndex = tri;

					patch_tri_from_incides(ptri, tri_indices1);

					if (filter_brush_selection)
					{
						if (sbox_is_triangle_inside(ptri, &mapexport_selectionbox_))
						{
							single_triangles.push_back(ptri);
						}
					}
					else
					{
						single_triangles.push_back(ptri);
					}
				}

				utils::clock_end_timer_print_seconds(export_time_createTrisStart, "|- Building triangles took (%.4f) seconds!\n");
				Game::Com_PrintMessage(0, utils::va("|- Initial amount of triangles = (%d)\n\n", static_cast<int>(single_triangles.size())), 0);
			}

			// *
			// Merge Tris to Quads

			// fix me daddy

			if (dvars::mapexport_writeQuads->current.enabled)
			{
				auto export_time_create_quads_start = utils::clock_start_timer_print("[MAP-EXPORT]: Merging triangles to quads ...\n");

				// create a list of merged patch triangles (quads)
				std::vector<Game::map_patchQuads_t*> single_quads;

				// reset quad epsilon
				mapexport_quad_eps_ = 0.0f;

				// count the amount of iterations till we are no longer able to merge any triangles to quads
				unsigned int quad_from_tris_loop_count = 0;

				// triangle index offset when we are no longer able to merge triangles (eg. if triangle 1-4 and 2-3 share an edge)
				int tri_num_offset = 0;

				// amount of "failed" iterations with triangle index offset included
				int merge_iteration_fail_count = 0;

				// defines how many triangles we look ahead from the current triangle to check for a shared edge
				unsigned int tri_foward_offset = 1;

				// try to find as many "easy" quads as possible till we no longer merge triangles
				bool easy_quads_first = true;

				// build single quads from triangles; iterate for as long as we have enough triangles & if we actually decreased the amount of triangles after merging
				for (quad_from_tris_loop_count = 0; single_triangles.size() > 1; quad_from_tris_loop_count++)
				{
					// get amount of triangles before merging
					auto start_tris = single_triangles.size();

					// merge triangles if they share an edge to create quads ... we will decrease the size of singlePatchTriangles, so do this for x amount of times ^
					for (auto tri_num = 0u + tri_num_offset; tri_num < single_triangles.size(); tri_num++)
					{
						// if we have atleast 2 triangles to work with
						if (tri_num + tri_foward_offset < single_triangles.size())
						{
							if (patch_tri_shares_edge(single_triangles[tri_num], single_triangles[tri_num + tri_foward_offset]))
							{
								Game::map_patchQuads_t *pquad = alloc_patch_quad();
								pquad->quadIndex = static_cast<int>(tri_num); // for debugging :)

								// try to merge both triangles to create a quad
								if (single_patch_quad_from_triangle(pquad, single_triangles[tri_num], single_triangles[tri_num + tri_foward_offset], easy_quads_first))
								{
									single_quads.push_back(pquad);

									// free triangles and erase them from singlePatchTriangles
									free(single_triangles[tri_num]);
									free(single_triangles[tri_num + tri_foward_offset]);

									single_triangles.erase(single_triangles.begin() + tri_num + tri_foward_offset);
									single_triangles.erase(single_triangles.begin() + tri_num);

									// decrease triNum by one
									if (tri_num - 1 > 0)
									{
										tri_num--;
									}
								}
							}
						}
					}

					// check if we merged any triangles in this iteration
					// stop trying to merge triangles if we didnt decrease the amount of triangles
					if (start_tris - single_triangles.size() == 0)
					{
						// catch :: tri 1-4 / 2-3 => offset triangle index on first fail so we catch 2-3
						tri_num_offset = 1;

						// still no merge? catch :: tri 1-3 / 2-4 => reset triNumOffset and add 1 to triFowardOffset 
						if (merge_iteration_fail_count == 1) 
						{
							tri_num_offset = 0;
							tri_foward_offset = 2;
						}

						// reset triFowardOffset after we swizzeled the triangles once
						if (merge_iteration_fail_count == 2) 
						{
							tri_foward_offset = 1;
						}

						// still unable to merge? .. (following doesnt seem to help)
						if (merge_iteration_fail_count == 3) 
						{
							tri_foward_offset = 3;
						}

						if (merge_iteration_fail_count == 4) 
						{
							tri_foward_offset = 4;
						}

						if (merge_iteration_fail_count == 5) 
						{
							tri_foward_offset = 1;
						}

						// start increasing the bounding box that has to encapsule the 4th coordinate
						if (merge_iteration_fail_count >= 6 && merge_iteration_fail_count <= 13) 
						{
							mapexport_quad_eps_ += 5.0f;
						}

						// now try to merge skewed quads (might still be valid ones)
						if (merge_iteration_fail_count == 14) 
						{
							easy_quads_first = false;
						}

						merge_iteration_fail_count++;

						if (merge_iteration_fail_count > 16)
						{
							break;
						}
					}
				}

				utils::clock_end_timer_print_seconds(export_time_create_quads_start, "|- Merging triangles to quads took (%.4f) seconds!\n");
				Game::Com_PrintMessage(0, utils::va("|- (%d) triangles couldn't be be merged.\n", (int)single_triangles.size()), 0);
				Game::Com_PrintMessage(0, utils::va("|- (%d) quads after (%d) iterations.\n", (int)single_quads.size(), quad_from_tris_loop_count), 0);

				// *
				// Write Quad Patches

				// coord x y z .... t .... (tc.x * 1024) (tc.y * 1024) (st.x * 1024 + 2)  -(st.y * 1024 + 2) 
				// v -108 -102 128 t -1728 5012.7217 -6.75 6.375

				for (auto quad : single_quads)
				{
					// start patch
					mapexport_mapfile_ << utils::va("// brush %d", mapexport_current_brush_index_) << std::endl;

					// global brush exporting index count
					mapexport_current_brush_index_++;

					mapexport_mapfile_ << " {" << std::endl;
					mapexport_mapfile_ << "  mesh" << std::endl;
					mapexport_mapfile_ << "  {" << std::endl;
					mapexport_mapfile_ << "   layer \"000_Global/SingleQuads\"" << std::endl;
					mapexport_mapfile_ << "   toolFlags splitGeo;" << std::endl;
					mapexport_mapfile_ << "   caulk" << std::endl;
					mapexport_mapfile_ << "   lightmap_gray" << std::endl;
					mapexport_mapfile_ << "   2 2 16 8" << std::endl;

					mapexport_mapfile_ << "   (" << std::endl;
					mapexport_mapfile_ << utils::va("    v %.1f %.1f %.1f t 0 0 0", quad->coords[0][0], quad->coords[0][1], quad->coords[0][2]) << std::endl;
					mapexport_mapfile_ << utils::va("    v %.1f %.1f %.1f t 0 0 0", quad->coords[1][0], quad->coords[1][1], quad->coords[1][2]) << std::endl;
					mapexport_mapfile_ << "   )" << std::endl;
					mapexport_mapfile_ << "   (" << std::endl;
					mapexport_mapfile_ << utils::va("    v %.1f %.1f %.1f t 0 0 0", quad->coords[2][0], quad->coords[2][1], quad->coords[2][2]) << std::endl;
					mapexport_mapfile_ << utils::va("    v %.1f %.1f %.1f t 0 0 0", quad->coords[3][0], quad->coords[3][1], quad->coords[3][2]) << std::endl;
					mapexport_mapfile_ << "   )" << std::endl;
					mapexport_mapfile_ << "  }" << std::endl;
					mapexport_mapfile_ << " }" << std::endl;
				}

				// free quads
				for(auto quad : single_quads)
				{
					free(quad);
				}

				Game::Com_PrintMessage(0, "|- Wrote quads to layer \"000_Global/SingleQuads\"\n", 0);
			}

			// *
			// Write Single Triangles

			if (dvars::mapexport_writeTriangles->current.enabled)
			{
				for (auto tri : single_triangles)
				{
					// start patch
					mapexport_mapfile_ << utils::va("// brush %d", mapexport_current_brush_index_) << std::endl;

					// global brush exporting index count
					mapexport_current_brush_index_++;

					mapexport_mapfile_ << " {" << std::endl;
					mapexport_mapfile_ << "  mesh" << std::endl;
					mapexport_mapfile_ << "  {" << std::endl;
					mapexport_mapfile_ << "   layer \"000_Global/Triangles\"" << std::endl;
					mapexport_mapfile_ << "   contents nonColliding;" << std::endl;
					mapexport_mapfile_ << "   toolFlags splitGeo;" << std::endl;
					mapexport_mapfile_ << "   caulk" << std::endl;
					mapexport_mapfile_ << "   lightmap_gray" << std::endl;
					mapexport_mapfile_ << "   2 2 16 8" << std::endl;

					mapexport_mapfile_ << "   (" << std::endl;
					mapexport_mapfile_ << utils::va("    v %.1f %.1f %.1f t 0 0 0", tri->coords[2].xyz[0], tri->coords[2].xyz[1], tri->coords[2].xyz[2]) << std::endl;
					mapexport_mapfile_ << utils::va("    v %.1f %.1f %.1f t 0 0 0", tri->coords[0].xyz[0], tri->coords[0].xyz[1], tri->coords[0].xyz[2]) << std::endl;
					mapexport_mapfile_ << "   )" << std::endl;
					mapexport_mapfile_ << "   (" << std::endl;
					mapexport_mapfile_ << utils::va("    v %.1f %.1f %.1f t 0 0 0", tri->coords[1].xyz[0], tri->coords[1].xyz[1], tri->coords[1].xyz[2]) << std::endl;
					mapexport_mapfile_ << utils::va("    v %.1f %.1f %.1f t 0 0 0", tri->coords[1].xyz[0], tri->coords[1].xyz[1], tri->coords[1].xyz[2]) << std::endl;
					mapexport_mapfile_ << "   )" << std::endl;
					mapexport_mapfile_ << "  }" << std::endl;
					mapexport_mapfile_ << " }" << std::endl;
				}

				Game::Com_PrintMessage(0, "|- Wrote triangles to layer \"000_Global/Triangles\"\n\n", 0);
			}

			// free triangles
			if (mapexport_needs_triangles)
			{
				for (auto tri : single_triangles)
				{
					free(tri);
				}
			}

			// close the worldspawn entity with all its brushes
			mapexport_mapfile_ << "}" << std::endl;
			Game::Com_PrintMessage(0, "|- Finished writing the world entity\n\n", 0);

			// *
			// Map Entities + Submodels/Brushmodels + Reflection Probes

			if (dvars::mapexport_writeEntities->current.enabled)
			{
				Game::Com_PrintMessage(0, "[MAP-EXPORT]: Building entities ...\n", 0);

				// already exported the worldspawn entity, so delete it from the list
				mapexport_entities_.delete_worldspawn();

				// *
				// map entities and brushmodels

				if (filter_brush_selection) // only exporting entities within the selection box
				{
					mapexport_mapfile_ << mapexport_entities_.buildSelection_FixBrushmodels(&mapexport_selectionbox_, map_brushmodel_list_);
				}
				else // build all other entities and fix up brushmodels
				{
					mapexport_mapfile_ << mapexport_entities_.buildAll_FixBrushmodels(map_brushmodel_list_);
				}

				// *
				// reflection probes (always skip the first one (not defined within the map file))

				int exported_probes = 0;

				for (auto probe = 1u; probe < Game::_gfxWorld->reflectionProbeCount; probe++)
				{
					if (filter_brush_selection && 
						!utils::polylib::is_point_within_bounds(glm::vec3(Game::_gfxWorld->reflectionProbes[probe].origin[0],
																	 Game::_gfxWorld->reflectionProbes[probe].origin[1],
																	 Game::_gfxWorld->reflectionProbes[probe].origin[2]),
																	 mapexport_selectionbox_.mins, mapexport_selectionbox_.maxs, 0.25f))
					{
						// skip probe if not in selection box
						continue;
					}

					mapexport_mapfile_ << utils::va("// reflection probe %d", exported_probes) << std::endl;
					mapexport_mapfile_ << "{" << std::endl;
					mapexport_mapfile_ << "\"angles\" \"0 0 0\"" << std::endl;
					mapexport_mapfile_ << utils::va("\"origin\" \"%.1f %.1f %.1f\"", Game::_gfxWorld->reflectionProbes[probe].origin[0], Game::_gfxWorld->reflectionProbes[probe].origin[1], Game::_gfxWorld->reflectionProbes[probe].origin[2]) << std::endl;
					mapexport_mapfile_ << "\"classname\" \"reflection_probe\"" << std::endl;
					mapexport_mapfile_ << "}" << std::endl;

					exported_probes++;
				}

				Game::Com_PrintMessage(0, utils::va("|- (%d) reflection probes.\n", exported_probes), 0);
			}

			// *
			// Static Models

			if (dvars::mapexport_writeModels->current.enabled)
			{
				auto timestamp_static_models_start = utils::clock_start_timer();
				int exported_static_model_counter = 0;

				for (auto smodel = 0u; smodel < Game::_gfxWorld->dpvs.smodelCount; smodel++)
				{
					// only export static models within the selection box
					if (filter_brush_selection &&
						!utils::polylib::is_point_within_bounds(glm::vec3(Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.origin[0],
																	 Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.origin[1],
																	 Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.origin[2]),
																	 mapexport_selectionbox_.mins, mapexport_selectionbox_.maxs, 0.25f)) 
					{
						// skip static model if not in selection box
						continue;
					}

					// copy model rotation axis
					Game::vec4_t matrix[4];

					// X
					matrix[0][0] = Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.axis[0][0];
					matrix[0][1] = Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.axis[0][1];
					matrix[0][2] = Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.axis[0][2];

					// Y
					matrix[1][0] = Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.axis[1][0];
					matrix[1][1] = Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.axis[1][1];
					matrix[1][2] = Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.axis[1][2];

					// Z
					matrix[2][0] = Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.axis[2][0];
					matrix[2][1] = Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.axis[2][1];
					matrix[2][2] = Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.axis[2][2];

					// calculate model angles
					Game::vec3_t angles;
					utils::vector::to_euler_angles_deg(matrix, angles);

					mapexport_mapfile_ << utils::va("// static model %d\n{", exported_static_model_counter) << std::endl;
					mapexport_mapfile_ << "layer \"000_Global/Models\"" << std::endl;
					mapexport_mapfile_ << utils::va("\"modelscale\" \"%.1f\"", Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.scale) << std::endl;
					mapexport_mapfile_ << utils::va("\"origin\" \"%.1f %.1f %.1f\"", Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.origin[0], Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.origin[1], Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].placement.origin[2]) << std::endl;
					mapexport_mapfile_ << utils::va("\"angles\" \"%.1f %.1f %.1f\"", angles[1], angles[2], angles[0]) << std::endl;
					mapexport_mapfile_ << utils::va("\"model\" \"%s\"", Game::_gfxWorld->dpvs.smodelDrawInsts[smodel].model->name) << std::endl;
					mapexport_mapfile_ << "\"classname\" \"misc_model\"" << std::endl;
					mapexport_mapfile_ << "}" << std::endl;
				}

				utils::clock_end_timer_print_seconds(timestamp_static_models_start, "|- Building static models took (%.4f) seconds!\n\n");
			}

			if(dvars::mapexport_writeDynModels->current.enabled)
			{
				if (Game::cm->dynEntDefList[0] && Game::cm->dynEntCount[0])
				{
					for (auto i = 0; i < Game::cm->dynEntCount[0]; i++)
					{
						if (auto next = &Game::cm->dynEntDefList[0][i]; 
								 next)
						{
							mapexport_mapfile_ << utils::va("// dyn model %d\n{", i) << std::endl;
							mapexport_mapfile_ << "layer \"000_Global/DynModels\"" << std::endl;

							if (next->health)
							{
								mapexport_mapfile_ << utils::va("\"health\" \"%d\"", next->health) << std::endl;
							}

							if (next->destroyFx)
							{
								mapexport_mapfile_ << utils::va("\"destroyEfx\" \"%s\"", next->destroyFx->name) << std::endl;
							}

							mapexport_mapfile_ << utils::va("\"type\" \"%s\"", next->type == 1 ? "clutter" : next->type == 2 ? "destruct" : "invalid") << std::endl;
							mapexport_mapfile_ << utils::va("\"origin\" \"%.1f %.1f %.1f\"", next->pose.origin[0], next->pose.origin[1], next->pose.origin[2]) << std::endl;

							// quat to angles
							{
								float axis[3][3] = {};
								float angles[3] = {};

								utils::vector::unit_quat_to_axis(next->pose.quat, axis);
								Game::AxisToAngles(angles, axis);

								mapexport_mapfile_ << utils::va("\"angles\" \"%.1f %.1f %.1f\"", angles[0], angles[1], angles[2]) << std::endl;
							}

							mapexport_mapfile_ << utils::va("\"model\" \"%s\"", next->xModel->name) << std::endl;
							mapexport_mapfile_ << "\"classname\" \"dyn_model\"" << std::endl;
							mapexport_mapfile_ << "}" << std::endl;
						}
					}
				}
			}


			// *
			// Map Export End

			mapexport_mapfile_.close();
			//export_mapFile_addon.close();
			mapexport_current_brush_index_ = 0;

			utils::clock_end_timer_print_seconds(mapexport_timestamp_start_, ">> DONE! Map export took (%.4f) seconds!\n");
			Game::Com_PrintMessage(0, "------------------------------------------------------\n\n", 0);
		}

		// ------------

		// update hud elements after we drew all brushes / planes
		if (Game::Globals::debug_collision_rendered_planes_amount != Game::Globals::debug_collision_rendered_planes_counter) 
		{
			Game::Globals::debug_collision_rendered_planes_amount = Game::Globals::debug_collision_rendered_planes_counter;
		}

		if (Game::Globals::debug_collision_rendered_brush_amount != last_drawn_brush_amount) 
		{
			Game::Globals::debug_collision_rendered_brush_amount = last_drawn_brush_amount;
		}
	}

	// *
	// _Debug::RB_AdditionalDebug :: entry for collision drawing (create view frustum)
	void draw_collision::debug_collision_frame(Game::GfxViewParms *viewParms)
	{
		if (!viewParms) 
		{
			Game::Com_Error(0, utils::va("RB_ShowCollision L#%d :: viewparams\n", __LINE__));
			return;
		}

		// enable drawcollision on mapexport_selectionMode
		if (dvars::mapexport_selectionMode && dvars::mapexport_selectionMode->current.integer != 0 && dvars::r_drawCollision && dvars::r_drawCollision->current.integer == 0)
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, "r_drawcollision 3\n");
		}

		if (dvars::r_drawCollision && dvars::r_drawCollision->current.integer > 0)
		{
			// turn off brush sorting if displaying brushIndices
			if (dvars::r_drawCollision_brushIndexVisible->current.enabled && dvars::r_drawCollision_brushSorting->current.integer != 0)
			{
				Game::Dvar_SetValue(dvars::r_drawCollision_brushSorting, 0);
				Game::Com_PrintMessage(0, utils::va("^1-> r_drawCollision_brushSorting ^7:: disabled due to r_drawCollision_brushIndexVisible \n"), 0);
			}

			// Disable r_drawCollision when using r_fullbright
			if (Game::Dvar_FindVar("r_fullbright")->current.enabled) 
			{
				Game::Com_PrintMessage(0, utils::va("^1-> r_drawCollision ^7:: disabled due to r_fullbright \n"), 0);
				Game::Dvar_SetValue(dvars::r_drawCollision, 0);
				
				return;
			}

			// Disable r_drawCollision when using r_debugShader
			if (Game::Dvar_FindVar("r_debugShader")->current.integer > 0)
			{
				Game::Com_PrintMessage(0, utils::va("^1-> r_drawCollision ^7:: disabled due to r_debugShader \n"), 0);
				Game::Dvar_SetValue(dvars::r_drawCollision, 0);
				
				return;
			}

			// *
			// Build culling frustum

			Game::cplane_s frustum_planes[6];
			build_frustum_planes(viewParms, frustum_planes);

			frustum_planes[5].normal[0] = -frustum_planes[4].normal[0];
			frustum_planes[5].normal[1] = -frustum_planes[4].normal[1];
			frustum_planes[5].normal[2] = -frustum_planes[4].normal[2];

			// max draw distance when brushDist is set to 0
			float draw_distance = dvars::r_drawCollision_brushDist->current.value;

			if (draw_distance == 0.0f) 
			{
				draw_distance = 999999.0f;
			}

			frustum_planes[5].dist = -frustum_planes[4].dist - draw_distance;

			char frustum_type;
			if (frustum_planes[5].normal[0] == 1.0f)
			{
				frustum_type = 0;
			}
			else
			{
				if (frustum_planes[5].normal[1] == 1.0f)
				{
					frustum_type = 1;
				}
				else
				{
					frustum_type = 2;
					if (frustum_planes[5].normal[2] != 1.0f)
					{
						frustum_type = 3;
					}
				}
			}

			frustum_planes[5].type = frustum_type;
			set_plane_signbits(&frustum_planes[5]);

			draw_brush_collision(viewParms, frustum_planes, 6);

			// draw added polys / lines
			if (Game::tess->indexCount)
			{
				Game::RB_EndTessSurface();
			}
		}
	}

	draw_collision::draw_collision()
	{
		// -----
		// dvars

		dvars::r_drawCollision = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision",
			/* desc		*/ "Enable collision drawing.\n0: Off\n1: Outlines\n2: Polys\n3: Both",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 3,
			/* flags	*/ Game::dvar_flags::none);

		dvars::r_drawCollision_brushAmount = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision_brushAmount",
			/* desc		*/ "Draw x amount of brushes, starting at brush index 0 and will limit itself to the total amount of brushes within the clipMap.\n0: disables this filter.",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ INT_MAX / 2 - 1,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::r_drawCollision_brushDist = Game::Dvar_RegisterFloat(
			/* name		*/ "r_drawCollision_brushDist",
			/* desc		*/ "Max distance to draw collision.\n0: disables this filter.\nWill reset itself on map load.",
			/* default	*/ 800.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 10000.0f,
			/* flags	*/ Game::dvar_flags::none);

		// r_drawCollision_brushIndexFilter @ (CM_BuildMaterialListForMapOnce)

		dvars::r_drawCollision_brushIndexVisible = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_brushIndexVisible",
			/* desc		*/ "Draw brush index numbers for use with ^1r_drawCollision_brushFilter^7. Unstable fps will cause flickering.",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::r_drawCollision_brushSorting = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision_brushSorting",
			/* desc		*/ "Sort brushes based on distance from the camera.\n0: Off\n1: Far to near\n2: Near to far",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 2,
			/* flags	*/ Game::dvar_flags::saved);

#if DEBUG
		dvars::r_drawCollision_brushDebug = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_brushDebug",
			/* desc		*/ "Draw debug prints. Only enabled when r_drawCollision_brushAmount = 1",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);
#endif

		dvars::r_drawCollision_lineWidth = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision_lineWidth",
			/* desc		*/ "Width of debug lines. (Only if using r_drawCollision 1)",
			/* default	*/ 1,
			/* minVal	*/ 0,
			/* maxVal	*/ 12,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::r_drawCollision_lineColor = Game::Dvar_RegisterVec4(
			/* name		*/ "r_drawCollision_lineColor",
			/* desc		*/ "Color of debug lines.",
			/* x		*/ 0.2f,
			/* y		*/ 1.0f,
			/* z		*/ 0.2f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::r_drawCollision_polyAlpha = Game::Dvar_RegisterFloat(
			/* name		*/ "r_drawCollision_polyAlpha",
			/* desc		*/ "Transparency of polygons.",
			/* default	*/ 0.8f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::r_drawCollision_polyDepth = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_polyDepth",
			/* desc		*/ "Enable depth test for polygons.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::r_drawCollision_polyFace = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_polyFace",
			/* desc		*/ "0: Back(only draw the front facing side)\n1: None(draw both sides)",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::r_drawCollision_polyLit = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_polyLit",
			/* desc		*/ "Enable fake lighting for polygons.",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::r_drawCollision_material = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision_material",
			/* desc		*/ "Will be populated when a map is loaded and r_drawCollision is enabled.",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 1,
			/* flags	*/ Game::dvar_flags::none);

		static std::vector <const char*> r_draw_collision_mat_filter = 
		{ 
			"none", 
			"clip", 
			"mantle", 
			"trigger", 
			"all", 
			"all-no-tools", 
			"all-no-tools-clip", 
		};

		dvars::r_drawCollision_materialInclude = Game::Dvar_RegisterEnum(
			/* name		*/ "r_drawCollision_materialInclude",
			/* desc		*/ "Filter by type. \nExample: <clip> will show \"clip_player\" / \"clip_metal\" etc. \n<all-no-tools> draws everything but portal, trigger, hint, mantle, sky and volumes materials.\n<all-no-tools-clip> draws everything but clip and previous mentioned materials.",
			/* default	*/ 0,
			/* enumSize	*/ r_draw_collision_mat_filter.size(),
			/* enumData */ r_draw_collision_mat_filter.data(),
			/* flags	*/ Game::dvar_flags::none);

		dvars::r_drawCollision_materialList = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_materialList",
			/* desc		*/ "CMD: Prints a list of materials in use by the current map (to the console).\nOnly works when a map is loaded and r_drawCollision is enabled!",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		dvars::r_drawCollision_hud = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_hud",
			/* desc		*/ "Display debug hud.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::r_drawCollision_hud_position = Game::Dvar_RegisterVec2(
			/* name		*/ "r_drawCollision_hud_position",
			/* desc		*/ "hud position offset",
			/* def x	*/ 10.0f,
			/* def y	*/ 250.0f,
			/* minVal	*/ -1000.0f,
			/* maxVal	*/ 1000.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::r_drawCollision_hud_fontScale = Game::Dvar_RegisterFloat(
			/* name		*/ "r_drawCollision_hud_fontScale",
			/* desc		*/ "font scale",
			/* default	*/ 0.75f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 100.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::r_drawCollision_hud_fontStyle = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision_hud_fontStyle",
			/* desc		*/ "font style",
			/* default	*/ 1,
			/* minVal	*/ 0,
			/* maxVal	*/ 8,
			/* flags	*/ Game::dvar_flags::none);

		dvars::r_drawCollision_hud_fontColor = Game::Dvar_RegisterVec4(
			/* name		*/ "r_drawCollision_hud_fontColor",
			/* desc		*/ "font color",
			/* x		*/ 1.0f,
			/* y		*/ 0.55f,
			/* z		*/ 0.4f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		// ---------------

		dvars::mapexport_brushEpsilon1 = Game::Dvar_RegisterFloat(
			/* name		*/ "mapexport_brushEpsilon1",
			/* desc		*/ "brushside epsilon 1 (debug)",
			/* default	*/ 0.4f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::mapexport_brushEpsilon2 = Game::Dvar_RegisterFloat(
			/* name		*/ "mapexport_brushEpsilon2",
			/* desc		*/ "brushside epsilon 2 (debug)",
			/* default	*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::mapexport_brushMinSize = Game::Dvar_RegisterFloat(
			/* name		*/ "mapexport_brushMinSize",
			/* desc		*/ "only export brushes (with more then 6 sides) if their diagonal length is greater then <this>",
			/* default	*/ 64.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1000.0f,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::mapexport_brush5Sides = Game::Dvar_RegisterBool(
			/* name		*/ "mapexport_brush5Sides",
			/* desc		*/ "enable exp. export of brushes with only 5 sides",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::mapexport_selectionMode = Game::Dvar_RegisterInt(
			/* name		*/ "mapexport_selectionMode",
			/* desc		*/ "Only export selected things. Use \"mapexport_selectionAdd\" and \"mapexport_selectionClear\" \n0: Off\n1: Bounding box (needs 2 defined points)",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 1,
			/* flags	*/ Game::dvar_flags::none);

		dvars::mapexport_writeTriangles = Game::Dvar_RegisterBool(
			/* name		*/ "mapexport_writeTriangles",
			/* desc		*/ "[MAP-EXPORT-OPTION] Export leftover unmerged triangles if enabled.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::mapexport_writeQuads = Game::Dvar_RegisterBool(
			/* name		*/ "mapexport_writeQuads",
			/* desc		*/ "[MAP-EXPORT-OPTION] Export resulting quads after triangle merging if enabled.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::mapexport_writeEntities = Game::Dvar_RegisterBool(
			/* name		*/ "mapexport_writeEntities",
			/* desc		*/ "[MAP-EXPORT-OPTION] Export map entities if enabled (no brushmodel support).",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::mapexport_writeModels = Game::Dvar_RegisterBool(
			/* name		*/ "mapexport_writeModels",
			/* desc		*/ "[MAP-EXPORT-OPTION] Export all static models if enabled.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::mapexport_writeDynModels = Game::Dvar_RegisterBool(
			/* name		*/ "mapexport_writeDynModels",
			/* desc		*/ "[MAP-EXPORT-OPTION] Export all dynamic models if enabled.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);


		// --------
		// Commands

		command::add("mapexport", [](command::params)
		{
			if (dvars::r_drawCollision && dvars::r_drawCollision->current.integer == 0)
			{
				Game::Com_PrintMessage(0, "Please enable \"r_drawCollision\" and re-run your command.\n", 0);
				return;
			}

			mapexport_cmd_ = true;

			Game::Cmd_ExecuteSingleCommand(0, 0, "pm_hud_enable 0\n");
			Game::Cmd_ExecuteSingleCommand(0, 0, "say \"Export Done!\"\n");
		});

		command::add("mapexport_selectionAdd", [](command::params)
		{
			if (dvars::mapexport_selectionMode && dvars::mapexport_selectionMode->current.integer != 0)
			{
				mapexport_selection_add_ = true;
			}
			else
			{
				Game::Com_PrintMessage(0, "mapexport_selectionMode needs to be enabled for this to work.\n", 0);
			}
		});

		command::add("mapexport_selectionClear", [](command::params)
		{
			if (dvars::mapexport_selectionMode && dvars::mapexport_selectionMode->current.integer != 0)
			{
				sbox_reset(&mapexport_selectionbox_);
			}
			else
			{
				Game::Com_PrintMessage(0, "mapexport_selectionMode needs to be enabled for this to work.\n", 0);
			}
		});
	}
}