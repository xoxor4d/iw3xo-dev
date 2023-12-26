#include "std_include.hpp"

namespace components
{
	bool XSurfaceOptimize(game::XSurface* surf)
	{
		const auto dev = game::glob::d3d9_device;
		bool allocated_any = false;


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

				allocated_any = true;
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

					allocated_any = true;
				}
			}
		}

		return allocated_any;
	}

	int XModelGetSurfaces(const game::XModel* model, game::XSurface** surfaces, const int submodel)
	{
		*surfaces = &model->surfs[model->lodInfo[submodel].surfIndex];
		return model->lodInfo[submodel].numsurfs;
	}

	void XModelOptimize(const game::XModel* model, bool is_loading_map = false)
	{
		game::XSurface* surfaces;

		const auto lodCount = model->numLods;
		for (auto lodIndex = 0; lodIndex < lodCount; ++lodIndex)
		{
			const auto surfCount = XModelGetSurfaces(model, &surfaces, lodIndex);
			for (auto surfIndex = 0; surfIndex < surfCount; ++surfIndex)
			{
				const bool allocated_any = XSurfaceOptimize(&surfaces[surfIndex]);

#if DEBUG
				if (!is_loading_map && allocated_any)
				{
					game::Com_PrintMessage(0, utils::va("Allocated buffers for smodel '%s'\n", model->name), 0);
				}
#endif
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

		// #
		// set streams

		IDirect3DIndexBuffer9* og_index_buffer = nullptr;
		IDirect3DVertexBuffer9* og_vertex_buffer = nullptr;
		std::uint32_t og_vertex_buffer_offset = 0;
		std::uint32_t og_vertex_buffer_stride = 0;

		if (drawstream->localSurf->custom_indexbuffer)
		{
			if (cmd->prim.indexBuffer != drawstream->localSurf->custom_indexbuffer)
			{
				// backup
				og_index_buffer = cmd->prim.indexBuffer;

				cmd->prim.indexBuffer = drawstream->localSurf->custom_indexbuffer;
				dev->SetIndices(drawstream->localSurf->custom_indexbuffer);
			}
		} //else __debugbreak();

		if (drawstream->localSurf->custom_vertexbuffer)
		{
			// backup
			og_vertex_buffer = cmd->prim.streams[0].vb;
			og_vertex_buffer_offset = cmd->prim.streams[0].offset;
			og_vertex_buffer_stride = cmd->prim.streams[0].stride;

			cmd->prim.streams[0].vb = drawstream->localSurf->custom_vertexbuffer;
			cmd->prim.streams[0].offset = 0;
			cmd->prim.streams[0].stride = 32; // pos-xyz ; normal-xyz ; texcoords uv = 32 byte 

			dev->SetStreamSource(0, drawstream->localSurf->custom_vertexbuffer, 0, 32);
		} //else __debugbreak();


		// #
		// setup fixed-function

		// vertex format
		dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

		// save shaders
		IDirect3DVertexShader9* og_vertex_shader;
		dev->GetVertexShader(&og_vertex_shader);

		IDirect3DPixelShader9* og_pixel_shader;
		dev->GetPixelShader(&og_pixel_shader);

		// def. needed or the game will render the mesh using shaders
		dev->SetVertexShader(nullptr);
		dev->SetPixelShader(nullptr);

		// not needed I think
		//dev->SetRenderState(D3DRS_ZENABLE, TRUE);
		//dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		//dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);


		for (auto index = 0u; index < smodel_count; index++)
		{
			const auto inst = &draw_inst[*((std::uint16_t*)&smodel_list->primDrawSurfPos + index)];

			// transform model into the scene by updating the worldmatrix
			R_DrawStaticModelDrawSurfPlacement(scr, inst);
			dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&scr->matrices.matrix[0].m));

			// draw the prim
			cmd->prim.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, drawstream->localSurf->vertCount, 0, drawstream->localSurf->triCount);
		}

		// #
		// restore everything for following meshes rendered via shaders

		dev->SetVertexShader(og_vertex_shader);
		dev->SetPixelShader(og_pixel_shader);

		// restore world matrix
		rtx::r_set_3d();
		dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->matrices.matrix[0].m));

		dev->SetFVF(0);

		// restor streams
		if (og_index_buffer)
		{
			cmd->prim.indexBuffer = og_index_buffer;
			dev->SetIndices(og_index_buffer);
		}

		if (og_vertex_buffer)
		{
			cmd->prim.streams[0].vb = og_vertex_buffer;
			cmd->prim.streams[0].offset = og_vertex_buffer_offset;
			cmd->prim.streams[0].stride = og_vertex_buffer_stride;

			dev->SetStreamSource(0, og_vertex_buffer, og_vertex_buffer_offset, og_vertex_buffer_stride);
		}
	}

	// ------------------------------------

	void warm_static_models_on_map_load()
	{
		if (dvars::rtx_warm_smodels && dvars::rtx_warm_smodels->current.enabled)
		{
			game::DB_EnumXAssets_FastFile(game::XAssetType::ASSET_TYPE_XMODEL, [](game::XAssetHeader header, [[maybe_unused]] void* data)
			{
				// HACK - ignore deformed (animated) models because they might use more then 2 partBits (partBit 3-4 are used for custom buffers)
				if (!header.model->surfs->deformed /*&& !header.model->surfs->custom_vertexbuffer && !header.model->surfs->custom_indexbuffer*/)
				{
					XModelOptimize(header.model, true);
				}
			}, nullptr, false);
		}
	}

	__declspec(naked) void warm_static_models_stub()
	{
		const static uint32_t stock_func = 0x431C80; // CG_NorthDirectionChanged
		const static uint32_t retn_addr = 0x440320;
		__asm
		{
			pushad;
			call	warm_static_models_on_map_load;
			popad;

			call	stock_func;
			jmp		retn_addr;
		}
	}

	// ------------------------------------

	void free_custom_xmodel_buffers()
	{
		game::DB_EnumXAssets_FastFile(game::XAssetType::ASSET_TYPE_XMODEL, [](game::XAssetHeader header, [[maybe_unused]] void* data)
		{
			const auto dev = game::glob::d3d9_device;

			// HACK - ignore deformed (animated) models because they might use more then 2 partBits (partBit 3-4 are used for custom buffers)
			if (dev && !header.model->surfs->deformed && (header.model->surfs->custom_vertexbuffer || header.model->surfs->custom_indexbuffer))
			{
				game::XSurface* surfaces;

				const auto lodCount = header.model->numLods;
				for (auto lodIndex = 0; lodIndex < lodCount; ++lodIndex)
				{
					const auto surfCount = XModelGetSurfaces(header.model, &surfaces, lodIndex);
					for (auto surfIndex = 0; surfIndex < surfCount; ++surfIndex)
					{
						if (surfaces[surfIndex].custom_indexbuffer)
						{
							surfaces[surfIndex].custom_indexbuffer->Release();
							surfaces[surfIndex].custom_indexbuffer = nullptr;
						}

						if (surfaces[surfIndex].custom_vertexbuffer)
						{
							surfaces[surfIndex].custom_vertexbuffer->Release();
							surfaces[surfIndex].custom_vertexbuffer = nullptr;
						}
					}
				}
			}
		}, nullptr, false);
	}

	__declspec(naked) void free_custom_xmodel_buffers_stub()
	{
		const static uint32_t stock_func = 0x62F550; // R_ResetModelLighting
		const static uint32_t retn_addr = 0x5F5057;
		__asm
		{
			pushad;
			call	free_custom_xmodel_buffers;
			popad;

			call	stock_func;
			jmp		retn_addr;
		}
	}

	// ------------------------------------

	rtx_fixed_function::rtx_fixed_function()
	{
		// todo - build buffers on map load?

		// fixed function rendering of static models (R_TessStaticModelRigidDrawSurfList)
		if (flags::has_flag("fixed_function"))
		{
			// R_TessStaticModelRigidDrawSurfList :: functions only useful for the shader pipeline
			// - not drawing a viewmodel (mantle / cg_drawgun 0) after drawing a static model using fixed-function messes up world rendering
			// - disabling these hooks and restoring the state of the shader pipeline (in R_DrawStaticModelDrawSurfNonOptimized) fixes that
			// - here to stay for the time being

			//utils::hook::nop(0x647BDF, 5); // R_SetPassPixelShaderStableArguments
			//utils::hook::nop(0x647C3B, 5); // R_UpdateVertexDecl
			//utils::hook::nop(0x647C91, 5); // R_SetPassPixelShaderStableArguments
			//utils::hook::nop(0x647CA3, 5); // R_UpdateVertexDecl
			//utils::hook::nop(0x647D8F, 5); // R_SetPassShaderObjectArguments - this sets a sampler (SetTexture -> needed)

			utils::hook(0x655A10, R_DrawStaticModelDrawSurfNonOptimized, HOOK_CALL).install()->quick();

			// ----

			// build custom index and vertex buffers for all xmodels on map load
			utils::hook(0x44031B, warm_static_models_stub, HOOK_JUMP).install()->quick(); // CG_Init :: CG_NorthDirectionChanged call

			dvars::rtx_warm_smodels = game::Dvar_RegisterBool(
				/* name		*/ "rtx_warm_smodels",
				/* desc		*/ "Build static model vertex buffers on map load (fixed-function rendering only)",
				/* default	*/ true,
				/* flags	*/ game::dvar_flags::saved);

			// ----

			utils::hook(0x5F5052, free_custom_xmodel_buffers_stub).install()->quick(); // R_Shutdown :: R_ResetModelLighting call
		}
	}
}