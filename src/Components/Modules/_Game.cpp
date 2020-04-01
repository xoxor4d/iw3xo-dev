#include "STDInclude.hpp"

#define MAX_SPAWN_VARS 64
#define MAX_SPAWN_VARS_CHARS 2048

namespace Components
{
	// -------
	// G_SPAWN

	char* _Game::GetPairValue(Game::SpawnVar* spawnVar, const char* key)
	{
		int i;

		for (i = 0; i < spawnVar->numSpawnVars; ++i)
		{
			if (!Utils::Q_stricmp(key, spawnVar->spawnVars[i][0]))
				return spawnVar->spawnVars[i][1];
		}

		return 0;
	}

	bool _Game::G_GetEntityToken(char* buffer, int bufferSize)
	{
		char* s;

		s = Game::Com_Parse(&Game::g_entityBeginParsePoint);
		Utils::Q_strncpyz(buffer, s, bufferSize);

		if (!Game::g_entityBeginParsePoint)
		{
			return 0;
		}

		Game::g_entityEndParsePoint = Game::g_entityBeginParsePoint;

		return 1;
	}

	char* _Game::G_AddSpawnVarToken(const char* string, Game::SpawnVar* spawnVar)
	{
		char* dest;
		unsigned int len;

		len = strlen(string);

		if ((signed int)(spawnVar->numSpawnVarChars + len + 1) > MAX_SPAWN_VARS_CHARS)
		{
			Game::Com_Error(0, "G_AddSpawnVarToken: MAX_SPAWN_CHARS");
		}

		dest = &spawnVar->spawnVarChars[spawnVar->numSpawnVarChars];
		memcpy(dest, string, len + 1);

		spawnVar->numSpawnVarChars += len + 1;
		return dest;
	}

	void _Game::G_SetEntityParsePoint(const char* beginParsePoint)
	{
		Game::g_entityBeginParsePoint = beginParsePoint;
		Game::g_entityEndParsePoint = beginParsePoint;
	}

	// Parses a brace bounded set of key / value pairs :: { "origin" "%.4f %.4f %.4f" \n "angles" "%.4f %.4f %.4f" }
	bool _Game::G_ParseSpawnVars(Game::SpawnVar* spawnVar)
	{
		char com_token[1024];
		char keyname[1024];

		spawnVar->spawnVarsValid = 0;
		spawnVar->numSpawnVars = 0;
		spawnVar->numSpawnVarChars = 0;

		// parse the opening brace
		if (!_Game::G_GetEntityToken(com_token, sizeof(com_token)))
		{
			return 0; // end of spawn string
		}

		if (com_token[0] != '{') // 123
		{
			Game::Com_Error(0, "G_ParseSpawnVars: found %s when expecting {", com_token);
		}

		// go through all the key / value pairs
		while (1)
		{
			// parse key
			if (!_Game::G_GetEntityToken(keyname, sizeof(keyname)))
			{
				Game::Com_Error(0, "G_ParseSpawnVars: EOF without closing brace");
			}

			if (keyname[0] == '}') // 125
			{
				break;
			}

			if (!_Game::G_GetEntityToken(com_token, sizeof(com_token))) // parse value 
			{
				Game::Com_Error(0, "G_ParseSpawnVars: EOF without closing brace");
			}

			if (com_token[0] == '}') // 125
			{
				Game::Com_Error(0, "G_ParseSpawnVars: closing brace without data");
			}

			if (spawnVar->numSpawnVars == MAX_SPAWN_VARS)
			{
				Game::Com_Error(0, "G_ParseSpawnVars: MAX_SPAWN_VARS");
			}

			spawnVar->spawnVars[spawnVar->numSpawnVars][0] = _Game::G_AddSpawnVarToken(keyname, spawnVar);
			spawnVar->spawnVars[spawnVar->numSpawnVars][1] = _Game::G_AddSpawnVarToken(com_token, spawnVar);
			spawnVar->numSpawnVars++;
		}

		spawnVar->spawnVarsValid = 1;
		return true;
	}

	// *
	// instantly rotate an entity
	void _Game::gEnt_RotateToInstant(Game::gentity_s* ent, const glm::vec3 &toAngles)
	{
		if (ent == nullptr)
		{
			Game::Com_PrintMessage(0, "gEnt_RotateToInstant:: ^1not a valid entity!\n", 0);
			return;
		}

		glm::setFloat3(ent->r.currentAngles, toAngles);
		glm::setFloat3(ent->s.lerp.apos.trBase, toAngles);
	}

	// *
	// instantly move an entity
	void _Game::gEnt_MoveToInstant(Game::gentity_s* ent, const glm::vec3 &toOrigin)
	{
		if (ent == nullptr)
		{
			Game::Com_PrintMessage(0, "gEnt_MoveToInstant:: ^1not a valid entity!\n", 0);
			return;
		}

		glm::setFloat3(ent->r.currentOrigin, toOrigin);
		glm::setFloat3(ent->s.lerp.pos.trBase, toOrigin);
	}

	// radiant remote only
	// edit brushmodel bounds
	void _Game::gEnt_BrushModelBounds(Game::dynBrushModel_t* bModel, const glm::vec3 &localMins, const glm::vec3 &localMaxs)
	{
		// entity : local bounds
		glm::setFloat3(bModel->ent->r.mins, localMins);
		glm::setFloat3(bModel->ent->r.maxs, localMaxs);

		// cmodel : local bounds
		glm::setFloat3(bModel->cmodel->mins, localMins);
		glm::setFloat3(bModel->cmodel->maxs, localMaxs);

		// cmodel : leaf local bounds ~ submodels don't reference the main tree ... always - (-0.875f)
		glm::setFloat3(bModel->cmodel->leaf.mins, (localMins - (-0.875f)));
		glm::setFloat3(bModel->cmodel->leaf.maxs, (localMaxs - 0.875f));

		// brush index from leafbrushnode ~ (T5 :: CM_TestPointInBrushModel)
		int brushIndex = (int)*Game::cm->leafbrushNodes[bModel->cmodel->leaf.leafBrushNode].data.leaf.brushes;

		// clipmap brush : local bounds ~ https://github.com/id-Software/Quake-III-Arena/blob/dbe4ddb10315479fc00086f08e25d968b4b43c49/code/qcommon/cm_load.c#L146
		glm::setFloat3(Game::cm->brushes[brushIndex].mins, (localMins + 1.0f));
		glm::setFloat3(Game::cm->brushes[brushIndex].maxs, (localMaxs - 1.0f));

		if (Game::cm->brushes[brushIndex].sides)
		{
			int rad_sBrushIdx = bModel->radBrushIndexForDyn;

			for (auto bSide = 0; bSide < (int)Game::cm->brushes[brushIndex].numsides; bSide++)
			{
				Game::cm->brushes[brushIndex].sides[bSide].plane->normal[0] = Game::Globals::rad_savedBrushes.brush[rad_sBrushIdx].face[bSide].normal[0];
				Game::cm->brushes[brushIndex].sides[bSide].plane->normal[1] = Game::Globals::rad_savedBrushes.brush[rad_sBrushIdx].face[bSide].normal[1];
				Game::cm->brushes[brushIndex].sides[bSide].plane->normal[2] = Game::Globals::rad_savedBrushes.brush[rad_sBrushIdx].face[bSide].normal[2];

				Game::cm->brushes[brushIndex].sides[bSide].plane->dist = fabs(Game::Globals::rad_savedBrushes.brush[rad_sBrushIdx].face[bSide].dist);
			}
		}
	}

	void _Game::gEnt_BrushModelSetCollision(Game::gentity_s* ent, bool collisionState)
	{
		if (ent == nullptr)
		{
			Game::Com_PrintMessage(0, "gEnt_BrushModelSetCollision:: ^1not a valid entity!\n", 0);
			return;
		}

		if (collisionState)
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

	const char* _Game::gEnt_GetGScrString(std::int16_t stringId)
	{
		if (stringId)
		{
			return (const char*)(*Game::gScrMemTreePub + 12 * (int)stringId + 4);
		}
		else
		{
			return 0;
		}
	}

	_Game::_Game()
	{ }

	_Game::~_Game()
	{ }
}