#pragma once

namespace Components
{
	class _Game : public Component
	{
	public:
		_Game();
		~_Game();
		const char* getName() override { return "_Game"; };

		static bool			G_ParseSpawnVars(Game::SpawnVar* spawnVar);
		static void			G_SetEntityParsePoint(const char* beginParsePoint);
		static char*		G_AddSpawnVarToken(const char* string, Game::SpawnVar* spawnVar);
		static bool			G_GetEntityToken(char* buffer, int bufferSize);
		static char*		GetPairValue(Game::SpawnVar* spawnVar, const char* key);

		static void			gEnt_RotateToInstant(Game::gentity_s* ent, const glm::vec3 &toAngles);
		static void			gEnt_MoveToInstant(Game::gentity_s* ent, const glm::vec3 &toOrigin);
		static void			gEnt_BrushModelBounds(Game::dynBrushModel_t* bModel, const glm::vec3 &localMins, const glm::vec3 &localMaxs);
		static void			gEnt_BrushModelSetCollision(Game::gentity_s* ent, bool collisionState);
		static const char*	gEnt_GetGScrString(std::int16_t stringId);

	private:
	};
}
