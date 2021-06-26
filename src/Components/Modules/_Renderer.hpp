#pragma once

namespace Components
{
	class _Renderer : public Component
	{
	public:
		_Renderer();
		~_Renderer();
		const char* getName() override { return "_Renderer"; };

		static void R_RegisterBufferDvars();

		static bool is_valid_technique_for_type(const Game::Material* mat, const Game::MaterialTechniqueType type);
		static void switch_technique(Game::switch_material_t* swm, Game::Material* material);
		static void switch_technique(Game::switch_material_t* swm, const char* material_name);
		static void switch_material(Game::switch_material_t* swm, const char* material_name);

	private:
	};
}
