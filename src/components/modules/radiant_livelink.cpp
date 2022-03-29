/* Thanks to Nukem for some of the base functions used here. Check out his linker-mod here: 
https://github.com/Nukem9/LinkerMod/blob/development/components/radiant_mod/remote_net.cpp */

#include "std_include.hpp"

#define DYN_COLL_BMODEL_AMOUNT 16

// show tooltip after x seconds
#define TTDELAY 1.0f 

// tooltip with delay
#define TT(tooltip) if (ImGui::IsItemHoveredDelay(TTDELAY)) { ImGui::SetTooltip(tooltip); }

// spacing dummy
#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 

// seperator with spacing
#define SEPERATORV(spacing) SPACING(0.0f, spacing); ImGui::Separator(); SPACING(0.0f, spacing); 

// execute a single command
#define CMDEXEC(command) game::Cmd_ExecuteSingleCommand(0, 0, command)

namespace components
{
	game::radiant_brush_s _brush_sorting_container[RADIANT_MAX_SEL_BRUSHES];

	// ---------------------
	// Radiant debug brushes

	// search brushmodels from prefab "dynamic_collision_bmodels.map"
	void radiant_livelink::find_dynamic_bmodels()
	{
		game::glob::dynamic_brush_models.mapped_bmodels = 0; // re-map already mapped brushmodels

		// if map is loaded
		if (game::cm->name && game::cm->isInUse)
		{
			bool ent_found_bb_model = false;
			game::Com_PrintMessage(0, "CM_FindDynamicBrushModels :: start ...\n", 0);

			// search the base brush model
			for (auto ent = 0; ent < 1024; ent++)
			{
				if (!game::scr_g_entities[ent].r.bmodel)
				{
					continue;
				}

				// compare brushmodel targetname
				if (utils::q_stricmp(_ggame::ent_get_gscr_string(game::scr_g_entities[ent].targetname), "dynbrush01"))
				{
					continue;
				}

				// found it
				ent_found_bb_model = true;
				game::glob::dynamic_brush_models.brushes[0].entityIndex = ent;
				game::glob::dynamic_brush_models.brushes[0].ent = &game::scr_g_entities[ent];

				// save the original origin
				glm::set_float3(game::glob::dynamic_brush_models.brushes[0].originalOrigin, glm::to_vec3(game::glob::dynamic_brush_models.brushes[0].ent->r.currentOrigin));

				game::Com_PrintMessage(0, utils::va("|-> found base-brushmodel @ g_entities[%d]\n", ent), 0);
				game::glob::dynamic_brush_models.mapped_bmodels++;

				break;
			}

			// could not find the base model
			if (!ent_found_bb_model)
			{
				game::Com_PrintMessage(0, "|-> couldnt find the base-brushmodel!\n", 0);
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
					if (!game::scr_g_entities[ent].r.bmodel)
					{
						continue;
					}

					if (game::scr_g_entities[ent].target != game::glob::dynamic_brush_models.brushes[0].ent->targetname)
					{
						continue;
					}	

					// found a child
					game::glob::dynamic_brush_models.brushes[child].entityIndex = ent;
					game::glob::dynamic_brush_models.brushes[child].ent = &game::scr_g_entities[ent];

					// save the original origin
					glm::set_float3(game::glob::dynamic_brush_models.brushes[child].originalOrigin, glm::to_vec3(game::glob::dynamic_brush_models.brushes[child].ent->r.currentOrigin));

					mapped_children.push_back(ent);

					game::Com_PrintMessage(0, utils::va("|-> found child @ g_entities[%d]\n", ent), 0);
					game::glob::dynamic_brush_models.mapped_bmodels++;

					break;
				}
			}

			game::Com_PrintMessage(0, utils::va("|-> found %d/%d brushmodel entities!\n", mapped_children.size() + 1, DYN_COLL_BMODEL_AMOUNT), 0);

			std::vector<int> mapped_cmodels;

			// find the corrosponding cmodels in the clipmap
			for (auto bModel = 0; bModel < game::glob::dynamic_brush_models.mapped_bmodels; bModel++)
			{
				// should not happen
				if (bModel > (int)game::cm->numSubModels)
				{
					game::Com_PrintMessage(0, "|-> ^1mapped brushmodels > clipMap->numSubModels[%d]\n", 0);
					return;
				}

				// for each clipMap->cmodel
				for (auto cmod = 0; cmod < (int)game::cm->numSubModels; cmod++)
				{
					// if we already mapped the current cmodel
					if (std::find(mapped_cmodels.begin(), mapped_cmodels.end(), cmod) != mapped_cmodels.end())
					{
						continue;
					}

					// compare mins
					if (!utils::vector::compare3(game::cm->cmodels[cmod].mins, game::glob::dynamic_brush_models.brushes[bModel].ent->r.mins))
					{
						continue;
					}	

					// compare maxs
					if (!utils::vector::compare3(game::cm->cmodels[cmod].maxs, game::glob::dynamic_brush_models.brushes[bModel].ent->r.maxs))
					{
						continue;
					}	

					// found it
					game::glob::dynamic_brush_models.brushes[bModel].cmodelIndex = cmod;
					game::glob::dynamic_brush_models.brushes[bModel].cmodel = &game::cm->cmodels[cmod];

					mapped_cmodels.push_back(cmod);

					game::Com_PrintMessage(0, utils::va("|-> dynBrushModels.brushes[%d] :: cm->cmodel[%d] == g_entities[%d]\n", bModel, cmod, game::glob::dynamic_brush_models.brushes[bModel].entityIndex), 0);
					
					break;
				}
			}

			game::Com_PrintMessage(0, utils::va("|-> found %d/%d cmodels!\n", mapped_cmodels.size(), DYN_COLL_BMODEL_AMOUNT), 0);

			_map::create_debug_collision();
		}

		else
		{
			game::Com_PrintMessage(0, "CM_FindDynamicBrushModels :: No map loaded!\n", 0);
		}
	}

	void modify_dynamic_bmodel(game::dynBrushModel_t* bmodel, glm::vec3& origin, glm::vec3& local_mins, glm::vec3& local_maxs, [[maybe_unused]] glm::vec3& angles)
	{
		// Moving a brushmodel needs:
		// > trBase + origin (absmins / maxs calculated by SV_LinkEntity)
		// brushmodels are utterly fucked in this engine. Rotating them fucks up the collision. Happens with gsc too.

		// Entity Server :: origin (for visibility)
		_ggame::ent_instant_move_to(bmodel->ent, origin);

		// angles => not using entity rotation anymore (we chop the brush by its sides)
		//_Game::gEnt_RotateToInstant(bModel->ent, angles);

		// Entity Remote :: local bounds
		_ggame::ent_edit_brushmodel_bounds(bmodel, local_mins, local_maxs);

		// ~ relink this entity, because it may have moved out of the current leaf
		// Update the clipmap leafs and absolute world bounds
		game::SV_LinkEntity(bmodel->ent);

		game::cgs->nextSnap->numEntities;
		auto x = game::cg_entities;
		auto asd = game::cg_entities[game::glob::dynamic_brush_models.brushes[1].entityIndex];
	}

	// *
	// Remove saved brushes and sort debug brush array (keeping the current non-saved selection)
	bool brushcontainer_remove_saved()
	{
		memcpy(&_brush_sorting_container, &game::glob::radiant_saved_brushes.brush, sizeof(game::radiant_brush_s) * RADIANT_MAX_SEL_BRUSHES);

		const int curr_selected_count = game::glob::radiant_saved_brushes.selected_brush_count;
		int non_saved_selected_count = 0;

		for (auto b = 0; b < RADIANT_MAX_SEL_BRUSHES; b++)
		{
			if (curr_selected_count == non_saved_selected_count)
			{
				// found all non-saved selected brushes
				break;
			}

			if (!game::glob::radiant_saved_brushes.brush[b].is_saved && game::glob::radiant_saved_brushes.brush[b].face_count)
			{
				memcpy(&_brush_sorting_container[non_saved_selected_count], &game::glob::radiant_saved_brushes.brush[b], sizeof(game::radiant_brush_s));
				non_saved_selected_count++;
			}
		}

		if (curr_selected_count != non_saved_selected_count)
		{
			game::glob::radiant_saved_brushes.using_saved = false;
			game::glob::radiant_saved_brushes.saved_brush_count = 0;

			return false;
		}

		// copy new brushes into the struct
		memcpy(&game::glob::radiant_saved_brushes.brush[0], &_brush_sorting_container[0], sizeof(game::radiant_brush_s) * non_saved_selected_count);

		game::glob::radiant_saved_brushes.using_saved = false;
		game::glob::radiant_saved_brushes.saved_brush_count = 0;

		return true;
	}

	// *
	// meme: shader_names hashing func
	void brushcontainer_generate_hash_for_brush(int brushIdx)
	{
		uint32_t eax, ebx, ecx, val;
		ebx = 0; //ebx ^ ebx;

		for (auto f = 0; f < game::glob::radiant_saved_brushes.brush[brushIdx].face_count; f++)
		{
			val = (int)game::glob::radiant_saved_brushes.brush[brushIdx].face[f].winding_pts[0][0] + (int)game::glob::radiant_saved_brushes.brush[brushIdx].face[f].winding_pts[0][1] + (int)game::glob::radiant_saved_brushes.brush[brushIdx].face[f].winding_pts[0][2];
			ecx = val;
			eax = ebx;
			eax = eax << 5;
			ebx = ebx + eax;
			ebx = ebx ^ ecx;
		}

		game::glob::radiant_saved_brushes.brush[brushIdx].hash = ebx;
	}

	// *
	// compare hash of rad_savedBrushes.brush[brushIdx] against all saved brushes
	// returns true if brush is a saved brush
	bool brushcontainer_is_selection_saved(int brushIdx)
	{
		for (auto bHash = 0; bHash < game::glob::radiant_saved_brushes.saved_brush_count; bHash++)
		{
			if (game::glob::radiant_saved_brushes.brush[brushIdx].hash 
				&& game::glob::radiant_saved_brushes.brush[brushIdx].hash == game::glob::radiant_saved_brushes.brush[bHash].hash)
			{
				return true;
			}
		}

		return false;
	}

	// *
	// Draw brushes sent from radiant
	void radiant_livelink::draw_debug_brush()
	{
		// reset brush collision on first iteration after resetCollision was set
		if (game::glob::radiant_saved_brushes.reset_collision)
		{
			if (game::glob::dynamic_brush_models.initiated && !game::glob::radiant_saved_brushes.disabled_collision)
			{
				for (auto b = 0; b < game::glob::dynamic_brush_models.mapped_bmodels; b++)
				{
					_ggame::ent_brushmodel_set_collision(game::glob::dynamic_brush_models.brushes[b].ent, false);
					_ggame::ent_instant_move_to(game::glob::dynamic_brush_models.brushes[b].ent, glm::to_vec3(game::glob::dynamic_brush_models.brushes[b].originalOrigin));
				}

				game::glob::radiant_saved_brushes.disabled_collision = true;
				game::glob::radiant_saved_brushes.was_modified = true;
				game::glob::radiant_saved_brushes.reset_collision = false;
			}
		}

		// build 1 winding list for face[0 - faceCount] -> windingPts[0 - windingPtCount]
		if (game::glob::cgs_addons.radiant_livelink_connected && (game::glob::radiant_saved_brushes.any_selected || game::glob::radiant_saved_brushes.using_saved))
		{
			for (auto b = 0; b < game::glob::radiant_saved_brushes.selected_brush_count + game::glob::radiant_saved_brushes.saved_brush_count; b++)
			{
				if (b >= RADIANT_MAX_SEL_BRUSHES || game::glob::radiant_saved_brushes.brush[b].face_count == 0)
				{
					return;
				}

				// check for duplicate brushes (brush still selected after saving)
				if (!game::glob::radiant_saved_brushes.brush[b].is_saved)
				{
					// only generate a new hash when selection changed
					if (game::glob::radiant_saved_brushes.was_modified)
					{
						brushcontainer_generate_hash_for_brush(b);
					}

					if (brushcontainer_is_selection_saved(b))
					{
						// current selection is already saved
						continue;
					}
				}

				// create a hash for the current brush (so we can identify them)
				if (!game::glob::radiant_saved_brushes.brush[b].hash || game::glob::radiant_saved_brushes.was_modified)
				{
					brushcontainer_generate_hash_for_brush(b);
				}

				for (auto f = 0; f < game::glob::radiant_saved_brushes.brush[b].face_count; f++)
				{
					float winding_pts[16][3];

					for (auto coord = 0; coord < game::glob::radiant_saved_brushes.brush[b].face[f].winding_count; coord++)
					{
						// windings are reversed in radiant or the game, dunno
						int invcoord = coord;

						// comment to reverse faces
						invcoord = game::glob::radiant_saved_brushes.brush[b].face[f].winding_count - 1 - coord;

						// create the coord
						winding_pts[invcoord][0] = game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[coord][0];
						winding_pts[invcoord][1] = game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[coord][1];
						winding_pts[invcoord][2] = game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[coord][2];
					}

					// draw the face
					_debug::draw_poly(
						game::glob::radiant_saved_brushes.brush[b].face[f].winding_count, 
						winding_pts, 
						dvars::radiant_brushColor->current.vector, 
						dvars::radiant_brushLit->current.enabled, 
						dvars::radiant_brushWireframe->current.enabled, 
						dvars::radiant_brushWireframeColor->current.vector,
						true,
						false);
				}

				// *
				// Debug brush collision

				if (!dvars::radiant_brushCollision->current.enabled)
				{
					if (game::glob::dynamic_brush_models.initiated)
					{
						for (auto bb = 0; bb < game::glob::dynamic_brush_models.mapped_bmodels; bb++)
						{
							_ggame::ent_brushmodel_set_collision(game::glob::dynamic_brush_models.brushes[bb].ent, false);
							_ggame::ent_instant_move_to(game::glob::dynamic_brush_models.brushes[bb].ent, glm::to_vec3(game::glob::dynamic_brush_models.brushes[bb].originalOrigin));
						}

						game::glob::dynamic_brush_models.initiated = false;
					}
					
					continue; // we still want to draw all the brushes
				}

				// one time message that we failed to find the brushmodels + disable continuous searching
				if (!game::glob::dynamic_brush_models.mapped_bmodels)
				{
					if (!game::glob::dynamic_brush_models.initiated)
					{
						game::Com_PrintMessage(0, "^1[LiveLink]: ^7Disabled debug-brush collision. Could not find any dynamic brushmodels on the map!^7\n|-> ^1Make sure that you included the \"dynamic_collision_bmodels.map\" prefab within your map -> RE-BSP!", 0);
						game::glob::dynamic_brush_models.initiated = true;
					}

					continue; // we still want to draw all the brushes
					
				}

				if (b > game::glob::dynamic_brush_models.mapped_bmodels - 1)
				{
					// if we ran out of avail. brushmodels, stop updating collision and notify the user
					if (game::glob::radiant_saved_brushes.was_modified)
					{
						game::Com_PrintMessage(0, "^1[LiveLink]: ^7Not enough dynamic brushmodels available! Using oldest selection for collision ...\n", 0);
						game::glob::radiant_saved_brushes.was_modified = false;
					}

					return;
				}

				game::glob::dynamic_brush_models.initiated = true;

				// only update collision when a brush was modified
				if (game::glob::radiant_saved_brushes.was_modified)
				{
					game::glob::radiant_saved_brushes.disabled_collision = false;

					// enable collision
					_ggame::ent_brushmodel_set_collision(game::glob::dynamic_brush_models.brushes[b].ent, true);

					glm::vec3 b_mins(FLT_MAX, FLT_MAX, FLT_MAX);
					glm::vec3 b_maxs(-FLT_MAX, -FLT_MAX, -FLT_MAX);

					// get bounds from windings
					for (auto f = 0; f < game::glob::radiant_saved_brushes.brush[b].face_count; f++)
					{
						// *
						// CM_WorldToLocalBounds()

						// get brush mins/maxs (could be send from radiant directly but we already have the winding points so theres no point)
						for (auto winding = 0; winding < game::glob::radiant_saved_brushes.brush[b].face[f].winding_count; winding++)
						{
							// mins :: find the closest point on each axis
							if (b_mins.x > game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[winding][0])
								b_mins.x = game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[winding][0];

							if (b_mins.y > game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[winding][1])
								b_mins.y = game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[winding][1];

							if (b_mins.z > game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[winding][2])
								b_mins.z = game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[winding][2];

							// maxs :: find the furthest point on each axis
							if (b_maxs.x < game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[winding][0])
								b_maxs.x = game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[winding][0];

							if (b_maxs.y < game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[winding][1])
								b_maxs.y = game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[winding][1];

							if (b_maxs.z < game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[winding][2])
								b_maxs.z = game::glob::radiant_saved_brushes.brush[b].face[f].winding_pts[winding][2];
						}
					}

					// calculate the brush origin
					glm::vec3 b_origin((b_mins + b_maxs) * 0.5f);

					// create a cube that encupsules the whole brush (we chop it by radiant's planes later ... no need for entity rotation doing it that way)
					const float largest_bound_dist = fmaxf(fmaxf(glm::distance(b_maxs.x, b_mins.x), glm::distance(b_maxs.y, b_mins.y)), glm::distance(b_maxs.z, b_mins.z));

					glm::vec3 b_local_maxs(largest_bound_dist * 0.5f);
					glm::vec3 b_local_mins(-b_local_maxs);

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
					utils::vector::_VectorToAnglesWithRoll(angles, forwardVec, upVec, false);*/

					//radBrushIndexForDyn
					game::glob::dynamic_brush_models.brushes[b].radBrushIndexForDyn = b;

					// modify one of our brushmodels
					modify_dynamic_bmodel(&game::glob::dynamic_brush_models.brushes[b], b_origin, b_local_mins, b_local_maxs, angles);
				}
			}

#pragma warning(push)
#pragma warning(disable: 6385)
			// make all unused brushmodels non-colliding
			for (auto b = game::glob::radiant_saved_brushes.selected_brush_count + game::glob::radiant_saved_brushes.saved_brush_count; b < game::glob::dynamic_brush_models.mapped_bmodels; b++)
			{
				_ggame::ent_brushmodel_set_collision(game::glob::dynamic_brush_models.brushes[b].ent, false);
				_ggame::ent_instant_move_to(game::glob::dynamic_brush_models.brushes[b].ent, glm::to_vec3(game::glob::dynamic_brush_models.brushes[b].originalOrigin));
			}
#pragma warning(pop)

			game::glob::radiant_saved_brushes.was_modified = false;
		}

		// make brushmodels non-colliding till something is selected
		else if (game::glob::dynamic_brush_models.initiated && !game::glob::radiant_saved_brushes.disabled_collision)
		{
			for (auto b = 0; b < game::glob::dynamic_brush_models.mapped_bmodels; b++)
			{
				_ggame::ent_brushmodel_set_collision(game::glob::dynamic_brush_models.brushes[b].ent, false);
				_ggame::ent_instant_move_to(game::glob::dynamic_brush_models.brushes[b].ent, glm::to_vec3(game::glob::dynamic_brush_models.brushes[b].originalOrigin));
			}

			game::glob::radiant_saved_brushes.disabled_collision = true;
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
				if (utils::vector::_VectorCompare(Game::cm->brushes[cmBrush].mins, dynBrushStart_mins))
				{
					// check maxs
					if (utils::vector::_VectorCompare(Game::cm->brushes[cmBrush].maxs, dynBrushStart_maxs))
					{
						// we found our brush
						Game::Com_PrintMessage(0, utils::va("dynBrush[%d] @ cm->brushes[%d] :: mins = { %.1f %.1f %.1f } || maxs = { %.1f %.1f %.1f }\n", dynBrush, cmBrush,
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
									if (utils::vector::_VectorCompare(Game::cm->planes[cmPlane].normal, planeNormal))
									{
										// we found our plane; add it
										Game::Globals::dynBrushes.brushes[dynBrush].cmPlane[planeForSide] = &Game::cm->planes[cmPlane];

										Game::Com_PrintMessage(0, utils::va("|-> plane[%d] at cm->planes[%d] :: normal = { %.1f %.1f %.1f } || dist = { %.1f }\n", planeForSide, cmPlane,
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
							if (utils::vector::_VectorCompareEpsilon(Game::cm->leafs[cmLeaf].mins, Game::cm->brushes[cmBrush].mins, 0.125f))
							{
								// compare cmLeaf maxs with brush maxs
								if (utils::vector::_VectorCompareEpsilon(Game::cm->leafs[cmLeaf].maxs, Game::cm->brushes[cmBrush].maxs, 0.125f))
								{
									// we found the leaf
									Game::Globals::dynBrushes.brushes[dynBrush].cmLeaf = &Game::cm->leafs[cmLeaf];

									Game::Com_PrintMessage(0, utils::va("|-> leaf at cm->leafs[%d] :: mins = { %.1f %.1f %.1f } || maxs = { %.1f %.1f %.1f }\n", cmLeaf,
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

		Game::Com_PrintMessage(0, utils::va("Found (%d) dynBrushes\n", dynBrushesFound), 0);
	}
#endif

	// ------------------------------
	// Using information from radiant
	// *Map-Exporting* -> "RB_DrawCollision"

	game::gentity_s *_livelink_cam_model;

	// check if the camera model entity is valid (player changed level etc.)
	bool is_camera_model_valid()
	{
		// if not spawned an entity yet
		if(!game::glob::cgs_addons.radiant_camera_model_spawned)
		{
			return false;
		}

		// check if the entity is valid (player changed level etc.)
		if (_livelink_cam_model == nullptr || _livelink_cam_model->classname == 0 || _livelink_cam_model->model != game::G_ModelIndex("xo_radiantcam"))
		{
			return false;
		}

		return true;
	}

	// spawn the camera model
	void spawn_camera()
	{
		// needs :: 
		// s->index = modelIndex
		// linked = 0x1;
		// svFlags = 0x04; // even = visible, uneven = hidden

		const std::int16_t model_index = game::G_ModelIndex("xo_radiantcam"); // axis

		_livelink_cam_model = game::G_Spawn();
		_livelink_cam_model->model = model_index;
		_livelink_cam_model->s.index = model_index;
		_livelink_cam_model->r.svFlags = 0x04;
		_livelink_cam_model->r.linked = 0x1;

		game::G_SetOrigin(_livelink_cam_model, game::glob::cgs_addons.radiant_camera_origin);
		game::G_SetAngles(_livelink_cam_model, game::glob::cgs_addons.radiant_camera_angles);

		game::G_CallSpawnEntity(_livelink_cam_model);

		//Radiant_CamModelSpawned = true;
		game::glob::cgs_addons.radiant_camera_model_spawned = true;
	}

	// spawn/update the radiant cam model when not using syncCamera or player moved
	void update_camera_pos()
	{
		if(game::glob::cgs_addons.radiant_camera_model_spawned)
		{
			_livelink_cam_model->r.svFlags = 0x04; // visible
			game::G_SetOrigin(_livelink_cam_model, game::glob::cgs_addons.radiant_camera_origin);
			game::G_SetAngles(_livelink_cam_model, game::glob::cgs_addons.radiant_camera_angles);
		}
	}

	// :: CG_CalcViewValues :: directly set refdef from parsed radiant camera commands
	void camera_frame()
	{
		game::usercmd_s cmd = game::usercmd_s();

		// spawn a camera model on first camera move or if invalid
		if (!is_camera_model_valid())
		{
			spawn_camera();
		}

		// update the camera model if not using the radiant camera to change the players viewpos
		if (dvars::radiant_syncCamera->current.integer == 0 || dvars::radiant_syncCamera->current.integer == 2)
		{
			update_camera_pos();

			// helper bool for our asm stub
			game::glob::cgs_addons.radiant_camera_sync_enabled = false;
			game::glob::cgs_addons.radiant_camera_in_use = false;

			return;
		}

		game::glob::cgs_addons.radiant_camera_sync_enabled = true;
		
		
		// show the camera model when the player starts to move (resetting the viewpos)
		if (game::CL_GetUserCmd(game::clients->cmdNumber, &cmd))
		{
			if ((abs(cmd.forwardmove) > 50 || abs(cmd.rightmove) > 50))
			{
				// radiantCamInUse gets set on every received camera command 
				game::glob::cgs_addons.radiant_camera_in_use = false;

				// show the camera model on player move
				_livelink_cam_model->r.svFlags = 0x04;

				// update the camera model pos once
				update_camera_pos();
					
				return;
			}
		}

		// hide camera model when radiant is moving the players view
		_livelink_cam_model->r.svFlags = 0x01;

		// player view
		game::cgs->refdefViewAngles[0] = game::glob::cgs_addons.radiant_camera_angles[0];
		game::cgs->refdefViewAngles[1] = game::glob::cgs_addons.radiant_camera_angles[1];
		game::cgs->refdefViewAngles[2] = game::glob::cgs_addons.radiant_camera_angles[2];

		utils::vector::angles_to_axis(game::glob::cgs_addons.radiant_camera_angles, game::cgs->refdef.viewaxis);

		game::cgs->refdef.vieworg[0] = game::glob::cgs_addons.radiant_camera_origin[0];
		game::cgs->refdef.vieworg[1] = game::glob::cgs_addons.radiant_camera_origin[1];
		game::cgs->refdef.vieworg[2] = game::glob::cgs_addons.radiant_camera_origin[2];

		//Game::Com_PrintMessage(0, utils::va("^1[CAM:R2G]: Angles: ( %.1f, %.1f, %.1f )  Origin: ( %.1f, %.1f, %.1f )\n",
		//	Game::Globals::cgs_addons.radiantCameraAngles[0], Game::Globals::cgs_addons.radiantCameraAngles[1], Game::Globals::cgs_addons.radiantCameraAngles[2],
		//	Game::Globals::cgs_addons.radiantCameraOrigin[0], Game::Globals::cgs_addons.radiantCameraOrigin[1], Game::Globals::cgs_addons.radiantCameraOrigin[2]), 0);

	}

	// :: Radiant_UpdateGameCamera
	__declspec(naked) void CG_CalcViewValues_stub()
	{
		const static uint32_t CG_CalcTurretViewValues_func = 0x4507B0;
		const static uint32_t retn_addr_stock = 0x451C69; // back to cmp cgs_renderingThirdPerson, 0
		const static uint32_t retn_addr = 0x451C7C; // back to CG_ShakeCamera
		__asm
		{
			call	CG_CalcTurretViewValues_func;	// overwritten op

			cmp		game::glob::cgs_addons.radiant_camera_in_use, 0;
			je		STOCK;

			pushad;
			call	camera_frame;		// update the radiant cam on camera move
			popad;

			// if we do not sync the camera <> viewpos but only the camera model position
			cmp		game::glob::cgs_addons.radiant_camera_sync_enabled, 0;
			je		STOCK;

			jmp		retn_addr;

		STOCK:
			jmp		retn_addr_stock;
		}
	}

	// radiantCamInUse :: disable CG_ApplyViewAnimation in CG_CalcViewValues (leave viewmodel at player origin if radiantcam is enabled)
	__declspec(naked) void CG_ApplyViewAnimation_stub()
	{
		const static uint32_t CG_ApplyViewAnimation_func = 0x450890;
		const static uint32_t retn_addr = 0x451C97; // back to next op
		__asm
		{
			cmp		game::glob::cgs_addons.radiant_camera_in_use, 0;
			je		STOCK;
			cmp		game::glob::cgs_addons.radiant_camera_sync_enabled, 0; // not needed
			je		STOCK;

			// if radiantCamInUse : skip CG_ApplyViewAnimation
			jmp		retn_addr;

		STOCK:
			call	CG_ApplyViewAnimation_func;
			jmp		retn_addr;
		}
	}

	// ++++++++++++++++++++++++++++
	// ++++++++++++++++++++++++++++

	SOCKET _livelink_server_socket = INVALID_SOCKET;
	SOCKET _livelink_client_socket = INVALID_SOCKET;

	// ---------------
	// Sending packets 

	int _livelink_commands_send = 0;

	// *
	// Send commands to the client
	void radiant_livelink::send_packet(game::radiant_server_command_s *cmd)
	{
		// check if radiant is connected to the server
		if (_livelink_client_socket == INVALID_SOCKET || _livelink_server_socket == INVALID_SOCKET)
		{
			return;
		}

		const int ret = send(_livelink_client_socket, (const char *)cmd, sizeof(game::radiant_server_command_s), 0);
		if (ret == SOCKET_ERROR)
		{
			game::Com_PrintMessage(0, "^1[LiveLink]: ^7Sending msg to client failed!\n", 0);
			return;

			//closesocket(g_RemoteSocket);
			//g_RemoteSocket = INVALID_SOCKET;
			//g_RemoteSocketStatus = INVALID_SOCKET;
		}

		if (dvars::radiant_liveDebug->current.enabled)
		{
			_livelink_commands_send++;

			std::string cmd_type;

			switch (cmd->type)
			{
			case 0:
				cmd_type = "SERVER_STRING_MSG";
				break;
			case 10:
				cmd_type = "SERVER_CAMERA_UPDATE";
				break;
			case 20:
				cmd_type = "SERVER_EXPORT_SINGLE_BRUSH_FACE_INDEX";
				break;
			case 21:
				cmd_type = "SERVER_EXPORT_SINGLE_BRUSH_FACE";
				break;
			
			default:
				cmd_type = "COMMAND_TYPE_UNKOWN";
				break;
			}

			game::Com_PrintMessage(0, utils::va("Send command num: [%d] of type: [%s]\n", _livelink_commands_send,  cmd_type.c_str()), 0);
		}
	}

	// *
	// called from _Pmove
	void radiant_livelink::cmd_send_camera(const float *origin, const float *angles)
	{
		game::radiant_server_command_s cmd = {};
		cmd.type = game::SERVER_CAMERA_UPDATE;

		sprintf_s(cmd.command, "%.1f %.1f %.1f %.1f %.1f %.1f",
					origin[0], origin[1], origin[2], angles[0], angles[1], angles[2]);

		radiant_livelink::send_packet(&cmd);
	}

	// -----------------
	// Receiving packets 

	// *
	// process dvars
	void radiant_livelink::cmd_process_dvar(game::SpawnVar* spawn_var)
	{
		if (const auto dvarname_string = _ggame::get_spawnvar_pair_value(spawn_var, "dvarname"))
		{
			if (const auto value_string = _ggame::get_spawnvar_pair_value(spawn_var, "value"))
			{
				game::Cmd_ExecuteSingleCommand(0, 0, utils::va("%s %s\n", dvarname_string, value_string));
			}
		}
	}

	// *
	// process camera commands from radiant
	void radiant_livelink::cmd_process_camera(game::SpawnVar *spawn_var)
	{
		if (const auto origin_string = _ggame::get_spawnvar_pair_value(spawn_var, "origin"))
		{
			if (!sscanf(origin_string, "%f %f %f", &game::glob::cgs_addons.radiant_camera_origin[0], &game::glob::cgs_addons.radiant_camera_origin[1], &game::glob::cgs_addons.radiant_camera_origin[2]))
			{
				game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_CAMERA_ORIGIN", 0);
			}
		}

		if (const auto angles_string = _ggame::get_spawnvar_pair_value(spawn_var, "angles"))
		{
			if (!sscanf(angles_string, "%f %f %f", &game::glob::cgs_addons.radiant_camera_angles[0], &game::glob::cgs_addons.radiant_camera_angles[1], &game::glob::cgs_addons.radiant_camera_angles[2]))
			{
				game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_CAMERA_ANGLES", 0);
			}
		}

		game::glob::cgs_addons.radiant_camera_angles[0] = -game::glob::cgs_addons.radiant_camera_angles[0];
		game::glob::cgs_addons.radiant_camera_in_use = true;

		if (dvars::radiant_liveDebug->current.enabled)
		{
			game::Com_PrintMessage(0, utils::va("[CAM:R2G]: Angles: ( %.1f, %.1f, %.1f )  Origin: ( %.1f, %.1f, %.1f )\n",
				game::glob::cgs_addons.radiant_camera_angles[0], game::glob::cgs_addons.radiant_camera_angles[1], game::glob::cgs_addons.radiant_camera_angles[2],
				game::glob::cgs_addons.radiant_camera_origin[0], game::glob::cgs_addons.radiant_camera_origin[1], game::glob::cgs_addons.radiant_camera_origin[2]), 0);
		}
	}

	// on each selection (once)
	// bool :: if valid selection or nothing selected
	void radiant_livelink::cmd_process_brush_select(game::SpawnVar *spawn_var)
	{
		if (const auto brush_select_string = _ggame::get_spawnvar_pair_value(spawn_var, "brushselect"))
		{
			int temp;

			if (!sscanf(brush_select_string, "%d", &temp))
			{
				game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_SELECT", 0);
			}
			else
			{
				game::glob::radiant_saved_brushes.any_selected = temp;
			}

			// reset brushSelectedCount when selecting / deselecting
			game::glob::radiant_saved_brushes.selected_brush_count = 0;
		}
	}

	// on each selection (once)
	// int :: amount of selected brushes
	void radiant_livelink::cmd_process_brush_amount(game::SpawnVar *spawn_var)
	{
		if (const auto brush_count_string = _ggame::get_spawnvar_pair_value(spawn_var, "brushcount"))
		{
			if (!sscanf(brush_count_string, "%d", &game::glob::radiant_saved_brushes.selected_brush_count))
			{
				game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_COUNT", 0);
			}

			// cap brush selection count
			if (game::glob::radiant_saved_brushes.selected_brush_count + game::glob::radiant_saved_brushes.saved_brush_count > RADIANT_MAX_SEL_BRUSHES)
			{
				game::glob::radiant_saved_brushes.selected_brush_count = RADIANT_MAX_SEL_BRUSHES;
			}

			// update collision of all brushmodels
			game::glob::radiant_saved_brushes.was_modified = true;
		}
	}

	// *
	// clear the last saved brush transmitted from radiant
	void brushcontainer_clear_saved_brush(int brush_num)
	{
		// reset the face index
		game::glob::radiant_saved_brushes.brush[brush_num].next_free_face_idx = 0;

		// clear brush-faces and face-windingpoints
		memset(game::glob::radiant_saved_brushes.brush[brush_num].face, 0, sizeof(game::glob::radiant_saved_brushes.brush[brush_num].face[16]));
	}

	// per brush
	// int :: the index of the current brush being transmitted
	void radiant_livelink::cmd_process_brush_num(game::SpawnVar *spawnVar)
	{
		if (const auto brushnum_string = _ggame::get_spawnvar_pair_value(spawnVar, "brushnum"))
		{
			if (!sscanf(brushnum_string, "%d", &game::glob::radiant_saved_brushes.selected_brush_num))
			{
				game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_CURRENT_NUM", 0);
			}
			else
			{
				// add new brushes after saved brushes
				game::glob::radiant_saved_brushes.selected_brush_num += game::glob::radiant_saved_brushes.saved_brush_count;
			}
		}
	}

	// per brush
	// int :: how many faces we transmit in total
	void radiant_livelink::cmd_process_brush_face_count(game::SpawnVar *spawnVar, int brush_num)
	{
		if (const auto brush_face_count_str = _ggame::get_spawnvar_pair_value(spawnVar, "brushfacecount"))
		{
			// set brush face-count
			if (!sscanf(brush_face_count_str, "%d", &game::glob::radiant_saved_brushes.brush[brush_num].face_count))
			{
				game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_FACE_COUNT", 0);
			}

			// clear old brush if we got atleast one valid face
			if (game::glob::radiant_saved_brushes.brush[brush_num].face_count)
			{
				brushcontainer_clear_saved_brush(brush_num);
			}
		}
	}

	// per brush-face
	// vec3_t :: face-normal (sent before windings)
	void radiant_livelink::cmd_process_brush_face_normals(game::SpawnVar *spawn_var, int brush_num)
	{
		// next free face in rad_savedBrushes.brush.face (0 on new brush, ++ on each new face (do not increment here!))
		const int next_free = game::glob::radiant_saved_brushes.brush[brush_num].next_free_face_idx;

		if (const auto brush_face_normal_str = _ggame::get_spawnvar_pair_value(spawn_var, "normal"))
		{
			if (!sscanf(brush_face_normal_str, "%f %f %f",
				&game::glob::radiant_saved_brushes.brush[brush_num].face[next_free].normal[0],
				&game::glob::radiant_saved_brushes.brush[brush_num].face[next_free].normal[1],
				&game::glob::radiant_saved_brushes.brush[brush_num].face[next_free].normal[2]))
			{
				game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_FACE_NORMALS", 0);
			}
		}

		if (const auto brush_face_dist_str = _ggame::get_spawnvar_pair_value(spawn_var, "dist"))
		{
			if (!sscanf(brush_face_dist_str, "%f", &game::glob::radiant_saved_brushes.brush[brush_num].face[next_free].dist))
			{
				game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_FACE_NORMALS_DIST", 0);
			}
		}
	}

	// per brush-face
	// int / vec3 :: how many windings per face we transmit + the points themselfs 
	void radiant_livelink::cmd_process_brush_face(game::SpawnVar *spawnVar, int brushNum)
	{
		// next free face in rad_savedBrushes.brush.face (0 on new brush, ++ on each new face)
		const int next_free = game::glob::radiant_saved_brushes.brush[brushNum].next_free_face_idx;

		// get the amount of winding points for the current face
		if (const auto brush_winding_count_str = _ggame::get_spawnvar_pair_value(spawnVar, "windingcount"))
		{
			if (!sscanf(brush_winding_count_str, "%d", &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_count))
			{
				game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_FACE_WINDING_COUNT", 0);
			}
		}

		// build winding points for the current face
		if (const auto brush_windings_str = _ggame::get_spawnvar_pair_value(spawnVar, "windingpoints"))
		{
			if (!sscanf(brush_windings_str, "%f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f  %f %f %f",
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[0][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[0][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[0][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[1][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[1][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[1][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[2][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[2][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[2][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[3][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[3][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[3][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[4][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[4][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[4][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[5][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[5][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[5][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[6][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[6][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[6][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[7][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[7][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[7][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[8][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[8][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[8][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[9][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[9][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[9][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[10][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[10][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[10][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[11][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[11][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[11][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[12][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[12][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[12][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[13][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[13][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[13][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[14][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[14][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[14][2],
				&game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[15][0], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[15][1], &game::glob::radiant_saved_brushes.brush[brushNum].face[next_free].winding_pts[15][2]))
			{
				game::Com_PrintMessage(0, "[!]: sscanf failed to read command of type : RADIANT_COMMAND_BRUSH_FACE_WINDING_POINTS", 0);
			}
		
			// next free face
			game::glob::radiant_saved_brushes.brush[brushNum].next_free_face_idx++;
		}
	}

	// *
	// Check radiant connection status - accept / reject
	bool radiant_livelink::update_socket()
	{
		// Skip this function if client already connected
		if (_livelink_client_socket != INVALID_SOCKET)
		{
			if (!game::glob::cgs_addons.radiant_livelink_connected)
			{
				game::Com_PrintMessage(0, "^2[LiveLink]: ^7Client connected!\n", 0);
				game::glob::cgs_addons.radiant_livelink_connected = true;
			}

			return true;
		}

		if (_livelink_server_socket == INVALID_SOCKET)
		{
			return false;
		}

		// Check if there's a pending client connection request
		fd_set read_set;
		FD_ZERO(&read_set);
		FD_SET(_livelink_server_socket, &read_set);

		// Zero timeout (poll)
		const timeval timeout = { 0, 0 };
		const int status = select(_livelink_server_socket, &read_set, nullptr, nullptr, &timeout);

		if (status == SOCKET_ERROR)
		{
			game::Com_Error(0, "^1[LiveLink]: ^7Failed to query socket status!\n");
		}

		// Must be 1 (handle) if someone is waiting
		if (status != 1)
		{
			return false;
		}

		_livelink_client_socket = accept(_livelink_server_socket, nullptr, nullptr);

		if (_livelink_client_socket == INVALID_SOCKET)
		{
			game::Com_Error(0, "^1[LiveLink]: ^7Failed to accept a connection!?\n");
		}

		// Set non-blocking flag
		u_long socket_mode = 1;
		ioctlsocket(_livelink_client_socket, FIONBIO, &socket_mode);

		game::Com_PrintMessage(0, "^2[LiveLink]: ^7Client connected!\n", 0);
		game::glob::cgs_addons.radiant_livelink_connected = true;


		// ---------

		game::radiant_server_command_s cmd = {};
		cmd.type = game::SERVER_STRING_MSG;

		const std::string build_date = IW3XO_BUILDVERSION_DATE;
		const std::string build_num = std::to_string((int)IW3X_BUILDNUMBER);
		sprintf_s(cmd.command, utils::va("(IW3XO::%s::%s) accepted request", build_num.c_str(), build_date.c_str()));

		radiant_livelink::send_packet(&cmd);

		// ---------

		return true;
	}

	game::radiant_command_s _recv_commands[1024];

	// *
	// Receive packets from radiant (would normally run within the server thread @ 20fps)
	// Current networking only supports local usage, so why not increase the receiving rate by calling it from pmove
	void radiant_livelink::receive_packets()
	{
		if (!dvars::radiant_live || !dvars::radiant_live->current.enabled || !radiant_livelink::update_socket())
		{
			game::glob::cgs_addons.radiant_livelink_connected = false;
			return;
		}

		// Non-blocking read
		memset(_recv_commands, 0, sizeof(_recv_commands));

		const int recv_size = recv(_livelink_client_socket, (char *)&_recv_commands, sizeof(_recv_commands), 0);

		// Skip everything if there's no data
		if (recv_size == SOCKET_ERROR) 
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				return;
			}
				
			// Some other problem occurred and now the socket is bad
			shutdown(_livelink_client_socket, 2 /*SD_BOTH*/);
			closesocket(_livelink_client_socket);

			_livelink_client_socket = INVALID_SOCKET;
			game::Com_PrintMessage(0, "^1[LiveLink]: ^7Client disconnected!\n", 0);

			return;
		}

		// Determine the number of commands sent, then tell the game
		const size_t command_count = recv_size / sizeof(game::radiant_command_s); 

		for (size_t i = 0; i < command_count; i++)
		{
			game::SpawnVar spawn_var = {};
			
			// set start and end of string command
			_ggame::set_entity_parse_point(_recv_commands[i].command);

			// parse it
			_ggame::parse_spawn_vars(&spawn_var);

			// parse the following commands in order (see IW3xRadiant :: onBrush_SelectDeselect) 
			// parse select first (once)
			if (_recv_commands[i].type == game::RADIANT_COMMAND_BRUSH_SELECT)
			{
				radiant_livelink::cmd_process_brush_select(&spawn_var);

				if (dvars::radiant_liveDebug->current.enabled)
				{
					game::Com_PrintMessage(0, "^2[LiveLink]: ^7Received command of type (RADIANT_COMMAND_BRUSH_SELECT).\n", 0);
				}
			}

			// amount of selected brushes (once)
			if (_recv_commands[i].type == game::RADIANT_COMMAND_BRUSH_COUNT)
			{
				radiant_livelink::cmd_process_brush_amount(&spawn_var);

				if (dvars::radiant_liveDebug->current.enabled)
				{
					game::Com_PrintMessage(0, "^2[LiveLink]: ^7Received command of type (RADIANT_COMMAND_BRUSH_COUNT).\n", 0);
				}
			}

			// on each brush
			// per brush current brush number (once)
			if (_recv_commands[i].type == game::RADIANT_COMMAND_BRUSH_CURRENT_NUM)
			{
				radiant_livelink::cmd_process_brush_num(&spawn_var);

				if (dvars::radiant_liveDebug->current.enabled)
				{
					game::Com_PrintMessage(0, "^2[LiveLink]: ^7Received command of type (RADIANT_COMMAND_BRUSH_CURRENT_NUM).\n", 0);
				}
			}

			// ^ above adds nextFree to selectedNum
			if (game::glob::radiant_saved_brushes.selected_brush_num < RADIANT_MAX_SEL_BRUSHES)
			{
				// on each brush
				// per brush face-count (once)
				if (_recv_commands[i].type == game::RADIANT_COMMAND_BRUSH_FACE_COUNT)
				{
					radiant_livelink::cmd_process_brush_face_count(&spawn_var, game::glob::radiant_saved_brushes.selected_brush_num);

					if (dvars::radiant_liveDebug->current.enabled)
					{
						game::Com_PrintMessage(0, "^2[LiveLink]: ^7Received command of type (RADIANT_COMMAND_BRUSH_FACE_COUNT).\n", 0);
					}
				}

				// on each brush
				// per brush-face normal (face 1 and 3 only) ... all of them :o
				if (_recv_commands[i].type == game::RADIANT_COMMAND_BRUSH_FACE_NORMALS)
				{
					radiant_livelink::cmd_process_brush_face_normals(&spawn_var, game::glob::radiant_saved_brushes.selected_brush_num);

					if (dvars::radiant_liveDebug->current.enabled)
					{
						game::Com_PrintMessage(0, "^2[LiveLink]: ^7Received command of type (RADIANT_COMMAND_BRUSH_FACE_NORMALS).\n", 0);
					}
				}

				// on each brush
				// parse all of the brush faces (runs multiple times as we send 1 cmd for each face)
				if (_recv_commands[i].type == game::RADIANT_COMMAND_BRUSH_FACE)
				{
					radiant_livelink::cmd_process_brush_face(&spawn_var, game::glob::radiant_saved_brushes.selected_brush_num);

					if (dvars::radiant_liveDebug->current.enabled)
					{
						game::Com_PrintMessage(0, "^2[LiveLink]: ^7Received command of type (RADIANT_COMMAND_BRUSH_FACE).\n", 0);
					}
				}
			}

			// -----------

			// parse cammera commands
			if (_recv_commands[i].type == game::RADIANT_COMMAND_CAMERA)
			{
				radiant_livelink::cmd_process_camera(&spawn_var);
			}

			// parse dvar
			if (_recv_commands[i].type == game::RADIANT_COMMAND_SET_DVAR)
			{
				radiant_livelink::cmd_process_dvar(&spawn_var);
			}
		}
	}

	// *
	// not needed anymore, see above (stays here for later usage)
	void SV_PostFrame()
	{
		//RadiantRemote::SV_ReceivePackets();
	}

	__declspec(naked) void SV_PostFrame_stub()
	{
		const static uint32_t SV_CheckTimeouts_func = 0x532D20;
		const static uint32_t retn_addr = 0x5335B3; // SV_SendClientMessages call
		__asm
		{
			pushad
			Call	SV_PostFrame
			popad

			Call	SV_CheckTimeouts_func // overwritten op
			jmp		retn_addr
		}
	}


	// -----------
	// Server Init

	// *
	// Shutdown live-link server
	void radiant_livelink::shutdown_livelink()
	{
		shutdown(_livelink_server_socket, 2 /*SD_BOTH*/);
		closesocket(_livelink_server_socket);

		shutdown(_livelink_client_socket, 2 /*SD_BOTH*/);
		closesocket(_livelink_client_socket);

		_livelink_server_socket = INVALID_SOCKET;
		_livelink_client_socket = INVALID_SOCKET;

		game::glob::cgs_addons.radiant_livelink_connected = false;

		game::Com_PrintMessage(0, "^1[LiveLink]: ^7Shutdown!\n", 0);
	}

	// *
	// Start live-link server on G_InitGame
	void radiant_livelink::init()
	{
		if (_livelink_server_socket != INVALID_SOCKET)
		{
			return;
		}

		WSADATA wsa_data;
		if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR)
		{
			game::Com_Error(0, "^1[LiveLink]: ^7Socket startup failed!\n");
		}
		
		// Create a TCP server socket
		_livelink_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 

		if (_livelink_server_socket == INVALID_SOCKET)
		{
			game::Com_Error(0, "^1[LiveLink]: ^7Socket creation failed!\n");
		}
			
		// Bind socket to any local address on port X
		sockaddr_in addrIn = {};
		addrIn.sin_family = AF_INET;
		addrIn.sin_port = htons((u_short)dvars::radiant_livePort->current.integer);
		addrIn.sin_addr.s_addr = inet_addr("127.0.0.1");

		game::Com_PrintMessage(0, utils::va("^2[LiveLink]: ^7Attempting to bind on port %d ... \n", static_cast<int>(ntohs(addrIn.sin_port))), 0);

		if (bind(_livelink_server_socket, (SOCKADDR *)&addrIn, sizeof(addrIn)) == SOCKET_ERROR)
		{
			game::Com_PrintMessage(0, "^1[LiveLink]: ^7Failed to bind socket. Port in use? ~> radiant_livePort\n", 0);

			radiant_livelink::shutdown_livelink();

			return;
		}

		// Listen for any number of incoming connections
		if (listen(_livelink_server_socket, SOMAXCONN) == SOCKET_ERROR)
		{
			game::Com_PrintMessage(0, "^1[LiveLink]: ^7Failed to listen for incoming connections!\n", 0);

			radiant_livelink::shutdown_livelink();
			return;
		}

		game::Com_PrintMessage(0, "^2[LiveLink]: ^7Succeeded!\n", 0);
	}

	// :: RadiantRemote::SV_Init()
	__declspec(naked) void G_InitGame_stub()
	{
		const static uint32_t SaveRegisteredItems_func = 0x4BCBE0;
		const static uint32_t retn_addr = 0x4BF5A8; // next op after hook loc
		__asm
		{
			Call	SaveRegisteredItems_func

			pushad
			Call	radiant_livelink::init
			popad

			jmp		retn_addr
		}
	}

	// *
	// *

	void radiant_livelink::devgui_tab(game::gui_menus_t& menu)
	{
		if (ImGui::CollapsingHeader("General Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			ImGui::Checkbox("Enable Radiant Live-Link", gui::dvar_get_set<bool*>(dvars::radiant_live)); TT("radiant_live");

			ImGui::SameLine();
			ImGui::Checkbox("Enable Live-Link Debug", gui::dvar_get_set<bool*>(dvars::radiant_liveDebug)); TT("radiant_liveDebug");

			SPACING(0.0f, 4.0f);

			ImGui::InputInt("Live-Link Port", gui::dvar_get_set<int*>(dvars::radiant_livePort)); TT("radiant_livePort");

			const char* radiant_cam_items[] = { "Disabled", "Radiant To Game", "Game To Radiant", "Both" };
			ImGui::Combo("Camera Synchronization", gui::dvar_get_set<int*>(dvars::radiant_syncCamera), radiant_cam_items, IM_ARRAYSIZE(radiant_cam_items)); TT("radiant_syncCamera");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		if (ImGui::CollapsingHeader("Brush Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			ImGui::Checkbox("Brush Collision", gui::dvar_get_set<bool*>(dvars::radiant_brushCollision)); TT("radiant_brushCollision");

			ImGui::SameLine();
			ImGui::Checkbox("Brush Fake-Light", gui::dvar_get_set<bool*>(dvars::radiant_brushLit)); TT("radiant_brushLit");

			ImGui::SameLine();
			ImGui::Checkbox("Brush Wireframe", gui::dvar_get_set<bool*>(dvars::radiant_brushWireframe)); TT("radiant_brushWireframe");

			SPACING(0.0f, 4.0f);

			ImGui::ColorEdit4("Brush Color", gui::dvar_get_set<float*>(dvars::radiant_brushColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("radiant_brushColor");

			ImGui::ColorEdit4("Wireframe Color", gui::dvar_get_set<float*>(dvars::radiant_brushWireframeColor), ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf); TT("radiant_brushWireframeColor");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}

		if (ImGui::CollapsingHeader("Commands", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(8.0f); SPACING(0.0f, 4.0f);

			if (ImGui::Button("Save Current Selection"))
			{
				CMDEXEC("radiant_saveSelection");
			} TT("radiant_saveSelection");

			ImGui::SameLine();
			if (ImGui::Button("Clear Saved Selection"))
			{
				CMDEXEC("radiant_clearSaved");
			} TT("radiant_clearSaved");

			SPACING(0.0f, 4.0f); ImGui::Indent(-8.0f);
		}
	}

	// *
	// *
	
	radiant_livelink::radiant_livelink()
	{
		command::add("radiant_saveSelection", [&](auto)
		{
			if (game::glob::cgs_addons.radiant_livelink_connected)
			{
				if (game::glob::radiant_saved_brushes.selected_brush_count)
				{
					if (game::glob::radiant_saved_brushes.was_modified)
					{
						game::Com_PrintMessage(0, "No new brushes selected!\n", 0);
						return;
					}

					// first iter bool
					game::glob::radiant_saved_brushes.reset_collision = true;

					memcpy(&_brush_sorting_container, &game::glob::radiant_saved_brushes.brush, sizeof(game::radiant_brush_s) * RADIANT_MAX_SEL_BRUSHES);

					const int first_free_saved_brush_index = game::glob::radiant_saved_brushes.saved_brush_count;
					
					int free_saved_brushes_count = game::glob::radiant_saved_brushes.selected_brush_count;

					if (free_saved_brushes_count + first_free_saved_brush_index >= RADIANT_MAX_SEL_BRUSHES)
					{
						free_saved_brushes_count = RADIANT_MAX_SEL_BRUSHES - 1 - first_free_saved_brush_index;
					}

					// clear all saved brushes first
					memset(&game::glob::radiant_saved_brushes.brush[0], 0, sizeof(game::radiant_brush_s) * RADIANT_MAX_SEL_BRUSHES);

					// copy new brushes into the struct
					memcpy(&game::glob::radiant_saved_brushes.brush[0], &_brush_sorting_container[first_free_saved_brush_index], sizeof(game::radiant_brush_s) * free_saved_brushes_count);
				
					game::glob::radiant_saved_brushes.using_saved = true;
					game::glob::radiant_saved_brushes.saved_brush_count = game::glob::radiant_saved_brushes.selected_brush_count;

					for(auto bSaved = 0; bSaved < game::glob::radiant_saved_brushes.saved_brush_count; bSaved++)
					{
						game::glob::radiant_saved_brushes.brush[bSaved].is_saved = true;
					}

					game::Com_PrintMessage(0, "Saved selection!\n", 0);
				}
				else
				{
					game::Com_PrintMessage(0, "^3No brushes selected!\n", 0);
				}
			}
			else
			{
				game::Com_PrintMessage(0, "^3Radiant Live-Link not active!\n", 0);
			}
		});

		command::add("radiant_clearSaved", [&](auto)
		{
			if (game::glob::cgs_addons.radiant_livelink_connected)
			{
				if (game::glob::radiant_saved_brushes.saved_brush_count)
				{
					if (!brushcontainer_remove_saved())
					{
						game::Com_PrintMessage(0, "Cleared! No non-saved brushes found.\n", 0);
						return;
					}

					game::Com_PrintMessage(0, "Cleared! Found non-saved brushes, recalculating collision.\n", 0);
					game::glob::radiant_saved_brushes.reset_collision = true;
				}
				else
				{
					game::Com_PrintMessage(0, "^3No brushes to clear!\n", 0);
				}
			}
			else
			{
				game::Com_PrintMessage(0, "^3Radiant Live-Link not active!\n", 0);
			}
		});

		/*radiant_connect*/ #if 0
		Command::Add("radiant_connect", [&](auto)
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
			args = utils::explode(params[1], ':');

			if (args.size() != 2)
			{
				Game::Com_PrintMessage(0, "Usage :: radiant_connect <ip:port>\n", 0);
				return;
			}

			std::string r_connectIp = args[0];
			int r_connectPort = utils::try_stoi(args[1], false);

			WSADATA wsaData;

			if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
			{
				Game::Com_PrintMessage(0, "^1[LiveLink]: WSAStartup ERROR!\n", 0);
				return;
			}

			if (!dvars::radiant_live->current.enabled)
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "radiant_live 1\n");
			}

			// connect to the remote game TCP server (doing this in the while loop too if radiant is running and the server is not )
			g_RemoteSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (g_RemoteSocket == INVALID_SOCKET)
			{
				Game::Com_PrintMessage(0, "^1[LiveLink]: Failed to initialize client TCP socket!\n", 0);
				return;
			}

			Game::Com_PrintMessage(0, utils::va("[LiveLink]: Connecting to <%s> : <%d> ...\n", r_connectIp.c_str(), r_connectPort), 0);

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
				Game::Com_PrintMessage(0, "^2[LiveLink]: Connected!\n", 0);
				g_RemoteSocketStatus = 1;

				Game::Globals::cgs_addons.radiantLiveConnected = true;
			}
		});
		#endif

		// -----
		// dvars

		dvars::radiant_live = game::Dvar_RegisterBool(
			/* name		*/ "radiant_live",
			/* desc		*/ "enable live-link between radiant <-> iw3",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved);

		dvars::radiant_livePort = game::Dvar_RegisterInt(
			/* name		*/ "radiant_livePort",
			/* desc		*/ "port to be used for live-link",
			/* default	*/ 3700,
			/* minVal	*/ 0,
			/* maxVal	*/ 99999,
			/* flags	*/ game::dvar_flags::saved);

		dvars::radiant_liveDebug = game::Dvar_RegisterBool(
			/* name		*/ "radiant_liveDebug",
			/* desc		*/ "enable live-link debug-prints",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved);

		static std::vector <const char*> radiant_syncCameraEnum = { "DISABLED", "RADIANT_TO_GAME", "GAME_TO_RADIANT", "BOTH",};
		dvars::radiant_syncCamera = game::Dvar_RegisterEnum(
			/* name		*/ "radiant_syncCamera",
			/* desc		*/ "enable live-link camera sync",
			/* default	*/ 0,
			/* enumSize	*/ 4,
			/* enumData */ radiant_syncCameraEnum.data(),
			/* flags	*/ game::dvar_flags::saved);

		dvars::radiant_brushCollision = game::Dvar_RegisterBool(
			/* name		*/ "radiant_brushCollision",
			/* desc		*/ "enable radiant debug-brush collision",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved);

		dvars::radiant_brushColor = game::Dvar_RegisterVec4(
			/* name		*/ "radiant_brushColor",
			/* desc		*/ "color of radiant debug-brushes",
			/* x		*/ 0.5f,
			/* y		*/ 0.5f,
			/* z		*/ 0.5f,
			/* w		*/ 0.5f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::radiant_brushLit = game::Dvar_RegisterBool(
			/* name		*/ "radiant_brushLit", 
			/* desc		*/ "enable fake lighting on debug-brushes", 
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved);

		dvars::radiant_brushWireframe = game::Dvar_RegisterBool(
			/* name		*/ "radiant_brushWireframe",
			/* desc		*/ "enable wireframe on debug-brushes",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved);

		dvars::radiant_brushWireframeColor = game::Dvar_RegisterVec4(
			/* name		*/ "radiant_brushWireframeColor",
			/* desc		*/ "wireframe color of debug-brush",
			/* x		*/ 1.0f,
			/* y		*/ 0.64f,
			/* z		*/ 0.0f,
			/* w		*/ 0.8f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		// -----
		// Hooks

		// SV_ServerThread :: add SV_PostFrame (now calling SV_ReceivePackets from Pmove)
		//utils::hook(0x5335AE, SV_PostFrame_stub, HOOK_JUMP).install()->quick();

		// G_InitGame :: add RadiantRemote::SV_Init()
		utils::hook(0x4BF5A3, G_InitGame_stub, HOOK_JUMP).install()->quick();

		// CG_CalcViewValues :: add CG_RadiantCamCalcView to change refdef with radiant-commands
		utils::hook(0x451C64, CG_CalcViewValues_stub, HOOK_JUMP).install()->quick();

		// CG_CalcViewValues :: skip CG_ApplyViewAnimation if radiant-cam is enabled so we dont move the viewmodel with the camera
		utils::hook(0x451C92, CG_ApplyViewAnimation_stub, HOOK_JUMP).install()->quick();
	}
}