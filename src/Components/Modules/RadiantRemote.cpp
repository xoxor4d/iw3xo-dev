/* Thanks to Nukem for some of the base functions used here. Check out his linker-mod here: 
https://github.com/Nukem9/LinkerMod/blob/development/components/radiant_mod/remote_net.cpp */

#include "STDInclude.hpp"

#define DYN_COLL_BMODEL_AMOUNT 16

namespace Components
{
	Game::radiantBrush_t RadiantDB_BrushSortingContainer[RADIANT_MAX_SEL_BRUSHES];

	// ------------------------------------------------------------------
	// Radiant debug brushes

	void RadiantRemote::CM_FindDynamicBrushModels()
	{
		//save ent->targetname and compare to children
		//ent->target = 0
		//ent->r.bmodel = 1
		//ent->r.currentOrigin = 0x012940ac {994.000000, 1008.00000, 992.000000}

		bool ent_found_bbModel = false;
		Game::Globals::dynBrushModels.mapped_bmodels = 0; // re-map already mapped brushmodels

		// base brushmodel origin
		//float ent_bbModelOrigin[3] =
		//{ 992.0f, 1008.0f, 992.0f };

		// if map is loaded
		if (Game::cm->name && Game::cm->isInUse)
		{
			Game::Com_PrintMessage(0, "CM_FindDynamicBrushModels :: start ...\n", 0);

			// search the base brush model
			for (auto ent = 0; ent < 1024; ent++)
			{
				if (!Game::scr_g_entities[ent].r.bmodel)
				{
					continue;
				}

				// compare brushmodel targetname
				if (Utils::Q_stricmp(_Game::gEnt_GetGScrString(Game::scr_g_entities[ent].targetname), "dynbrush01"))
				{
					continue;
				}

				/*if (!Utils::vector::_VectorCompare(Game::scr_g_entities[ent].r.currentOrigin, ent_bbModelOrigin))
					continue;*/

				// found it
				ent_found_bbModel = true;
				Game::Globals::dynBrushModels.brushes[0].entityIndex = ent;
				Game::Globals::dynBrushModels.brushes[0].ent = &Game::scr_g_entities[ent];

				// save the original origin
				glm::setFloat3(Game::Globals::dynBrushModels.brushes[0].originalOrigin, glm::toVec3(Game::Globals::dynBrushModels.brushes[0].ent->r.currentOrigin));

				Game::Com_PrintMessage(0, Utils::VA("|-> found base-brushmodel @ g_entities[%d]\n", ent), 0);
				Game::Globals::dynBrushModels.mapped_bmodels++;

				break;
			}

			// could not find the base model
			if (!ent_found_bbModel)
			{
				Game::Com_PrintMessage(0, "|-> couldnt find the base-brushmodel!\n", 0);
				return;
			}

			std::vector<int> mapped_children;

			// look for its children
			for (auto child = 1; child < DYN_COLL_BMODEL_AMOUNT; child++)
			{
				for (auto ent = 0; ent < 1024; ent++)
				{
					// if we already mapped the current entity
					if (std::find(mapped_children.begin(), mapped_children.end(), ent) != mapped_children.end())
					{
						continue;
					}
						
					// if entity is not a brush model
					if (!Game::scr_g_entities[ent].r.bmodel)
					{
						continue;
					}

					//if(Utils::Q_stricmp(_Game::gEnt_GetGScrString(Game::scr_g_entities[ent].targetname), "dynbrush01"))
					// if brushmodel is not a child of our base brush (no need to get the target / targetname)
					if (Game::scr_g_entities[ent].target != Game::Globals::dynBrushModels.brushes[0].ent->targetname)
					{
						continue;
					}	

					// found a child
					Game::Globals::dynBrushModels.brushes[child].entityIndex = ent;
					Game::Globals::dynBrushModels.brushes[child].ent = &Game::scr_g_entities[ent];

					// save the original origin
					glm::setFloat3(Game::Globals::dynBrushModels.brushes[child].originalOrigin, glm::toVec3(Game::Globals::dynBrushModels.brushes[child].ent->r.currentOrigin));

					mapped_children.push_back(ent);

					Game::Com_PrintMessage(0, Utils::VA("|-> found child @ g_entities[%d]\n", ent), 0);
					Game::Globals::dynBrushModels.mapped_bmodels++;

					break;
				}
			}

			Game::Com_PrintMessage(0, Utils::VA("|-> found %d/%d brushmodel entities!\n", mapped_children.size() + 1, DYN_COLL_BMODEL_AMOUNT), 0);

			std::vector<int> mapped_cmodels;

			// *
			// find the corrosponding cmodels in the clipmap
			for (auto bModel = 0; bModel < Game::Globals::dynBrushModels.mapped_bmodels; bModel++)
			{
				// should not happen
				if (bModel > (int)Game::cm->numSubModels)
				{
					Game::Com_PrintMessage(0, "|-> ^1mapped brushmodels > clipMap->numSubModels[%d]\n", 0);
					return;
				}

				// for each clipMap->cmodel
				for (auto cmod = 0; cmod < (int)Game::cm->numSubModels; cmod++)
				{
					// if we already mapped the current cmodel
					if (std::find(mapped_cmodels.begin(), mapped_cmodels.end(), cmod) != mapped_cmodels.end())
					{
						continue;
					}

					// compare mins
					if (!Utils::vector::_VectorCompare(Game::cm->cmodels[cmod].mins, Game::Globals::dynBrushModels.brushes[bModel].ent->r.mins))
					{
						continue;
					}	

					// compare maxs
					if (!Utils::vector::_VectorCompare(Game::cm->cmodels[cmod].maxs, Game::Globals::dynBrushModels.brushes[bModel].ent->r.maxs))
					{
						continue;
					}	

					// found it
					Game::Globals::dynBrushModels.brushes[bModel].cmodelIndex = cmod;
					Game::Globals::dynBrushModels.brushes[bModel].cmodel = &Game::cm->cmodels[cmod];

					mapped_cmodels.push_back(cmod);

					Game::Com_PrintMessage(0, Utils::VA("|-> dynBrushModels.brushes[%d] :: cm->cmodel[%d] == g_entities[%d]\n", bModel, cmod, Game::Globals::dynBrushModels.brushes[bModel].entityIndex), 0);
					
					break;
				}
			}

			Game::Com_PrintMessage(0, Utils::VA("|-> found %d/%d cmodels!\n", mapped_cmodels.size(), DYN_COLL_BMODEL_AMOUNT), 0);
		}

		else
		{
			Game::Com_PrintMessage(0, "CM_FindDynamicBrushModels :: No map loaded!\n", 0);
		}
	}

	void DynBrushModel_Modify(Game::dynBrushModel_t* bModel, glm::vec3& origin, glm::vec3& localMins, glm::vec3& localMaxs, glm::vec3& angles)
	{
		// Moving a brushmodel needs:
		// > trBase + origin (absmins / maxs calculated by SV_LinkEntity)
		// brushmodels are utterly fucked in this engine. Rotating them fucks up the collision. Happens with gsc too.

		// Entity Server :: origin (for visibility)
		_Game::gEnt_MoveToInstant(bModel->ent, origin);

		// angles => not using entity rotation anymore (we chop the brush by its sides)
		//_Game::gEnt_RotateToInstant(bModel->ent, angles);

		// Entity Remote :: local bounds
		_Game::gEnt_BrushModelBounds(bModel, localMins, localMaxs);

		// ~ relink this entity, because it may have moved out of the current leave
		// Update the clipmap leafs and absolute world bounds
		Game::SV_LinkEntity(bModel->ent);
	}

	// *
	// Remove saved brushes and sort debug brush array (keeping the current non-saved selection)
	bool RadiantDB_RemoveSaved()
	{
		//auto RadiantDB_temp = Game::savedRadiantBrushes();
		//memcpy(&RadiantDB_temp, &Game::Globals::rad_savedBrushes, sizeof(Game::savedRadiantBrushes));
		memcpy(&RadiantDB_BrushSortingContainer, &Game::Globals::rad_savedBrushes.brush, sizeof(Game::radiantBrush_t) * RADIANT_MAX_SEL_BRUSHES);

		int currSelected_count = Game::Globals::rad_savedBrushes.brushSelectedCount;
		int nonSavedSelected_count = 0;

		for (auto b = 0; b < RADIANT_MAX_SEL_BRUSHES; b++)
		{
			if (currSelected_count == nonSavedSelected_count)
			{
				// found all non-saved selected brushes
				break;
			}

			if (!Game::Globals::rad_savedBrushes.brush[b].isSaved && Game::Globals::rad_savedBrushes.brush[b].faceCount)
			{
				memcpy(&RadiantDB_BrushSortingContainer[nonSavedSelected_count], &Game::Globals::rad_savedBrushes.brush[b], sizeof(Game::radiantBrush_t));
				nonSavedSelected_count++;
			}
		}

		if (currSelected_count != nonSavedSelected_count)
		{
			Game::Globals::rad_savedBrushes.usingSaved = false;
			Game::Globals::rad_savedBrushes.brushSavedCount = 0;

			return false;
		}

		// copy new brushes into the struct
		memcpy(&Game::Globals::rad_savedBrushes.brush[0], &RadiantDB_BrushSortingContainer[0], sizeof(Game::radiantBrush_t) * nonSavedSelected_count);

		Game::Globals::rad_savedBrushes.usingSaved = false;
		Game::Globals::rad_savedBrushes.brushSavedCount = 0;

		return true;
	}

	// *
	// meme: shader_names hashing func
	void RadiantDB_GenerateHash(int brushIdx)
	{
		uint32_t eax, ebx, ecx, val;
		ebx = 0; //ebx ^ ebx;

		for (auto f = 0; f < Game::Globals::rad_savedBrushes.brush[brushIdx].faceCount; f++)
		{
			val = (int)Game::Globals::rad_savedBrushes.brush[brushIdx].face[f].windingPts[0][0] + (int)Game::Globals::rad_savedBrushes.brush[brushIdx].face[f].windingPts[0][1] + (int)Game::Globals::rad_savedBrushes.brush[brushIdx].face[f].windingPts[0][2];
			ecx = val;
			eax = ebx;
			eax = eax << 5;
			ebx = ebx + eax;
			ebx = ebx ^ ecx;
		}

		Game::Globals::rad_savedBrushes.brush[brushIdx].hash = ebx;
	}

	// *
	// compare hash of rad_savedBrushes.brush[brushIdx] against all saved brushes
	// returns true if brush is a saved brush
	bool RadiantDB_IsSelectionSaved(int brushIdx)
	{
		for (auto bHash = 0; bHash < Game::Globals::rad_savedBrushes.brushSavedCount; bHash++)
		{
			if (Game::Globals::rad_savedBrushes.brush[brushIdx].hash 
				&& Game::Globals::rad_savedBrushes.brush[brushIdx].hash == Game::Globals::rad_savedBrushes.brush[bHash].hash)
			{
				return true;
			}
		}

		return false;
	}

	// *
	// Draw brushes sent from radiant
	void RadiantRemote::RadiantDebugBrush()
	{
		// reset brush collision on first iteration after resetCollision was set
		if (Game::Globals::rad_savedBrushes.resetCollision)
		{
			if (Game::Globals::dynBrushModels.initiated && !Game::Globals::rad_savedBrushes.disabledCollision)
			{
				for (auto dynB = 0; dynB < Game::Globals::dynBrushModels.mapped_bmodels; dynB++)
				{
					_Game::gEnt_BrushModelSetCollision(Game::Globals::dynBrushModels.brushes[dynB].ent, false);
					_Game::gEnt_MoveToInstant(Game::Globals::dynBrushModels.brushes[dynB].ent, glm::toVec3(Game::Globals::dynBrushModels.brushes[dynB].originalOrigin));
				}

				Game::Globals::rad_savedBrushes.disabledCollision = true;
				Game::Globals::rad_savedBrushes.wasModified = true;
				Game::Globals::rad_savedBrushes.resetCollision = false;
			}
		}

		// build 1 winding list for face[0 - faceCount] -> windingPts[0 - windingPtCount]
		if (Game::Globals::cgsAddons.radiantLiveConnected && (Game::Globals::rad_savedBrushes.brushSelected || Game::Globals::rad_savedBrushes.usingSaved))
		{
			for (auto b = 0; b < Game::Globals::rad_savedBrushes.brushSelectedCount + Game::Globals::rad_savedBrushes.brushSavedCount; b++)
			{
				if (b >= RADIANT_MAX_SEL_BRUSHES || Game::Globals::rad_savedBrushes.brush[b].faceCount == 0)
				{
					return;
				}

				// check for duplicate brushes (brush still selected after saving)
				if (!Game::Globals::rad_savedBrushes.brush[b].isSaved)
				{
					// only generate a new hash when selection changed
					if (Game::Globals::rad_savedBrushes.wasModified)
					{
						RadiantDB_GenerateHash(b);
					}

					if (RadiantDB_IsSelectionSaved(b))
					{
						// current non-saved selection is already saved
						continue;
					}
				}

				// create a hash for the current brush (so we can identify them)
				if (!Game::Globals::rad_savedBrushes.brush[b].hash || Game::Globals::rad_savedBrushes.wasModified)
				{
					RadiantDB_GenerateHash(b);
				}

				for (auto f = 0; f < Game::Globals::rad_savedBrushes.brush[b].faceCount; f++)
				{
					float windingPts[16][3];

					for (auto coord = 0; coord < Game::Globals::rad_savedBrushes.brush[b].face[f].windingCount; coord++)
					{
						// windings are reversed in radiant or the game
						int invcoord = coord;

						// uncomment to inverse faces
						invcoord = Game::Globals::rad_savedBrushes.brush[b].face[f].windingCount - 1 - coord;

						// create the coord
						windingPts[invcoord][0] = Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[coord][0];
						windingPts[invcoord][1] = Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[coord][1];
						windingPts[invcoord][2] = Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[coord][2];
					}

					// draw the face
					_Debug::RB_DrawPoly(Game::Globals::rad_savedBrushes.brush[b].face[f].windingCount, windingPts, (const float*)&Dvars::radiant_brushColor->current.integer, Dvars::radiant_brushLit->current.enabled, 
						Dvars::radiant_brushWireframe->current.enabled, (const float*)&Dvars::radiant_brushWireframeColor->current.integer);
				}

				// *
				// Debug brush collision

				if (!Dvars::radiant_brushCollision->current.enabled)
				{
					if (Game::Globals::dynBrushModels.initiated)
					{
						for (auto dynB = 0; dynB < Game::Globals::dynBrushModels.mapped_bmodels; dynB++)
						{
							_Game::gEnt_BrushModelSetCollision(Game::Globals::dynBrushModels.brushes[dynB].ent, false);
							_Game::gEnt_MoveToInstant(Game::Globals::dynBrushModels.brushes[dynB].ent, glm::toVec3(Game::Globals::dynBrushModels.brushes[dynB].originalOrigin));
						}

						Game::Globals::dynBrushModels.initiated = false;
					}
					
					continue; // we still want to draw all the brushes
				}

				// one time message that we failed to find the brushmodels + disable continuous searching
				if (!Game::Globals::dynBrushModels.mapped_bmodels)
				{
					if (!Game::Globals::dynBrushModels.initiated)
					{
						Game::Com_PrintMessage(0, "^1[LiveRadiant]: ^7Disabled debug-brush collision. Could not find any dynamic brushmodels on the map!^7\n|-> ^1Make sure that you included the \"dynamic_collision_bmodels.map\" prefab within your map -> RE-BSP!", 0);
						Game::Globals::dynBrushModels.initiated = true;

						continue; // we still want to draw all the brushes
					}

					continue; // we still want to draw all the brushes
					
				}

				if (b >= Game::Globals::dynBrushModels.mapped_bmodels - 1)
				{
					// if we ran out of avail. brushmodels, stop updating collision and notify the user
					if (Game::Globals::rad_savedBrushes.wasModified)
					{
						Game::Com_PrintMessage(0, "^1[LiveRadiant]: ^7Not enough dynamic brushmodels available! Using oldest selection for collision ...\n", 0);
						Game::Globals::rad_savedBrushes.wasModified = false;
					}

					return;
				}

				Game::Globals::dynBrushModels.initiated = true;

				// only update collision when a brush was modified
				if (Game::Globals::rad_savedBrushes.wasModified)
				{
					Game::Globals::rad_savedBrushes.disabledCollision = false;

					// enable collision
					_Game::gEnt_BrushModelSetCollision(Game::Globals::dynBrushModels.brushes[b].ent, true);

					glm::vec3 rad_brushMins(FLT_MAX, FLT_MAX, FLT_MAX);
					glm::vec3 rad_brushMaxs(-FLT_MAX, -FLT_MAX, -FLT_MAX);

					// get bounds from windings
					for (auto f = 0; f < Game::Globals::rad_savedBrushes.brush[b].faceCount; f++)
					{
						// *
						// CM_WorldToLocalBounds()

						// get brush mins/maxs (could be send from radiant directly but we already have the winding points so theres no point)
						for (auto winding = 0; winding < Game::Globals::rad_savedBrushes.brush[b].face[f].windingCount; winding++)
						{
							// mins :: find the closest point on each axis
							if (rad_brushMins.x > Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[winding][0])
								rad_brushMins.x = Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[winding][0];

							if (rad_brushMins.y > Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[winding][1])
								rad_brushMins.y = Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[winding][1];

							if (rad_brushMins.z > Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[winding][2])
								rad_brushMins.z = Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[winding][2];

							// maxs :: find the furthest point on each axis
							if (rad_brushMaxs.x < Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[winding][0])
								rad_brushMaxs.x = Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[winding][0];

							if (rad_brushMaxs.y < Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[winding][1])
								rad_brushMaxs.y = Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[winding][1];

							if (rad_brushMaxs.z < Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[winding][2])
								rad_brushMaxs.z = Game::Globals::rad_savedBrushes.brush[b].face[f].windingPts[winding][2];
						}
					}

					// calculate the brush origin
					glm::vec3 rad_brushOrigin((rad_brushMins + rad_brushMaxs) * 0.5f);

					// create a cube that encupsules the whole brush (we chop it by radiant's planes later ... no need for entity rotation doing it that way)
					float largestBoundDist = fmaxf(fmaxf(glm::distance(rad_brushMaxs.x, rad_brushMins.x), glm::distance(rad_brushMaxs.y, rad_brushMins.y)), glm::distance(rad_brushMaxs.z, rad_brushMins.z));

					glm::vec3 rad_brushLocalMaxs(largestBoundDist * 0.5f);
					glm::vec3 rad_brushLocalMins(-rad_brushLocalMaxs);

					// *
					// brush rotation => not needed anymore
					// calculate rotation angles by using the top/right brushface normals (facing Z (0 0 1)) (facing X (1 0 0))
					glm::vec3 angles(0.0f);

					/*float upVec[3], forwardVec[3];
					upVec[0] = Game::Globals::rad_savedBrushes.brush[b].face[1].normal[0];
					upVec[1] = Game::Globals::rad_savedBrushes.brush[b].face[1].normal[1];
					upVec[2] = Game::Globals::rad_savedBrushes.brush[b].face[1].normal[2];
					forwardVec[0] = Game::Globals::rad_savedBrushes.brush[b].face[3].normal[0];
					forwardVec[1] = Game::Globals::rad_savedBrushes.brush[b].face[3].normal[1];
					forwardVec[2] = Game::Globals::rad_savedBrushes.brush[b].face[3].normal[2];
					Utils::vector::_VectorToAnglesWithRoll(angles, forwardVec, upVec, false);*/

					//radBrushIndexForDyn
					Game::Globals::dynBrushModels.brushes[b].radBrushIndexForDyn = b;

					// modify one of our brushmodels
					DynBrushModel_Modify(&Game::Globals::dynBrushModels.brushes[b], rad_brushOrigin, rad_brushLocalMins, rad_brushLocalMaxs, angles);
				}
			}

			// make all unused brushmodels non-colliding
			for (auto dynA = Game::Globals::rad_savedBrushes.brushSelectedCount + Game::Globals::rad_savedBrushes.brushSavedCount; dynA < Game::Globals::dynBrushModels.mapped_bmodels; dynA++)
			{
				_Game::gEnt_BrushModelSetCollision(Game::Globals::dynBrushModels.brushes[dynA].ent, false);
				_Game::gEnt_MoveToInstant(Game::Globals::dynBrushModels.brushes[dynA].ent, glm::toVec3(Game::Globals::dynBrushModels.brushes[dynA].originalOrigin));
			}

			Game::Globals::rad_savedBrushes.wasModified = false;
		}

		// make brushmodels non-colliding till something is selected
		else if (Game::Globals::dynBrushModels.initiated && !Game::Globals::rad_savedBrushes.disabledCollision)
		{
			for (auto dynB = 0; dynB < Game::Globals::dynBrushModels.mapped_bmodels; dynB++)
			{
				_Game::gEnt_BrushModelSetCollision(Game::Globals::dynBrushModels.brushes[dynB].ent, false);
				_Game::gEnt_MoveToInstant(Game::Globals::dynBrushModels.brushes[dynB].ent, glm::toVec3(Game::Globals::dynBrushModels.brushes[dynB].originalOrigin));
			}

			Game::Globals::rad_savedBrushes.disabledCollision = true;
		}
	}

	//CM_FindBrushesInClipMap
#if 0 
	//search for our "16 brushes prefab" whose brushes we use to "create" new dynamic collision
	//this requires the map maker to include the "dynamic_collision" prefab in his map at origin 0 0 0
	//we will search for the 16 brushes and their corrosponding planes / leafes and map them accordingly
	void CM_FindBrushesInClipMap()
	{
		float dynBrushStart_mins[3] = { 970.0f, 988.0f, -58.0f };
		float dynBrushStart_maxs[3] = { 1016.0f, 1026.0f, 90.0f };

		int dynBrushesFound = 0;

		for (auto dynBrush = 0; dynBrush < 16; dynBrush++)
		{
			// add offsets to dynBrushBounds, except for the first one (all equally spaced and reducing in size (see prefab))
			if (dynBrush > 0)
			{
				dynBrushStart_mins[0] += 4.0f;
				dynBrushStart_mins[1] -= 2.0f;
				dynBrushStart_mins[2] += 2.0f;

				dynBrushStart_maxs[0] += 4.0f;
				dynBrushStart_maxs[1] -= 2.0f;
				dynBrushStart_maxs[2] -= 2.0f;
			}

			// *
			// cm->brush

			// for each brush in the clipmap
			for (auto cmBrush = 0; cmBrush < Game::cm->numBrushes; cmBrush++)
			{
				// compare current cmBrush bounds with our first dynBrush bounds
				// check mins
				if (Utils::vector::_VectorCompare(Game::cm->brushes[cmBrush].mins, dynBrushStart_mins))
				{
					// check maxs
					if (Utils::vector::_VectorCompare(Game::cm->brushes[cmBrush].maxs, dynBrushStart_maxs))
					{
						// we found our brush
						Game::Com_PrintMessage(0, Utils::VA("dynBrush[%d] @ cm->brushes[%d] :: mins = { %.1f %.1f %.1f } || maxs = { %.1f %.1f %.1f }\n", dynBrush, cmBrush,
							Game::cm->brushes[cmBrush].mins[0], Game::cm->brushes[cmBrush].mins[1], Game::cm->brushes[cmBrush].mins[2],
							Game::cm->brushes[cmBrush].maxs[0], Game::cm->brushes[cmBrush].maxs[1], Game::cm->brushes[cmBrush].maxs[2]), 0);

						// assign pointers
						Game::Globals::dynBrushes.brushes[dynBrush].cmBrushIndex = cmBrush;
						Game::Globals::dynBrushes.brushes[dynBrush].cmBrush = &Game::cm->brushes[cmBrush];

						// *
						// cm->planes

						int dynPlanesFound = 0;

						// find and assign all planes that make up our current brush (axialplanes from brush-bounds)
						for (auto planeForSide = 0; planeForSide < 6; planeForSide++)
						{
							float planeDist;
							float planeNormal[3];

							/*
							0 =		mins: 992 990  880		should result in planes mins[0/left] -1 0 0 992; mins[1/front] 0 -1 0 990; mins[2/bottom] 0 0 -1 880
									maxs: 994 1024 1024		^ maxs[0/right] 1 0 0 994; maxs[1/back] 0 1 0 1024; maxs[2/top] 0 0 1 1024
							*/

							/*		back
									_____			  +y
							  left	|	|  right	__|__ +x
									|	|			  |
									^^^^^
									front
							*/

							// axial planes for brush mins
							if (planeForSide < 3)
							{
								planeNormal[0] = planeForSide == 0 ? 1.0f : 0.0f; //left
								planeNormal[1] = planeForSide == 1 ? 1.0f : 0.0f; //front
								planeNormal[2] = planeForSide == 2 ? 1.0f : 0.0f; //bottom

								planeDist = Game::cm->brushes[cmBrush].mins[planeForSide];
							}
							// axial planes for brush maxs
							else
							{
								planeNormal[0] = planeForSide == 3 ? 1.0f : 0.0f; //right
								planeNormal[1] = planeForSide == 4 ? 1.0f : 0.0f; //back
								planeNormal[2] = planeForSide == 5 ? 1.0f : 0.0f; //top

								planeDist = Game::cm->brushes[cmBrush].maxs[planeForSide - 3];
							}

							// search for our plane in the clipMap
							for (auto cmPlane = 0; cmPlane < Game::cm->planeCount; cmPlane++)
							{
								// compare plane distance first
								if (Game::cm->planes[cmPlane].dist == planeDist)
								{
									// now check the normal
									if (Utils::vector::_VectorCompare(Game::cm->planes[cmPlane].normal, planeNormal))
									{
										// we found our plane; add it
										Game::Globals::dynBrushes.brushes[dynBrush].cmPlane[planeForSide] = &Game::cm->planes[cmPlane];

										Game::Com_PrintMessage(0, Utils::VA("|-> plane[%d] at cm->planes[%d] :: normal = { %.1f %.1f %.1f } || dist = { %.1f }\n", planeForSide, cmPlane,
											Game::cm->planes[cmPlane].normal[0], Game::cm->planes[cmPlane].normal[1], Game::cm->planes[cmPlane].normal[2],
											Game::cm->planes[cmPlane].dist), 0);

										dynPlanesFound++;
										break;
									}
								}
							}
						}

						if (dynPlanesFound != 6)
						{
							Game::Com_PrintMessage(0, "|-> ^1could not find all planes for the current brush!\n", 0);
						}

						// *
						// cm->leafs

						// find the brush leaf
						for (auto cmLeaf = 0; cmLeaf < (int)Game::cm->numLeafs; cmLeaf++)
						{
							// compare cmLeaf mins with brush mins
							if (Utils::vector::_VectorCompareEpsilon(Game::cm->leafs[cmLeaf].mins, Game::cm->brushes[cmBrush].mins, 0.125f))
							{
								// compare cmLeaf maxs with brush maxs
								if (Utils::vector::_VectorCompareEpsilon(Game::cm->leafs[cmLeaf].maxs, Game::cm->brushes[cmBrush].maxs, 0.125f))
								{
									// we found the leaf
									Game::Globals::dynBrushes.brushes[dynBrush].cmLeaf = &Game::cm->leafs[cmLeaf];

									Game::Com_PrintMessage(0, Utils::VA("|-> leaf at cm->leafs[%d] :: mins = { %.1f %.1f %.1f } || maxs = { %.1f %.1f %.1f }\n", cmLeaf,
										Game::cm->leafs[cmLeaf].mins[0], Game::cm->leafs[cmLeaf].mins[1], Game::cm->leafs[cmLeaf].mins[2],
										Game::cm->leafs[cmLeaf].maxs[0], Game::cm->leafs[cmLeaf].maxs[1], Game::cm->leafs[cmLeaf].maxs[2]), 0);
									
									break;
								}
							}
						}

						dynBrushesFound++;
						continue;
					}
				}
			}
		}

		Game::Com_PrintMessage(0, Utils::VA("Found (%d) dynBrushes\n", dynBrushesFound), 0);
	}
#endif

	// ------------------------------
	// Using information from radiant
	// *Map-Exporting* -> "RB_DrawCollision"

	Game::gentity_s *Radiant_CamModel;

	// check if the camera model entity is valid (player changed level etc.)
	bool Radiant_IsCamModelValid()
	{
		// if not spawned an entity yet
		if(!Game::Globals::cgsAddons.radiantCamModelSpawned)
		{
			return false;
		}
		else
		{
			std::int16_t modelIdx = Game::G_ModelIndex("xo_radiantcam");

			// check if the entity is valid (player changed level etc.)
			if (Radiant_CamModel == nullptr || Radiant_CamModel->classname == 0 || Radiant_CamModel->model != modelIdx) 
			{
				return false;
			}
		}

		return true;
	}

	// spawn the camera model
	void Radiant_SpawnCameraModel()
	{
		// needs :: 
		// s->index = modelIndex
		// linked = 0x1;
		// svFlags = 0x04; // even = visible, uneven = hidden

		std::int16_t modelIdx = Game::G_ModelIndex("xo_radiantcam"); // axis

		Radiant_CamModel = Game::G_Spawn();
		Game::G_SetOrigin(Radiant_CamModel, Game::Globals::cgsAddons.radiantCamOrigin);
		Game::G_SetAngles(Radiant_CamModel, Game::Globals::cgsAddons.radiantCamAngles);

		Radiant_CamModel->model = modelIdx;
		Radiant_CamModel->s.index = modelIdx;
		Radiant_CamModel->r.svFlags = 0x04;
		Radiant_CamModel->r.linked = 0x1;

		Game::G_CallSpawnEntity(Radiant_CamModel);

		//Radiant_CamModelSpawned = true;
		Game::Globals::cgsAddons.radiantCamModelSpawned = true;
	}

	// spawn/update the radiant cam model when not using syncCamera or player moved
	void Radiant_CamUpdateModelPos()
	{
		//if (Radiant_CamModelSpawned)
		if(Game::Globals::cgsAddons.radiantCamModelSpawned)
		{
			Radiant_CamModel->r.svFlags = 0x04; // visible
			Game::G_SetOrigin(Radiant_CamModel, Game::Globals::cgsAddons.radiantCamOrigin);
			Game::G_SetAngles(Radiant_CamModel, Game::Globals::cgsAddons.radiantCamAngles);
		}
	}

	// :: CG_CalcViewValues :: directly set refdef from parsed radiant camera commands
	void Radiant_UpdateGameCamera()
	{
		int cmdNum = *Game::clientActive_cmdNumber;
		Game::usercmd_s cmd = Game::usercmd_s();

		// spawn a camera model on first camera move or if invalid
		if (!Radiant_IsCamModelValid())
		{
			Radiant_SpawnCameraModel();
		}

		// update the camera model if not using the radiant camera to change the players viewpos
		if (Dvars::radiant_syncCamera->current.integer == 0 || Dvars::radiant_syncCamera->current.integer == 2)
		{
			Radiant_CamUpdateModelPos();

			// helper bool for our asm stub
			Game::Globals::cgsAddons.radiantCamSyncEnabled = false;
			Game::Globals::cgsAddons.radiantCamInUse = 0;

			return;
		}
		else
		{
			Game::Globals::cgsAddons.radiantCamSyncEnabled = true;
		}
		
		// show the camera model when the player starts to move (resetting the viewpos)
		if (Game::CL_GetUserCmd(cmdNum, &cmd))
		{
			if ((abs(cmd.forwardmove) > 50 || abs(cmd.rightmove) > 50))
			{
				// radiantCamInUse gets set on every received camera command 
				Game::Globals::cgsAddons.radiantCamInUse = 0;

				// show the camera model on player move
				Radiant_CamModel->r.svFlags = 0x04;
				// update the camera model pos once
				Radiant_CamUpdateModelPos();
					
				return;
			}
		}

		// hide camera model when radiant is moving the players view
		Radiant_CamModel->r.svFlags = 0x01;

		// player view
		Game::cgs->refdefViewAngles[0] = Game::Globals::cgsAddons.radiantCamAngles[0];
		Game::cgs->refdefViewAngles[1] = Game::Globals::cgsAddons.radiantCamAngles[1];
		Game::cgs->refdefViewAngles[2] = Game::Globals::cgsAddons.radiantCamAngles[2];

		Utils::vector::_AnglesToAxis(Game::Globals::cgsAddons.radiantCamAngles, Game::cgs->refdef.viewaxis);

		Game::cgs->refdef.vieworg[0] = Game::Globals::cgsAddons.radiantCamOrigin[0];
		Game::cgs->refdef.vieworg[1] = Game::Globals::cgsAddons.radiantCamOrigin[1];
		Game::cgs->refdef.vieworg[2] = Game::Globals::cgsAddons.radiantCamOrigin[2];

		//Game::Com_PrintMessage(0, Utils::VA("^1[CAM:R2G]: Angles: ( %.1f, %.1f, %.1f )  Origin: ( %.1f, %.1f, %.1f )\n",
		//	Game::Globals::cgsAddons.radiantCameraAngles[0], Game::Globals::cgsAddons.radiantCameraAngles[1], Game::Globals::cgsAddons.radiantCameraAngles[2],
		//	Game::Globals::cgsAddons.radiantCameraOrigin[0], Game::Globals::cgsAddons.radiantCameraOrigin[1], Game::Globals::cgsAddons.radiantCameraOrigin[2]), 0);

	}

	// :: Radiant_UpdateGameCamera
	__declspec(naked) void CG_CalcViewValues_stub()
	{
		const static uint32_t CG_CalcTurretViewValues_Func = 0x4507B0;
		const static uint32_t retnRadiantCam = 0x451C7C; // back to CG_ShakeCamera
		const static uint32_t retnStock = 0x451C69; // back to cmp cgs_renderingThirdPerson, 0
		__asm
		{
			Call	CG_CalcTurretViewValues_Func	// overwritten op

			cmp		Game::Globals::cgsAddons.radiantCamInUse, 0
			je		STOCK

			pushad
			Call	Radiant_UpdateGameCamera		// update the radiant cam on camera move
			popad

			// if we do not sync the camera <> viewpos but only the camera model position
			cmp		Game::Globals::cgsAddons.radiantCamSyncEnabled, 0
			je		STOCK

			jmp		retnRadiantCam


			STOCK:
				jmp		retnStock
		}
	}

	// radiantCamInUse :: disable CG_ApplyViewAnimation in CG_CalcViewValues (leave viewmodel at player origin if radiantcam is enabled)
	__declspec(naked) void CG_ApplyViewAnimation_stub()
	{
		const static uint32_t CG_ApplyViewAnimation_Func = 0x450890;
		const static uint32_t retnPt = 0x451C97; // back to next op
		__asm
		{
			cmp		Game::Globals::cgsAddons.radiantCamInUse, 0
			je		STOCK
			cmp		Game::Globals::cgsAddons.radiantCamSyncEnabled, 0 // not needed
			je		STOCK

								// if radiantCamInUse
			jmp		retnPt		// skip CG_ApplyViewAnimation

			STOCK :
				Call	CG_ApplyViewAnimation_Func
				jmp		retnPt
		}
	}

	// ++++++++++++++++++++++++++++
	// ++++++++++++++++++++++++++++

	SOCKET g_ServerSocket = INVALID_SOCKET;
	SOCKET g_ClientSocket = INVALID_SOCKET;

	// ---------------
	// Sending packets 

	int commandsSend = 0;

	// *
	// Send commands to the client
	void RadiantRemote::SV_SendPacket(Game::ServerCommand *Command)
	{
		// check if radiant is connected to the server
		if (g_ClientSocket == INVALID_SOCKET || g_ServerSocket == INVALID_SOCKET)
		{
			return;
		}

		int ret = send(g_ClientSocket, (const char *)Command, sizeof(Game::ServerCommand), 0);
		
		if (ret == SOCKET_ERROR)
		{
			Game::Com_PrintMessage(0, "^1[LiveRadiant]: ^7Sending msg to client failed!\n", 0);
			return;

			//closesocket(g_RemoteSocket);
			//g_RemoteSocket = INVALID_SOCKET;
			//g_RemoteSocketStatus = INVALID_SOCKET;
		}

		if (Dvars::radiant_liveDebug->current.enabled)
		{
			commandsSend++;

			const char *cmdType = "";

			switch (Command->type)
			{
			case 0:
				cmdType = "SERVER_STRING_MSG";
				break;
			case 10:
				cmdType = "SERVER_CAMERA_UPDATE";
				break;
			case 20:
				cmdType = "SERVER_EXPORT_SINGLE_BRUSH_FACE_INDEX";
				break;
			case 21:
				cmdType = "SERVER_EXPORT_SINGLE_BRUSH_FACE";
				break;
			
			default:
				cmdType = "COMMAND_TYPE_UNKOWN";
				break;
			}

			Game::Com_PrintMessage(0, Utils::VA("Send command num: [%d] of type: [%s]\n", commandsSend, cmdType), 0);
		}
	}

	// *
	// called from _Pmove
	void RadiantRemote::Cmd_SendCamera(const float *origin, const float *angles)
	{
		Game::ServerCommand cmd;
		memset(&cmd, 0, sizeof(Game::ServerCommand));

		cmd.type = Game::SERVER_CAMERA_UPDATE;

		sprintf_s(cmd.strCommand, "%.1f %.1f %.1f %.1f %.1f %.1f",
					origin[0], origin[1], origin[2], angles[0], angles[1], angles[2]);

		RadiantRemote::SV_SendPacket(&cmd);
	}

	// -----------------
	// Receiving packets 

	// *
	// process dvars
	void RadiantRemote::Cmd_ProcessDvar(Game::SpawnVar* spawnVar)
	{
		if (auto dvarname_string = _Game::GetPairValue(spawnVar, "dvarname"))
		{
			if (auto value_string = _Game::GetPairValue(spawnVar, "value"))
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, Utils::VA("%s %s\n", dvarname_string, value_string));
			}
		}
	}

	// *
	// process camera commands from radiant
	void RadiantRemote::Cmd_ProcessCamera(Game::SpawnVar *spawnVar)
	{
		char* origin_string;
		origin_string = _Game::GetPairValue(spawnVar, "origin");

		char *angles_string;
		angles_string = _Game::GetPairValue(spawnVar, "angles");
		
		if (origin_string)
		{
			if (!sscanf(origin_string, "%f %f %f", &Game::Globals::cgsAddons.radiantCamOrigin[0], &Game::Globals::cgsAddons.radiantCamOrigin[1], &Game::Globals::cgsAddons.radiantCamOrigin[2]))
			{
				Game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_CAMERA_ORIGIN", 0);
			}
		}

		if (angles_string)
		{
			if (!sscanf(angles_string, "%f %f %f", &Game::Globals::cgsAddons.radiantCamAngles[0], &Game::Globals::cgsAddons.radiantCamAngles[1], &Game::Globals::cgsAddons.radiantCamAngles[2]))
			{
				Game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_CAMERA_ANGLES", 0);
			}
		}

		Game::Globals::cgsAddons.radiantCamAngles[0] = -Game::Globals::cgsAddons.radiantCamAngles[0];
		Game::Globals::cgsAddons.radiantCamInUse = true;

		if (Dvars::radiant_liveDebug->current.enabled)
		{
			Game::Com_PrintMessage(0, Utils::VA("[CAM:R2G]: Angles: ( %.1f, %.1f, %.1f )  Origin: ( %.1f, %.1f, %.1f )\n",
				Game::Globals::cgsAddons.radiantCamAngles[0], Game::Globals::cgsAddons.radiantCamAngles[1], Game::Globals::cgsAddons.radiantCamAngles[2],
				Game::Globals::cgsAddons.radiantCamOrigin[0], Game::Globals::cgsAddons.radiantCamOrigin[1], Game::Globals::cgsAddons.radiantCamOrigin[2]), 0);
		}
	}

	// on each selection (once)
	// bool :: if valid selection or nothing selected
	void RadiantRemote::Cmd_ProcessBrushSelect(Game::SpawnVar *spawnVar)
	{
		char *brushselect_string;
		brushselect_string = _Game::GetPairValue(spawnVar, "brushselect");

		if (brushselect_string)
		{
			int temp;

			if (!sscanf(brushselect_string, "%d", &temp))
			{
				Game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_SELECT", 0);
			}
			else
			{
				Game::Globals::rad_savedBrushes.brushSelected = temp;
			}

			// reset brushSelectedCount when selecting / deselecting
			Game::Globals::rad_savedBrushes.brushSelectedCount = 0;
		}
	}

	// on each selection (once)
	// int :: amount of selected brushes
	void RadiantRemote::Cmd_ProcessBrushAmount(Game::SpawnVar *spawnVar)
	{
		char *brushcount_string;
		brushcount_string = _Game::GetPairValue(spawnVar, "brushcount");

		if (brushcount_string)
		{
			if (!sscanf(brushcount_string, "%d", &Game::Globals::rad_savedBrushes.brushSelectedCount))
			{
				Game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_COUNT", 0);
			}

			// cap brush selection count
			if (Game::Globals::rad_savedBrushes.brushSelectedCount + Game::Globals::rad_savedBrushes.brushSavedCount > RADIANT_MAX_SEL_BRUSHES)
			{
				Game::Globals::rad_savedBrushes.brushSelectedCount = RADIANT_MAX_SEL_BRUSHES;
			}

			// update collision of all brushmodels
			Game::Globals::rad_savedBrushes.wasModified = true;
		}
	}

	// *
	// clear the last saved brush transmitted from radiant
	void RadiantDB_ClearSavedBrush(int brushNum)
	{
		// reset the face index
		Game::Globals::rad_savedBrushes.brush[brushNum].nextFreeFaceIdx = 0;

		// clear brush-faces and face-windingpoints
		memset(Game::Globals::rad_savedBrushes.brush[brushNum].face, 0, sizeof(Game::Globals::rad_savedBrushes.brush[brushNum].face[16]));
	}

	// per brush
	// int :: the index of the current brush being transmitted
	void RadiantRemote::Cmd_ProcessBrushNum(Game::SpawnVar *spawnVar)
	{
		char *brushnum_string;
		brushnum_string = _Game::GetPairValue(spawnVar, "brushnum");

		if (brushnum_string)
		{
			if (!sscanf(brushnum_string, "%d", &Game::Globals::rad_savedBrushes.brushSelectedNum))
			{
				Game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_CURRENT_NUM", 0);
			}
			else
			{
				// add new brushes after saved brushes
				Game::Globals::rad_savedBrushes.brushSelectedNum += Game::Globals::rad_savedBrushes.brushSavedCount;
			}
		}
	}

	// per brush
	// int :: how many faces we transmit in total
	void RadiantRemote::Cmd_ProcessBrushFaceCount(Game::SpawnVar *spawnVar, int brushNum)
	{
		char *brushFaceCount_str;
		brushFaceCount_str = _Game::GetPairValue(spawnVar, "brushfacecount");

		if (brushFaceCount_str)
		{
			// set brush face-count
			if (!sscanf(brushFaceCount_str, "%d", &Game::Globals::rad_savedBrushes.brush[brushNum].faceCount))
			{
				Game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_FACE_COUNT", 0);
			}

			// clear old brush if we got atleast one valid face
			if (Game::Globals::rad_savedBrushes.brush[brushNum].faceCount)
			{
				RadiantDB_ClearSavedBrush(brushNum);
			}
		}
	}

	// per brush-face
	// vec3_t :: face-normal (sent before windings)
	void RadiantRemote::Cmd_ProcessBrushFaceNormals(Game::SpawnVar *spawnVar, int brushNum)
	{
		char *brushFaceNormal_str;
		brushFaceNormal_str = _Game::GetPairValue(spawnVar, "normal");

		char *brushFaceDist_str;
		brushFaceDist_str = _Game::GetPairValue(spawnVar, "dist");

		// next free face in rad_savedBrushes.brush.face (0 on new brush, ++ on each new face (do not increment here!))
		int fIdx = Game::Globals::rad_savedBrushes.brush[brushNum].nextFreeFaceIdx;

		if (brushFaceNormal_str)
		{
			if (!sscanf(brushFaceNormal_str, "%f %f %f",
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].normal[0],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].normal[1],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].normal[2]))
			{
				Game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_FACE_NORMALS", 0);
			}
		}

		if (brushFaceDist_str)
		{
			if (!sscanf(brushFaceDist_str, "%f",
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].dist))
			{
				Game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_FACE_NORMALS_DIST", 0);
			}
		}
	}

	// per brush-face
	// int / vec3 :: how many windings per face we transmit + the points themselfs 
	void RadiantRemote::Cmd_ProcessBrushFace(Game::SpawnVar *spawnVar, int brushNum)
	{
		char *brushWindingCount_str;
		brushWindingCount_str = _Game::GetPairValue(spawnVar, "windingcount");
		
		char *brushWindings_str;
		brushWindings_str = _Game::GetPairValue(spawnVar, "windingpoints");

		// next free face in rad_savedBrushes.brush.face (0 on new brush, ++ on each new face)
		int fIdx = Game::Globals::rad_savedBrushes.brush[brushNum].nextFreeFaceIdx;

		// get the amount of winding points for the current face
		if (brushWindingCount_str)
		{
			if (!sscanf(brushWindingCount_str, "%d", &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingCount))
			{
				Game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_FACE_WINDING_COUNT", 0);
			}
		}

		// build winding points for the current face
		if (brushWindings_str)
		{
			if (!sscanf(brushWindings_str, "%f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f",
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[0][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[0][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[0][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[1][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[1][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[1][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[2][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[2][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[2][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[3][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[3][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[3][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[4][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[4][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[4][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[5][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[5][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[5][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[6][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[6][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[6][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[7][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[7][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[7][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[8][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[8][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[8][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[9][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[9][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[9][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[10][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[10][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[10][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[11][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[11][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[11][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[12][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[12][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[12][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[13][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[13][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[13][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[14][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[14][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[14][2],
				&Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[15][0], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[15][1], &Game::Globals::rad_savedBrushes.brush[brushNum].face[fIdx].windingPts[15][2]))
			{
				Game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_FACE_WINDING_POINTS", 0);
			}
		
			// next free face
			Game::Globals::rad_savedBrushes.brush[brushNum].nextFreeFaceIdx++;
		}
	}

	// *
	// Check radiant connection status - accept / reject
	bool RadiantRemote::SV_UpdateSocket()
	{
		// Skip this function if client already connected
		if (g_ClientSocket != INVALID_SOCKET)
		{
			if (!Game::Globals::cgsAddons.radiantLiveConnected)
			{
				Game::Com_PrintMessage(0, "^2[LiveRadiant]: ^7Client connected!\n", 0);
				Game::Globals::cgsAddons.radiantLiveConnected = true;
			}

			return true;
		}

		if (g_ServerSocket == INVALID_SOCKET)
		{
			return false;
		}

		// Check if there's a pending client connection request
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(g_ServerSocket, &readSet);

		// Zero timeout (poll)
		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		int status = select(g_ServerSocket, &readSet, nullptr, nullptr, &timeout);

		if (status == SOCKET_ERROR)
		{
			Game::Com_Error(0, "^1[LiveRadiant]: ^7Failed to query socket status!\n");
		}

		// Must be 1 (handle) if someone is waiting
		if (status != 1)
		{
			return false;
		}

		g_ClientSocket = accept(g_ServerSocket, nullptr, nullptr);

		if (g_ClientSocket == INVALID_SOCKET)
		{
			Game::Com_Error(0, "^1[LiveRadiant]: ^7Failed to accept a connection!?\n");
		}

		// Set non-blocking flag
		u_long socketMode = 1;
		ioctlsocket(g_ClientSocket, FIONBIO, &socketMode);

		Game::Com_PrintMessage(0, "^2[LiveRadiant]: ^7Client connected!\n", 0);
		Game::Globals::cgsAddons.radiantLiveConnected = true;


		// ---------

		Game::ServerCommand cmd;
		memset(&cmd, 0, sizeof(Game::ServerCommand));

		cmd.type = Game::SERVER_STRING_MSG;
		std::string buildDate = IW3XO_BUILDVERSION_DATE;
		std::string buildNum = std::to_string((int)IW3X_BUILDNUMBER);
		sprintf_s(cmd.strCommand, Utils::VA("(IW3XO::%s::%s) accepted request", buildNum.c_str(), buildDate.c_str()));

		RadiantRemote::SV_SendPacket(&cmd);

		// ---------

		return true;
	}

	Game::RadiantCommand recvCommands[1024];

	// *
	// Receive packets from radiant (would normally run within the server thread @ 20fps)
	// Current networking only supports local usage, so why not increase the receiving rate by calling it from pmove
	void RadiantRemote::SV_ReceivePackets()
	{
		if (!Dvars::radiant_live->current.enabled || !RadiantRemote::SV_UpdateSocket())
		{
			Game::Globals::cgsAddons.radiantLiveConnected = false;
			return;
		}

		// Non-blocking read
		memset(recvCommands, 0, sizeof(recvCommands));

		int recvSize = recv(g_ClientSocket, (char *)&recvCommands, sizeof(recvCommands), 0);

		// Skip everything if there's no data
		if (recvSize == SOCKET_ERROR) 
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				return;
			}
				
			// Some other problem occurred and now the socket is bad
			shutdown(g_ClientSocket, 2 /*SD_BOTH*/);
			closesocket(g_ClientSocket);

			g_ClientSocket = INVALID_SOCKET;
			Game::Com_PrintMessage(0, "^1[LiveRadiant]: ^7Client disconnected!\n", 0);

			return;
		}

		// Determine the number of commands sent, then tell the game
		size_t commandCount = recvSize / sizeof(Game::RadiantCommand); 

		for (size_t i = 0; i < commandCount; i++)
		{
			Game::SpawnVar spawnVar;
			
			// set start and end of string command
			_Game::G_SetEntityParsePoint(recvCommands[i].strCommand);

			// parse it
			_Game::G_ParseSpawnVars(&spawnVar);

			// parse the following commands in order (see IW3R :: onBrush_SelectDeselect) 
			// parse select first (once)
			if (recvCommands[i].type == Game::RADIANT_COMMAND_BRUSH_SELECT)
			{
				RadiantRemote::Cmd_ProcessBrushSelect(&spawnVar);

				if (Dvars::radiant_liveDebug->current.enabled)
					Game::Com_PrintMessage(0, "^2[LiveRadiant]: ^7Received command of type (RADIANT_COMMAND_BRUSH_SELECT).\n", 0);
			}

			// amount of selected brushes (once)
			if (recvCommands[i].type == Game::RADIANT_COMMAND_BRUSH_COUNT)
			{
				RadiantRemote::Cmd_ProcessBrushAmount(&spawnVar);

				if (Dvars::radiant_liveDebug->current.enabled)
					Game::Com_PrintMessage(0, "^2[LiveRadiant]: ^7Received command of type (RADIANT_COMMAND_BRUSH_COUNT).\n", 0);
			}


			// on each brush
			// per brush current brush number (once)
			if (recvCommands[i].type == Game::RADIANT_COMMAND_BRUSH_CURRENT_NUM)
			{
				RadiantRemote::Cmd_ProcessBrushNum(&spawnVar);

				if (Dvars::radiant_liveDebug->current.enabled)
					Game::Com_PrintMessage(0, "^2[LiveRadiant]: ^7Received command of type (RADIANT_COMMAND_BRUSH_CURRENT_NUM).\n", 0);
			}

			// ^ above adds nextFree to selectedNum
			if (Game::Globals::rad_savedBrushes.brushSelectedNum < RADIANT_MAX_SEL_BRUSHES)
			{
				// on each brush
				// per brush face-count (once)
				if (recvCommands[i].type == Game::RADIANT_COMMAND_BRUSH_FACE_COUNT)
				{
					RadiantRemote::Cmd_ProcessBrushFaceCount(&spawnVar, Game::Globals::rad_savedBrushes.brushSelectedNum);

					if (Dvars::radiant_liveDebug->current.enabled)
						Game::Com_PrintMessage(0, "^2[LiveRadiant]: ^7Received command of type (RADIANT_COMMAND_BRUSH_FACE_COUNT).\n", 0);
				}

				// on each brush
				// per brush-face normal (face 1 and 3 only) ... all of them :o
				if (recvCommands[i].type == Game::RADIANT_COMMAND_BRUSH_FACE_NORMALS)
				{
					RadiantRemote::Cmd_ProcessBrushFaceNormals(&spawnVar, Game::Globals::rad_savedBrushes.brushSelectedNum);

					if (Dvars::radiant_liveDebug->current.enabled)
						Game::Com_PrintMessage(0, "^2[LiveRadiant]: ^7Received command of type (RADIANT_COMMAND_BRUSH_FACE_NORMALS).\n", 0);
				}

				// on each brush
				// parse all of the brush faces (runs multiple times as we send 1 cmd for each face)
				if (recvCommands[i].type == Game::RADIANT_COMMAND_BRUSH_FACE)
				{
					RadiantRemote::Cmd_ProcessBrushFace(&spawnVar, Game::Globals::rad_savedBrushes.brushSelectedNum);

					if (Dvars::radiant_liveDebug->current.enabled)
						Game::Com_PrintMessage(0, "^2[LiveRadiant]: ^7Received command of type (RADIANT_COMMAND_BRUSH_FACE).\n", 0);
				}
			}

			// -----------

			// parse cammera commands
			if (recvCommands[i].type == Game::RADIANT_COMMAND_CAMERA)
			{
				RadiantRemote::Cmd_ProcessCamera(&spawnVar);
			}

			// parse dvar
			if (recvCommands[i].type == Game::RADIANT_COMMAND_SET_DVAR)
			{
				RadiantRemote::Cmd_ProcessDvar(&spawnVar);
			}
		}
	}

	// *
	// not needed anymore, see above (stays here for later usage)
	void SV_PostFrame()
	{
		//RadiantRemote::SV_ReceivePackets();
	}

	// :: SV_PostFrame
	__declspec(naked) void SV_PostFrame_stub()
	{
		const static uint32_t SV_CheckTimeouts_Func = 0x532D20;
		const static uint32_t retnPt = 0x5335B3; // SV_SendClientMessages Call
		__asm
		{
			pushad
			Call	SV_PostFrame
			popad

			Call	SV_CheckTimeouts_Func // overwritten op
			jmp		retnPt
		}
	}


	// -----------
	// Server Init

	// *
	// Shutdown live-link server
	void RadiantRemote::SV_Shutdown()
	{
		shutdown(g_ServerSocket, 2 /*SD_BOTH*/);
		closesocket(g_ServerSocket);

		shutdown(g_ClientSocket, 2 /*SD_BOTH*/);
		closesocket(g_ClientSocket);

		g_ServerSocket = INVALID_SOCKET;
		g_ClientSocket = INVALID_SOCKET;

		Game::Globals::cgsAddons.radiantLiveConnected = false;

		Game::Com_PrintMessage(0, "^1[LiveRadiant]: ^7Shutdown!\n", 0);
	}

	// *
	// Start live-link server on G_InitGame
	void RadiantRemote::SV_Init()
	{
		if (g_ServerSocket != INVALID_SOCKET)
		{
			return;
		}

		WSADATA wsaData;

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
		{
			Game::Com_Error(0, "^1[LiveRadiant]: ^7Socket startup failed!\n");
		}
		
		// Create a TCP server socket
		g_ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 

		if (g_ServerSocket == INVALID_SOCKET)
		{
			Game::Com_Error(0, "^1[LiveRadiant]: ^7Socket creation failed!\n");
		}
			
		// Bind socket to any local address on port X
		sockaddr_in addrIn;
		addrIn.sin_family = AF_INET;
		addrIn.sin_port = htons((u_short)Dvars::radiant_livePort->current.integer); //htons(3700);
		addrIn.sin_addr.s_addr = inet_addr("127.0.0.1");

		Game::Com_PrintMessage(0, Utils::VA("^2[LiveRadiant]: ^7Attempting to bind on port %d ... \n", (int)ntohs(addrIn.sin_port)), 0);

		if (bind(g_ServerSocket, (SOCKADDR *)&addrIn, sizeof(addrIn)) == SOCKET_ERROR)
		{
			Game::Com_PrintMessage(0, "^1[LiveRadiant]: ^7Failed to bind socket. Port in use? ~> radiant_livePort\n", 0);

			RadiantRemote::SV_Shutdown();

			return;
		}

		// Listen for any number of incoming connections
		if (listen(g_ServerSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			Game::Com_PrintMessage(0, "^1[LiveRadiant]: ^7Failed to listen for incoming connections!\n", 0);

			RadiantRemote::SV_Shutdown();
			return;
		}

		Game::Com_PrintMessage(0, "^2[LiveRadiant]: ^7Succeeded!\n", 0);
	}

	// :: RadiantRemote::SV_Init()
	__declspec(naked) void G_InitGame_stub()
	{
		const static uint32_t SaveRegisteredItems_Func = 0x4BCBE0;
		const static uint32_t retnPt = 0x4BF5A8; // next op after hook loc
		__asm
		{
			Call	SaveRegisteredItems_Func

			pushad
			Call	RadiantRemote::SV_Init
			popad

			jmp		retnPt
		}
	}


	RadiantRemote::RadiantRemote()
	{
		Command::Add("radiant_saveSelection", [](Command::Params)
		{
			if (Game::Globals::cgsAddons.radiantLiveConnected)
			{
				if (Game::Globals::rad_savedBrushes.brushSelectedCount)
				{
					if (Game::Globals::rad_savedBrushes.wasModified)
					{
						Game::Com_PrintMessage(0, "No new brushes selected!\n", 0);
						return;
					}

					// first iter bool
					Game::Globals::rad_savedBrushes.resetCollision = true;

					//RadiantDB_BrushSortingContainer
					//auto RadiantDB_temp = Game::savedRadiantBrushes();
					//memcpy(&RadiantDB_temp, &Game::Globals::rad_savedBrushes, sizeof(Game::savedRadiantBrushes));
					memcpy(&RadiantDB_BrushSortingContainer, &Game::Globals::rad_savedBrushes.brush, sizeof(Game::radiantBrush_t) * RADIANT_MAX_SEL_BRUSHES);

					int firstNonSavedDB_idx = Game::Globals::rad_savedBrushes.brushSavedCount;
					int lastNonSavedDB_idx = Game::Globals::rad_savedBrushes.brushSavedCount + Game::Globals::rad_savedBrushes.brushSelectedCount - 1;

					if (lastNonSavedDB_idx >= RADIANT_MAX_SEL_BRUSHES)
					{
						lastNonSavedDB_idx = RADIANT_MAX_SEL_BRUSHES - 1;
					}

					int nonSavedDB_count = Game::Globals::rad_savedBrushes.brushSelectedCount;

					if (nonSavedDB_count + firstNonSavedDB_idx >= RADIANT_MAX_SEL_BRUSHES)
					{
						nonSavedDB_count = RADIANT_MAX_SEL_BRUSHES - 1 - firstNonSavedDB_idx;
					}

					// clear all saved brushes first
					memset(&Game::Globals::rad_savedBrushes.brush[0], 0, sizeof(Game::radiantBrush_t) * RADIANT_MAX_SEL_BRUSHES);

					// copy new brushes into the struct
					memcpy(&Game::Globals::rad_savedBrushes.brush[0], &RadiantDB_BrushSortingContainer[firstNonSavedDB_idx], sizeof(Game::radiantBrush_t) * nonSavedDB_count);
				
					Game::Globals::rad_savedBrushes.usingSaved = true;
					Game::Globals::rad_savedBrushes.brushSavedCount = Game::Globals::rad_savedBrushes.brushSelectedCount;

					for(auto bSaved = 0; bSaved < Game::Globals::rad_savedBrushes.brushSavedCount; bSaved++)
					{
						Game::Globals::rad_savedBrushes.brush[bSaved].isSaved = true;
					}

					Game::Com_PrintMessage(0, "Saved selection!\n", 0);
				}
				else
				{
					Game::Com_PrintMessage(0, "^3No brushes selected!\n", 0);
				}
			}
			else
			{
				Game::Com_PrintMessage(0, "^3Radiant Live-Link not active!\n", 0);
			}
		});

		Command::Add("radiant_clearSaved", [](Command::Params)
		{
			if (Game::Globals::cgsAddons.radiantLiveConnected)
			{
				if (Game::Globals::rad_savedBrushes.brushSavedCount)
				{
					if (!RadiantDB_RemoveSaved())
					{
						Game::Com_PrintMessage(0, "Cleared! No non-saved brushes found.\n", 0);
						return;
					}

					Game::Com_PrintMessage(0, "Cleared! Found non-saved brushes, recalculating collision.\n", 0);
					Game::Globals::rad_savedBrushes.resetCollision = true;
				}
				else
				{
					Game::Com_PrintMessage(0, "^3No brushes to clear!\n", 0);
				}
			}
			else
			{
				Game::Com_PrintMessage(0, "^3Radiant Live-Link not active!\n", 0);
			}
		});

		/*radiant_connect*/ #if 0
		Command::Add("radiant_connect", [](Command::Params params)
		{
			if (Game::Dvar_FindVar("sv_running")->current.enabled)
			{
				Game::Com_PrintMessage(0, "A server cannot connect to another server ...\n", 0);
				return;
			}

			if (params.Length() < 2)
			{
				Game::Com_PrintMessage(0, "Usage :: radiant_connect <ip:port>\n", 0);
				return;
			}

			std::vector<std::string> args;
			args = Utils::Explode(params[1], ':');

			if (args.size() != 2)
			{
				Game::Com_PrintMessage(0, "Usage :: radiant_connect <ip:port>\n", 0);
				return;
			}

			std::string r_connectIp = args[0];
			int r_connectPort = Utils::try_stoi(args[1], false);

			WSADATA wsaData;

			if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
			{
				Game::Com_PrintMessage(0, "^1[LiveRadiant]: WSAStartup ERROR!\n", 0);
				return;
			}

			if (!Dvars::radiant_live->current.enabled)
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "radiant_live 1\n");
			}

			// connect to the remote game TCP server (doing this in the while loop too if radiant is running and the server is not )
			g_RemoteSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (g_RemoteSocket == INVALID_SOCKET)
			{
				Game::Com_PrintMessage(0, "^1[LiveRadiant]: Failed to initialize client TCP socket!\n", 0);
				return;
			}

			Game::Com_PrintMessage(0, Utils::VA("[LiveRadiant]: Connecting to <%s> : <%d> ...\n", r_connectIp.c_str(), r_connectPort), 0);

			g_RemoteSocketStatus = INVALID_SOCKET;

			sockaddr_in remoteAddr;
			remoteAddr.sin_family = AF_INET;

			remoteAddr.sin_port = htons((u_short)r_connectPort);
			remoteAddr.sin_addr.s_addr = inet_addr(r_connectIp.c_str());

			// check if the remote socket was created
			if (g_RemoteSocket == INVALID_SOCKET)
			{
				g_RemoteSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			}

			// if connected sucessfully
			if (connect(g_RemoteSocket, (sockaddr*)&remoteAddr, sizeof(sockaddr)) != SOCKET_ERROR)
			{
				Game::Com_PrintMessage(0, "^2[LiveRadiant]: Connected!\n", 0);
				g_RemoteSocketStatus = 1;

				Game::Globals::cgsAddons.radiantLiveConnected = true;
			}
		});
		#endif

		// -----
		// Dvars

		Dvars::radiant_live = Game::Dvar_RegisterBool(
			/* name		*/ "radiant_live",
			/* desc		*/ "enable live-link between radiant <-> iw3",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::radiant_livePort = Game::Dvar_RegisterInt(
			/* name		*/ "radiant_livePort",
			/* desc		*/ "port to be used for live-link",
			/* default	*/ 3700,
			/* minVal	*/ 0,
			/* maxVal	*/ 99999,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::radiant_liveDebug = Game::Dvar_RegisterBool(
			/* name		*/ "radiant_liveDebug",
			/* desc		*/ "enable live-link debug-prints",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		static std::vector <char*> radiant_syncCameraEnum = { "DISABLED", "RADIANT_TO_GAME", "GAME_TO_RADIANT", "BOTH",};
		Dvars::radiant_syncCamera = Game::Dvar_RegisterEnum(
			/* name		*/ "radiant_syncCamera",
			/* desc		*/ "enable live-link camera sync",
			/* default	*/ 0,
			/* enumSize	*/ 4,
			/* enumData */ radiant_syncCameraEnum.data(),
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::radiant_brushCollision = Game::Dvar_RegisterBool(
			/* name		*/ "radiant_brushCollision",
			/* desc		*/ "enable radiant debug-brush collision",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::radiant_brushColor = Game::Dvar_RegisterVec4(
			/* name		*/ "radiant_brushColor",
			/* desc		*/ "color of radiant debug-brushes",
			/* x		*/ 0.5f,
			/* y		*/ 0.5f,
			/* z		*/ 0.5f,
			/* w		*/ 0.5f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::radiant_brushLit = Game::Dvar_RegisterBool(
			/* name		*/ "radiant_brushLit", 
			/* desc		*/ "enable fake lighting on debug-brushes", 
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::radiant_brushWireframe = Game::Dvar_RegisterBool(
			/* name		*/ "radiant_brushWireframe",
			/* desc		*/ "enable wireframe on debug-brushes",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::radiant_brushWireframeColor = Game::Dvar_RegisterVec4(
			/* name		*/ "radiant_brushWireframeColor",
			/* desc		*/ "wireframe color of debug-brush",
			/* x		*/ 1.0f,
			/* y		*/ 0.64f,
			/* z		*/ 0.0f,
			/* w		*/ 0.8f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		// -----
		// Hooks

		// SV_ServerThread :: add SV_PostFrame (now calling SV_ReceivePackets from Pmove)
		//Utils::Hook(0x5335AE, SV_PostFrame_stub, HOOK_JUMP).install()->quick();

		// G_InitGame :: add RadiantRemote::SV_Init()
		Utils::Hook(0x4BF5A3, G_InitGame_stub, HOOK_JUMP).install()->quick();

		// CG_CalcViewValues :: add CG_RadiantCamCalcView to change refdef with radiant-commands
		Utils::Hook(0x451C64, CG_CalcViewValues_stub, HOOK_JUMP).install()->quick();

		// CG_CalcViewValues :: skip CG_ApplyViewAnimation if radiant-cam is enabled so we dont move the viewmodel with the camera
		Utils::Hook(0x451C92, CG_ApplyViewAnimation_stub, HOOK_JUMP).install()->quick();
	}

	RadiantRemote::~RadiantRemote()
	{ }
}