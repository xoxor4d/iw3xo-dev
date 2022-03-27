#pragma once

namespace components
{
	class _renderer final : public component
	{
	public:
		_renderer();
		const char* get_name() override { return "_renderer"; };

		static void register_dvars();

		static bool is_valid_technique_for_type(const Game::Material* mat, const Game::MaterialTechniqueType type);
		static void switch_technique(Game::switch_material_t* swm, Game::Material* material);
		static void switch_technique(Game::switch_material_t* swm, const char* material_name);
		static void switch_material(Game::switch_material_t* swm, const char* material_name);
	};
}
