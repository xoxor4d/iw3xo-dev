#include "STDInclude.hpp"

#define CM_CONTENTS_SOLID       0x1
#define CM_CONTENTS_CLIPSHOT    0x2000      // weapon clip
#define CM_CONTENTS_DETAIL      0x8000000   // detail

#define CM_MAX_BRUSHPOINTS_FROM_INTERSECTIONS 128

std::chrono::time_point<std::chrono::steady_clock> export_time_exportStart;
std::chrono::time_point<std::chrono::steady_clock> export_time_brushGenerationStart;

bool  export_mapExportCmd = false;
bool  export_inProgress = false;
bool  export_selectionAdd;
int	  export_currentBrushIndex = 0;
float export_quadEpsilon = 0.0f;
std::ofstream	export_mapFile;
Utils::Entities	export_mapEntities;
Game::boundingBox_t export_selectionBox;

int collisionFlickerCounter;
const char *g_mapNameCm = "";

// last origin we sorted brushes at
glm::vec3 g_oldSortingOrigin = glm::vec3(0.0f);

// winding pool
char windingPool[12292];

// dynamic dvar strings
//std::string g_dvarMaterialList_str;

// material lists
std::vector<Game::dmaterial_t*> g_mapMaterialList;
std::vector<std::string> g_mapMaterialListDuplicates;
std::vector<std::string> g_mapMaterialListSingle;

// brush lists
std::vector<Game::cbrush_t*> g_mapBrushList;
std::vector<Game::cbrush_t*> g_mapBrushListForIndexFiltering;
std::vector<Game::brushmodelEnt_t> g_mapBrushModelList;

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

namespace Components
{
	// scale strings with distance :: when looking directly at them
	float StringScaleDot(Game::GfxViewParms *viewParms, const glm::vec3 origin)
	{
		glm::vec3 delta = glm::vec3(origin.x, origin.y, origin.z) - glm::vec3(viewParms->origin[0], viewParms->origin[1], viewParms->origin[2]);
		
		float scale = Utils::vector::_GLM_VectorNormalize(delta);
		float dot = glm::dot(delta, glm::vec3(viewParms->axis[0][0], viewParms->axis[0][1], viewParms->axis[0][2]));

		scale = (dot - 0.995f) * scale;

		if (scale < 1.0f) 
		{
			scale = 1.0f;
		}

		return scale;
	}

	// show brush index numbers (used for r_drawCollision_brushIndexFilter) ---- adding strings will cause flicker or duplicates because renderthread runs at diff. framerate?
	void CM_ShowBrushIndexNumbers(Game::GfxViewParms *viewParms, int brushIndex, const glm::vec3 origin, int sortedBrushIndex, int maxStringAmount)
	{
		float scale;
		glm::vec4 color;

		// we need the viewAxis to scale strings, calling Set3D populates our struct
		Game::R_Set3D();

		// scale strings with distance when looking directly at them
		scale = StringScaleDot(viewParms, origin);
		color = glm::vec4(1.0f);

		const char *brushIndexStr = Utils::VA("Index: %d", brushIndex);

		//char color_packed[4];
		//float color_test[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		//Game::R_ConvertColorToBytes(color_test, color_packed);

		//float pixel_x[3], pixel_y[3], org[3];
		//Utils::vector::_VectorScale(viewParms->axis[1], -scale, pixel_x);
		//Utils::vector::_VectorScale(viewParms->axis[2], -scale, pixel_y);

		////auto font = Game::R_RegisterFont(FONT_NORMAL, sizeof(FONT_NORMAL));
		//Game::Font_s* font = (Game::Font_s*) *(DWORD*)(0xD070C74);
		
		//glm::setFloat3(org, origin);

		//Game::RB_DrawTextInSpace(pixel_x, pixel_y, brushIndexStr, font, org, color_packed); // no idea why thats not working

		_Debug::AddDebugStringClient(origin, color, scale, brushIndexStr, 2); // not propper but will do for now
	}

	// some kind of wizardry
	void SetPlaneSignbits(Game::cplane_s *out)
	{
		int j; char bits;

		// for fast box on planeside test
		bits = 0;
		for (j = 0; j < 3; j++) 
		{
			if (out->normal[j] < 0.0f) 
			{
				bits |= 1 << j;
			}
		}

		out->signbits = bits;
	}

	// build view frustum
	void R_SetDpvsPlaneSides(Game::DpvsPlane *plane)
	{
		plane->side[0] = char(plane->coeffs[0]) <= 0 ? 0 : 12;
		plane->side[1] = char(plane->coeffs[1]) <= 0 ? 4 : 16;
		plane->side[2] = char(plane->coeffs[2]) <= 0 ? 8 : 20;
	}

	// build view frustum
	void R_FrustumClipPlanes(const Game::GfxMatrix *viewProjMtx, const float(*sidePlanes)[4], const int sidePlaneCount, Game::DpvsPlane *frustumPlanes)
	{
		int term, planeIndex; 
		float length;

		Game::DpvsPlane *plane;

		for (planeIndex = 0; planeIndex < sidePlaneCount; ++planeIndex)
		{
			for (term = 0; term < 4; ++term) 
			{
				frustumPlanes[planeIndex].coeffs[term] = ((((*sidePlanes)[4 * planeIndex + 0] * viewProjMtx->m[term][0])
														 + ((*sidePlanes)[4 * planeIndex + 1] * viewProjMtx->m[term][1]))
														 + ((*sidePlanes)[4 * planeIndex + 2] * viewProjMtx->m[term][2]))
														 + ((*sidePlanes)[4 * planeIndex + 3] * viewProjMtx->m[term][3]);
			}

			length = Utils::vector::_VectorLength(frustumPlanes[planeIndex].coeffs);

			if (length <= 0.0f) 
			{
				Game::Com_PrintMessage(0, Utils::VA("^1R_FrustumClipPlanes L#%d ^7:: length <= 0 \n", __LINE__), 0);
			}

			plane = &frustumPlanes[planeIndex];

			plane->coeffs[0] = (1.0f / length) * plane->coeffs[0];
			plane->coeffs[1] = (1.0f / length) * plane->coeffs[1];
			plane->coeffs[2] = (1.0f / length) * plane->coeffs[2];
			plane->coeffs[3] = (1.0f / length) * plane->coeffs[3];

			R_SetDpvsPlaneSides(&frustumPlanes[planeIndex]);
		}
	}

	// build view frustum
	void BuildFrustumPlanes(Game::GfxViewParms *viewParms, Game::cplane_s *frustumPlanes)
	{
		char frustumType; 
		unsigned int planeIndex;

		Game::DpvsPlane dpvsFrustumPlanes[5];
		Game::DpvsPlane *dpvsPlane;
		Game::cplane_s *cplane;

		if (!viewParms) 
		{
			Game::Com_Error(0, Utils::VA("BuildFrustumPlanes L#%d :: viewparams \n", __LINE__));
			return;
		}

		if (!frustumPlanes) 
		{
			Game::Com_Error(0, Utils::VA("BuildFrustumPlanes L#%d :: frustumPlanes \n", __LINE__));
			return;
		}

		R_FrustumClipPlanes(&viewParms->viewProjectionMatrix, frustumSidePlanes, 5, dpvsFrustumPlanes);
		
		for (planeIndex = 0; planeIndex < 5; ++planeIndex)
		{
			cplane = &frustumPlanes[planeIndex];
			dpvsPlane = &dpvsFrustumPlanes[planeIndex];
			
			cplane->normal[0] = dpvsPlane->coeffs[0];
			cplane->normal[1] = dpvsPlane->coeffs[1];
			cplane->normal[2] = dpvsPlane->coeffs[2];
			cplane->dist	  = dpvsPlane->coeffs[3];
			
			frustumPlanes[planeIndex].dist = frustumPlanes[planeIndex].dist * -1.0f;

			if (frustumPlanes[planeIndex].normal[0] == 1.0f) 
			{ 
				frustumType = 0; 
			}
			else 
			{
				if (frustumPlanes[planeIndex].normal[1] == 1.0f) 
				{ 
					frustumType = 1; 
				}
				else 
				{
					if (frustumPlanes[planeIndex].normal[2] == 1.0f) 
					{ 
						frustumType = 2; 
					}
					else 
					{ 
						frustumType = 3; 
					}	
				}
			}

			frustumPlanes[planeIndex].type = frustumType;
			SetPlaneSignbits(&frustumPlanes[planeIndex]);
		}
	}

	// create plane for intersection
	void CM_GetPlaneVec4Form(const Game::cbrushside_t* sides, const Game::axialPlane_t* axialPlanes, const int index, float* expandedPlane)
	{
		if (index >= 6)
		{
			if (!sides ) 
			{
				Game::Com_Error(0, Utils::VA("CM_GetPlaneVec4Form L#%d :: sides \n", __LINE__));
				return;
			}

			expandedPlane[0] = sides[index - 6].plane->normal[0];
			expandedPlane[1] = sides[index - 6].plane->normal[1];
			expandedPlane[2] = sides[index - 6].plane->normal[2];
			expandedPlane[3] = sides[index - 6].plane->dist;
		}
		else
		{
			/*expandedPlane[0] = axialPlanes[index][0];
			expandedPlane[1] = axialPlanes[index][1];
			expandedPlane[2] = axialPlanes[index][2];
			expandedPlane[3] = axialPlanes[index][3];*/

			glm::setFloat3(expandedPlane, axialPlanes[index].plane);
			expandedPlane[3] = axialPlanes[index].dist;
		}
	}

	// intersect 3 planes
	int IntersectPlanes(const float *plane0, const float *plane1, const float *plane2, float *xyz)
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
	bool IsOnGrid(const float *snapped, const float *xyz)
	{
		return xyz[0] == snapped[0] && xyz[1] == snapped[1] && xyz[2] == snapped[2];
	}

	// snap points to grid. might prod. some issues
	void SnapPointToIntersectingPlanes(const float *plane0, const float *plane1, const float *plane2, float *xyz, float snapGrid, const float snapEpsilon)
	{
		int axis, planeIndex;
		float rounded, delta, baseError, maxBaseError, snapError, maxSnapError, snapped[3], currentPlane[4];

		snapGrid = 1.0f / snapGrid;

		// cod4map :
		for (axis = 0; axis < 3; ++axis)
		{
			rounded = round(xyz[axis] * snapGrid) / snapGrid;
			delta	= fabs(rounded - xyz[axis]);

			if (snapEpsilon <= delta)
			{
				snapped[axis] = xyz[axis];
			}
			else
			{
				snapped[axis] = rounded;
			}
		}

		if (!IsOnGrid(snapped, xyz))
		{
			maxSnapError = 0.0f;
			maxBaseError = snapEpsilon;

			for (planeIndex = 0; planeIndex < 3; ++planeIndex)
			{
				if (planeIndex == 0)
					memcpy(&currentPlane, plane0, sizeof(currentPlane));

				else if (planeIndex == 1)
					memcpy(&currentPlane, plane1, sizeof(currentPlane));

				else if (planeIndex == 2)
					memcpy(&currentPlane, plane2, sizeof(currentPlane));


				snapError = log((currentPlane[0] * snapped[0] + currentPlane[1] * snapped[1] + currentPlane[2] * snapped[2]) - currentPlane[3]);
				if (snapError > maxSnapError)
				{
					maxSnapError = snapError;
				}
					
				baseError = log((currentPlane[0] * xyz[0] + currentPlane[1] * xyz[1] + currentPlane[2] * xyz[2]) - currentPlane[3]);
				if (baseError > maxBaseError) 
				{
					maxBaseError = baseError;
				}
			}

			if (maxBaseError > maxSnapError)
			{
				xyz[0] = snapped[0];
				xyz[1] = snapped[1];
				xyz[2] = snapped[2];
			}
		}
	}

	// add valid vertices from 3 plane intersections
	int CM_AddSimpleBrushPoint(const Game::cbrush_t* brush, const Game::axialPlane_t* axialPlanes, const __int16* sideIndices, const float* xyz, int ptCount, Game::ShowCollisionBrushPt* brushPts)
	{
		unsigned int sideIndex;
		Game::cplane_s *plane;

		if (!brush)
		{
			Game::Com_Error(0, Utils::VA("CM_AddSimpleBrushPoint L#%d :: brush \n", __LINE__));
			return 0;
		}

		if (!brushPts)
		{
			Game::Com_Error(0, Utils::VA("CM_AddSimpleBrushPoint L#%d :: brushPts \n", __LINE__));
			return 0;
		}

		for (sideIndex = 0; sideIndex < 6; ++sideIndex)
		{
			if(( (axialPlanes[sideIndex].plane.x * xyz[0] + axialPlanes[sideIndex].plane.y * xyz[1] + axialPlanes[sideIndex].plane.z * xyz[2])
				- axialPlanes[sideIndex].dist) > 0.1f)
			{
				return ptCount;
			}
		}

		for (sideIndex = 0; sideIndex < brush->numsides; ++sideIndex)
		{
			plane = brush->sides[sideIndex].plane;

			if (   plane != brush->sides[sideIndices[0] - 6].plane
				&& plane != brush->sides[sideIndices[1] - 6].plane
				&& plane != brush->sides[sideIndices[2] - 6].plane
				&& ((((plane->normal[0] * xyz[0]) + (plane->normal[1] * xyz[1])) + (plane->normal[2] * xyz[2])) - plane->dist) > 0.1f)
			{
				return ptCount;
			}
		}

		if (ptCount > CM_MAX_BRUSHPOINTS_FROM_INTERSECTIONS - 2) // T5: 1024 ...
		{
			Game::Com_PrintMessage(0, Utils::VA("CM_AddSimpleBrushPoint :: More than %i points from plane intersections on %i-sided brush\n", ptCount, brush->numsides), 0);
			return ptCount;
		}

		brushPts[ptCount].xyz[0] = xyz[0];
		brushPts[ptCount].xyz[1] = xyz[1];
		brushPts[ptCount].xyz[2] = xyz[2];

		brushPts[ptCount].sideIndex[0] = sideIndices[0];
		brushPts[ptCount].sideIndex[1] = sideIndices[1];
		brushPts[ptCount].sideIndex[2] = sideIndices[2];

		return ptCount + 1;
	}

	// intersect 3 planes (for all planes) to reconstruct vertices
	int CM_ForEachBrushPlaneIntersection(const Game::cbrush_t* brush, const Game::axialPlane_t* axialPlanes, Game::ShowCollisionBrushPt* brushPts)
	{
		int ptCount = 0, sideCount; 
		__int16 sideIndex[3];

		float xyz[3]; 
		float expandedPlane[3][4];

		Game::cbrushside_t *sides;

		if (!brush) 
		{
			Game::Com_Error(0, Utils::VA("CM_ForEachBrushPlaneIntersection L#%d :: brush \n", __LINE__));
			return 0;
		}

		if (!brushPts) 
		{
			Game::Com_Error(0, Utils::VA("CM_ForEachBrushPlaneIntersection L#%d :: brushPts \n", __LINE__));
			return 0;
		}

		sideCount = brush->numsides + 6;
		sides = brush->sides;

		// first loop should only get the axial planes till brush->numsides < 3
		for (sideIndex[0] = 0; sideIndex[0] < sideCount - 2; ++sideIndex[0])
		{
			// sideIndex[0]-[5] are axial planes only; move the current plane into expandedPlane[0]
			CM_GetPlaneVec4Form(sides, axialPlanes, sideIndex[0], (float *)expandedPlane);

			// get a plane 1 plane ahead of our first plane
			for (sideIndex[1] = sideIndex[0] + 1; sideIndex[1] < sideCount - 1; ++sideIndex[1])
			{
				// check if we're using an axial plane and 2 different planes
				if (sideIndex[0] < 6 || sideIndex[1] < 6 || sides[sideIndex[0] - 6].plane != sides[sideIndex[1] - 6].plane)
				{
					// move the current plane into expandedPlane[1]
					CM_GetPlaneVec4Form(sides, axialPlanes, sideIndex[1], expandedPlane[1]);

					// get a plane 1 plane ahead of our second plane
					for (sideIndex[2] = sideIndex[1] + 1; sideIndex[2] < sideCount - 0; ++sideIndex[2])
					{
						// check if we use axial planes or atleast 3 different sides
						if ((   sideIndex[0] < 6 || sideIndex[2] < 6 || sides[sideIndex[0] - 6].plane != sides[sideIndex[2] - 6].plane)
							&& (sideIndex[1] < 6 || sideIndex[2] < 6 || sides[sideIndex[1] - 6].plane != sides[sideIndex[2] - 6].plane))
						{
							// move the current plane into expandedPlane[2]
							CM_GetPlaneVec4Form(sides, axialPlanes, sideIndex[2], expandedPlane[2]);

							// intersect the 3 planes
							if (IntersectPlanes(expandedPlane[0], expandedPlane[1], expandedPlane[2], xyz))
							{
								// snap our verts in xyz onto the grid
								SnapPointToIntersectingPlanes(expandedPlane[0], expandedPlane[1], expandedPlane[2], xyz, 0.25f, 0.0099999998f);

								// if the planes intersected, put verts into brushPts and increase our pointCount
								ptCount = CM_AddSimpleBrushPoint(brush, axialPlanes, sideIndex, xyz, ptCount, brushPts);

								if (ptCount >= CM_MAX_BRUSHPOINTS_FROM_INTERSECTIONS - 1)
								{
									return 0;
								}
							}
						}
					}
				}
			}
		}

		return ptCount;
	}

	// check for float precision errors and check if a point lies within an epsilon 
	bool VecNCompareCustomEpsilon(const glm::vec3* xyzList, const int xyzIndex, const float* v1, const float epsilon, const int coordCount)
	{
		for (auto i = 0; i < coordCount; ++i)
		{
			if (((xyzList[xyzIndex][i] - v1[i]) * (xyzList[xyzIndex][i] - v1[i])) > (epsilon * epsilon))
			{
				return false;
			}
		}

		return true;
	}
	
	// check if point in list exists
	int CM_PointInList(const float* point, const glm::vec3* xyzList, const int xyzCount)
	{
		int xyzIndex;

		for (xyzIndex = 0; xyzIndex < xyzCount; ++xyzIndex)
		{
			if (VecNCompareCustomEpsilon(xyzList, xyzIndex, point, 0.1f, 3)) // larger epsilon decreases quality
			{
				return 1;
			}
		}

		return 0;
	}

	// create a list of vertex points
	int CM_GetXyzList(const int sideIndex, const Game::ShowCollisionBrushPt* pts, const int ptCount, glm::vec3* xyzList, const int xyzLimit)
	{
		int ptIndex, xyzCount = 0;

		if (!pts) 
		{
			Game::Com_Error(0, Utils::VA("CM_GetXyzList L#%d :: pts\n", __LINE__));
			return 0;
		}

		for (ptIndex = 0; ptIndex < ptCount; ++ptIndex)
		{
			if ((sideIndex == pts[ptIndex].sideIndex[0] || sideIndex == pts[ptIndex].sideIndex[1] || sideIndex == pts[ptIndex].sideIndex[2]) 
				&& !CM_PointInList(pts[ptIndex].xyz, xyzList, xyzCount))
			{
				if (xyzCount == xyzLimit) 
				{
					Game::Com_PrintMessage(0, Utils::VA("^1CM_GetXyzList L#%d ^7:: Winding point limit (%i) exceeded on brush face \n", __LINE__, xyzLimit), 0);
					return 0;
				}
#if DEBUG
				if (Dvars::r_drawCollision_brushDebug->current.enabled)
				{
					Game::Com_PrintMessage(0, Utils::VA("^4CM_GetXyzList L#%d ^7:: Adding X:^2 %.2lf ^7Y:^2 %.2lf ^7Z:^2 %.2lf ^7 \n", __LINE__, xyzList[xyzCount][0], xyzList[xyzCount][1], xyzList[xyzCount][2]), 0);
				}
#endif
				xyzList[xyzCount] = glm::toVec3(pts[ptIndex].xyz);
				++xyzCount;
			}
		}

#if DEBUG
		if (Dvars::r_drawCollision_brushDebug->current.enabled)
		{
			Game::Com_PrintMessage(0, Utils::VA("^1CM_GetXyzList L#%d ^7:: Total XYZCOUNT: %d \n", __LINE__, xyzCount), 0);
		}
#endif

		return xyzCount;
	}

	// pick the major axis
	void CM_PickProjectionAxes(const float *normal, int *i, int *j)
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
	
	// cross product
	float CM_SignedAreaForPointsProjected(const float* pt0, const glm::vec3& pt1, const float* pt2, const int i, const int j)
	{
		return (pt2[j] - pt1[j]) * pt0[i] + (pt0[j] - pt2[j]) * pt1[i] + (pt1[j] - pt0[j]) * pt2[i];
	}

	// add a point that intersected behind another plane that still is within the bounding box?
	void CM_AddColinearExteriorPointToWindingProjected(Game::winding_t* w, const glm::vec3& pt, int i, int j, int index0, int index1)
	{
		float delta; int axis; 

#if DEBUG
		if (w->p[index0][i] == w->p[index1][i] && w->p[index0][j] == w->p[index1][j])
		{
			Game::Com_PrintMessage(0, Utils::VA("^1CM_AddColinearExteriorPointToWindingProjected L#%d ^7:: w->p[%d][%d] %.2lf == w->p[%d][%d] %.2lf && w->p[%d][%d] %.2lf == w->p[%d][%d] %.2lf \n", 
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
				Game::Com_PrintMessage(0, Utils::VA("^1CM_AddColinearExteriorPointToWindingProjected L#%d ^7:: w->p[%d][%d] %.2lf <= w->p[%d][%d] %.2lf \n", 
													__LINE__, index0, axis, w->p[index0][axis], index1, axis, w->p[index1][axis]), 0);
			}
#endif

			if (pt[axis] <= w->p[index0][axis]) 
			{
				if (w->p[index1][axis] > pt[axis])
				{
					glm::setFloat3(w->p[index1], pt);
				}
			}
			else
			{
				glm::setFloat3(w->p[index0], pt);
			}
		}

		else
		{

#if DEBUG
			if (w->p[index1][axis] <= w->p[index0][axis])
			{
				Game::Com_PrintMessage(0, Utils::VA("^1CM_AddColinearExteriorPointToWindingProjected L#%d ^7:: w->p[%d][%d] %.2lf < w->p[%d][%d] %.2lf \n", 
													__LINE__, index1, axis, w->p[index1][axis], index0, axis, w->p[index0][axis]), 0);
			}
#endif

			if (w->p[index0][axis] <= pt[axis]) 
			{
				if (pt[axis] > w->p[index1][axis])
				{
					glm::setFloat3(w->p[index1], pt);
				}
			}
			else
			{
				glm::setFloat3(w->p[index0], pt);
			}
		}
	}

	// Source :: PolyFromPlane || Q3 :: RemoveColinearPoints ?
	void CM_AddExteriorPointToWindingProjected(Game::winding_t* w, const glm::vec3& pt, int i, int j)
	{
		int index, indexPrev, bestIndex = -1;
		float signedArea, bestSignedArea = FLT_MAX;

		indexPrev = w->numpoints - 1;

		for (index = 0; index < w->numpoints; ++index)
		{
			signedArea = CM_SignedAreaForPointsProjected(w->p[indexPrev], pt, w->p[index], i, j);
			
			if (bestSignedArea > signedArea)
			{
				bestSignedArea = signedArea;
				bestIndex = index;
			}

			indexPrev = index;
		}

#if DEBUG
		if (bestIndex < 0)
		{
			Game::Com_PrintMessage(0, Utils::VA("^1CM_AddExteriorPointToWindingProjected L#%d ^7:: bestIndex < 0 \n", __LINE__), 0);
		}
#endif

		if (bestSignedArea < -0.001f)
		{
			memmove((char *)w->p[bestIndex + 1], (char *)w->p[bestIndex], 12 * (w->numpoints - bestIndex));
			
			glm::setFloat3(w->p[bestIndex], pt);
			++w->numpoints;
		}

		else if (bestSignedArea <= 0.001) 
		{
			CM_AddColinearExteriorPointToWindingProjected(w, pt, i, j, (bestIndex + w->numpoints - 1) % w->numpoints, bestIndex);
		}
	}

	// create a triangle to check the winding order?
	float CM_RepresentativeTriangleFromWinding(const Game::winding_t *w, const float *normal, int *i0, int *i1, int *i2)
	{
		int i, k; signed int j;
		float testAgainst, areaBest = 0.0f;
		float va[3], vb[3], vc[3];
		
		*i0 = 0; *i1 = 1; *i2 = 2;

		for (k = 2; k < w->numpoints; ++k)
		{
			for (j = 1; j < k; ++j)
			{
				vb[0] = w->p[k][0] - w->p[j][0];	
				vb[1] = w->p[k][1] - w->p[j][1];	
				vb[2] = w->p[k][2] - w->p[j][2];
				for (i = 0; i < j; ++i)
				{
					va[0] = w->p[i][0] - w->p[j][0];	
					va[1] = w->p[i][1] - w->p[j][1];	
					va[2] = w->p[i][2] - w->p[j][2];

					Utils::vector::_Vec3Cross(va, vb, vc);
					testAgainst = fabs(((vc[0] * normal[0]) + (vc[1] * normal[1])) + (vc[2] * normal[2]));

					if(testAgainst > 0.0f)
					{
						areaBest = testAgainst;
						*i0 = i;
						*i1 = j;
						*i2 = k;
					}
				}
			}
		}

		return areaBest;
	}

	// create a plane from points
	bool PlaneFromPoints(float *plane, const float *v0, const float *v1, const float *v2)
	{
		float v2_v0[3], v1_v0[3];
		float length, lengthSq;

		v1_v0[0] = v1[0] - v0[0];
		v1_v0[1] = v1[1] - v0[1];
		v1_v0[2] = v1[2] - v0[2];
		v2_v0[0] = v2[0] - v0[0];
		v2_v0[1] = v2[1] - v0[1];
		v2_v0[2] = v2[2] - v0[2];

		Utils::vector::_Vec3Cross(v2_v0, v1_v0, plane);

		lengthSq = ((plane[0] * plane[0]) + (plane[1] * plane[1])) + (plane[2] * plane[2]);
		
		if (lengthSq >= 2.0f)
			goto WEGOOD;
		
		if (lengthSq == 0.0f)
			return false;
		
		if ((((((v2_v0[0] * v2_v0[0]) + (v2_v0[1] * v2_v0[1])) + (v2_v0[2] * v2_v0[2])) 
			* (((v1_v0[0] * v1_v0[0]) + (v1_v0[1] * v1_v0[1])) + (v1_v0[2] * v1_v0[2]))) * 0.0000010000001) >= lengthSq)
		{
			v1_v0[0] = v2[0] - v1[0];
			v1_v0[1] = v2[1] - v1[1];
			v1_v0[2] = v2[2] - v1[2];
			v2_v0[0] = v0[0] - v1[0];
			v2_v0[1] = v0[1] - v1[1];
			v2_v0[2] = v0[2] - v1[2];

			Utils::vector::_Vec3Cross(v2_v0, v1_v0, plane);

			if ((((((v2_v0[0] * v2_v0[0]) + (v2_v0[1] * v2_v0[1])) + (v2_v0[2] * v2_v0[2]))
				* (((v1_v0[0] * v1_v0[0]) + (v1_v0[1] * v1_v0[1])) + (v1_v0[2] * v1_v0[2]))) * 0.0000010000001) >= lengthSq) 
			{
				return false;
			}
		}

	WEGOOD:
		length = sqrt(lengthSq);
		plane[0] = plane[0] / length;
		plane[1] = plane[1] / length;
		plane[2] = plane[2] / length;
		plane[3] = ((v0[0] * plane[0]) + (v0[1] * plane[1])) + (v0[2] * plane[2]);
		
		return true;
	}

	// reverse clock-wise windings
	void CM_ReverseWinding(Game::winding_t *w)
	{
		int i; float windingSave[3];

		for (i = 0; i < w->numpoints / 2; ++i)
		{
			windingSave[0] = w->p[i][0];
			windingSave[1] = w->p[i][1];
			windingSave[2] = w->p[i][2];

			w->p[i][0] = w->p[w->numpoints - 1 - i][0];
			w->p[i][1] = w->p[w->numpoints - 1 - i][1];
			w->p[i][2] = w->p[w->numpoints - 1 - i][2];

			w->p[w->numpoints - 1 - i][0] = windingSave[0];
			w->p[w->numpoints - 1 - i][1] = windingSave[1];
			w->p[w->numpoints - 1 - i][2] = windingSave[2];
		}
	}

	// Map Export - CM_BuildBrushWindingForSide
	bool CM_BuildBrushWindingForSideMapExport(Game::winding_t *winding, const float *planeNormal, const int sideIndex, Game::ShowCollisionBrushPt *pts, int ptCount, Game::map_brushSide_t *bSide)
	{
		int xyzCount, i, i0, i1, i2, j, k;
		Game::vec4_t plane;

		Utils::vector::_VectorZero4(plane);

		if (!winding)
		{
			Game::Com_Error(0, Utils::VA("CM_BuildBrushWindingForSide L#%d :: winding \n", __LINE__));
			return false;
		}

		if (!planeNormal)
		{
			Game::Com_Error(0, Utils::VA("CM_BuildBrushWindingForSide L#%d :: planeNormal \n", __LINE__));
			return false;
		}

		if (!pts)
		{
			Game::Com_Error(0, Utils::VA("CM_BuildBrushWindingForSide L#%d :: pts \n", __LINE__));
			return false;
		}

		glm::vec3 _xyzList[1024];
		xyzCount = CM_GetXyzList(sideIndex, pts, ptCount, _xyzList, 1024);

		if (xyzCount < 3) 
		{
			return false;
		}

		CM_PickProjectionAxes(planeNormal, &i, &j);

		glm::setFloat3(winding->p[0], _xyzList[0]);
		glm::setFloat3(winding->p[1], _xyzList[1]);

		winding->numpoints = 2;

		for (k = 2; k < xyzCount; ++k) 
		{
			CM_AddExteriorPointToWindingProjected(winding, _xyzList[k], i, j);
		}

		if (CM_RepresentativeTriangleFromWinding(winding, planeNormal, &i0, &i1, &i2) < 0.001) 
		{
			return false;
		}

		PlaneFromPoints(&*plane, winding->p[i0], winding->p[i1], winding->p[i2]);

		if ((((plane[0] * planeNormal[0]) + (plane[1] * planeNormal[1])) + (plane[2] * planeNormal[2])) < 0.0f) 
		{
			CM_ReverseWinding(winding);
		}

		// huh
		Game::winding_t *w = winding;

		for (auto _i = 0; _i < 3; _i++)
		{ for (auto _j = 0; _j < 3; _j++)
		  { 
			if (fabs(w->p[_i][_j]) < Dvars::mapexport_brushEpsilon1->current.value)
			{
				w->p[_i][_j] = 0;
			}
			else if (fabs((int)w->p[_i][_j] - w->p[_i][_j]) < Dvars::mapexport_brushEpsilon2->current.value)
			{
				w->p[_i][_j] = (float)(int)w->p[_i][_j];
			}
		  }
		}

		int p1;
		int planenum = 0;

		//three non-colinear points to define the plane
		if (planenum & 1) { p1 = 1; }
		else { p1 = 0; }

		// *
		// create the brushside

		// plane 0
		for (auto idx = 0; idx < 3; idx++)
		{
			bSide->brushPlane[0].point[idx] = w->p[p1][idx];
		}

		// plane 1
		for (auto idx = 0; idx < 3; idx++) 
		{
			bSide->brushPlane[1].point[idx] = w->p[!p1][idx];
		}

		// plane 2
		for (auto idx = 0; idx < 3; idx++)
		{
			bSide->brushPlane[2].point[idx] = w->p[2][idx];
		}

		/*if (!Utils::polylib::CheckWinding(w))
		{
			Game::Com_PrintMessage(0, "removed degenerate brushside.\n", 0);
			return false;
		}*/
		
		return true;
	}

	// build winding (poly) for side
	bool CM_BuildBrushWindingForSide(Game::winding_t* winding, const float* planeNormal, const int sideIndex, Game::ShowCollisionBrushPt* pts, int ptCount)
	{
		int xyzCount, i, i0, i1, i2, j, k;
		Game::vec4_t plane; Utils::vector::_VectorZero4(plane);

		if (!winding) 
		{
			Game::Com_Error(0, Utils::VA("CM_BuildBrushWindingForSide L#%d :: winding \n", __LINE__));
			return false;
		}

		if (!planeNormal) 
		{
			Game::Com_Error(0, Utils::VA("CM_BuildBrushWindingForSide L#%d :: planeNormal \n", __LINE__));
			return false;
		}

		if (!pts) 
		{
			Game::Com_Error(0, Utils::VA("CM_BuildBrushWindingForSide L#%d :: pts \n", __LINE__));
			return false;
		}

		// create a list of vertex points
		glm::vec3 _xyzList[1024];

		xyzCount = CM_GetXyzList(sideIndex, pts, ptCount, _xyzList, 1024);

		// we need atleast a triangle to create a poly
		if (xyzCount < 3) 
		{
			return false;
		}

		// direction of camera plane
		glm::vec3 cameraDirectionToPlane = _xyzList[0] - Game::Globals::locPmove_cameraOrigin;

		// dot product between line from camera to the plane and the normal
		// if dot > 0 then the plane is facing away from the camera (dot = 1 = plane is facing the same way as the camera; dot = -1 = plane looking directly towards the camera)
		if (glm::dot( glm::vec3(planeNormal[0], planeNormal[1], planeNormal[2]), cameraDirectionToPlane ) > 0.0f && !Dvars::r_drawCollision_polyFace->current.enabled) 
		{
			return false;
		}
		
		// find the major axis
		CM_PickProjectionAxes(planeNormal, &i, &j);

		glm::setFloat3(winding->p[0], _xyzList[0]);
		glm::setFloat3(winding->p[1], _xyzList[1]);

		winding->numpoints = 2;

		for (k = 2; k < xyzCount; ++k) 
		{
			CM_AddExteriorPointToWindingProjected(winding, _xyzList[k], i, j);
		}

		// build a triangle of our winding points so we can check if the winding is clock-wise
		if (CM_RepresentativeTriangleFromWinding(winding, planeNormal, &i0, &i1, &i2) < 0.001) 
		{	
			// do nothing if it is counter clock-wise
			return false;
		}

		// *
		// winding is clock-wise ..

		// create a temp plane
		PlaneFromPoints(&*plane, winding->p[i0], winding->p[i1], winding->p[i2]);

		// if our winding has a clock-wise winding, reverse it
		if (((plane[0] * planeNormal[0]) + (plane[1] * planeNormal[1]) + (plane[2] * planeNormal[2])) > 0.0f) 
		{
			CM_ReverseWinding(winding);
		}

		return 1;
	}

	// Allocates a single brushside
	Game::map_brushSide_t *Alloc_BrushSide(void)
	{
		Game::map_brushSide_t *bSide;
		bSide = (Game::map_brushSide_t*)malloc(sizeof(*bSide));
		
		if (bSide)
		{
			memset(bSide, 0, sizeof(Game::map_brushSide_t));
			return bSide;
		}
		
		Game::Com_Error(0, "Alloc_BrushSide :: alloc failed!");
		return 0;
	}

	bool CM_IsMapBrushSideWithinBounds(const Game::map_brushSide_t* bSide, const glm::vec3& mins, const glm::vec3& maxs)
	{
		if (!bSide)
		{
			return false;
		}

		for (auto plane = 0; plane < 3; plane++)
		{
			if (!Utils::polylib::PointWithinBounds(glm::toVec3(bSide->brushPlane[plane].point), mins, maxs, 0.25f))
			{
				return false;
			}
		}

		return true;
	}
	
	// rebuild and draw brush from bounding box and dynamic sides
	void CM_ShowSingleBrushCollision(Game::cbrush_t *brush, const float *color, int brushIndex, bool enableExport = true)
	{
		// skip all brush calculations when flicker mode is on and not exporting a map file
		if (Dvars::r_drawCollision_flickerBrushes->current.enabled && !export_inProgress)
		{
			// on-time :: if amount of passed frames > flickerOn + flickerOff
			if (collisionFlickerCounter > Dvars::r_drawCollision_flickerOnTime->current.integer + Dvars::r_drawCollision_flickerOffTime->current.integer)
			{
				collisionFlickerCounter = 0;
			}

			// off-time :: skip collision drawing if amount of "on-frames" are larger then our "on-frames-dvar"
			else if (collisionFlickerCounter > Dvars::r_drawCollision_flickerOnTime->current.integer)
			{
				return;
			}
		}

		int ptCount, sideIndex;
		Game::ShowCollisionBrushPt brushPts[CM_MAX_BRUSHPOINTS_FROM_INTERSECTIONS]; // T5: 1024 .. wtf

		if (!brush) 
		{
			Game::Com_Error(0, Utils::VA("CM_ShowSingleBrushCollision L#%d :: brush \n", __LINE__));
			return;
		}

		if (!color) 
		{
			Game::Com_Error(0, Utils::VA("CM_ShowSingleBrushCollision L#%d :: color\n", __LINE__));
			return;
		}

		// Create static sides (CM_BuildAxialPlanes)
		Game::axialPlane_t _axialPlanes[6];
		_axialPlanes[0].plane = glm::vec3(-1.0f, 0.0f, 0.0f);
		_axialPlanes[0].dist = -brush->mins[0];

		_axialPlanes[1].plane = glm::vec3(1.0f, 0.0f, 0.0f);
		_axialPlanes[1].dist = brush->maxs[0];

		_axialPlanes[2].plane = glm::vec3(0.0f, -1.0f, 0.0f);
		_axialPlanes[2].dist = -brush->mins[1];

		_axialPlanes[3].plane = glm::vec3(0.0f, 1.0f, 0.0f);
		_axialPlanes[3].dist = brush->maxs[1];

		_axialPlanes[4].plane = glm::vec3(0.0f, 0.0f, -1.0f);
		_axialPlanes[4].dist = -brush->mins[2];

		_axialPlanes[5].plane = glm::vec3(0.0f, 0.0f, 1.0f);
		_axialPlanes[5].dist = brush->maxs[2];

		// intersect all planes, 3 at a time, to to reconstruct face windings 
		ptCount = CM_ForEachBrushPlaneIntersection(brush, _axialPlanes, brushPts);

		// we need atleast 4 valid points
		if (ptCount >= 4)
		{
			// list of brushsides we are going to create within "CM_BuildBrushWindingForSideMapExport"
			std::vector<Game::map_brushSide_t*> mapBrush;

			float planeNormal[3];

			auto poly_lit		= Dvars::r_drawCollision_polyLit->current.enabled;
			auto poly_outlines	= Dvars::r_drawCollision->current.integer == 3 ? true : false;
			auto poly_linecolor = Dvars::r_drawCollision_lineColor->current.vector;
			auto poly_depth		= Dvars::r_drawCollision_polyDepth->current.enabled;
			auto poly_face		= Dvars::r_drawCollision_polyFace->current.enabled;

			// -------------------------------
			// brushside [0]-[5] (axialPlanes)

			for (sideIndex = 0; (unsigned int)sideIndex < 6; ++sideIndex)
			{
				glm::setFloat3(planeNormal, _axialPlanes[sideIndex].plane);

				// build winding for the current brushside and check if it is visible (culling)
				if (CM_BuildBrushWindingForSide((Game::winding_t *)windingPool, planeNormal, sideIndex, brushPts, ptCount))
				{
					if (Dvars::r_drawCollision->current.integer == 1)
					{
						_Debug::RB_AddAndDrawDebugLines(*(DWORD*)windingPool, (float(*)[3])& windingPool[4], Dvars::r_drawCollision_lineColor->current.vector);
					}
					else
					{
						_Debug::RB_DrawPoly(
							/* numPts	*/ *(DWORD*)windingPool, 
							/* points	*/ (float(*)[3])& windingPool[4], 
							/* pColor	*/ color, 
							/* pLit		*/ poly_lit,
							/* pOutline */ poly_outlines,
							/* pLineCol	*/ poly_linecolor,
							/* pDepth	*/ poly_depth,
							/* pFace	*/ poly_face);
					}
				
					Game::Globals::dbgColl_drawnPlanesAmountTemp++;
				}

				// create brushsides from brush bounds (side [0]-[5])
				if (export_inProgress && enableExport)
				{
					// allocate a brushside
					Game::map_brushSide_t *bSide = Alloc_BrushSide();

					// create a brushside from windings
					if (CM_BuildBrushWindingForSideMapExport((Game::winding_t *)windingPool, planeNormal, sideIndex, brushPts, ptCount, bSide))
					{
						// brushside is valid
						mapBrush.push_back(bSide);
					}

					else
					{
						// not a valid brushside so free it
						free(bSide);
					}
				}
			}

			// ---------------------------------
			// brushside [6] and up (additional)

			for (sideIndex = 6; sideIndex < (std::int32_t)brush->numsides + 6; ++sideIndex)
			{
				if (CM_BuildBrushWindingForSide((Game::winding_t *)windingPool, brush->sides[sideIndex - 6].plane->normal, sideIndex, brushPts, ptCount)) 
				{
					if (Dvars::r_drawCollision->current.integer == 1)
					{
						_Debug::RB_AddAndDrawDebugLines(*(DWORD*)windingPool, (float(*)[3]) & windingPool[4], Dvars::r_drawCollision_lineColor->current.vector);
					}
					else
					{
						_Debug::RB_DrawPoly(
							/* numPts	*/ *(DWORD*)windingPool,
							/* points	*/ (float(*)[3]) & windingPool[4],
							/* pColor	*/ color,
							/* pLit		*/ poly_lit,
							/* pOutline */ poly_outlines,
							/* pLineCol	*/ poly_linecolor,
							/* pDepth	*/ poly_depth,
							/* pFace	*/ poly_face);
					}

					Game::Globals::dbgColl_drawnPlanesAmountTemp++;
				}

				// create brushsides from cm->brushes->sides (side [6] and up)
				if (export_inProgress && enableExport)
				{
					// allocate a brushside
					Game::map_brushSide_t *bSide = Alloc_BrushSide();

					// create a brushside from windings
					if (CM_BuildBrushWindingForSideMapExport((Game::winding_t *)windingPool, brush->sides[sideIndex - 6].plane->normal, sideIndex, brushPts, ptCount, bSide))
					{
						// brushside is valid
						mapBrush.push_back(bSide);
					}

					else
					{
						// not a valid brushside so free it
						free(bSide);
					}
				}
			}

			// if we are exporting the map
			if (export_inProgress && enableExport)
			{
				bool dirty_hack_5_sides = false;

				glm::vec3 bMins = glm::toVec3(brush->mins);
				glm::vec3 bMaxs = glm::toVec3(brush->maxs);

				// we need atleast 6 valid brushsides
				if (mapBrush.size() < 6)
				{
					if (mapBrush.size() == 5 && Dvars::mapexport_brush5Sides && Dvars::mapexport_brush5Sides->current.enabled)
					{
						dirty_hack_5_sides = true;
					}
					else
					{
						return;
					}
				}

				// check brushes defined by more then their axialplanes
				if(mapBrush.size() > 6)
				{
					if (glm::distance(bMins, bMaxs) < Dvars::mapexport_brushMinSize->current.value)
					{
						return;
					}
				}

				for (auto side = 0; side < static_cast<int>(mapBrush.size()); side++)
				{
					if (!CM_IsMapBrushSideWithinBounds(mapBrush[side], bMins, bMaxs))
					{
						return;
					}
				}

				// swap brushsides (bottom, top, right, back, left, front)
				if (!dirty_hack_5_sides)
				{
					std::swap(mapBrush[0], mapBrush[5]);
				}
					
				std::swap(mapBrush[3], mapBrush[4]);
				std::swap(mapBrush[1], mapBrush[3]);
				std::swap(mapBrush[0], mapBrush[1]);

				// * 
				// do not export brushmodels as normal brushes
				// write brushside strings to g_mapBrushModelList instead

				if (brush->isSubmodel)
				{
					// clear any existing sides
					g_mapBrushModelList[brush->cmSubmodelIndex].brushSides.clear();
				}
				else
				{
					// start brush
					export_mapFile << Utils::VA("// brush %d\n{", export_currentBrushIndex) << std::endl;
					export_mapFile << "layer \"000_Global/Brushes\"" << std::endl;

					// global brush exporting index count
					export_currentBrushIndex++;

					// write brush contents
					if (brush->contents & CM_CONTENTS_DETAIL)
					{
						export_mapFile << "contents detail;" <<  std::endl;
					}
					else if (brush->contents & CM_CONTENTS_CLIPSHOT)
					{
						export_mapFile << "contents weaponClip;" << std::endl;
					}
				}

				// print brush sides and material info
				for (auto bs = 0; bs < (int)mapBrush.size(); bs++)
				{
					std::string currBrushSide = Utils::VA(" ( %d %d %d ) ( %d %d %d ) ( %d %d %d ) ",
						(int)mapBrush[bs]->brushPlane[0].point[0], (int)mapBrush[bs]->brushPlane[0].point[1], (int)mapBrush[bs]->brushPlane[0].point[2],
						(int)mapBrush[bs]->brushPlane[1].point[0], (int)mapBrush[bs]->brushPlane[1].point[1], (int)mapBrush[bs]->brushPlane[1].point[2],
						(int)mapBrush[bs]->brushPlane[2].point[0], (int)mapBrush[bs]->brushPlane[2].point[1], (int)mapBrush[bs]->brushPlane[2].point[2]);

					if (!brush->isSubmodel)
					{
						export_mapFile << currBrushSide.c_str();
					}

					// get material index for the current brush side
					int matIdx = 0;

					// for the 6 brush sides created from axialplanes (brush bounds)
					if (bs < 6)
					{
						// get material (brush->axialMaterialNum[array][index]) for the current brush side
						// mapping axialnum order to .map brush side order
						switch (bs)
						{
						case 0: // bottom
							matIdx = (int)brush->axialMaterialNum[0][2];
							break;
						case 1: // top
							matIdx = (int)brush->axialMaterialNum[1][2];
							break;
						case 2: // right
							matIdx = (int)brush->axialMaterialNum[0][1];
							break;
						case 3: // back
							matIdx = (int)brush->axialMaterialNum[1][0];
							break;
						case 4: // left
							matIdx = (int)brush->axialMaterialNum[1][1];
							break;
						case 5: // front
							matIdx = (int)brush->axialMaterialNum[0][0];
							break;
						}
					}

					// we have atleast 1 additional brush side
					else
					{
						if (!dirty_hack_5_sides)
						{
							// additional brush sides start at index 0
							matIdx = brush->sides[bs - 6].materialNum;
						}
					}

					// *
					// Material Dimensions

					// default values if we fail to find the correct texture size
					int texWidth = 128;
					int texHeight = 128;

					// texture size scalar (depends on texture quality settings)
					float tex_scalar = 1.0f;

					const auto r_picmip = Game::Dvar_FindVar("r_picmip");
					if (r_picmip)
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
						default: // should not happen
							tex_scalar = 1.0f;
						}
					}

					// get the world material and its size
					std::string material_name_for_brushside = Game::cm->materials[matIdx].material;
					std::string materialForSide = "wc/"s + material_name_for_brushside;

					const auto mat = Game::Material_RegisterHandle(materialForSide.c_str(), 3);
					if (mat)
					{
						for (auto tex = 0; tex < mat->textureCount; tex++)
						{
							// 0x2 = color, 0x5 = normal, 0x8 = spec
							if (mat->textureTable[tex].u.image->semantic == 0x2)
							{
								texWidth = static_cast<int>((mat->textureTable[tex].u.image->width * tex_scalar)); // loaded texture sizes vary (texture quality settings)
								texHeight = static_cast<int>((mat->textureTable[tex].u.image->height * tex_scalar)); // so we need to use a scalar to get a 0.25 stretch in radiant

								break;
							}
						}
					}

					//std::string materialForSide = Game::cm->materials[matIdx].material;

					// for each material in memory
					//for (auto matMem = 0; matMem < Game::_gfxWorld->materialMemoryCount; matMem++)
					//{
					//	// current material name
					//	std::string currMatName = Game::_gfxWorld->materialMemory[matMem].material->info.name; // material with suffix wc/ mc/ ..

					//	// check if material is our brush side material
					//	if (currMatName.find(materialForSide) != std::string::npos)
					//	{
					//		// find the colormap of the brush side material
					//		for (auto texture = 0; texture < Game::_gfxWorld->materialMemory[matMem].material->textureCount; texture++)
					//		{
					//			// current texture name
					//			//std::string currTexName = Game::_gfxWorld->materialMemory[matMem].material->textureTable[texture].u.image->name;
					//			// .... //if (Utils::has_suffix(currTexName, std::string("_col")) || Utils::has_suffix(currTexName, std::string("_c")))

					//			// 0x2 = color, 0x5 = normal, 0x8 = spec
					//			if (Game::_gfxWorld->materialMemory[matMem].material->textureTable[texture].u.image->semantic == 0x2)
					//			{
					//				texWidth = (int)(Game::_gfxWorld->materialMemory[matMem].material->textureTable[texture].u.image->width * 0.25f); // assuming 0.25 horizontal stretch
					//				texHeight = (int)(Game::_gfxWorld->materialMemory[matMem].material->textureTable[texture].u.image->height * 0.25f); // assuming 0.25 vertical stretch

					//				break;
					//			}
					//		}

					//		// we found the correct material
					//		break;
					//	}
					//}

					if (!brush->isSubmodel)
					{
						// materialname, width, height, xpos, ypos, rotation, ?, lightmapMat, lmap_sampleSizeX, lmap_sampleSizeY, lmap_xpos, lmap_ypos, lmap_rotation, ?
						export_mapFile << Utils::VA("%s %d %d 0 0 0 0 lightmap_gray 16384 16384 0 0 0 0\n", material_name_for_brushside.c_str(), texWidth, texHeight);
					}
					else
					{
						g_mapBrushModelList[brush->cmSubmodelIndex].brushSides.push_back(currBrushSide + Utils::VA("%s %d %d 0 0 0 0 lightmap_gray 16384 16384 0 0 0 0\n", material_name_for_brushside.c_str(), texWidth, texHeight));
					}
					
				}

				if (!brush->isSubmodel)
				{
					// end brush
					export_mapFile << "}" << std::endl;
				}
	
				// single brush for radiant ......
				//	Game::ServerCommand cmd;

				//	Game::Com_PrintMessage(0, "exporting brushsides for selected brush:\n", 0);

				//	// for all brushsides ::
				//	// send index first, then the brushside
				//	for (auto bs = 0; bs < (int)mapBrush.size(); bs++)
				//	{
				//		memset(&cmd, 0, sizeof(Game::ServerCommand));
				//		cmd.type = Game::SERVER_EXPORT_SINGLE_BRUSH_FACE_INDEX;
				//		sprintf_s(cmd.strCommand, "%d", bs);

				//		RadiantRemote::RemoteNet_SendPacket(&cmd);

				//		memset(&cmd, 0, sizeof(Game::ServerCommand));
				//		cmd.type = Game::SERVER_EXPORT_SINGLE_BRUSH_FACE;
				//		sprintf_s(cmd.strCommand, "%.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f",
				//			mapBrush[bs]->brushPlane[0].point[0], mapBrush[bs]->brushPlane[0].point[1], mapBrush[bs]->brushPlane[0].point[2],
				//			mapBrush[bs]->brushPlane[1].point[0], mapBrush[bs]->brushPlane[1].point[1], mapBrush[bs]->brushPlane[1].point[2],
				//			mapBrush[bs]->brushPlane[2].point[0], mapBrush[bs]->brushPlane[2].point[1], mapBrush[bs]->brushPlane[2].point[2]);

				//		RadiantRemote::RemoteNet_SendPacket(&cmd);

				//		Game::Com_PrintMessage(0, Utils::VA("< ( %.2f %.2f %.2f ) ( %.2f %.2f %.2f ) ( %.2f %.2f %.2f ) >\n",
				//			mapBrush[bs]->brushPlane[0].point[0], mapBrush[bs]->brushPlane[0].point[1], mapBrush[bs]->brushPlane[0].point[2],
				//			mapBrush[bs]->brushPlane[1].point[0], mapBrush[bs]->brushPlane[1].point[1], mapBrush[bs]->brushPlane[1].point[2],
				//			mapBrush[bs]->brushPlane[2].point[0], mapBrush[bs]->brushPlane[2].point[1], mapBrush[bs]->brushPlane[2].point[2]), 0);
				//	}
				//}
			}
		}
	}

	// Brush View Culling
	int BoxOnPlaneSide(const float *emins, const float *emaxs, const Game::cplane_s *p)
	{
		float dist1, dist2; 
		int sides = 0;

		// fast axial cases
		if (p->type < 3)
		{
			if (p->dist <= emins[p->type]) { return 1; }
			if (p->dist >= emaxs[p->type]) { return 2; }
			
			return 3;
		}

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

	// check if any side of a brush is within the view frustum
	bool CM_BrushInView(Game::cbrush_t *brush, Game::cplane_s *frustumPlanes, int frustumPlaneCount)
	{
		int frustumPlaneIndex;

		if (!frustumPlanes) 
		{ 
			Game::Com_Error(0, Utils::VA("CM_BrushInView L#%d :: frustumPlanes\n", __LINE__));
			return false;
		}

		for (frustumPlaneIndex = 0; frustumPlaneIndex < frustumPlaneCount; ++frustumPlaneIndex)
		{
			if (!(BoxOnPlaneSide(brush->mins, brush->maxs, &frustumPlanes[frustumPlaneIndex]) & 1))
			{
				return 0;
			}
		}

		return 1;
	}

	// check if the material selected via dvar equals the current brush material or one of its sides if the first side uses caulk
	bool CM_ValidBrushMaterialSelection(Game::cbrush_t *brush, int materialNumFromDvar)
	{
		// we can also filter materials by substrings with "r_drawCollision_materialInclude" ( "clip_player" returns true when using "clip" )
		std::string includeString = "";

		// workaround till we get dvar strings to register - actually, cba. to implement that now
		switch (Dvars::r_drawCollision_materialInclude->current.integer)
		{
		case 1:
			includeString = "clip";
			break;

		case 2:
			includeString = "mantle";
			break;

		case 3:
			includeString = "trigger";
			break;

		case 4:
			includeString = "all"; 
			return true; // no need to check any material so return true
			//break; // removing brushes using a trigger texture

		case 5:
			includeString = "all-no-tools";
			break;
		case 6:
			includeString = "all-no-tools-clip";
			break;
		}

		// check if we are within array bounds
		if (static_cast<size_t>(brush->axialMaterialNum[0][0]) >= g_mapMaterialListDuplicates.size())
		{
			return false;
		}

		if (static_cast<size_t>(materialNumFromDvar) >= g_mapMaterialListSingle.size())
		{
			return false;
		}

		// bridge dupe materials list with the cleaned list
		std::string materialNameFromDuplicates = g_mapMaterialListDuplicates[(int)brush->axialMaterialNum[0][0]];
		std::string materialNameFromSingle = g_mapMaterialListSingle[materialNumFromDvar];
		
		// instantly found our material
		if (materialNameFromDuplicates == materialNameFromSingle) 
		{
			return true;
		}

		// if filter is not empty
		if (!includeString.empty())
		{
			// if we found a matching substring
			if (materialNameFromDuplicates.find(includeString) != std::string::npos) 
			{
				return true;
			}

			// draw all materials (mainly for map exporting)
			else if (includeString == "all") 
			{
				// no longer skip triggers
				/*if (materialNameFromDuplicates.find("trigger") != std::string::npos)
				{
					return false;
				}*/

				return true;
			}

			// draw all materials without info volumes
			else if (includeString == "all-no-tools") 
			{
				if(materialNameFromDuplicates.find("portal") != std::string::npos)
					return false;

				if (materialNameFromDuplicates.find("hint") != std::string::npos)
					return false;

				if (materialNameFromDuplicates.find("volume") != std::string::npos)
					return false;

				if (materialNameFromDuplicates.find("mantle") != std::string::npos)
					return false;

				if (materialNameFromDuplicates.find("trigger") != std::string::npos)
					return false;

				if (materialNameFromDuplicates.find("sky") != std::string::npos)
					return false;

				// if no excluded materials were found, return true
				return true;
			}

			else if (includeString == "all-no-tools-clip")
			{
				if (materialNameFromDuplicates.find("portal") != std::string::npos)
					return false;

				if (materialNameFromDuplicates.find("hint") != std::string::npos)
					return false;

				if (materialNameFromDuplicates.find("volume") != std::string::npos)
					return false;

				if (materialNameFromDuplicates.find("mantle") != std::string::npos)
					return false;

				if (materialNameFromDuplicates.find("trigger") != std::string::npos)
					return false;

				if (materialNameFromDuplicates.find("sky") != std::string::npos)
					return false;

				if (materialNameFromDuplicates.find("clip") != std::string::npos)
					return false;

				// if no excluded materials were found, return true
				return true;
			}
		}

		// if the brush has caulk as its first side, check the other sides for our material/substring
		else if (materialNameFromDuplicates == "caulk")
		{
			int currentMatArray;

			// check the first and second material array
			for (currentMatArray = 0; currentMatArray < 2; currentMatArray++)
			{
				// check its 3 elements
				for (int matIndex = 0; matIndex < 3; matIndex++)
				{
					// if one of the sides uses our material
					if (g_mapMaterialListDuplicates[(int)brush->axialMaterialNum[currentMatArray][matIndex]] == materialNameFromSingle) 
					{
						return true;
					}

					// if we have a filter set
					if (Dvars::r_drawCollision_materialInclude->current.integer != 0)
					{
						// if one of the sides matches our substring
						if (g_mapMaterialListDuplicates[(int)brush->axialMaterialNum[currentMatArray][matIndex]].find(includeString) != std::string::npos) 
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
	void CM_ResetSelectionBox(Game::boundingBox_t* box)
	{
		memset(box, 0, sizeof(Game::boundingBox_t));

		box->mins = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		box->maxs = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		box->wasReset = true;
	}

	// get mins/maxs from points
	void CM_BoundsFromPoints(const int& numPoints, const glm::vec3* points, glm::vec3& mins, glm::vec3& maxs)
	{
		mins = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		maxs = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (auto sPoint = 0; sPoint < numPoints; sPoint++)
		{
			for (auto pAxis = 0; pAxis < 3; pAxis++)
			{
				// mins :: find the closest point on each axis
				if (mins[pAxis] > points[sPoint][pAxis])
					mins[pAxis] = points[sPoint][pAxis];

				// maxs :: find the furthest point on each axis
				if (maxs[pAxis] < points[sPoint][pAxis])
					maxs[pAxis] = points[sPoint][pAxis];
			}
		}
	}

	// calculate brush midpoint
	glm::vec3 CM_GetBrushMidpoint(const Game::cbrush_t* brush, bool xyzMidpoint = false)
	{
		if (!xyzMidpoint)
		{
			return (glm::vec3(brush->mins[0], brush->mins[1], 0.0f)
				+ glm::vec3(brush->maxs[0], brush->maxs[1], 0.0f)) * 0.5f;
		}

		else
		{
			return (glm::vec3(brush->mins[0], brush->mins[1], brush->mins[2])
				+ glm::vec3(brush->maxs[0], brush->maxs[1], brush->maxs[2])) * 0.5f;
		}
	}

	// check if a brushes midpoint is within a selection box
	bool CM_IsBrushWithinSelectionBox(const Game::cbrush_t* brush, const Game::boundingBox_t* selectionBox)
	{
		if (brush && selectionBox->isBoxValid)
		{
			return Utils::polylib::PointWithinBounds(CM_GetBrushMidpoint(brush, true), selectionBox->mins, selectionBox->maxs, 0.25f);
		}
		
		return false;
	}

	// check if a triangles midpoint is within a selection box
	bool CM_IsTriangleWithinSelectionBox(const Game::map_patchTris_t* tris, Game::boundingBox_t* selectionBox)
	{
		for (auto triCoord = 0; triCoord < 3; triCoord++)
		{
			if(!Utils::polylib::PointWithinBounds(glm::toVec3(tris->coords[triCoord].xyz), selectionBox->mins, selectionBox->maxs, 0.25f))
			{
				return false;
			}
		}

		return true;
	}

	// -------------
	// CM_OnceOnInit

	// :: create "g_mapMaterialListDuplicates" and write all materials without \0
	// :: create "g_mapMaterialListSingle" add all material from matListDuplicates without creating duplicates
	// :: create a dvar string from "g_mapMaterialListSingle"
	// :: compare axialMaterialNum with "g_mapMaterialListDuplicates" to get the original materialName
	// :: compare "g_mapMaterialListDuplicates" materialName with the choosen dvar Material name
	// :: draw the brush if they match
	
	// stuff we only do once per map after r_drawcollision was turned on
	void CM_OnceOnInit()
	{
		// assign a color to each brush (modified cbrush_t struct) so we use persistent colors even when sorting brushes
		int colorCounter = 0;

		// only once per map
		//if (g_mapNameCm != Game::cm->name)
		if(!Game::Globals::dbgColl_initialized || Utils::Q_stricmp(g_mapNameCm, Game::cm->name))
		{
			Game::Com_PrintMessage(0, "[Debug Collision] : initializing ...\n", 0);

			// list of all brushmodels within the current map mapped to their respective brushes in cm->brushes
			Utils::Entities mapEnts(Game::cm->mapEnts->entityString);
			g_mapBrushModelList = mapEnts.getBrushModels();

			Game::Com_PrintMessage(0, Utils::VA("|-> found %d submodels\n", static_cast<int>(g_mapBrushModelList.size())), 0);

			// hacky string dvar
			auto dvarName = "r_drawCollision_brushIndexFilter";
			Game::Dvar_RegisterString_hacky(dvarName, "null", "Specifies which brushes to draw. Ignores all other filters and will disable brush sorting.\nInput example: \"101 99 2\" :: \"^1null^7\" disables this filter");

			// assign hacky dvar to the global dvar
			Dvars::r_drawCollision_brushIndexFilter = Game::Dvar_FindVar(dvarName);

#if DEBUG
			// change dvar to only draw 1 brush when using a debug build (debug build has really bad performance)
			Game::Dvar_SetValue(Dvars::r_drawCollision_brushAmount, 1);
#endif
			// cap brushAmount
			Dvars::r_drawCollision_brushAmount->domain.integer.max = Game::cm->numBrushes - 1;

			// reset brush distance filter
			Game::Dvar_SetValue(Dvars::r_drawCollision_brushDist, 800.0f);

			// clear global vectors
			g_mapBrushList.clear();
			g_mapMaterialList.clear();
			g_mapMaterialListDuplicates.clear();
			g_mapMaterialListSingle.clear();
			//g_dvarMaterialList_str.clear();
			Game::Globals::r_drawCollision_materialList_string.clear();

			// init/reset the selection box
			CM_ResetSelectionBox(&export_selectionBox);

			// create a dmaterial_t vector
			std::int32_t cmMaterialNum = Game::cm->numMaterials;
			std::vector<Game::dmaterial_t*> currMapMaterials(cmMaterialNum);

			// assign clipMap material pointers
			for (int num = 0; num < cmMaterialNum; num++) 
			{
				currMapMaterials[num] = &Game::cm->materials[num];
			}

			// create a cbrush_t vector
			std::vector<Game::cbrush_t*> currBrushList(Game::cm->numBrushes);

			// assign clipMap brush pointers
			for (int num = 0; num < Game::cm->numBrushes; num++)
			{
				currBrushList[num] = &Game::cm->brushes[num];
				currBrushList[num]->colorCounter = (short)colorCounter++;
				currBrushList[num]->cmBrushIndex = (short)num;

				colorCounter %= 8;
			}

			// set globals
			Game::Globals::dbgColl_initialized = true;
			g_mapNameCm = Game::cm->name;
			g_mapMaterialList = currMapMaterials;
			g_mapBrushList = currBrushList;

			// create a string vector of all material (contains duplicates)
			for (std::uint32_t num = 0; num < g_mapMaterialList.size(); num++)
			{
				std::string materialName = Utils::convertToString(g_mapMaterialList[num]->material, sizeof(g_mapMaterialList[num]->material));
				materialName.erase(std::remove(materialName.begin(), materialName.end(), '\0'), materialName.end());

				g_mapMaterialListDuplicates.push_back(materialName);
			}

			// create a string vector that without duplicate materials (used for dvar description)
			for (std::uint32_t num = 0; num < g_mapMaterialListDuplicates.size(); num++)
			{
				std::string materialName = g_mapMaterialListDuplicates[num];

				// if the vector is empty or the materialName does not exist within the array
				if (g_mapMaterialListSingle.empty() || std::find(g_mapMaterialListSingle.begin(), g_mapMaterialListSingle.end(), materialName) == g_mapMaterialListSingle.end()) 
				{
					g_mapMaterialListSingle.push_back(materialName);
				}
			}

			// create the dvar string
			auto singleMaterialCount = g_mapMaterialListSingle.size();

			for (std::uint32_t num = 0; num < singleMaterialCount; num++) 
			{
				// do not print "empty" materials
				if (!Utils::StartsWith(g_mapMaterialListSingle[num], "*"))
				{
					/*g_dvarMaterialList_str*/ 
					Game::Globals::r_drawCollision_materialList_string += std::to_string(num) + ": " + g_mapMaterialListSingle[num] + "\n";
				}
			}

			Game::Com_PrintMessage(0, Utils::VA("|-> found %d materials\n", static_cast<int>(singleMaterialCount)), 0);

			// Set material dvar back to 0, update description and max value
			Game::Dvar_SetValue(Dvars::r_drawCollision_material, 0);
			Dvars::r_drawCollision_material->domain.integer.max = singleMaterialCount - 1;

			//print the material list to the large console if the list has more then 20 materials
			if (singleMaterialCount > 20)
			{
				Dvars::r_drawCollision_material->description = "Too many materials to show here! Use ^1\"r_drawCollision_materialList\" ^7to print a list of all materials.\nOnly works with ^1\"r_drawCollision\" ^7enabled!";
			}
			else
			{
				Dvars::r_drawCollision_material->description = Game::Globals::r_drawCollision_materialList_string.c_str(); //g_dvarMaterialList_str.c_str();
			}

			Game::Com_PrintMessage(0, "|-> done\n", 0);
		}
	}

	// sort globBrushList depending on distance from brush to camera
	void CM_SortBrushListOnDistanceCamera(bool farToNear = true, bool useCurrentBrushesDrawnList = false, bool updateAlways = false)
	{
		if (!useCurrentBrushesDrawnList)
		{
			// sort brushes by distance from brush midpoint(xy) to camera origin :: only sort if the players origin has changed
			if (g_oldSortingOrigin[0] != Game::Globals::locPmove_playerOrigin[0] || g_oldSortingOrigin[1] != Game::Globals::locPmove_playerOrigin[1] || updateAlways)
			{
				// sort from far to near (used for rendering)
				if (farToNear)
				{
					std::sort(g_mapBrushList.begin(), g_mapBrushList.end(), [](Game::cbrush_t *brush1, Game::cbrush_t *brush2)
					{
						glm::vec3 b1_mid = (glm::vec3(brush1->mins[0], brush1->mins[1], 0.0f)
							+ glm::vec3(brush1->maxs[0], brush1->maxs[1], 0.0f)) * 0.5f;

						glm::vec3 b2_mid = (glm::vec3(brush2->mins[0], brush2->mins[1], 0.0f)
							+ glm::vec3(brush2->maxs[0], brush2->maxs[1], 0.0f)) * 0.5f;

						float dist1 = glm::distance(b1_mid, Game::Globals::locPmove_playerOrigin);
						float dist2 = glm::distance(b2_mid, Game::Globals::locPmove_playerOrigin);

						return dist1 > dist2;
					});
				}

				// sort from near to far (get closest brush)
				else
				{
					std::sort(g_mapBrushList.begin(), g_mapBrushList.end(), [](Game::cbrush_t *brush1, Game::cbrush_t *brush2)
					{
						glm::vec3 b1_mid = (glm::vec3(brush1->mins[0], brush1->mins[1], brush1->mins[2])
							+ glm::vec3(brush1->maxs[0], brush1->maxs[1], brush1->maxs[2])) * 0.5f;

						glm::vec3 b2_mid = (glm::vec3(brush2->mins[0], brush2->mins[1], brush2->mins[2])
							+ glm::vec3(brush2->maxs[0], brush2->maxs[1], brush2->maxs[2])) * 0.5f;

						float dist1 = glm::distance(b1_mid, Game::Globals::locPmove_playerOrigin);
						float dist2 = glm::distance(b2_mid, Game::Globals::locPmove_playerOrigin);

						return dist1 < dist2;
					});
				}

				//memcpy(prev_Origin, Game::Globals::locPmove_playerOrigin, sizeof(prev_Origin));
				g_oldSortingOrigin = Game::Globals::locPmove_playerOrigin;
			}
		}

		// use drawn brushes list if not empty
		else if(!g_mapBrushListForIndexFiltering.empty())
		{
			// sort brushes by distance from brush midpoint(xy) to camera origin :: only sort if the players origin has changed
			if (g_oldSortingOrigin[0] != Game::Globals::locPmove_playerOrigin[0] || g_oldSortingOrigin[1] != Game::Globals::locPmove_playerOrigin[1] || updateAlways)
			{
				// sort from far to near (used for rendering)
				if (farToNear)
				{
					std::sort(g_mapBrushListForIndexFiltering.begin(), g_mapBrushListForIndexFiltering.end(), [](Game::cbrush_t *brush1, Game::cbrush_t *brush2)
					{
						glm::vec3 b1_mid = (glm::vec3(brush1->mins[0], brush1->mins[1], 0.0f)
							+ glm::vec3(brush1->maxs[0], brush1->maxs[1], 0.0f)) * 0.5f;

						glm::vec3 b2_mid = (glm::vec3(brush2->mins[0], brush2->mins[1], 0.0f)
							+ glm::vec3(brush2->maxs[0], brush2->maxs[1], 0.0f)) * 0.5f;

						float dist1 = glm::distance(b1_mid, Game::Globals::locPmove_playerOrigin);
						float dist2 = glm::distance(b2_mid, Game::Globals::locPmove_playerOrigin);

						return dist1 > dist2;
					});
				}

				// sort from near to far (get closest brush)
				else
				{
					std::sort(g_mapBrushListForIndexFiltering.begin(), g_mapBrushListForIndexFiltering.end(), [](Game::cbrush_t *brush1, Game::cbrush_t *brush2)
					{
						glm::vec3 b1_mid = (glm::vec3(brush1->mins[0], brush1->mins[1], 0.0f)
							+ glm::vec3(brush1->maxs[0], brush1->maxs[1], 0.0f)) * 0.5f;

						glm::vec3 b2_mid = (glm::vec3(brush2->mins[0], brush2->mins[1], 0.0f)
							+ glm::vec3(brush2->maxs[0], brush2->maxs[1], 0.0f)) * 0.5f;

						float dist1 = glm::distance(b1_mid, Game::Globals::locPmove_playerOrigin);
						float dist2 = glm::distance(b2_mid, Game::Globals::locPmove_playerOrigin);

						return dist1 < dist2;
					});
				}

				g_oldSortingOrigin = Game::Globals::locPmove_playerOrigin;
			}
		}
	}

	// color brushes depending on their index within cm->brushes
	void CM_GetShowCollisionColor(float *colorFloat, const int colorCounter)
	{
		if (!colorFloat) 
		{
			Game::Com_Error(0, Utils::VA("CM_GetShowCollisionColor L#%d :: colorFloat\n", __LINE__));
			return;
		}

		if (colorCounter & 1) 
		{ colorFloat[0] = 1.0f; }
		else 
		{ colorFloat[0] = 0.0f; }

		if (colorCounter & 2) 
		{ colorFloat[1] = 1.0f; }
		else 
		{ colorFloat[1] = 0.0f; }

		if (colorCounter & 4) 
		{ colorFloat[2] = 1.0f; }
		else 
		{ colorFloat[2] = 0.0f; }

		colorFloat[3] = Dvars::r_drawCollision_polyAlpha->current.value;
	}
	
	// check if 2 triangles share an edge
	bool PatchTriangle_SharesEdge(const Game::map_patchTris_t *pTri1, const Game::map_patchTris_t *pTri2)
	{
		int matchedPoints = 0;

		// for each edge of triangle 1
		for (auto edgeTri1 = 0; edgeTri1 < 3; edgeTri1++)
		{
			// for each edge of triangle 2
			for (auto edgeTri2 = 0; edgeTri2 < 3; edgeTri2++)
			{
				matchedPoints += Utils::vector::_VectorCompare(pTri1->coords[edgeTri1].xyz, pTri2->coords[edgeTri2].xyz);
			}

			// shouldnt happen
			if (matchedPoints > 2)
			{
				Game::Com_PrintMessage(0, "^1[MAP-EXPORT]: ^7 PatchTriangle_SharesEdge matchedPoints > 2", 0);
			}

			if (matchedPoints == 2)
			{
				return true;
			}
		}

		return false;
	}

	// if (k < 0.001f) = counter clock wise
	bool PatchTriangle_ClockwiseWinding(const float *pt0, const float *pt1, const float *pt2)
	{
		float k = (pt1[1] - pt0[1]) * (pt2[0] - pt1[0]) - (pt1[0] - pt0[0]) * (pt2[1] - pt1[1]);
		return k > 0.001f;
	}

	// not in map format order
	void PatchTriangle_FromIncides(Game::map_patchTris_t *pTris, const unsigned short *incides)
	{
		bool foundFirst = false, foundSecond = false, foundThird = false, secondIteration = false;

		// find the first match
		for (auto gfxVert = 0; gfxVert < static_cast<int>(Game::_gfxWorld->vertexCount); gfxVert++)
		{
			// try to match our first clipmap vertex to a gfxworld vertex
			if (!foundFirst && Utils::vector::_VectorCompare(Game::cm->verts[incides[0]], Game::_gfxWorld->vd.vertices[gfxVert].xyz))
			{
				// found the first corrosponding gfx vertex :: copy gfx vertex data to our temp vertex
				memcpy(&pTris->coords[0], &Game::_gfxWorld->vd.vertices[gfxVert], sizeof(Game::GfxWorldVertex));

				// go back 6 verts in the gfxworld vertex array and start searching for our second point
				// :: assuming that our verts are close to each other
				if (gfxVert - 6 > 0) 
				{
					gfxVert -= 6; 
				}

				else 
				{
					gfxVert = 0; 
				}

				// do not match first vert again
				foundFirst = true;
			}

			// try to match our second clipmap vertex to a gfxworld vertex
			if (!foundSecond && Utils::vector::_VectorCompare(Game::cm->verts[incides[1]], Game::_gfxWorld->vd.vertices[gfxVert].xyz))
			{
				// found the second corrosponding gfx vertex :: copy gfx vertex data to our temp vertex
				memcpy(&pTris->coords[1], &Game::_gfxWorld->vd.vertices[gfxVert], sizeof(Game::GfxWorldVertex));

				// go back 6 verts in the gfxworld vertex array
				// :: assuming that our verts are close to each other
				if (gfxVert - 6 > 0)
				{
					gfxVert -= 6;
				}

				else
				{
					gfxVert = 0;
				}

				// do not match second vert again
				foundSecond = true;
			}

			// try to match our third clipmap vertex to a gfxworld vertex
			if (!foundThird && Utils::vector::_VectorCompare(Game::cm->verts[incides[2]], Game::_gfxWorld->vd.vertices[gfxVert].xyz))
			{
				// found the third corrosponding gfx vertex :: copy gfx vertex data to our temp vertex
				memcpy(&pTris->coords[2], &Game::_gfxWorld->vd.vertices[gfxVert], sizeof(Game::GfxWorldVertex));

				// go back 6 verts in the gfxworld vertex array
				// :: assuming that our verts are close to each other
				if (gfxVert - 6 > 0)
				{
					gfxVert -= 6;
				}

				else
				{
					gfxVert = 0;
				}

				// do not match third vert again
				foundThird = true;
			}

			if (foundFirst && foundSecond && foundThird)
			{
				break;
			}
			// if we did not match all 3 and looped all of the gfxworld vertices
			else if (gfxVert + 1 > (int)Game::_gfxWorld->vertexCount && !secondIteration)
			{
				// check the whole array once again (as we only gone back 6 verts each time we found a corrosponding vert)
				gfxVert = 0;

				// break out of the loop if we failed to match all 3 verts within the second iteration
				secondIteration = true;
			}
		}

		// if we failed to match all points, write clipmap vert data
		if (!foundFirst)
		{
			pTris->coords[0].xyz[0] = Game::cm->verts[incides[0]][0];
			pTris->coords[0].xyz[1] = Game::cm->verts[incides[0]][1];
			pTris->coords[0].xyz[2] = Game::cm->verts[incides[0]][2];
		}

		if (!foundSecond)
		{
			pTris->coords[1].xyz[0] = Game::cm->verts[incides[1]][0];
			pTris->coords[1].xyz[1] = Game::cm->verts[incides[1]][1];
			pTris->coords[1].xyz[2] = Game::cm->verts[incides[1]][2];
		}

		if (!foundThird)
		{
			pTris->coords[2].xyz[0] = Game::cm->verts[incides[2]][0];
			pTris->coords[2].xyz[1] = Game::cm->verts[incides[2]][1];
			pTris->coords[2].xyz[2] = Game::cm->verts[incides[2]][2];
		}
	}

	// Allocates a single patch triangle
	Game::map_patchTris_t *Alloc_PatchTriangle(void)
	{
		Game::map_patchTris_t *pTris;
		pTris = (Game::map_patchTris_t*)malloc(sizeof(*pTris));
		
		if (pTris)
		{
			memset(pTris, 0, sizeof(Game::map_patchTris_t));
			return pTris;
		}

		Game::Com_Error(0, "Alloc_PatchTriangle :: alloc failed!");
		return 0;
	}

	// Allocates a single patch quad
	Game::map_patchQuads_t *Alloc_PatchQuad(void)
	{
		Game::map_patchQuads_t *pQuad;
		pQuad = (Game::map_patchQuads_t*)malloc(sizeof(*pQuad));
		
		if (pQuad)
		{
			memset(pQuad, 0, sizeof(Game::map_patchQuads_t));
			return pQuad;
		}

		Game::Com_Error(0, "Alloc_PatchQuad :: alloc failed!");
		return 0;
	}

	// coord to check = xy; bounds = xywh
	bool IsCoordWithinBoundsXY(float *coordToCheck, float *bounds)
	{
		// if pt to right of left border
		if (coordToCheck[0] >= bounds[0]) // bounds x
		{
			// if pt to left of right border
			if (coordToCheck[0] <= (bounds[0] + bounds[2])) // bounds x + w
			{
				// if pt above bottom border
				if (coordToCheck[1] >= bounds[1]) // bounds y
				{
					// if pt below top border
					if (coordToCheck[1] <= (bounds[1] + bounds[3])) // bounds y + h
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	// quad in map format order (tiangles need to share an edge or it will fail)
	bool PatchQuad_SingleFromTris(Game::map_patchQuads_t *pQuad, const Game::map_patchTris_t *pTri1, const Game::map_patchTris_t *pTri2, const bool checkSkewness = false)
	{
		// temp quad points
		std::vector<float>coord(3, 0.0f);
		std::vector<std::vector<float>> t_quadPts(4, coord);

		int uniquePtCount = 0;

		// for each coord of triangle 1
		for (auto crdTri1 = 0; crdTri1 < 3; crdTri1++)
		{
			int sharedPtCount = 0;

			// for each coord of triangle 2
			for (auto crdTri2 = 0; crdTri2 < 3; crdTri2++)
			{
				// check if the current coord from tri 1 is unique and not part of tri 2
				if (Utils::vector::_VectorCompare(pTri1->coords[crdTri1].xyz, pTri2->coords[crdTri2].xyz))
				{
					// found shared point
					sharedPtCount++;
					break;
				}
			}

			// check if we found the unique coord of triangle 1
			if (!sharedPtCount)
			{
				// add unique point that isnt on the shared edge
				t_quadPts[0][0] = pTri1->coords[crdTri1].xyz[0];
				t_quadPts[0][1] = pTri1->coords[crdTri1].xyz[1];
				t_quadPts[0][2] = pTri1->coords[crdTri1].xyz[2];
				
				uniquePtCount++;
				break;
			}
		}

		// if we found the unique point
		if (uniquePtCount)
		{
			// we should only have 1 point from the first tiangle in our list, otherwise, throw an error?
			if (uniquePtCount > 1)
			{
				Game::Com_PrintMessage(0, "^1[MAP-EXPORT]: ^7 PatchQuad_SingleFromTris t_quadPts.size() > 1", 0);
				return false;
			}

			for (auto secondTriCoords = 0; secondTriCoords < 3; secondTriCoords++)
			{
				// add point that isnt on the shared edge
				t_quadPts[secondTriCoords + 1][0] = pTri2->coords[secondTriCoords].xyz[0];
				t_quadPts[secondTriCoords + 1][1] = pTri2->coords[secondTriCoords].xyz[1];
				t_quadPts[secondTriCoords + 1][2] = pTri2->coords[secondTriCoords].xyz[2];
			}

			// unpack the normal .. might be needed for texture info later down the line
			Game::vec3_t normal; 
			Utils::vector::_Vec3UnpackUnitVec(pTri2->coords[1].normal, normal);

			// sort x accending
			std::sort(t_quadPts.begin(), t_quadPts.end());

			// x was sorted, now sort by y
			std::sort(t_quadPts.begin(), t_quadPts.end());

			// always check clockwise ordering?
			float pt0[3], pt1[3], pt2[3], pt3[3];

			// why did i do that
			memcpy(&pt0, &t_quadPts[0][0], sizeof(float[3]));
			memcpy(&pt1, &t_quadPts[1][0], sizeof(float[3]));
			memcpy(&pt2, &t_quadPts[2][0], sizeof(float[3]));
			memcpy(&pt3, &t_quadPts[3][0], sizeof(float[3]));

			// cross :: triangle clockwise (k > 0.001f) | counter clockwise (k < 0.001f)
			float k = (pt1[1] - pt0[1]) * (pt2[0] - pt1[0]) - (pt1[0] - pt0[0]) * (pt2[1] - pt1[1]);

			if (k < 0.001f)
			{
				// triangle is counter clockwise, discard
				return false;
			}

			if (checkSkewness)
			{
				/*Game::vec3_t dirVec1, dirVec2; 
				Utils::vector::_VectorZero(dirVec1); Utils::vector::_VectorZero(dirVec2);
				
				Utils::vector::_VectorSubtract(pt0, pt1, dirVec1);
				Utils::vector::_VectorSubtract(pt2, pt1, dirVec2);

				Utils::vector::_VectorNormalize(dirVec1);
				Utils::vector::_VectorNormalize(dirVec2);

				float dot = Utils::vector::_DotProduct(dirVec1, dirVec2);
				float angle = acosf(dot) * 180.0f / 3.141592f;*/

				//if(angle > 70.0f || angle < 10.0f )
				//	return false;

				// triangle can be in clockwise order and still be using a point of a neighboring triangle 
				// project a quad from the 3 coords >> skip the triangle if the 4th point is not within the bounds

				// horizontal check from left to right
				float ptToCheck[2] = 
				{ pt3[0], pt3[1] };
				
				float bounds[4] = 
				{ 
					/*x*/ pt0[0], 
					/*y*/ pt0[1], 
					/*w*/ fabs(pt2[0] - pt0[0]) + export_quadEpsilon,
					/*h*/ fabs(pt1[1] - pt0[1]) + export_quadEpsilon
				};

				// discard point if not within bounds 
				if (!IsCoordWithinBoundsXY(ptToCheck, bounds)) 
				{
					return false;
				}

				// vertical check from top to bottom
				float ptToCheck2[2] = 
				{ pt2[0], pt2[1] };

				float bounds2[4] =
				{
					/*x*/ pt0[0],
					/*y*/ pt0[1],
					/*w*/ fabs(pt3[0] - pt0[0]) + export_quadEpsilon,
					/*h*/ fabs(pt1[1] - pt0[1]) + export_quadEpsilon
				};
				
				// discard point if not within bounds 
				if (!IsCoordWithinBoundsXY(ptToCheck2, bounds2)) 
				{
					return false;
				}
			}

			// build the quad from the 4 sorted coordinates
			for (auto quadCrd = 0; quadCrd < 4; quadCrd++)
			{
				for (auto quadCrdIdx = 0; quadCrdIdx < 3; quadCrdIdx++)
				{
					pQuad->coords[quadCrd][quadCrdIdx] = t_quadPts[quadCrd][quadCrdIdx];
				}
			}

			return true;
		}

		return false;
	}

	// handle filters
	void CM_BrushFilters(int &filter_BrushAmount, bool &filter_BrushIndex, bool &filter_BrushSorting, bool &filter_BrushSelection)
	{
		int cmTotalBrushAmount = static_cast<int>(Game::cm->numBrushes);

		// brush bounding box filter
		if (Dvars::mapexport_selectionMode && Dvars::mapexport_selectionMode->current.integer != 0)
		{
			if(Dvars::r_drawCollision_brushAmount && Dvars::r_drawCollision_brushAmount->current.integer != cmTotalBrushAmount)
				Game::Dvar_SetValue(Dvars::r_drawCollision_brushAmount, cmTotalBrushAmount);

			if (Dvars::r_drawCollision_brushSorting && Dvars::r_drawCollision_brushSorting->current.integer != 0)
				Game::Dvar_SetValue(Dvars::r_drawCollision_brushSorting, 0);

			if (Dvars::r_drawCollision_brushIndexFilter && (std::string)Dvars::r_drawCollision_brushIndexFilter->current.string != "null")
				Game::Dvar_SetString("null", Dvars::r_drawCollision_brushIndexFilter);

			filter_BrushAmount = cmTotalBrushAmount;
			filter_BrushSelection = true;

			return;
		}

		// brush index filter & none of the above
		else if(Dvars::r_drawCollision_brushIndexFilter && (std::string)Dvars::r_drawCollision_brushIndexFilter->current.string != "null")
		{
			if (Dvars::r_drawCollision_brushAmount && Dvars::r_drawCollision_brushAmount->current.integer != cmTotalBrushAmount)
				Game::Dvar_SetValue(Dvars::r_drawCollision_brushAmount, cmTotalBrushAmount);

			if (Dvars::r_drawCollision_brushSorting && Dvars::r_drawCollision_brushSorting->current.integer != 0)
				Game::Dvar_SetValue(Dvars::r_drawCollision_brushSorting, 0);

			filter_BrushAmount = cmTotalBrushAmount;
			filter_BrushIndex = true;

			return;
		}

		// brush sorting & none of the above
		else if(Dvars::r_drawCollision_brushSorting && Dvars::r_drawCollision_brushSorting->current.integer != 0)
		{
			filter_BrushSorting = true;
		}

		// brush amount filter & none of the above
		if (Dvars::r_drawCollision_brushAmount && Dvars::r_drawCollision_brushAmount->current.integer != 0 && Dvars::r_drawCollision_brushAmount->current.integer <= cmTotalBrushAmount)
		{
			filter_BrushAmount = Dvars::r_drawCollision_brushAmount->current.integer;
			return;
		}
		else
		{
			if (Dvars::r_drawCollision_brushAmount && Dvars::r_drawCollision_brushAmount->current.integer != cmTotalBrushAmount)
				Game::Dvar_SetValue(Dvars::r_drawCollision_brushAmount, cmTotalBrushAmount);

			filter_BrushAmount = cmTotalBrushAmount;
		}
	}

	// handle brush dvar commands
	void CM_BrushCommands()
	{
		// cmd :: print material list to console
		if (Dvars::r_drawCollision_materialList && Dvars::r_drawCollision_materialList->current.enabled)
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, "clear\n");

			// add spaces to the console so we can scroll the mini console
			Game::Com_PrintMessage(0, Utils::VA(" \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n"), 0);

			// reset the dvar and print our material list
			Game::Dvar_SetValue(Dvars::r_drawCollision_materialList, false);
			Game::Com_PrintMessage(0, Utils::VA("%s\n", /*g_dvarMaterialList_str.c_str()*/ Game::Globals::r_drawCollision_materialList_string.c_str()), 0);
		}
	}

	// create a bounding box to select things that should be exported
	void CM_BrushSelectionBox(bool isActive)
	{
		if (isActive)
		{
			float selectionBoxColor[4] = { 0.0f, 1.0f, 0.0f, 0.6f };
			export_selectionBox.wasReset = false;

			if (!export_selectionBox.isBoxValid)
			{
				// we need atleast 2 valid points to define the selection box
				if (export_selectionBox.numPoints < 2)
				{
					if(export_selectionAdd)
					{
						export_selectionBox.points[export_selectionBox.numPoints] = Game::Globals::locPmove_playerOrigin;
						export_selectionBox.numPoints++;

						export_selectionAdd = false;
					}

					// use the players origin as a temporary second point till a second point was defined
					if (export_selectionBox.numPoints == 1)
					{
						export_selectionBox.points[1] = Game::Globals::locPmove_playerOrigin;

						CM_BoundsFromPoints(2, export_selectionBox.points, export_selectionBox.mins, export_selectionBox.maxs);
						glm::setFloat3(export_selectionBox.box.mins, export_selectionBox.mins);
						glm::setFloat3(export_selectionBox.box.maxs, export_selectionBox.maxs);

						CM_ShowSingleBrushCollision(&export_selectionBox.box, selectionBoxColor, 0, false);
					}
				}
				else
				{
					// calculate mins and maxs for our selection box once
					if (export_selectionBox.numPoints == 2 && !export_selectionBox.isBoxValid)
					{
						CM_BoundsFromPoints(export_selectionBox.numPoints, export_selectionBox.points, export_selectionBox.mins, export_selectionBox.maxs);
						glm::setFloat3(export_selectionBox.box.mins, export_selectionBox.mins);
						glm::setFloat3(export_selectionBox.box.maxs, export_selectionBox.maxs);

						export_selectionBox.isBoxValid = true;
					}
				}
			}
			else
			{
				CM_ShowSingleBrushCollision(&export_selectionBox.box, selectionBoxColor, 0, false);
			}
		}
		else
		{
			if (!export_selectionBox.wasReset)
			{
				CM_ResetSelectionBox(&export_selectionBox);
			}
		}
	}

	// main logic for brush drawing
	void CM_ShowBrushCollision(Game::GfxViewParms *viewParms, Game::cplane_s *frustumPlanes, int frustumPlaneCount)
	{
		int debugPrints, colorCounter = 0, lastDrawnBrushAmount = 0, filter_BrushAmount = 0;
		bool filter_BrushIndex = false, filter_BrushSorting = false, filter_ExportSelection = false;

		float colorFloat[4];
		Game::cbrush_t *brush;
		
		if (!frustumPlanes) 
		{
			Game::Com_Error(0, Utils::VA("CM_ShowBrushCollision L#%d :: frustumPlanes \n", __LINE__));
			return;
		}

		// One time init per map when r_drawcollison was enabled
		CM_OnceOnInit();

		// Handle filter dvars
		CM_BrushFilters(filter_BrushAmount, filter_BrushIndex, filter_BrushSorting, filter_ExportSelection);

		// Handle commands
		CM_BrushCommands();

		// *
		// Map Export
		
		export_inProgress = false;		// reset after exporting
		export_currentBrushIndex = 0;	// brush index for .map file brushes
		
		// cmd :: export current map
		if (export_mapExportCmd)
		{
			// reset the the command bool
			export_mapExportCmd = false;

			// let our code know that we are about to export a map
			export_inProgress = true;

			// map file name
			std::string mapName = Game::cm->name;
			Utils::replaceAll(mapName, std::string("maps/mp/"), std::string(""));
			Utils::replaceAll(mapName, std::string(".d3dbsp"), std::string(".map"));

			// export to root/map_export
			std::string basePath = Game::Dvar_FindVar("fs_basepath")->current.string;
						basePath += "\\iw3xo\\map_export\\";

			std::string filePath = basePath + mapName;

			Game::Com_PrintMessage(0, "\n------------------------------------------------------\n", 0);
			export_time_exportStart = Utils::Clock_StartTimerPrint(Utils::VA("[MAP-EXPORT]: Starting to export %s to %s ...\n", Game::cm->name, filePath.c_str()));

			// create directory root/map_export if it doesnt exist
			// client is only able to export to a sub-directory of "menu_export"

			if (std::filesystem::create_directories(basePath))
			{
				Game::Com_PrintMessage(0, "|- Created directory \"root/iw3xo/map_export\"\n", 0);
			}

			// steam to .map file
			export_mapFile.open(filePath.c_str());

			// build entity list
			export_mapEntities = Utils::Entities(Game::cm->mapEnts->entityString);
			Game::Com_PrintMessage(0, "|- Writing header and world entity ...\n\n", 0);

			// write header
			export_mapFile <<	"iwmap 4\n"
								"\"000_Global\" flags expanded  active\n"
								"\"000_Global/Brushes\" flags\n"
								"\"000_Global/SingleQuads\" flags\n"
								"\"000_Global/Triangles\" flags\n"
								"\"000_Global/Models\" flags\n"
								"\"The Map\" flags" << std::endl; // header

			// write worldspawn
			export_mapFile <<	"// entity 0\n" 
								"{\n"  
								+ export_mapEntities.buildWorldspawnKeys();

			// Use debug collision methods to create our brushes ...
			export_time_brushGenerationStart = Utils::Clock_StartTimerPrint("[MAP-EXPORT]: Creating brushes ...\n");
		}

		// Handle box selection
		CM_BrushSelectionBox(filter_ExportSelection);

		// do not draw brushes when using the selection box
		if (!export_inProgress && filter_ExportSelection)
		{
			return;
		}

		// --------

		bool brushIndexVisible = Dvars::r_drawCollision_brushIndexVisible && Dvars::r_drawCollision_brushIndexVisible->current.enabled;

		if (filter_BrushSorting)
		{
			// sort brushes far to near
			if (Dvars::r_drawCollision_brushSorting->current.integer == 1)
			{
				CM_SortBrushListOnDistanceCamera();
			}
			// sort brushes near to far
			else
			{
				CM_SortBrushListOnDistanceCamera(false);
			}

			// disable sorting if sorted brushList is empty .. should not happen
			if (g_mapBrushList.empty())
			{
				Game::Dvar_SetValue(Dvars::r_drawCollision_brushSorting, 0);
				filter_BrushSorting = false;
			}
		}

		// clear drawn brushes vector
		g_mapBrushListForIndexFiltering.clear();

		// reset hud element
		Game::Globals::dbgColl_drawnPlanesAmountTemp = 0;

		int BRUSH_INDEX, BRUSH_COUNT;
		std::vector<int> Integers;

		// brush index filtering
		if (filter_BrushIndex)
		{
			Utils::extractIntegerWords(Dvars::r_drawCollision_brushIndexFilter->current.string, Integers, true);
			BRUSH_COUNT = static_cast<int>(Integers.size());
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
			if (lastDrawnBrushAmount >= filter_BrushAmount)
			{
				break;
			}

			// brush index filtering
			if (filter_BrushIndex)
			{
				brushIndex = Integers[BRUSH_INDEX];

				// check if the index is within bounds
				if (brushIndex < 0 || brushIndex >= Game::cm->numBrushes)
				{
					// find and remove the <out of bounds> index
					Integers.erase(std::remove(Integers.begin(), Integers.end(), brushIndex), Integers.end());

					// vector to string
					std::string vecToString = "";
					for (int num = 0; num < (int)Integers.size(); num++)
					{
						vecToString += std::to_string(Integers[num]) + " ";
					}

					Game::Dvar_SetString(vecToString.c_str(), Dvars::r_drawCollision_brushIndexFilter);
					Game::Com_PrintMessage(0, Utils::VA("^1-> r_drawCollision_brushIndexFilter ^7:: found and removed <out of bounds> index: %d \n", brushIndex), 0);

					return;
				}
			}

			// sorted / unsorted
			else
			{
				brushIndex = BRUSH_INDEX;
			}

			if (filter_BrushSorting)
			{
				brush = g_mapBrushList[brushIndex];
			} 
			else 
			{
				brush = &Game::cm->brushes[brushIndex];
			}

			// if brush is part of a submodel, translate brushmodel bounds by the submodel origin
			if (brush->isSubmodel)
			{
				Game::cbrush_t dupe = Game::cbrush_t();
				memcpy(&dupe, brush, sizeof(Game::cbrush_t));

				Utils::vector::_VectorAdd(g_mapBrushModelList[dupe.cmSubmodelIndex].cmSubmodelOrigin, dupe.mins, dupe.mins);
				Utils::vector::_VectorAdd(g_mapBrushModelList[dupe.cmSubmodelIndex].cmSubmodelOrigin, dupe.maxs, dupe.maxs);

				brush = &dupe;
			}

			// when not exporting a map
			if (!export_inProgress)
			{
				// always cull if not exporting
				if (!CM_BrushInView(brush, frustumPlanes, frustumPlaneCount)) {
					continue;
				}

				// disable material filter when using index filter
				if (!filter_BrushIndex)
				{
					// check if its a material we selected otherwise
					if (!CM_ValidBrushMaterialSelection(brush, Dvars::r_drawCollision_material->current.integer)) {
						continue;
					}
				}
			}

			// on exporting
			else
			{
				// exclude current brush if its not part of the selection box (when using selectionMode)
				if (filter_ExportSelection && !CM_IsBrushWithinSelectionBox(brush, &export_selectionBox)) {
					continue;
				}

				// skip material check if using index filtering or selectionMode
				if (!filter_BrushIndex && !filter_ExportSelection)
				{
					// check if its a material we selected otherwise
					if (!CM_ValidBrushMaterialSelection(brush, Dvars::r_drawCollision_material->current.integer)) {
						continue;
					}
				}
			}

			// always use the brush index within clipmap->brushes to define its color
			CM_GetShowCollisionColor(colorFloat, brush->colorCounter);

			// draw the current brush
			CM_ShowSingleBrushCollision(brush, colorFloat, brushIndex);

			if (brushIndexVisible)
			{
				g_mapBrushListForIndexFiltering.push_back(brush);
			}

			lastDrawnBrushAmount++;	
		}

		// *
		// draw brush indices as 3D text (only when: unsorted brushes / index filtering)

		if (brushIndexVisible && !g_mapBrushListForIndexFiltering.empty())
		{
			// debug strings are normally handled within G_RunFrame (server running @ 20fps) so we have to skip drawing them -> ((renderfps / sv_fps)) times

			// if new loop allowed
			//if (!SvFramerateToRendertime_Counter)
			//{
			//	SvFramerateToRendertime_CurrentDelta = (1000 / Game::Globals::pmlFrameTime) / Game::Dvar_FindVar("sv_fps")->current.integer;
			//	SvFramerateToRendertime_CurrentDelta = static_cast<int>(floor(SvFramerateToRendertime_CurrentDelta));
			//}

			//// increase counter
			//SvFramerateToRendertime_Counter++;

			//// if we reached the delta, we can draw strings again
			//if (SvFramerateToRendertime_Counter >= SvFramerateToRendertime_CurrentDelta)
			//{
			//	// reset counter
			//	SvFramerateToRendertime_Counter = 0;

				// sort brushes from near to far
				CM_SortBrushListOnDistanceCamera(false, true, true);

				// should not happen
				if (g_mapBrushListForIndexFiltering.empty())
				{
					Game::Dvar_SetValue(Dvars::r_drawCollision_brushIndexVisible, false);
					Game::Com_PrintMessage(0, Utils::VA("^1CM_ShowBrushCollision L#%d ^7:: GlobalBrushList was empty. Disabled r_drawCollision_brushIndexVisible! \n", __LINE__), 0);
				}

				// maximum amount of brush indices to draw
				int debugPrintsMax = 64;

				// only draw as many indices as the map has brushes
				if (static_cast<int>(g_mapBrushListForIndexFiltering.size()) < debugPrintsMax)
				{
					debugPrintsMax = g_mapBrushListForIndexFiltering.size();
				}

				// draw strings near - far
				for (debugPrints = 0; debugPrints < debugPrintsMax; ++debugPrints)
				{
					// get distance-sorted brush
					brush = g_mapBrushListForIndexFiltering[debugPrints];

					// get midpoint of brush bounds (xyz)
					glm::vec3 printOrigin = CM_GetBrushMidpoint(brush, true);

					// draw original brush index in the middle of the collision poly
					CM_ShowBrushIndexNumbers(viewParms, brush->cmBrushIndex, printOrigin, debugPrints, debugPrintsMax);
				}
			//}
		}
		else
		{
			// if not drawing debug strings
			SvFramerateToRendertime_Counter = 0;
		}

		// *
		// Map Export

		if (export_inProgress)
		{
			const char* brush_str = Utils::VA("|- Building (%d) brushes took", export_currentBrushIndex);
			std::string timefmt = " took (%.4f)\n\n";
			timefmt = brush_str + timefmt;

			Utils::Clock_EndTimerPrintSeconds(export_time_brushGenerationStart, timefmt.c_str());

			// *
			// Create Tris

			std::vector<Game::map_patchTris_t*> singleTriangles;
			bool export_needsTriangles = false;

			// only generate triangles if exporting triangles/quads or both
			if (Dvars::mapexport_writeQuads->current.enabled || Dvars::mapexport_writeTriangles->current.enabled)
			{
				export_needsTriangles = true;

				auto export_time_createTrisStart = Utils::Clock_StartTimerPrint("[MAP-EXPORT]: Building triangles ...\n");

				for (auto triNum = 0; triNum < static_cast<int>(Game::cm->triCount); triNum++)
				{
					// get incides that define the triangle
					unsigned short triIncides1[3] =
					{
						Game::cm->triIndices[triNum * 3 + 0],
						Game::cm->triIndices[triNum * 3 + 1],
						Game::cm->triIndices[triNum * 3 + 2]
					};

					Game::map_patchTris_t *pTris = Alloc_PatchTriangle();
					pTris->triIndex = triNum;

					PatchTriangle_FromIncides(pTris, triIncides1);

					if (filter_ExportSelection)
					{
						if (CM_IsTriangleWithinSelectionBox(pTris, &export_selectionBox))
						{
							singleTriangles.push_back(pTris);
						}
					}
					else
					{
						singleTriangles.push_back(pTris);
					}
				}

				Utils::Clock_EndTimerPrintSeconds(export_time_createTrisStart, "|- Building triangles took (%.4f) seconds!\n");
				Game::Com_PrintMessage(0, Utils::VA("|- Initial amount of triangles = (%d)\n\n", static_cast<int>(singleTriangles.size())), 0);
			}

			// *
			// Merge Tris to Quads

			// fix me daddy

			if (Dvars::mapexport_writeQuads->current.enabled)
			{
				auto export_time_createQuadsStart = Utils::Clock_StartTimerPrint("[MAP-EXPORT]: Merging triangles to quads ...\n");

				// create a list of merged patch triangles (quads)
				std::vector<Game::map_patchQuads_t*> singleQuads;

				// reset quad epsilon
				export_quadEpsilon = 0.0f;

				// count the amount of iterations till we are no longer able to merge any triangles to quads
				int quadFromTrisLoopCount = 0;

				// triangle index offset when we are no longer able to merge triangles (eg. if triangle 1-4 and 2-3 share an edge)
				int triNumOffset = 0;

				// amount of "failed" iterations with triangle index offset included
				int mergeIterationFailCount = 0;

				// defines how many triangles we look ahead from the current triangle to check for a shared edge
				int triFowardOffset = 1;

				// try to find as many "easy" quads as possible till we no longer merge triangles
				bool easyQuadsFirst = true;

				// build single quads from triangles; iterate for as long as we have enough triangles & if we actually decreased the amount of triangles after merging
				for (quadFromTrisLoopCount = 0; static_cast<int>(singleTriangles.size()) > 1; quadFromTrisLoopCount++)
				{
					// get amount of triangles before merging
					int startTris = (int)singleTriangles.size();

					// merge triangles if they share an edge to create quads ... we will decrease the size of singlePatchTriangles, so do this for x amount of times ^
					for (auto triNum = 0 + triNumOffset; triNum < (int)singleTriangles.size(); triNum++)
					{
						// if we have atleast 2 triangles to work with
						if (triNum + triFowardOffset < (int)singleTriangles.size())
						{
							if (PatchTriangle_SharesEdge(singleTriangles[triNum], singleTriangles[triNum + triFowardOffset]))
							{
								Game::map_patchQuads_t *pQuad = Alloc_PatchQuad();
								pQuad->quadIndex = triNum; // for debugging :)

								// try to merge both triangles to create a quad
								if (PatchQuad_SingleFromTris(pQuad, singleTriangles[triNum], singleTriangles[triNum + triFowardOffset], easyQuadsFirst))
								{
									singleQuads.push_back(pQuad);

									// free triangles and erase them from singlePatchTriangles
									free(singleTriangles[triNum]);
									free(singleTriangles[triNum + triFowardOffset]);

									singleTriangles.erase(singleTriangles.begin() + triNum + triFowardOffset);
									singleTriangles.erase(singleTriangles.begin() + triNum);

									// decrease triNum by one
									if (triNum - 1 > 0)
									{
										triNum--;
									}
								}
							}
						}
					}

					// check if we merged any triangles in this iteration
					// stop trying to merge triangles if we didnt decrease the amount of triangles
					if (startTris - (int)singleTriangles.size() == 0)
					{
						// catch :: tri 1-4 / 2-3 => offset triangle index on first fail so we catch 2-3
						triNumOffset = 1;

						// still no merge? catch :: tri 1-3 / 2-4 => reset triNumOffset and add 1 to triFowardOffset 
						if (mergeIterationFailCount == 1) {
							triNumOffset = 0;
							triFowardOffset = 2;
						}

						// reset triFowardOffset after we swizzeled the triangles once
						if (mergeIterationFailCount == 2) {
							triFowardOffset = 1;
						}

						// still unable to merge? .. (following doesnt seem to help)
						if (mergeIterationFailCount == 3) {
							triFowardOffset = 3;
						}

						if (mergeIterationFailCount == 4) {
							triFowardOffset = 4;
						}

						if (mergeIterationFailCount == 5) {
							triFowardOffset = 1;
						}

						// start increasing the bounding box that has to encapsule the 4th coordinate
						if (mergeIterationFailCount >= 6 && mergeIterationFailCount <= 13) {
							export_quadEpsilon += 5.0f;
						}

						// now try to merge skewed quads (might still be valid ones)
						if (mergeIterationFailCount == 14) {
							easyQuadsFirst = false;
						}

						mergeIterationFailCount++;

						if (mergeIterationFailCount > 16) {
							break;
						}
					}
				}

				Utils::Clock_EndTimerPrintSeconds(export_time_createQuadsStart, "|- Merging triangles to quads took (%.4f) seconds!\n");
				Game::Com_PrintMessage(0, Utils::VA("|- (%d) triangles couldn't be be merged.\n", (int)singleTriangles.size()), 0);
				Game::Com_PrintMessage(0, Utils::VA("|- (%d) quads after (%d) iterations.\n", (int)singleQuads.size(), quadFromTrisLoopCount), 0);

				// *
				// Write Quad Patches

				// coord x y z .... t .... (tc.x * 1024) (tc.y * 1024) (st.x * 1024 + 2)  -(st.y * 1024 + 2) 
				// v -108 -102 128 t -1728 5012.7217 -6.75 6.375

				for (auto quadNum = 0; quadNum < (int)singleQuads.size(); quadNum++)
				{
					// start patch
					export_mapFile << Utils::VA("// brush %d", export_currentBrushIndex) << std::endl;

					// global brush exporting index count
					export_currentBrushIndex++;

					export_mapFile << " {" << std::endl;
					export_mapFile << "  mesh" << std::endl;
					export_mapFile << "  {" << std::endl;
					export_mapFile << "   layer \"000_Global/SingleQuads\"" << std::endl;
					export_mapFile << "   toolFlags splitGeo;" << std::endl;
					export_mapFile << "   caulk" << std::endl;
					export_mapFile << "   lightmap_gray" << std::endl;
					export_mapFile << "   2 2 16 8" << std::endl;

					export_mapFile << "   (" << std::endl;
					export_mapFile << Utils::VA("    v %.1f %.1f %.1f t 0 0 0", singleQuads[quadNum]->coords[0][0], singleQuads[quadNum]->coords[0][1], singleQuads[quadNum]->coords[0][2]) << std::endl;
					export_mapFile << Utils::VA("    v %.1f %.1f %.1f t 0 0 0", singleQuads[quadNum]->coords[1][0], singleQuads[quadNum]->coords[1][1], singleQuads[quadNum]->coords[1][2]) << std::endl;
					export_mapFile << "   )" << std::endl;
					export_mapFile << "   (" << std::endl;
					export_mapFile << Utils::VA("    v %.1f %.1f %.1f t 0 0 0", singleQuads[quadNum]->coords[2][0], singleQuads[quadNum]->coords[2][1], singleQuads[quadNum]->coords[2][2]) << std::endl;
					export_mapFile << Utils::VA("    v %.1f %.1f %.1f t 0 0 0", singleQuads[quadNum]->coords[3][0], singleQuads[quadNum]->coords[3][1], singleQuads[quadNum]->coords[3][2]) << std::endl;
					export_mapFile << "   )" << std::endl;
					export_mapFile << "  }" << std::endl;
					export_mapFile << " }" << std::endl;
				}

				// free quads
				for (auto quad = 0; quad < (int)singleQuads.size(); quad++)
				{
					free(singleQuads[quad]);
				}

				Game::Com_PrintMessage(0, "|- Wrote quads to layer \"000_Global/SingleQuads\"\n", 0);
			}

			// *
			// Write Single Triangles

			if (Dvars::mapexport_writeTriangles->current.enabled)
			{
				for (auto tri = 0; tri < (int)singleTriangles.size(); tri++)
				{
					// start patch
					export_mapFile << Utils::VA("// brush %d", export_currentBrushIndex) << std::endl;

					// global brush exporting index count
					export_currentBrushIndex++;

					export_mapFile << " {" << std::endl;
					export_mapFile << "  mesh" << std::endl;
					export_mapFile << "  {" << std::endl;
					export_mapFile << "   layer \"000_Global/Triangles\"" << std::endl;
					export_mapFile << "   contents nonColliding;" << std::endl;
					export_mapFile << "   toolFlags splitGeo;" << std::endl;
					export_mapFile << "   caulk" << std::endl;
					export_mapFile << "   lightmap_gray" << std::endl;
					export_mapFile << "   2 2 16 8" << std::endl;

					export_mapFile << "   (" << std::endl;
					export_mapFile << Utils::VA("    v %.1f %.1f %.1f t 0 0 0", singleTriangles[tri]->coords[2].xyz[0], singleTriangles[tri]->coords[2].xyz[1], singleTriangles[tri]->coords[2].xyz[2]) << std::endl;
					export_mapFile << Utils::VA("    v %.1f %.1f %.1f t 0 0 0", singleTriangles[tri]->coords[0].xyz[0], singleTriangles[tri]->coords[0].xyz[1], singleTriangles[tri]->coords[0].xyz[2]) << std::endl;
					export_mapFile << "   )" << std::endl;
					export_mapFile << "   (" << std::endl;
					export_mapFile << Utils::VA("    v %.1f %.1f %.1f t 0 0 0", singleTriangles[tri]->coords[1].xyz[0], singleTriangles[tri]->coords[1].xyz[1], singleTriangles[tri]->coords[1].xyz[2]) << std::endl;
					export_mapFile << Utils::VA("    v %.1f %.1f %.1f t 0 0 0", singleTriangles[tri]->coords[1].xyz[0], singleTriangles[tri]->coords[1].xyz[1], singleTriangles[tri]->coords[1].xyz[2]) << std::endl;
					export_mapFile << "   )" << std::endl;
					export_mapFile << "  }" << std::endl;
					export_mapFile << " }" << std::endl;
				}

				Game::Com_PrintMessage(0, "|- Wrote triangles to layer \"000_Global/Triangles\"\n\n", 0);
			}

			// free triangles
			if (export_needsTriangles)
			{
				for (auto tri = 0; tri < (int)singleTriangles.size(); tri++)
				{
					free(singleTriangles[tri]);
				}
			}

			// close the worldspawn entity with all its brushes
			export_mapFile << "}" << std::endl;
			Game::Com_PrintMessage(0, "|- Finished writing the world entity\n\n", 0);

			// *
			// Map Entities + Submodels/Brushmodels + Reflection Probes

			if (Dvars::mapexport_writeEntities->current.enabled)
			{
				Game::Com_PrintMessage(0, "[MAP-EXPORT]: Building entities ...\n", 0);

				// already exported the worldspawn entity, so delete it from the list
				export_mapEntities.deleteWorldspawn();

				// *
				// map entities and brushmodels

				if (filter_ExportSelection) // only exporting entities within the selection box
				{
					export_mapFile << export_mapEntities.buildSelection_FixBrushmodels(&export_selectionBox, g_mapBrushModelList);
				}
				else // build all other entities and fix up brushmodels
				{
					export_mapFile << export_mapEntities.buildAll_FixBrushmodels(g_mapBrushModelList);
				}

				// *
				// reflection probes (always skip the first one (not defined within the map file))

				int exportedProbes = 0;

				for (auto probe = 1; probe < (int)Game::_gfxWorld->reflectionProbeCount; probe++)
				{
					if (filter_ExportSelection && 
						!Utils::polylib::PointWithinBounds(glm::vec3(Game::_gfxWorld->reflectionProbes[probe].origin[0],
																	 Game::_gfxWorld->reflectionProbes[probe].origin[1],
																	 Game::_gfxWorld->reflectionProbes[probe].origin[2]),
																	 export_selectionBox.mins, export_selectionBox.maxs, 0.25f))
					{
						// skip probe if not in selection box
						continue;
					}
					else
					{
						export_mapFile << Utils::VA("// reflection probe %d", exportedProbes) << std::endl;
						export_mapFile << "{" << std::endl;
						export_mapFile << "\"angles\" \"0 0 0\"" << std::endl;
						export_mapFile << Utils::VA("\"origin\" \"%.1f %.1f %.1f\"", Game::_gfxWorld->reflectionProbes[probe].origin[0], Game::_gfxWorld->reflectionProbes[probe].origin[1], Game::_gfxWorld->reflectionProbes[probe].origin[2]) << std::endl;
						export_mapFile << "\"classname\" \"reflection_probe\"" << std::endl;
						export_mapFile << "}" << std::endl;

						exportedProbes++;
					}
				}

				Game::Com_PrintMessage(0, Utils::VA("|- (%d) reflection probes.\n", exportedProbes), 0);
			}

			// *
			// Static Models

			if (Dvars::mapexport_writeModels->current.enabled)
			{
				auto map_mapModelsStart = Utils::Clock_StartTimer();
				int exportedSModels = 0;

				for (auto sModel = 0u; sModel < Game::_gfxWorld->dpvs.smodelCount; sModel++)
				{
					// only export static models within the selection box
					if (filter_ExportSelection &&
						!Utils::polylib::PointWithinBounds(glm::vec3(Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.origin[0],
																	 Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.origin[1],
																	 Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.origin[2]),
																	 export_selectionBox.mins, export_selectionBox.maxs, 0.25f)) 
					{
						// skip static model if not in selection box
						continue;
					}
					else
					{
						// copy model rotation axis
						Game::vec4_t matrix[4];

						// X
						matrix[0][0] = Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.axis[0][0];
						matrix[0][1] = Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.axis[0][1];
						matrix[0][2] = Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.axis[0][2];

						// Y
						matrix[1][0] = Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.axis[1][0];
						matrix[1][1] = Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.axis[1][1];
						matrix[1][2] = Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.axis[1][2];

						// Z
						matrix[2][0] = Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.axis[2][0];
						matrix[2][1] = Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.axis[2][1];
						matrix[2][2] = Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.axis[2][2];

						// calculate model angles
						Game::vec3_t angles;
						Utils::vector::_ToEulerAnglesDegrees(matrix, angles);

						export_mapFile << Utils::VA("// static model %d\n{", exportedSModels) << std::endl;
						export_mapFile << "layer \"000_Global/Models\"" << std::endl;
						export_mapFile << Utils::VA("\"modelscale\" \"%.1f\"", Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.scale) << std::endl;
						export_mapFile << Utils::VA("\"origin\" \"%.1f %.1f %.1f\"", Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.origin[0], Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.origin[1], Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].placement.origin[2]) << std::endl;
						export_mapFile << Utils::VA("\"angles\" \"%.1f %.1f %.1f\"", angles[1], angles[2], angles[0]) << std::endl;
						export_mapFile << Utils::VA("\"model\" \"%s\"", Game::_gfxWorld->dpvs.smodelDrawInsts[sModel].model->name) << std::endl;
						export_mapFile << "\"classname\" \"misc_model\"" << std::endl;
						export_mapFile << "}" << std::endl;
					}
				}

				Utils::Clock_EndTimerPrintSeconds(map_mapModelsStart, "|- Building static models took (%.4f) seconds!\n\n");
			}

			// *
			// Map Export End

			export_mapFile.close();
			export_currentBrushIndex = 0;

			Utils::Clock_EndTimerPrintSeconds(export_time_exportStart, ">> DONE! Map export took (%.4f) seconds!\n");
			Game::Com_PrintMessage(0, "------------------------------------------------------\n\n", 0);
		}

		// ------------

		// update hud elements after we drew all brushes / planes
		if (Game::Globals::dbgColl_drawnPlanesAmount != Game::Globals::dbgColl_drawnPlanesAmountTemp) {
			Game::Globals::dbgColl_drawnPlanesAmount = Game::Globals::dbgColl_drawnPlanesAmountTemp;
		}

		if (Game::Globals::dbgColl_drawnBrushAmount != lastDrawnBrushAmount) {
			Game::Globals::dbgColl_drawnBrushAmount = lastDrawnBrushAmount;
		}
	}

	// *
	// _Debug::RB_AdditionalDebug :: entry for collision drawing (create view frustum)
	void RB_DrawCollision::RB_ShowCollision(Game::GfxViewParms *viewParms)
	{
		char frustumType;
		float drawDistance;

		Game::cplane_s frustumPlanes[6];

		if (!viewParms) 
		{
			Game::Com_Error(0, Utils::VA("RB_ShowCollision L#%d :: viewparams\n", __LINE__));
			return;
		}

		// enable drawcollision on mapexport_selectionMode
		if (Dvars::mapexport_selectionMode && Dvars::mapexport_selectionMode->current.integer != 0 && Dvars::r_drawCollision && Dvars::r_drawCollision->current.integer == 0)
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, "r_drawcollision 3\n");
		}

		if (Dvars::r_drawCollision && Dvars::r_drawCollision->current.integer > 0)
		{
			// turn off brush sorting if displaying brushIndices
			if (Dvars::r_drawCollision_brushIndexVisible->current.enabled && Dvars::r_drawCollision_brushSorting->current.integer != 0)
			{
				Game::Dvar_SetValue(Dvars::r_drawCollision_brushSorting, 0);
				Game::Com_PrintMessage(0, Utils::VA("^1-> r_drawCollision_brushSorting ^7:: disabled due to r_drawCollision_brushIndexVisible \n"), 0);
			}

			// Disable r_drawCollision when using r_fullbright
			if (Game::Dvar_FindVar("r_fullbright")->current.enabled) 
			{
				Game::Com_PrintMessage(0, Utils::VA("^1-> r_drawCollision ^7:: disabled due to r_fullbright \n"), 0);
				Game::Dvar_SetValue(Dvars::r_drawCollision, 0);
				
				return;
			}

			// Disable r_drawCollision when using r_debugShader
			if (Game::Dvar_FindVar("r_debugShader")->current.integer > 0)
			{
				Game::Com_PrintMessage(0, Utils::VA("^1-> r_drawCollision ^7:: disabled due to r_debugShader \n"), 0);
				Game::Dvar_SetValue(Dvars::r_drawCollision, 0);
				
				return;
			}

			// increment our frame counter if we use brush flickering mode
			if (Dvars::r_drawCollision_flickerBrushes->current.enabled)
			{
				collisionFlickerCounter++;
			}

			// *
			// Build culling frustum

			BuildFrustumPlanes(viewParms, frustumPlanes);

			frustumPlanes[5].normal[0] = -frustumPlanes[4].normal[0];
			frustumPlanes[5].normal[1] = -frustumPlanes[4].normal[1];
			frustumPlanes[5].normal[2] = -frustumPlanes[4].normal[2];

			// max draw distance when brushDist is set to 0
			drawDistance = Dvars::r_drawCollision_brushDist->current.value;

			if (drawDistance == 0) 
			{
				drawDistance = 999999.0f;
			}

			frustumPlanes[5].dist = -frustumPlanes[4].dist - drawDistance;

			if (frustumPlanes[5].normal[0] == 1.0f)
			{
				frustumType = 0;
			}

			else
			{
				if (frustumPlanes[5].normal[1] == 1.0)
				{
					frustumType = 1;
				}

				else
				{
					frustumType = 2;
					if (frustumPlanes[5].normal[2] != 1.0)
					{
						frustumType = 3;
					}
				}
			}

			frustumPlanes[5].type = frustumType;
			SetPlaneSignbits(&frustumPlanes[5]);

			// *
			// Main brush drawing logic

			CM_ShowBrushCollision(viewParms, frustumPlanes, 6);

			// draw added polys / lines
			if (*Game::tessSurface) 
			{
				Game::RB_EndTessSurface();
			}
		}
	}

	RB_DrawCollision::RB_DrawCollision()
	{
		// -----
		// Dvars

		Dvars::r_drawCollision = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision",
			/* desc		*/ "Enable collision drawing.\n0: Off\n1: Outlines\n2: Polys\n3: Both",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 3,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::r_drawCollision_brushAmount = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision_brushAmount",
			/* desc		*/ "Draw x amount of brushes, starting at brush index 0 and will limit itself to the total amount of brushes within the clipMap.\n0: disables this filter.",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ INT_MAX / 2 - 1,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::r_drawCollision_brushDist = Game::Dvar_RegisterFloat(
			/* name		*/ "r_drawCollision_brushDist",
			/* desc		*/ "Max distance to draw collision.\n0: disables this filter.\nWill reset itself on map load.",
			/* default	*/ 800.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 10000.0f,
			/* flags	*/ Game::dvar_flags::none);

		// r_drawCollision_brushIndexFilter @ (CM_BuildMaterialListForMapOnce)

		Dvars::r_drawCollision_brushIndexVisible = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_brushIndexVisible",
			/* desc		*/ "Draw brush index numbers for use with ^1r_drawCollision_brushFilter^7. Unstable fps will cause flickering.",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::r_drawCollision_brushSorting = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision_brushSorting",
			/* desc		*/ "Sort brushes based on distance from the camera.\n0: Off\n1: Far to near\n2: Near to far",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 2,
			/* flags	*/ Game::dvar_flags::saved);

#if DEBUG
		Dvars::r_drawCollision_brushDebug = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_brushDebug",
			/* desc		*/ "Draw debug prints. Only enabled when r_drawCollision_brushAmount = 1",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);
#endif

		Dvars::r_drawCollision_lineWidth = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision_lineWidth",
			/* desc		*/ "Width of debug lines. (Only if using r_drawCollision 1)",
			/* default	*/ 1,
			/* minVal	*/ 0,
			/* maxVal	*/ 12,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::r_drawCollision_lineColor = Game::Dvar_RegisterVec4(
			/* name		*/ "r_drawCollision_lineColor",
			/* desc		*/ "Color of debug lines.",
			/* x		*/ 0.2f,
			/* y		*/ 1.0f,
			/* z		*/ 0.2f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::r_drawCollision_polyAlpha = Game::Dvar_RegisterFloat(
			/* name		*/ "r_drawCollision_polyAlpha",
			/* desc		*/ "Transparency of polygons.",
			/* default	*/ 0.8f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::r_drawCollision_polyDepth = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_polyDepth",
			/* desc		*/ "Enable depth test for polygons.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::r_drawCollision_polyFace = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_polyFace",
			/* desc		*/ "0: Back(only draw the front facing side)\n1: None(draw both sides)",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::r_drawCollision_polyLit = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_polyLit",
			/* desc		*/ "Enable fake lighting for polygons.",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::r_drawCollision_material = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision_material",
			/* desc		*/ "Will be populated when a map is loaded and r_drawCollision is enabled.",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 1,
			/* flags	*/ Game::dvar_flags::none);

		static std::vector <char*> r_drawCollisionMatFilter = 
		{ 
			"none", 
			"clip", 
			"mantle", 
			"trigger", 
			"all", 
			"all-no-tools", 
			"all-no-tools-clip", 
		};

		Dvars::r_drawCollision_materialInclude = Game::Dvar_RegisterEnum(
			/* name		*/ "r_drawCollision_materialInclude",
			/* desc		*/ "Filter by type. \nExample: <clip> will show \"clip_player\" / \"clip_metal\" etc. \n<all-no-tools> draws everything but portal, trigger, hint, mantle, sky and volumes materials.\n<all-no-tools-clip> draws everything but clip and previous mentioned materials.",
			/* default	*/ 0,
			/* enumSize	*/ r_drawCollisionMatFilter.size(),
			/* enumData */ r_drawCollisionMatFilter.data(),
			/* flags	*/ Game::dvar_flags::none);

		Dvars::r_drawCollision_materialList = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_materialList",
			/* desc		*/ "CMD: Prints a list of materials in use by the current map (to the console).\nOnly works when a map is loaded and r_drawCollision is enabled!",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::r_drawCollision_flickerBrushes = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_flickerBrushes",
			/* desc		*/ "[VIS] Enable debug collision flicker mode. Remove me.",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::r_drawCollision_flickerOnTime = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision_flickerOnTime",
			/* desc		*/ "[VIS] Amount of frames to show brush collision.",
			/* default	*/ 60,
			/* minVal	*/ 0,
			/* maxVal	*/ INT_MAX / 2 - 1,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::r_drawCollision_flickerOffTime = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision_flickerOffTime",
			/* desc		*/ "[VIS] Amount of frames to hide brush collision.",
			/* default	*/ 500,
			/* minVal	*/ 0,
			/* maxVal	*/ INT_MAX/2 - 1,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::r_drawCollision_hud = Game::Dvar_RegisterBool(
			/* name		*/ "r_drawCollision_hud",
			/* desc		*/ "Display debug hud.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::r_drawCollision_hud_position = Game::Dvar_RegisterVec2(
			/* name		*/ "r_drawCollision_hud_position",
			/* desc		*/ "hud position offset",
			/* def x	*/ 10.0f,
			/* def y	*/ 250.0f,
			/* minVal	*/ -1000.0f,
			/* maxVal	*/ 1000.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::r_drawCollision_hud_fontScale = Game::Dvar_RegisterFloat(
			/* name		*/ "r_drawCollision_hud_fontScale",
			/* desc		*/ "font scale",
			/* default	*/ 0.75f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 100.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::r_drawCollision_hud_fontStyle = Game::Dvar_RegisterInt(
			/* name		*/ "r_drawCollision_hud_fontStyle",
			/* desc		*/ "font style",
			/* default	*/ 1,
			/* minVal	*/ 0,
			/* maxVal	*/ 8,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::r_drawCollision_hud_fontColor = Game::Dvar_RegisterVec4(
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

		Dvars::mapexport_brushEpsilon1 = Game::Dvar_RegisterFloat(
			/* name		*/ "mapexport_brushEpsilon1",
			/* desc		*/ "brushside epsilon 1 (debug)",
			/* default	*/ 0.4f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mapexport_brushEpsilon2 = Game::Dvar_RegisterFloat(
			/* name		*/ "mapexport_brushEpsilon2",
			/* desc		*/ "brushside epsilon 2 (debug)",
			/* default	*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mapexport_brushMinSize = Game::Dvar_RegisterFloat(
			/* name		*/ "mapexport_brushMinSize",
			/* desc		*/ "only export brushes (with more then 6 sides) if their diagonal length is greater then <this>",
			/* default	*/ 64.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1000.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mapexport_brush5Sides = Game::Dvar_RegisterBool(
			/* name		*/ "mapexport_brush5Sides",
			/* desc		*/ "enable exp. export of brushes with only 5 sides",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mapexport_selectionMode = Game::Dvar_RegisterInt(
			/* name		*/ "mapexport_selectionMode",
			/* desc		*/ "Only export selected things. Use \"mapexport_selectionAdd\" and \"mapexport_selectionClear\" \n0: Off\n1: Bounding box (needs 2 defined points)",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 1,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::mapexport_writeTriangles = Game::Dvar_RegisterBool(
			/* name		*/ "mapexport_writeTriangles",
			/* desc		*/ "[MAP-EXPORT-OPTION] Export leftover unmerged triangles if enabled.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mapexport_writeQuads = Game::Dvar_RegisterBool(
			/* name		*/ "mapexport_writeQuads",
			/* desc		*/ "[MAP-EXPORT-OPTION] Export resulting quads after triangle merging if enabled.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mapexport_writeEntities = Game::Dvar_RegisterBool(
			/* name		*/ "mapexport_writeEntities",
			/* desc		*/ "[MAP-EXPORT-OPTION] Export map entities if enabled (no brushmodel support).",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::mapexport_writeModels = Game::Dvar_RegisterBool(
			/* name		*/ "mapexport_writeModels",
			/* desc		*/ "[MAP-EXPORT-OPTION] Export all static models if enabled.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		// --------
		// Commands

		Command::Add("mapexport", [](Command::Params)
		{
			if (Dvars::r_drawCollision && Dvars::r_drawCollision->current.integer == 0)
			{
				Game::Com_PrintMessage(0, "Please enable \"r_drawCollision\" and re-run your command.\n", 0);
				return;
			}

			export_mapExportCmd = true;

			Game::Cmd_ExecuteSingleCommand(0, 0, "pm_hud_enable 0\n");
			Game::Cmd_ExecuteSingleCommand(0, 0, "say \"Export Done!\"\n");
		});

		Command::Add("mapexport_selectionAdd", [](Command::Params)
		{
			if (Dvars::mapexport_selectionMode && Dvars::mapexport_selectionMode->current.integer != 0)
			{
				export_selectionAdd = true;
			}
			else
			{
				Game::Com_PrintMessage(0, "mapexport_selectionMode needs to be enabled for this to work.\n", 0);
			}
		});

		Command::Add("mapexport_selectionClear", [](Command::Params)
		{
			if (Dvars::mapexport_selectionMode && Dvars::mapexport_selectionMode->current.integer != 0)
			{
				CM_ResetSelectionBox(&export_selectionBox);
			}
			else
			{
				Game::Com_PrintMessage(0, "mapexport_selectionMode needs to be enabled for this to work.\n", 0);
			}
		});
	}

	RB_DrawCollision::~RB_DrawCollision()
	{ }
}