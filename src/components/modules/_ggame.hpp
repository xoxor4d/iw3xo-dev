#pragma once

namespace components
{
	class _ggame final : public component
	{
	public:
		_ggame();
		const char* get_name() override { return "_ggame"; };

		static bool			parse_spawn_vars(game::SpawnVar* spawnVar);
		static void			set_entity_parse_point(const char* begin_parse_point);
		static char*		add_spawn_var_token(const char* string, game::SpawnVar* spawnVar);
		static bool			get_entity_token(char* buffer, int buffer_size);
		static char*		get_spawnvar_pair_value(game::SpawnVar* spawn_var, const char* key);

		static void			ent_instant_rotate_to(game::gentity_s* ent, const glm::vec3 &to_angles);
		static void			ent_instant_move_to(game::gentity_s* ent, const glm::vec3 &to_origin);
		static void			ent_edit_brushmodel_bounds(game::dynBrushModel_t* bModel, const glm::vec3 &localMins, const glm::vec3 &localMaxs);
		static void			ent_brushmodel_set_collision(game::gentity_s* ent, bool collision_state);
		static const char*	ent_get_gscr_string(std::int16_t stringId);
	};
}
