#pragma once

namespace components
{
	class rtx_fixed_function final : public component
	{
	public:
		rtx_fixed_function();
		const char* get_name() override { return "rtx_fixed_function"; }

		static inline IDirect3DVertexBuffer9* dynamic_codemesh_vb = nullptr;
		static inline IDirect3DVertexBuffer9* dynamic_markmesh_world_vb = nullptr;
		static inline IDirect3DVertexBuffer9* dynamic_markmesh_model_vb = nullptr;

		static void unpack_normal(const game::PackedUnitVec* packed, game::vec3_t out);
		static void build_worldmatrix_for_object(float(*mtx)[4], const float* quat, const float* origin, const float scale);
		static void build_worldmatrix_for_object(float(*mtx)[4], float(*model_axis)[3], const float* origin, const float scale);
		static void copy_fx_buffer();
		static void copy_marks_buffer();
	};
}
