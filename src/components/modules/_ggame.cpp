#include "std_include.hpp"

#define MAX_SPAWN_VARS 64
#define MAX_SPAWN_VARS_CHARS 2048

namespace components
{
	// -------
	// G_SPAWN

	char* _ggame::get_spawnvar_pair_value(game::SpawnVar* spawn_var, const char* key)
	{
		for (int i = 0; i < spawn_var->numSpawnVars; ++i)
		{
			if (!utils::q_stricmp(key, spawn_var->spawnVars[i][0]))
			{
				return spawn_var->spawnVars[i][1];
			}
		}

		return nullptr;
	}

	bool _ggame::get_entity_token(char* buffer, int buffer_size)
	{
		char* s = game::Com_Parse(&game::g_entityBeginParsePoint);
		utils::q_strncpyz(buffer, s, buffer_size);

		if (!game::g_entityBeginParsePoint)
		{
			return false;
		}

		game::g_entityEndParsePoint = game::g_entityBeginParsePoint;

		return true;
	}

	char* _ggame::add_spawn_var_token(const char* string, game::SpawnVar* spawnVar)
	{
		const auto len = static_cast<int>(strlen(string));

		if ((spawnVar->numSpawnVarChars + len + 1) > MAX_SPAWN_VARS_CHARS)
		{
			game::Com_Error(0, "G_AddSpawnVarToken: MAX_SPAWN_CHARS");
		}

		char* dest = &spawnVar->spawnVarChars[spawnVar->numSpawnVarChars];
		memcpy(dest, string, len + 1);

		spawnVar->numSpawnVarChars += len + 1;
		return dest;
	}

	void _ggame::set_entity_parse_point(const char* begin_parse_point)
	{
		game::g_entityBeginParsePoint = begin_parse_point;
		game::g_entityEndParsePoint = begin_parse_point;
	}

	// parse a brace bounded set of key / value pairs :: { "origin" "%.4f %.4f %.4f" \n "angles" "%.4f %.4f %.4f" }
	bool _ggame::parse_spawn_vars(game::SpawnVar* spawnVar)
	{
		char com_token[1024];
		char key_name[1024];

		spawnVar->spawnVarsValid = 0;
		spawnVar->numSpawnVars = 0;
		spawnVar->numSpawnVarChars = 0;

		// parse opening brace
		if (!_ggame::get_entity_token(com_token, sizeof(com_token)))
		{
			return false; // end of spawn string
		}

		if (com_token[0] != '{') // 123
		{
			game::Com_Error(0, "_ggame::parse_spawn_vars: found %s when expecting {", com_token);
		}

		// go through all key/value pairs
		while (true)
		{
			// parse key
			if (!_ggame::get_entity_token(key_name, sizeof(key_name)))
			{
				game::Com_Error(0, "G_ParseSpawnVars: EOF without closing brace");
			}

			if (key_name[0] == '}') // 125
			{
				break;
			}

			if (!_ggame::get_entity_token(com_token, sizeof(com_token))) // parse value 
			{
				game::Com_Error(0, "_ggame::parse_spawn_vars: EOF without closing brace");
			}

			if (com_token[0] == '}') // 125
			{
				game::Com_Error(0, "_ggame::parse_spawn_vars: closing brace without data");
			}

			if (spawnVar->numSpawnVars == MAX_SPAWN_VARS)
			{
				game::Com_Error(0, "_ggame::parse_spawn_vars: MAX_SPAWN_VARS");
			}

			spawnVar->spawnVars[spawnVar->numSpawnVars][0] = _ggame::add_spawn_var_token(key_name, spawnVar);
			spawnVar->spawnVars[spawnVar->numSpawnVars][1] = _ggame::add_spawn_var_token(com_token, spawnVar);
			spawnVar->numSpawnVars++;
		}

		spawnVar->spawnVarsValid = true;
		return true;
	}

	// *
	// instantly rotate an entity
	void _ggame::ent_instant_rotate_to(game::gentity_s* ent, const glm::vec3 &to_angles)
	{
		if (ent == nullptr)
		{
			game::Com_PrintMessage(0, "_ggame::ent_instant_rotate_to: ^1not a valid entity!\n", 0);
			return;
		}

		glm::set_float3(ent->r.currentAngles, to_angles);
		glm::set_float3(ent->s.lerp.apos.trBase, to_angles);
	}

	// *
	// instantly move an entity
	void _ggame::ent_instant_move_to(game::gentity_s* ent, const glm::vec3 &to_origin)
	{
		if (ent == nullptr)
		{
			game::Com_PrintMessage(0, "_ggame::ent_instant_move_to: ^1not a valid entity!\n", 0);
			return;
		}

		glm::set_float3(ent->r.currentOrigin, to_origin);
		glm::set_float3(ent->s.lerp.pos.trBase, to_origin);
	}

	// radiant remote only
	// edit brushmodel bounds
	void _ggame::ent_edit_brushmodel_bounds(game::dynBrushModel_t* bModel, const glm::vec3 &localMins, const glm::vec3 &localMaxs)
	{
		// entity : local bounds
		glm::set_float3(bModel->ent->r.mins, localMins);
		glm::set_float3(bModel->ent->r.maxs, localMaxs);

		// cmodel : local bounds
		glm::set_float3(bModel->cmodel->mins, localMins);
		glm::set_float3(bModel->cmodel->maxs, localMaxs);

		// cmodel : leaf local bounds ~ submodels don't reference the main tree ... always - (-0.875f)
		glm::set_float3(bModel->cmodel->leaf.mins, (localMins - (-0.875f)));
		glm::set_float3(bModel->cmodel->leaf.maxs, (localMaxs - 0.875f));

		// brush index from leafbrushnode ~ (T5 :: CM_TestPointInBrushModel)
		const int brush_index = (int)*game::cm->leafbrushNodes[bModel->cmodel->leaf.leafBrushNode].data.leaf.brushes;

		// clipmap brush : local bounds ~ https://github.com/id-Software/Quake-III-Arena/blob/dbe4ddb10315479fc00086f08e25d968b4b43c49/code/qcommon/cm_load.c#L146
		glm::set_float3(game::cm->brushes[brush_index].mins, (localMins + 1.0f));
		glm::set_float3(game::cm->brushes[brush_index].maxs, (localMaxs - 1.0f));

		if (game::cm->brushes[brush_index].sides)
		{
			const int radiant_brush_index = bModel->radBrushIndexForDyn;

			for (auto bSide = 0u; bSide < game::cm->brushes[brush_index].numsides; bSide++)
			{
				game::cm->brushes[brush_index].sides[bSide].plane->normal[0] = game::glob::radiant_saved_brushes.brush[radiant_brush_index].face[bSide].normal[0];
				game::cm->brushes[brush_index].sides[bSide].plane->normal[1] = game::glob::radiant_saved_brushes.brush[radiant_brush_index].face[bSide].normal[1];
				game::cm->brushes[brush_index].sides[bSide].plane->normal[2] = game::glob::radiant_saved_brushes.brush[radiant_brush_index].face[bSide].normal[2];
				game::cm->brushes[brush_index].sides[bSide].plane->dist = fabs(game::glob::radiant_saved_brushes.brush[radiant_brush_index].face[bSide].dist);
			}
		}
	}

	void _ggame::ent_brushmodel_set_collision(game::gentity_s* ent, bool collision_state)
	{
		if (ent == nullptr)
		{
			game::Com_PrintMessage(0, "_ggame::ent_brushmodel_set_collision: ^1not a valid entity!\n", 0);
			return;
		}

		if (collision_state)
		{
			ent->s.lerp.eFlags &= 0xFFFFFFFE;
			ent->r.contents = 1;
		}
		else
		{
			ent->s.lerp.eFlags |= 1u;
			ent->r.contents = 0;
		}
	}

	const char* _ggame::ent_get_gscr_string(std::int16_t stringId)
	{
		if (stringId)
		{
			return (const char*)(*game::gScrMemTreePub + 12 * (int)stringId + 4);
		}

		return nullptr;
	}

	_ggame::_ggame()
	{ 
		// *
		// Commands

		command::add("ent_rotateTo", [](command::params params)
		{
			if (params.length() < 3)
			{
				game::Com_PrintMessage(0, "Usage :: ent_rotateTo entityID <angles vec3>\n", 0);
				return;
			}

			float angles[3] = {};
			float rotation[3] = {};

			angles[0] = utils::try_stof(params[2], true);

			if (params.length() >= 4)
			{
				angles[1] = utils::try_stof(params[3], true);
			}

			if (params.length() >= 5)
			{
				angles[2] = utils::try_stof(params[4], true);
			}

			const int ent_index = utils::try_stoi(params[1], true);

			for (auto i = 0; i < 3; ++i)
			{
				rotation[i] = game::scr_g_entities[ent_index].r.currentAngles[i] + utils::vector::angle_normalize180(angles[i] - game::scr_g_entities[ent_index].r.currentAngles[i]);
			}

			float tAngles[3] =
			{
				game::scr_g_entities[ent_index].r.currentAngles[0],
				game::scr_g_entities[ent_index].r.currentAngles[1],
				game::scr_g_entities[ent_index].r.currentAngles[2],
			};

			game::ScriptMover_SetupMove(
				&game::scr_g_entities[ent_index].s.lerp.apos,
				rotation,
				4.0f,
				0.1f,
				0.1f,
				tAngles,
				&game::scr_g_entities[ent_index].___u30.mover.aSpeed,
				&game::scr_g_entities[ent_index].___u30.mover.aMidTime,
				&game::scr_g_entities[ent_index].___u30.mover.aDecelTime,
				game::scr_g_entities[ent_index].___u30.mover.apos1,
				game::scr_g_entities[ent_index].___u30.mover.apos2,
				game::scr_g_entities[ent_index].___u30.mover.apos3);

			game::SV_LinkEntity(&game::scr_g_entities[ent_index]);
		});
	}
}