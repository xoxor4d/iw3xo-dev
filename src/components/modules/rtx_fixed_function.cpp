#include "std_include.hpp"

namespace components
{
	void XSurfaceOptimize(game::XSurface* surf)
	{
		const auto dev = game::glob::d3d9_device;

		// setup indexbuffer
		if (!surf->custom_indexbuffer)
		{
			void* index_buffer_data = nullptr;
			const auto raw_index_bytes = 6 * surf->triCount;
			const auto index_bytes = (raw_index_bytes + 31) & -32;

			if (auto hr = dev->CreateIndexBuffer(index_bytes, 8, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &surf->custom_indexbuffer, nullptr);
				hr >= 0)
			{
				if (hr = surf->custom_indexbuffer->Lock(0, 0, &index_buffer_data, 0);
					hr >= 0)
				{
					memcpy(index_buffer_data, surf->triIndices, raw_index_bytes);
					memset((char*)index_buffer_data + raw_index_bytes, 0, index_bytes - raw_index_bytes);

					//R_FinishStaticIndexBuffer(surf->custom_indexbuffer);
					surf->custom_indexbuffer->Unlock();
				}
				else
				{
					surf->custom_indexbuffer->Release();
				}
			}
		}

		// setup vertexbuffer
		if (surf->custom_indexbuffer)
		{
			//XSurfaceOptimizeRigid(model, surf);
			if (!surf->custom_vertexbuffer)
			{
				void* vertex_buffer_data = nullptr;
				const auto vertex_bytes = surf->vertCount * 32; // pos-xyz ; normal-xyz ; texcoords uv = 32 byte 

				if (auto hr = dev->CreateVertexBuffer(vertex_bytes, 8, 0, D3DPOOL_DEFAULT, &surf->custom_vertexbuffer, nullptr);
					hr >= 0)
				{
					if (hr = surf->custom_vertexbuffer->Lock(0, 0, &vertex_buffer_data, 0);
						hr >= 0)
					{
						// we need to unpack the normal and texcoords in 'GfxPackedVertex' to be able to use them for fixed-function rendering

						/*struct GfxPackedVertex
						{
							float xyz[3];
							float binormalSign;
							GfxColor color;
							PackedTexCoords texCoord;
							PackedUnitVec normal;
							PackedUnitVec tangent;
						};*/

						struct unpacked_vert
						{
							game::vec3_t pos;
							game::vec3_t normal;
							game::vec2_t texcoord;
						};

						for (auto i = 0; i < surf->vertCount; i++)
						{
							// packed source vertex
							const auto src_vert = surf->verts0[i];

							// position of our unpacked vert within the vertex buffer
							const auto v_pos_in_buffer = i * 32; // pos-xyz ; normal-xyz ; texcoords uv = 32 byte 
							const auto v = reinterpret_cast<unpacked_vert*>(((DWORD)vertex_buffer_data + v_pos_in_buffer));

							// assign pos and unpack normal and texcoords
							v->pos[0] = src_vert.xyz[0];
							v->pos[1] = src_vert.xyz[1];
							v->pos[2] = src_vert.xyz[2];

							utils::vector::unpack_unit_vec3(src_vert.normal, v->normal);
							game::Vec2UnpackTexCoords(src_vert.texCoord.packed, v->texcoord);
						}

						// og
						//memcpy((char*)vertex_buffer_data, (char*)surf->verts0, vertex_bytes); 

						//R_FinishStaticVertexBuffer(vb);
						surf->custom_vertexbuffer->Unlock();
					}
					else
					{
						surf->custom_vertexbuffer->Release();
					}
				}
			}
		}
	}

	int XModelGetSurfaces(const game::XModel* model, game::XSurface** surfaces, const int submodel)
	{
		*surfaces = &model->surfs[model->lodInfo[submodel].surfIndex];
		return model->lodInfo[submodel].numsurfs;
	}

	void XModelOptimize(const game::XModel* model)
	{
		game::XSurface* surfaces;

		const auto lodCount = model->numLods;
		for (auto lodIndex = 0; lodIndex < lodCount; ++lodIndex)
		{
			const auto surfCount = XModelGetSurfaces(model, &surfaces, lodIndex);
			for (auto surfIndex = 0; surfIndex < surfCount; ++surfIndex)
			{
				XSurfaceOptimize(&surfaces[surfIndex]);
			}
		}
	}


	/**
	 * @brief builds a world matrix for the mesh
	 */
	void R_DrawStaticModelDrawSurfPlacement(game::GfxCmdBufSourceState* src, const game::GfxStaticModelDrawInst* inst)
	{
		float temp[3][3] = {};
		temp[0][0] = inst->placement.axis[0][0];
		temp[0][1] = inst->placement.axis[0][1];
		temp[0][2] = inst->placement.axis[0][2];
		temp[1][0] = inst->placement.axis[2][2];
		temp[1][1] = inst->placement.axis[1][0];
		temp[1][2] = inst->placement.axis[1][1];
		temp[2][0] = inst->placement.axis[1][2];
		temp[2][1] = inst->placement.axis[2][1];
		temp[2][2] = inst->placement.axis[2][0];

		float axis[3][3] = {};
		axis[0][0] = temp[0][0];
		axis[0][1] = temp[0][1];
		axis[0][2] = temp[0][2];
		axis[1][0] = temp[1][1];
		axis[1][1] = temp[1][2];
		axis[1][2] = temp[2][0];
		axis[2][0] = temp[2][2];
		axis[2][1] = temp[2][1];
		axis[2][2] = temp[1][0];

		// not needed for fixed-function
		/*++src->matrixVersions[3];
		++src->matrixVersions[5];
		++src->matrixVersions[7];
		src->constVersions[58] = ++src->matrixVersions[0];*/

		// dont offset with eyepos
		/*float origin[3] = {};
		origin[0] = inst->placement.origin[0] - src->eyeOffset[0];
		origin[1] = inst->placement.origin[1] - src->eyeOffset[1];
		origin[2] = inst->placement.origin[2] - src->eyeOffset[2];
		ikMatrixSet44(origin, (float*)axis, src, inst->placement.scale);*/

		const auto scale = inst->placement.scale;

		// inlined ikMatrixSet44
		src->matrices.matrix[0].m[0][0] = axis[0][0] * scale;
		src->matrices.matrix[0].m[0][1] = axis[0][1] * scale;
		src->matrices.matrix[0].m[0][2] = axis[0][2] * scale;
		src->matrices.matrix[0].m[0][3] = 0.0f;

		src->matrices.matrix[0].m[1][0] = axis[1][0] * scale;
		src->matrices.matrix[0].m[1][1] = axis[1][1] * scale;
		src->matrices.matrix[0].m[1][2] = axis[1][2] * scale;
		src->matrices.matrix[0].m[1][3] = 0.0f;

		src->matrices.matrix[0].m[2][0] = axis[2][0] * scale;
		src->matrices.matrix[0].m[2][1] = axis[2][1] * scale;
		src->matrices.matrix[0].m[2][2] = axis[2][2] * scale;
		src->matrices.matrix[0].m[2][3] = 0.0f;

		src->matrices.matrix[0].m[3][0] = inst->placement.origin[0];
		src->matrices.matrix[0].m[3][1] = inst->placement.origin[1];
		src->matrices.matrix[0].m[3][2] = inst->placement.origin[2];
		src->matrices.matrix[0].m[3][3] = 1.0f;
	}

	/**
	 * @brief completely rewritten R_DrawStaticModelDrawSurfNonOptimized to render static models using the fixed-function pipeline
	 */
	void R_DrawStaticModelDrawSurfNonOptimized(const game::GfxStaticModelDrawStream* drawstream, game::GfxCmdBufSourceState* scr, game::GfxCmdBufState* cmd)
	{
		const auto smodel_count = drawstream->smodelCount;
		const auto smodel_list = (const game::GfxStaticModelDrawStream*) reinterpret_cast<const void*>(drawstream->smodelList);
		const auto draw_inst = game::rgp->world->dpvs.smodelDrawInsts;
		const auto dev = game::glob::d3d9_device;

		// create buffers for all surfaces of the model (including LODs)
		for (auto index = 0u; index < smodel_count; index++)
		{
			const auto inst = &draw_inst[*((std::uint16_t*)&smodel_list->primDrawSurfPos + index)];
			XModelOptimize(inst->model);
		}

		if (drawstream->localSurf->custom_indexbuffer)
		{
			if (cmd->prim.indexBuffer != drawstream->localSurf->custom_indexbuffer)
			{
				cmd->prim.indexBuffer = drawstream->localSurf->custom_indexbuffer;
				dev->SetIndices(drawstream->localSurf->custom_indexbuffer);
			}
		} //else __debugbreak();


		if (drawstream->localSurf->custom_vertexbuffer)
		{
			cmd->prim.streams[0].vb = drawstream->localSurf->custom_vertexbuffer;
			cmd->prim.streams[0].offset = 0;
			cmd->prim.streams[0].stride = 32; // pos-xyz ; normal-xyz ; texcoords uv = 32 byte 

			dev->SetStreamSource(0, drawstream->localSurf->custom_vertexbuffer, 0, 32);
		} //else __debugbreak();


		for (auto index = 0u; index < smodel_count; index++)
		{
			const auto inst = &draw_inst[*((std::uint16_t*)&smodel_list->primDrawSurfPos + index)];

			// transform model into the scene by updating the worldmatrix
			R_DrawStaticModelDrawSurfPlacement(scr, inst);

			dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&scr->matrices.matrix[0].m));
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

			// def. needed or the game will render the mesh using shaders
			dev->SetVertexShader(nullptr);
			dev->SetPixelShader(nullptr);

			dev->SetRenderState(D3DRS_ZENABLE, TRUE);
			dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

			cmd->prim.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, drawstream->localSurf->vertCount, 0, drawstream->localSurf->triCount);
		}
	}

	// ------------------------------------

	rtx_fixed_function::rtx_fixed_function()
	{
		// fixed function rendering of static models (R_TessStaticModelRigidDrawSurfList)
		if (flags::has_flag("fixed_function"))
		{
			// disable functions used for the shader pipeline
			utils::hook::nop(0x647BDF, 5); // R_SetPassPixelShaderStableArguments
			utils::hook::nop(0x647C3B, 5); // R_UpdateVertexDecl
			utils::hook::nop(0x647C91, 5); // R_SetPassPixelShaderStableArguments
			utils::hook::nop(0x647CA3, 5); // R_UpdateVertexDecl

			// this sets a sampler (SetTexture -> needed)
			//utils::hook::nop(0x647D8F, 5); // R_SetPassShaderObjectArguments

			utils::hook(0x655A10, R_DrawStaticModelDrawSurfNonOptimized, HOOK_CALL).install()->quick();
		}
	}
}