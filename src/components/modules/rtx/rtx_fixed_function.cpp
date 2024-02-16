#include "std_include.hpp"

// notes:
// - r_pretess (surface batching) might cause some messed up normals (sometimes noticeable in water puddles)

// todo
// - disable normal and specularmaps so that remix does not pick them up (less clutter in remix ui)

//#define TESS_TESTS // R_DrawTessTechnique test (2d, debug visualization etc.)

namespace components
{
	struct unpacked_model_vert
	{
		game::vec3_t pos;
		game::vec3_t normal;
		game::vec2_t texcoord;
	};

	constexpr auto MODEL_VERTEX_STRIDE = 32u;
	constexpr auto MODEL_VERTEX_FORMAT = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

	// #

	struct unpacked_world_vert
	{
		game::vec3_t pos;
		game::vec3_t normal;
		unsigned int color;
		game::vec2_t texcoord;
	};

	constexpr auto WORLD_VERTEX_STRIDE = 36u;
	constexpr auto WORLD_VERTEX_FORMAT = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	IDirect3DVertexBuffer9* gfx_world_vertexbuffer = nullptr;

	// *
	// static models (rigid)

	bool XSurfaceOptimize(game::XSurface* surf)
	{
		const auto dev = game::glob::d3d9_device;
		bool allocated_any = false;

		// setup vertexbuffer
		{
			//XSurfaceOptimizeRigid(model, surf);
			if (!surf->custom_vertexbuffer)
			{
				void* vertex_buffer_data = nullptr;
				const auto vertex_bytes = surf->vertCount * MODEL_VERTEX_STRIDE;

				if (auto hr = dev->CreateVertexBuffer(vertex_bytes, 8, 0, D3DPOOL_DEFAULT, &surf->custom_vertexbuffer, nullptr);
					hr >= 0)
				{
					if (hr = surf->custom_vertexbuffer->Lock(0, 0, &vertex_buffer_data, 0);
						hr >= 0)
					{
						// we need to unpack normal and texcoords in 'GfxPackedVertex' to be able to use them for fixed-function rendering
						for (auto i = 0; i < surf->vertCount; i++)
						{
							// packed source vertex
							const auto src_vert = surf->verts0[i];

							// position of our unpacked vert within the vertex buffer
							const auto v_pos_in_buffer = i * MODEL_VERTEX_STRIDE;
							const auto v = reinterpret_cast<unpacked_model_vert*>(((DWORD)vertex_buffer_data + v_pos_in_buffer));

							// assign pos
							v->pos[0] = src_vert.xyz[0];
							v->pos[1] = src_vert.xyz[1];
							v->pos[2] = src_vert.xyz[2];

							// unpack normal and texcoords
							const auto scale = static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[3])) * (1.0f / 255.0f) + 0.7529412f;
							v->normal[0] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[0])) * (1.0f / 127.0f) + -1.0f) * scale;
							v->normal[1] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[1])) * (1.0f / 127.0f) + -1.0f) * scale;
							v->normal[2] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[2])) * (1.0f / 127.0f) + -1.0f) * scale;

							game::Vec2UnpackTexCoords(src_vert.texCoord.packed, v->texcoord);
						}

						surf->custom_vertexbuffer->Unlock();
					}
					else
					{
						surf->custom_vertexbuffer->Release();
						surf->custom_vertexbuffer = nullptr;
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

		const auto lod_count = model->numLods;
		for (auto lod_index = 0; lod_index < lod_count; ++lod_index)
		{
			const auto surf_count = XModelGetSurfaces(model, &surfaces, lod_index);
			for (auto surf_index = 0; surf_index < surf_count; ++surf_index)
			{
				const bool allocated_any = XSurfaceOptimize(&surfaces[surf_index]);
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
	void R_DrawStaticModelDrawSurfPlacement(game::GfxCmdBufSourceState* src, const game::GfxStaticModelDrawInst* inst, bool fixed_function = true)
	{
		const auto scale = inst->placement.scale;

		// inlined ikMatrixSet44
		src->matrices.matrix[0].m[0][0] = inst->placement.axis[0][0] * scale;
		src->matrices.matrix[0].m[0][1] = inst->placement.axis[0][1] * scale;
		src->matrices.matrix[0].m[0][2] = inst->placement.axis[0][2] * scale;
		src->matrices.matrix[0].m[0][3] = 0.0f;

		src->matrices.matrix[0].m[1][0] = inst->placement.axis[1][0] * scale;
		src->matrices.matrix[0].m[1][1] = inst->placement.axis[1][1] * scale;
		src->matrices.matrix[0].m[1][2] = inst->placement.axis[1][2] * scale;
		src->matrices.matrix[0].m[1][3] = 0.0f;

		src->matrices.matrix[0].m[2][0] = inst->placement.axis[2][0] * scale;
		src->matrices.matrix[0].m[2][1] = inst->placement.axis[2][1] * scale;
		src->matrices.matrix[0].m[2][2] = inst->placement.axis[2][2] * scale;
		src->matrices.matrix[0].m[2][3] = 0.0f;

		src->matrices.matrix[0].m[3][0] = inst->placement.origin[0];
		src->matrices.matrix[0].m[3][1] = inst->placement.origin[1];
		src->matrices.matrix[0].m[3][2] = inst->placement.origin[2];
		src->matrices.matrix[0].m[3][3] = 1.0f;

		if (!fixed_function)
		{
			src->matrices.matrix[0].m[3][0] -= src->eyeOffset[0];
			src->matrices.matrix[0].m[3][1] -= src->eyeOffset[1];
			src->matrices.matrix[0].m[3][2] -= src->eyeOffset[2];
		}
	}

	/**
	 * @brief completely rewritten R_DrawStaticModelDrawSurfNonOptimized to render static models using the fixed-function pipeline
	 */
	void R_DrawStaticModelDrawSurfNonOptimized(const game::GfxStaticModelDrawStream* drawstream, game::GfxCmdBufSourceState* src, game::GfxCmdBufState* cmd)
	{
		const auto smodel_count = drawstream->smodelCount;
		const auto smodel_list = (const game::GfxStaticModelDrawStream*) reinterpret_cast<const void*>(drawstream->smodelList);
		const auto draw_inst = game::rgp->world->dpvs.smodelDrawInsts;
		const auto dev = game::glob::d3d9_device;

		// fixed function rendering
		bool render_ff = true;

		// create buffers for all surfaces of the model (including LODs)
		// ^ already done on map load (when 'rtx_warm_smodels' is enabled) but this also covers dynamically spawned models
		for (auto index = 0u; index < smodel_count; index++)
		{
			const auto inst = &draw_inst[*((std::uint16_t*)&smodel_list->primDrawSurfPos + index)];
			XModelOptimize(inst->model);
		}

		// #
		// set streams

		// index and vertex buffers are within the fastfile that includes the model
		const auto mem = &game::g_zones[static_cast<std::int8_t>(drawstream->localSurf->zoneHandle)].mem;

		// index buffer
		if (cmd->prim.indexBuffer != mem->indexBuffer)
		{
			cmd->prim.indexBuffer = mem->indexBuffer;
			dev->SetIndices(mem->indexBuffer);
		}

		// custom vertexbuffer
		if (drawstream->localSurf->custom_vertexbuffer)
		{
			cmd->prim.streams[0].vb = drawstream->localSurf->custom_vertexbuffer;
			cmd->prim.streams[0].offset = 0;
			cmd->prim.streams[0].stride = MODEL_VERTEX_STRIDE;
			dev->SetStreamSource(0, drawstream->localSurf->custom_vertexbuffer, 0, MODEL_VERTEX_STRIDE);
		}
		else // fallback to shader rendering if there is no custom vertexbuffer
		{
			render_ff = false;
			const auto offset = (char*)drawstream->localSurf->verts0 - mem->blocks[7].data;

			cmd->prim.streams[0].vb = mem->vertexBuffer;
			cmd->prim.streams[0].offset = offset;
			cmd->prim.streams[0].stride = MODEL_VERTEX_STRIDE;
			dev->SetStreamSource(0, mem->vertexBuffer, offset, MODEL_VERTEX_STRIDE);
		}

		IDirect3DVertexShader9* og_vertex_shader = nullptr;
		IDirect3DPixelShader9* og_pixel_shader = nullptr;

		if (render_ff)
		{
			// vertex format
			dev->SetFVF(MODEL_VERTEX_FORMAT);

			// save shaders
			dev->GetVertexShader(&og_vertex_shader);
			dev->GetPixelShader(&og_pixel_shader);

			// def. needed or the game will render the mesh using shaders
			dev->SetVertexShader(nullptr);
			dev->SetPixelShader(nullptr);
		}

		// #
		// draw prim

		for (auto index = 0u; index < smodel_count; index++)
		{
			const auto inst = &draw_inst[*((std::uint16_t*)&smodel_list->primDrawSurfPos + index)];

			// transform model into the scene by updating the worldmatrix
			R_DrawStaticModelDrawSurfPlacement(src, inst, render_ff);
			dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&src->matrices.matrix[0].m));

			// get indexbuffer offset
			const auto offset = ((char*)drawstream->localSurf->triIndices - mem->blocks[8].data) >> 1;

			// draw the prim
			cmd->prim.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, drawstream->localSurf->vertCount, offset, drawstream->localSurf->triCount);
		}

		if (render_ff)
		{
			// #
			// restore everything for following meshes rendered via shaders

			if (og_vertex_shader) dev->SetVertexShader(og_vertex_shader);
			if (og_pixel_shader) dev->SetPixelShader(og_pixel_shader);

			// restore world matrix
			rtx::r_set_3d();
			dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->matrices.matrix[0].m));

			dev->SetFVF(NULL);
		}
	}


	// *
	// xmodels (rigid/skinned)

	void R_DrawXModelRigidModelSurf(game::GfxModelRigidSurface* model, [[maybe_unused]] game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state)
	{
		const auto dev = game::glob::d3d9_device;
		const auto surf = model->surf.xsurf;

		// fixed function rendering
		bool rendering_ff = true;

		if (surf->deformed)
		{
			__debugbreak();
		}

		// build custom vertexbuffer for dynamically spawned models
		if (!surf->custom_vertexbuffer)
		{
			bool allocated_any = XSurfaceOptimize(surf);
#if DEBUG
			if (allocated_any && state->material && state->material->info.name)
			{
				game::Com_PrintMessage(0, utils::va("Allocated buffers for smodel using material '%s'\n", state->material->info.name), 0);
			}
#endif
		}

		// #
		// set streams

		// index and vertex buffers are within the fastfile that includes the model
		const auto mem = &game::g_zones[surf->zoneHandle].mem;

		state->prim.indexBuffer = mem->indexBuffer;
		state->prim.device->SetIndices(mem->indexBuffer);

		float custom_scalar = 1.0f;

		// skysphere materials need to have sort = sky in assetmanager
		if (state->material && state->material->info.sortKey == 5)
		{
			custom_scalar = rtx_gui::skysphere_scale;

			// disable fog for skysphere
			state->prim.device->SetRenderState(D3DRS_FOGENABLE, FALSE);
		}

		// #
		// build world matrix

		float model_axis[3][3] = {};
		utils::vector::unit_quat_to_axis(model->placement.base.quat, model_axis);

		//const auto mtx = source->matrices.matrix[0].m;
		float mtx[4][4] = {};
		const auto scale = model->placement.scale;

		// inlined ikMatrixSet44
		mtx[0][0] = model_axis[0][0] * scale * custom_scalar;
		mtx[0][1] = model_axis[0][1] * scale * custom_scalar;
		mtx[0][2] = model_axis[0][2] * scale * custom_scalar;
		mtx[0][3] = 0.0f;

		mtx[1][0] = model_axis[1][0] * scale * custom_scalar;
		mtx[1][1] = model_axis[1][1] * scale * custom_scalar;
		mtx[1][2] = model_axis[1][2] * scale * custom_scalar;
		mtx[1][3] = 0.0f;

		mtx[2][0] = model_axis[2][0] * scale * custom_scalar;
		mtx[2][1] = model_axis[2][1] * scale * custom_scalar;
		mtx[2][2] = model_axis[2][2] * scale * custom_scalar;
		mtx[2][3] = 0.0f;

		mtx[3][0] = model->placement.base.origin[0];
		mtx[3][1] = model->placement.base.origin[1];
		mtx[3][2] = model->placement.base.origin[2];
		mtx[3][3] = 1.0f;


		IDirect3DVertexShader9* og_vertex_shader = nullptr;
		IDirect3DPixelShader9* og_pixel_shader = nullptr;

		if (surf->custom_vertexbuffer)
		{
			// save shaders
			dev->GetVertexShader(&og_vertex_shader);
			dev->GetPixelShader(&og_pixel_shader);

			// def. needed or the game will render the mesh using shaders
			dev->SetVertexShader(nullptr);
			dev->SetPixelShader(nullptr);

			state->prim.streams[0].vb = surf->custom_vertexbuffer;
			state->prim.streams[0].offset = 0;
			state->prim.streams[0].stride = MODEL_VERTEX_STRIDE;
			state->prim.device->SetStreamSource(0, surf->custom_vertexbuffer, 0, MODEL_VERTEX_STRIDE);

			// vertex format
			dev->SetFVF(MODEL_VERTEX_FORMAT);

			// set world matrix
			dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&mtx));
		}
		else // fallback to shader rendering in case there is an xmodel without a custom vertexbuffer (shouldn't happen?)
		{
			rendering_ff = false;
			const auto offset = (char*)surf->verts0 - mem->blocks[7].data;

			state->prim.streams[0].vb = mem->vertexBuffer;
			state->prim.streams[0].offset = offset;
			state->prim.streams[0].stride = MODEL_VERTEX_STRIDE;
			dev->SetStreamSource(0, mem->vertexBuffer, offset, MODEL_VERTEX_STRIDE);

			// setup vertex shader constants
			const auto primArgCount = static_cast<unsigned int>(state->pass->perPrimArgCount);
			const auto materialArgs = state->pass->args;

			if (primArgCount)
			{
				// R_SetupPassPerPrimArgs
				const static uint32_t func_addr = 0x64BC50;
				__asm
				{
					pushad;
					push	state;
					push	source;
					push	primArgCount;
					mov		eax, materialArgs;
					call	func_addr;
					add		esp, 12;
					popad;
				}
			}
		}

		// #
		// draw prim

		const auto offset = ((char*)surf->triIndices - mem->blocks[8].data) >> 1;
		state->prim.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, surf->vertCount, offset, surf->triCount);


		// #
		// restore everything for following meshes rendered via shaders

		if (rendering_ff)
		{
			if (og_vertex_shader) dev->SetVertexShader(og_vertex_shader);
			if (og_pixel_shader) dev->SetPixelShader(og_pixel_shader);
			dev->SetFVF(NULL);
		}
	}

	__declspec(naked) void R_DrawXModelRigidModelSurf1_stub()
	{
		const static uint32_t retn_addr = 0x657231;
		__asm
		{
			// state pushed
			// source pushed
			push	eax; // GfxModelRigidSurface
			call	R_DrawXModelRigidModelSurf;
			add		esp, 4;
			jmp		retn_addr;
		}
	}

	__declspec(naked) void R_DrawXModelRigidModelSurf2_stub()
	{
		const static uint32_t retn_addr = 0x6575CA;
		__asm
		{
			// state pushed
			// source pushed
			push	ecx; // GfxModelRigidSurface
			call	R_DrawXModelRigidModelSurf;
			add		esp, 4;
			jmp		retn_addr;
		}
	}

	// ------------------------

	int R_SetIndexData(game::GfxCmdBufPrimState* state, const std::uint16_t* indices, std::uint32_t tri_count)
	{
		const auto index_count = 3 * tri_count;

		if (static_cast<int>(index_count) + game::gfx_buf->dynamicIndexBuffer->used > game::gfx_buf->dynamicIndexBuffer->total)
		{
			game::gfx_buf->dynamicIndexBuffer->used = 0;
		}

		if (!game::gfx_buf->dynamicIndexBuffer->used)
		{
			game::gfx_buf->dynamicIndexBuffer = game::gfx_buf->dynamicIndexBufferPool;
		}

		void* buffer_data;
		if (const auto hr = game::gfx_buf->dynamicIndexBuffer->buffer->Lock(2 * game::gfx_buf->dynamicIndexBuffer->used, 6 * tri_count, &buffer_data, game::gfx_buf->dynamicIndexBuffer->used != 0 ? 0x1000 : 0x2000);
			hr < 0)
		{
			//R_FatalLockError(hr);
			game::Com_Error(game::ERR_DROP, "Fatal lock error :: R_SetIndexData");
		}

		memcpy(buffer_data, indices, 2 * index_count);
		game::gfx_buf->dynamicIndexBuffer->buffer->Unlock();

		if (state->indexBuffer != game::gfx_buf->dynamicIndexBuffer->buffer)
		{
			state->indexBuffer = game::gfx_buf->dynamicIndexBuffer->buffer;
			state->device->SetIndices(game::gfx_buf->dynamicIndexBuffer->buffer);
		}

		const auto base_index = game::gfx_buf->dynamicIndexBuffer->used;
		game::gfx_buf->dynamicIndexBuffer->used += static_cast<int>(index_count);

		return base_index;
	}

	void R_DrawXModelSkinnedUncached(game::GfxModelSkinnedSurface* skinned_surf, game::GfxCmdBufSourceState* src, game::GfxCmdBufState* state)
	{
		// fixed function rendering
		constexpr auto render_ff = true;

		const auto surf = skinned_surf->xsurf;
		const auto start_index = R_SetIndexData(&state->prim, surf->triIndices, surf->triCount);

		if ((int)(MODEL_VERTEX_STRIDE * surf->vertCount + game::gfx_buf->dynamicVertexBuffer->used) > game::gfx_buf->dynamicVertexBuffer->total)
		{
			game::gfx_buf->dynamicVertexBuffer->used = 0;
		}

		// R_SetVertexData
		void* buffer_data;
		if (const auto hr = game::gfx_buf->dynamicVertexBuffer->buffer->Lock(game::gfx_buf->dynamicVertexBuffer->used, MODEL_VERTEX_STRIDE * surf->vertCount, &buffer_data, game::gfx_buf->dynamicVertexBuffer->used != 0 ? 0x1000 : 0x2000);
			hr < 0)
		{
			//R_FatalLockError(hr);
			game::Com_Error(game::ERR_DROP, "Fatal lock error :: R_DrawXModelSkinnedUncached");
		}

		if (!render_ff)
		{
			memcpy(buffer_data, skinned_surf->u.skinnedVert, MODEL_VERTEX_STRIDE * surf->vertCount);
		}
		else
		{
			for (auto i = 0u; i < surf->vertCount; i++)
			{
				// packed source vertex
				const auto src_vert = skinned_surf->u.skinnedVert[i];

				// position of our unpacked vert within the vertex buffer
				const auto v_pos_in_buffer = i * MODEL_VERTEX_STRIDE;
				const auto v = reinterpret_cast<unpacked_model_vert*>(((DWORD)buffer_data + v_pos_in_buffer));

				// vert pos
				v->pos[0] = src_vert.xyz[0];
				v->pos[1] = src_vert.xyz[1];
				v->pos[2] = src_vert.xyz[2];

				// unpack and assign vert normal

				// normal unpacking in a cod4 hlsl shader:
				// temp0	 = i.normal * float4(0.007874016, 0.007874016, 0.007874016, 0.003921569) + float4(-1, -1, -1, 0.7529412);
				// temp0.xyz = temp0.www * temp0;

				const auto scale = static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[3])) * (1.0f / 255.0f) + 0.7529412f;
				v->normal[0] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[0])) * (1.0f / 127.0f) + -1.0f) * scale;
				v->normal[1] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[1])) * (1.0f / 127.0f) + -1.0f) * scale;
				v->normal[2] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[2])) * (1.0f / 127.0f) + -1.0f) * scale;

				// uv's
				game::Vec2UnpackTexCoords(src_vert.texCoord.packed, v->texcoord);
			}

		}

		game::gfx_buf->dynamicVertexBuffer->buffer->Unlock();
		const std::uint32_t vert_offset = game::gfx_buf->dynamicVertexBuffer->used;
		game::gfx_buf->dynamicVertexBuffer->used += (MODEL_VERTEX_STRIDE * surf->vertCount);


		// #
		// #

		if (state->prim.streams[0].vb != game::gfx_buf->dynamicVertexBuffer->buffer || state->prim.streams[0].offset != vert_offset || state->prim.streams[0].stride != MODEL_VERTEX_STRIDE)
		{
			state->prim.streams[0].vb = game::gfx_buf->dynamicVertexBuffer->buffer;
			state->prim.streams[0].offset = vert_offset;
			state->prim.streams[0].stride = MODEL_VERTEX_STRIDE;
			state->prim.device->SetStreamSource(0, game::gfx_buf->dynamicVertexBuffer->buffer, vert_offset, MODEL_VERTEX_STRIDE);
		}

		IDirect3DVertexShader9* og_vertex_shader;
		IDirect3DPixelShader9* og_pixel_shader;

		if (render_ff)
		{
			// save shaders
			state->prim.device->GetVertexShader(&og_vertex_shader);
			state->prim.device->GetPixelShader(&og_pixel_shader);

			// needed or game renders mesh with shaders
			state->prim.device->SetVertexShader(nullptr);
			state->prim.device->SetPixelShader(nullptr);

			// vertex format
			state->prim.device->SetFVF(MODEL_VERTEX_FORMAT);


			// #
			// build world matrix

			float model_axis[3][3] = {};
			utils::vector::unit_quat_to_axis(src->skinnedPlacement.base.quat, model_axis);

			//const auto mtx = source->matrices.matrix[0].m;
			float mtx[4][4] = {};
			const auto scale = src->skinnedPlacement.scale;

			// inlined ikMatrixSet44
			mtx[0][0] = model_axis[0][0] * scale;
			mtx[0][1] = model_axis[0][1] * scale;
			mtx[0][2] = model_axis[0][2] * scale;
			mtx[0][3] = 0.0f;

			mtx[1][0] = model_axis[1][0] * scale;
			mtx[1][1] = model_axis[1][1] * scale;
			mtx[1][2] = model_axis[1][2] * scale;
			mtx[1][3] = 0.0f;

			mtx[2][0] = model_axis[2][0] * scale;
			mtx[2][1] = model_axis[2][1] * scale;
			mtx[2][2] = model_axis[2][2] * scale;
			mtx[2][3] = 0.0f;

			mtx[3][0] = src->skinnedPlacement.base.origin[0];
			mtx[3][1] = src->skinnedPlacement.base.origin[1];
			mtx[3][2] = src->skinnedPlacement.base.origin[2];
			mtx[3][3] = 1.0f;

			// set world matrix
			state->prim.device->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&mtx));
		}

		state->prim.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, surf->vertCount, start_index, surf->triCount);

		if (render_ff)
		{
			state->prim.device->SetFVF(NULL);
			state->prim.device->SetVertexShader(og_vertex_shader);
			state->prim.device->SetPixelShader(og_pixel_shader);
		}
	}

	__declspec(naked) void R_DrawXModelSkinnedUncached_stub()
	{
		const static uint32_t retn_addr = 0x646ED9;
		__asm
		{
			// GfxPackedVertex (skinnedVert) pushed (we ignore that because we push GfxModelSkinnedSurface which holds it)
			// state pushed
			// source pushed
			push	eax; // GfxModelSkinnedSurface
			call	R_DrawXModelSkinnedUncached;
			add		esp, 4;
			jmp		retn_addr;
		}
	}

	void R_DrawStaticModelsSkinnedDrawSurf(game::GfxStaticModelDrawStream* draw_stream, game::GfxCmdBufSourceState* src, game::GfxCmdBufState* state)
	{
		const auto surf = draw_stream->localSurf;
		const auto start_index = R_SetIndexData(&state->prim, surf->triIndices, surf->triCount);

		if (!surf->deformed && surf->custom_vertexbuffer)
		{
			if (state->prim.streams[0].vb != surf->custom_vertexbuffer || state->prim.streams[0].offset != 0 || state->prim.streams[0].stride != MODEL_VERTEX_STRIDE)
			{
				state->prim.streams[0].vb = surf->custom_vertexbuffer;
				state->prim.streams[0].offset = 0;
				state->prim.streams[0].stride = MODEL_VERTEX_STRIDE;
				state->prim.device->SetStreamSource(0, surf->custom_vertexbuffer, 0, MODEL_VERTEX_STRIDE);
			}
		}
		else
		{
			if ((int)(MODEL_VERTEX_STRIDE * surf->vertCount + game::gfx_buf->dynamicVertexBuffer->used) > game::gfx_buf->dynamicVertexBuffer->total)
			{
				game::gfx_buf->dynamicVertexBuffer->used = 0;
			}

			// R_SetVertexData
			void* buffer_data;
			if (const auto hr = game::gfx_buf->dynamicVertexBuffer->buffer->Lock(game::gfx_buf->dynamicVertexBuffer->used, MODEL_VERTEX_STRIDE * surf->vertCount, &buffer_data, game::gfx_buf->dynamicVertexBuffer->used != 0 ? 0x1000 : 0x2000);
				hr < 0)
			{
				//R_FatalLockError(hr);
				game::Com_Error(game::ERR_DROP, "Fatal lock error :: R_DrawXModelSkinnedUncached");
			}
			{
				for (auto i = 0u; i < surf->vertCount; i++)
				{
					// packed source vertex
					const auto src_vert = surf->verts0[i];

					// position of our unpacked vert within the vertex buffer
					const auto v_pos_in_buffer = i * MODEL_VERTEX_STRIDE;
					const auto v = reinterpret_cast<unpacked_model_vert*>(((DWORD)buffer_data + v_pos_in_buffer));

					// vert pos
					v->pos[0] = src_vert.xyz[0];
					v->pos[1] = src_vert.xyz[1];
					v->pos[2] = src_vert.xyz[2];

					// unpack and assign vert normal

					// normal unpacking in a cod4 hlsl shader:
					// temp0	 = i.normal * float4(0.007874016, 0.007874016, 0.007874016, 0.003921569) + float4(-1, -1, -1, 0.7529412);
					// temp0.xyz = temp0.www * temp0;

					const auto scale = static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[3])) * (1.0f / 255.0f) + 0.7529412f;
					v->normal[0] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[0])) * (1.0f / 127.0f) + -1.0f) * scale;
					v->normal[1] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[1])) * (1.0f / 127.0f) + -1.0f) * scale;
					v->normal[2] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[2])) * (1.0f / 127.0f) + -1.0f) * scale;

					// uv's
					game::Vec2UnpackTexCoords(src_vert.texCoord.packed, v->texcoord);
				}
			}

			game::gfx_buf->dynamicVertexBuffer->buffer->Unlock();
			const std::uint32_t vert_offset = game::gfx_buf->dynamicVertexBuffer->used;
			game::gfx_buf->dynamicVertexBuffer->used += (MODEL_VERTEX_STRIDE * surf->vertCount);

			// #
			// #

			if (state->prim.streams[0].vb != game::gfx_buf->dynamicVertexBuffer->buffer || state->prim.streams[0].offset != vert_offset || state->prim.streams[0].stride != MODEL_VERTEX_STRIDE)
			{
				state->prim.streams[0].vb = game::gfx_buf->dynamicVertexBuffer->buffer;
				state->prim.streams[0].offset = vert_offset;
				state->prim.streams[0].stride = MODEL_VERTEX_STRIDE;
				state->prim.device->SetStreamSource(0, game::gfx_buf->dynamicVertexBuffer->buffer, vert_offset, MODEL_VERTEX_STRIDE);
			}
		}

		IDirect3DVertexShader9* og_vertex_shader;
		IDirect3DPixelShader9* og_pixel_shader;

		{
			// save shaders
			state->prim.device->GetVertexShader(&og_vertex_shader);
			state->prim.device->GetPixelShader(&og_pixel_shader);

			// needed or game renders mesh with shaders
			state->prim.device->SetVertexShader(nullptr);
			state->prim.device->SetPixelShader(nullptr);

			// vertex format
			state->prim.device->SetFVF(MODEL_VERTEX_FORMAT);

			// #
			// build world matrix

			for (auto index = 0u; index < draw_stream->smodelCount; index++)
			{
				const auto inst = &game::rgp->world->dpvs.smodelDrawInsts[draw_stream->smodelList[index]];

				// transform model into the scene by updating the worldmatrix
				//R_DrawStaticModelDrawSurfPlacement(src, inst);
				//state->prim.device->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&src->matrices.matrix[0].m));

				float mtx[4][4] = {};
				const auto scale = src->skinnedPlacement.scale;

				// inlined ikMatrixSet44
				mtx[0][0] = inst->placement.axis[0][0] * scale;
				mtx[0][1] = inst->placement.axis[0][1] * scale;
				mtx[0][2] = inst->placement.axis[0][2] * scale;
				mtx[0][3] = 0.0f;

				mtx[1][0] = inst->placement.axis[1][0] * scale;
				mtx[1][1] = inst->placement.axis[1][1] * scale;
				mtx[1][2] = inst->placement.axis[1][2] * scale;
				mtx[1][3] = 0.0f;

				mtx[2][0] = inst->placement.axis[2][0] * scale;
				mtx[2][1] = inst->placement.axis[2][1] * scale;
				mtx[2][2] = inst->placement.axis[2][2] * scale;
				mtx[2][3] = 0.0f;

				mtx[3][0] = inst->placement.origin[0];
				mtx[3][1] = inst->placement.origin[1];
				mtx[3][2] = inst->placement.origin[2];
				mtx[3][3] = 1.0f;

				// set world matrix
				state->prim.device->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&mtx));

				// draw the prim
				state->prim.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, surf->vertCount, start_index, surf->triCount);
			}
		}

		{
			state->prim.device->SetFVF(NULL);
			state->prim.device->SetVertexShader(og_vertex_shader);
			state->prim.device->SetPixelShader(og_pixel_shader);
		}
	}
	
	


	// *
	// world (bsp/terrain) drawing

	unsigned int R_ReadPrimDrawSurfInt(game::GfxReadCmdBuf* cmdBuf)
	{
		return *cmdBuf->primDrawSurfPos++;
	}

	const unsigned int* R_ReadPrimDrawSurfData(game::GfxReadCmdBuf* cmdBuf, unsigned int count)
	{
		const auto pos = cmdBuf->primDrawSurfPos;
		cmdBuf->primDrawSurfPos += count;

		return pos;
	}

	bool R_ReadBspPreTessDrawSurfs(game::GfxReadCmdBuf* cmdBuf, game::GfxBspPreTessDrawSurf** list, unsigned int* count, unsigned int* baseIndex)
	{
		*count = R_ReadPrimDrawSurfInt(cmdBuf);
		if (!*count)
		{
			return false;
		}

		*baseIndex = R_ReadPrimDrawSurfInt(cmdBuf);
		*list = (game::GfxBspPreTessDrawSurf*)R_ReadPrimDrawSurfData(cmdBuf, *count);

		return true;
	}

	void R_DrawPreTessTris(game::GfxCmdBufSourceState* src, game::GfxCmdBufPrimState* state, game::srfTriangles_t* tris, unsigned int baseIndex, unsigned int triCount)
	{
		const auto dev = game::glob::d3d9_device;

		src->matrices.matrix[0].m[3][0] = 0.0f;
		src->matrices.matrix[0].m[3][1] = 0.0f;
		src->matrices.matrix[0].m[3][2] = 0.0f;
		src->matrices.matrix[0].m[3][3] = 1.0f;
		dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&src->matrices.matrix[0].m));

		// texture alpha + vertex alpha (decal blending)
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
#if DEBUG
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, rtx_gui::d3d_alpha_blend);
#else
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
#endif

		dev->SetStreamSource(0, gfx_world_vertexbuffer, WORLD_VERTEX_STRIDE * tris->firstVertex, WORLD_VERTEX_STRIDE);
		state->device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, tris->vertexCount, baseIndex, triCount);
	}

	/**
	 * @brief completely rewritten R_DrawBspDrawSurfsPreTess to render bsp surfaces (world) using the fixed-function pipeline
	 */
	void R_DrawBspDrawSurfsPreTess(const unsigned int* primDrawSurfPos, game::GfxCmdBufSourceState* src, game::GfxCmdBufState* state)
	{
		const auto dev = game::glob::d3d9_device;

		// #
		// setup fixed-function

		// vertex format
		dev->SetFVF(WORLD_VERTEX_FORMAT);

		// save shaders
		IDirect3DVertexShader9* og_vertex_shader;
		dev->GetVertexShader(&og_vertex_shader);

		IDirect3DPixelShader9* og_pixel_shader;
		dev->GetPixelShader(&og_pixel_shader);

		// def. needed or the game will render the mesh using shaders
		dev->SetVertexShader(nullptr);
		dev->SetPixelShader(nullptr);


		// #
		// draw prims

		unsigned int base_index, count;
		game::GfxBspPreTessDrawSurf* list;
		game::GfxReadCmdBuf cmd_buf = { primDrawSurfPos };

		while (R_ReadBspPreTessDrawSurfs(&cmd_buf, &list, &count, &base_index))
		{
			game::srfTriangles_t* prev_tris = nullptr;
			auto tri_count = 0u;
			auto base_vertex = -1;

			for (auto index = 0u; index < count; ++index)
			{
				const auto surf_index = static_cast<unsigned>(list[index].baseSurfIndex);
				if (surf_index >= static_cast<unsigned>(game::rgp->world->surfaceCount))
				{
					__debugbreak();
					game::Com_Error(game::ERR_DISCONNECT, "R_DrawBspDrawSurfsPreTess :: surf_index >= static_cast<unsigned>(game::rgp->world->surfaceCount)");
				}

				const auto bsp_surf = &game::rgp->world->dpvs.surfaces[surf_index];
				const auto tris = &bsp_surf->tris;

				if (base_vertex != bsp_surf->tris.firstVertex)
				{
					// never triggers?
					if (tri_count)
					{
						R_DrawPreTessTris(src, &state->prim, prev_tris, base_index, tri_count);
						base_index += 3 * tri_count;
						tri_count = 0;
					}

					prev_tris = tris;
					base_vertex = tris->firstVertex;
				}

				tri_count += list[index].totalTriCount;
			}

			R_DrawPreTessTris(src, &state->prim, prev_tris, base_index, tri_count);
		}

		// #
		// restore everything for following meshes rendered via shaders

		dev->SetVertexShader(og_vertex_shader);
		dev->SetPixelShader(og_pixel_shader);

		// restore world matrix
		rtx::r_set_3d();
		dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->matrices.matrix[0].m));

		dev->SetFVF(NULL);
	}

	bool R_ReadBspDrawSurfs(const unsigned int** primDrawSurfPos, const unsigned __int16** list, unsigned int* count)
	{
		*count = *(*primDrawSurfPos)++;
		if (!*count)
		{
			return false;
		}
		*list = (const unsigned __int16*)*primDrawSurfPos;
		*primDrawSurfPos += (*count + 1) >> 1;
		return true;
	}

	void R_DrawBspTris(game::GfxCmdBufSourceState* src, game::GfxCmdBufPrimState* state, game::srfTriangles_t* tris, unsigned int baseIndex, unsigned int triCount)
	{
		const auto dev = game::glob::d3d9_device;

		src->matrices.matrix[0].m[3][0] = 0.0f;
		src->matrices.matrix[0].m[3][1] = 0.0f;
		src->matrices.matrix[0].m[3][2] = 0.0f;
		src->matrices.matrix[0].m[3][3] = 1.0f;
		dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&src->matrices.matrix[0].m));

		// texture alpha + vertex alpha (decal blending)
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
#if DEBUG
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, rtx_gui::d3d_alpha_blend);
#else
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
#endif

		//dev->SetStreamSource(0, gfx_world_vertexbuffer, WORLD_VERTEX_STRIDE * tris->firstVertex, WORLD_VERTEX_STRIDE);
		state->device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, tris->vertexCount, baseIndex, triCount);
	}

	void R_DrawBspDrawSurfs(game::GfxTrianglesDrawStream* drawStream, game::GfxCmdBufPrimState* state)
	{
		const auto dev = game::glob::d3d9_device;

		// #
		// setup fixed-function

		// vertex format
		dev->SetFVF(WORLD_VERTEX_FORMAT);

		// save shaders
		IDirect3DVertexShader9* og_vertex_shader;
		dev->GetVertexShader(&og_vertex_shader);

		IDirect3DPixelShader9* og_pixel_shader;
		dev->GetPixelShader(&og_pixel_shader);

		// def. needed or the game will render the mesh using shaders
		dev->SetVertexShader(nullptr);
		dev->SetPixelShader(nullptr);


		// #
		// draw prims

		const std::uint16_t* list;
		game::srfTriangles_t* prev_tris = nullptr;
		unsigned int base_index = 0u, count = 0u, tri_count = 0u;
		auto base_vertex = -1;

		while (R_ReadBspDrawSurfs(&drawStream->primDrawSurfPos, &list, &count))
		{
			for (auto index = 0u; index < count; ++index)
			{
				const auto bsp_surf = &game::rgp->world->dpvs.surfaces[list[index]];
				if (base_vertex != bsp_surf->tris.firstVertex || base_index + 3u * tri_count != static_cast<unsigned>(bsp_surf->tris.baseIndex))
				{
					if (prev_tris)
					{
						const auto base = R_SetIndexData(state, &game::rgp->world->indices[prev_tris->baseIndex], tri_count);
						R_DrawBspTris(game::gfxCmdBufSourceState, state, prev_tris, base, tri_count);
					}

					tri_count = 0;
					prev_tris  = &bsp_surf->tris;
					base_index = bsp_surf->tris.baseIndex;

					if (base_vertex != bsp_surf->tris.firstVertex)
					{
						base_vertex = bsp_surf->tris.firstVertex;
						dev->SetStreamSource(0, gfx_world_vertexbuffer, WORLD_VERTEX_STRIDE * bsp_surf->tris.firstVertex, WORLD_VERTEX_STRIDE);
					}
				}

				tri_count += bsp_surf->tris.triCount;
			}
		}

		if (prev_tris)
		{
			const auto base = R_SetIndexData(state, &game::rgp->world->indices[prev_tris->baseIndex], tri_count);
			R_DrawBspTris(game::gfxCmdBufSourceState, state, prev_tris, base, tri_count);
		}

		// #
		// restore everything for following meshes rendered via shaders

		dev->SetVertexShader(og_vertex_shader);
		dev->SetPixelShader(og_pixel_shader);

		// restore world matrix
		rtx::r_set_3d();
		dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->matrices.matrix[0].m));
		dev->SetFVF(NULL);
	}

	/**
	 * @brief completely rewritten R_TessBModel to render brushmodels using the fixed-function pipeline
	 * - most challenging issue yet
	 */
	std::uint32_t R_TessBModel(game::GfxDrawSurfListArgs* listArgs, [[maybe_unused]] void* x, [[maybe_unused]] void* y)
	{
		const auto source = listArgs->context.source;
		const auto prim = &listArgs->context.state->prim;

		// #
		// setup fixed-function

		// vertex format
		prim->device->SetFVF(WORLD_VERTEX_FORMAT);

		// save shaders
		IDirect3DVertexShader9* og_vertex_shader;
		prim->device->GetVertexShader(&og_vertex_shader);

		IDirect3DPixelShader9* og_pixel_shader;
		prim->device->GetPixelShader(&og_pixel_shader);

		// def. needed or the game will render the mesh using shaders
		prim->device->SetVertexShader(nullptr);
		prim->device->SetPixelShader(nullptr);

		// texture alpha + vertex alpha (decal blending)
		prim->device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		prim->device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		prim->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);


		// #
		// draw prims

		const auto draw_surf_list = &listArgs->info->drawSurfs[listArgs->firstDrawSurfIndex];
		const auto draw_surf_count = listArgs->info->drawSurfCount - listArgs->firstDrawSurfIndex;

		const auto draw_surf_sub_mask = 0xFFFFFFFFE0000000;

		game::GfxDrawSurf draw_surf = {};
		draw_surf.fields = draw_surf_list->fields;

		game::GfxDrawSurf draw_surf_key = {};
		draw_surf_key.packed = draw_surf.packed & draw_surf_sub_mask;

		const std::uint64_t first_surf = draw_surf_key.packed;
		auto draw_surf_index = 0u;

		while (true)
		{
			const auto bsurf = reinterpret_cast<game::BModelSurface*>(&source->input.data->surfsBuffer[4u * draw_surf.fields.objectId]);

			// #
			// transform

			float model_axis[3][3] = {};

			const auto placement = bsurf->placement;
			utils::vector::unit_quat_to_axis(placement->base.quat, model_axis);
			const auto scale = placement->scale;

			//const auto mtx = source->matrices.matrix[0].m;
			float mtx[4][4] = {};

			// inlined ikMatrixSet44
			mtx[0][0] = model_axis[0][0] * scale;
			mtx[0][1] = model_axis[0][1] * scale;
			mtx[0][2] = model_axis[0][2] * scale;
			mtx[0][3] = 0.0f;

			mtx[1][0] = model_axis[1][0] * scale;
			mtx[1][1] = model_axis[1][1] * scale;
			mtx[1][2] = model_axis[1][2] * scale;
			mtx[1][3] = 0.0f;

			mtx[2][0] = model_axis[2][0] * scale;
			mtx[2][1] = model_axis[2][1] * scale;
			mtx[2][2] = model_axis[2][2] * scale;
			mtx[2][3] = 0.0f;

			mtx[3][0] = placement->base.origin[0];
			mtx[3][1] = placement->base.origin[1];
			mtx[3][2] = placement->base.origin[2];
			mtx[3][3] = 1.0f;

			prim->device->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&mtx));

			// #
			// ------

			const auto gfxsurf = bsurf->surf;
			const auto base_vertex = WORLD_VERTEX_STRIDE * gfxsurf->tris.firstVertex;

			if (prim->streams[0].vb != gfx_world_vertexbuffer || prim->streams[0].offset != base_vertex || prim->streams[0].stride != WORLD_VERTEX_STRIDE)
			{
				prim->streams[0].vb = gfx_world_vertexbuffer;
				prim->streams[0].offset = base_vertex;
				prim->streams[0].stride = WORLD_VERTEX_STRIDE;
				prim->device->SetStreamSource(0, gfx_world_vertexbuffer, base_vertex, WORLD_VERTEX_STRIDE);
			}

			const auto base_index = R_SetIndexData(prim, &game::rgp->world->indices[gfxsurf->tris.baseIndex], gfxsurf->tris.triCount);
			prim->device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, gfxsurf->tris.vertexCount, base_index, gfxsurf->tris.triCount);

			++draw_surf_index;
			if (draw_surf_index == draw_surf_count)
			{
				break;
			}

			draw_surf.fields = draw_surf_list[draw_surf_index].fields;
			draw_surf_key.packed = draw_surf_list[draw_surf_index].packed & draw_surf_sub_mask;

			if (draw_surf_key.packed != first_surf)
			{
				break;
			}
		}

		//prim->device->SetVertexShader(og_vertex_shader);
		//prim->device->SetPixelShader(og_pixel_shader);
		prim->device->SetFVF(NULL);
		return draw_surf_index;
	}


	// *
	// effects

	IDirect3DVertexShader9* _og_codemesh_vertex_shader;
	IDirect3DPixelShader9* _og_codemesh_pixel_shader;
	//constexpr auto MAX_EFFECT_VERTS = 0x1000; // !ADJUST hook::set (2x)

	void R_TessCodeMeshList_begin(game::GfxDrawSurfListArgs* listArgs)
	{
		const auto source = listArgs->context.source;
		const auto prim = &listArgs->context.state->prim;

		// #
		// setup fixed-function

		// vertex format
		prim->device->SetFVF(MODEL_VERTEX_FORMAT);

		// save shaders
		prim->device->GetVertexShader(&_og_codemesh_vertex_shader);
		prim->device->GetPixelShader(&_og_codemesh_pixel_shader);

		// def. needed or the game will render the mesh using shaders
		prim->device->SetVertexShader(nullptr);
		prim->device->SetPixelShader(nullptr);

		// texture alpha + vertex alpha (decal blending)
		//prim->device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		//prim->device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		//prim->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_BLENDFACTORALPHA);

		// #
		// vertices are already in 'world space' so origin is at 0 0 0

		// inlined ikMatrixSet44
		float mtx[4][4] = {};
		mtx[0][0] = 1.0f; mtx[0][1] = 0.0f; mtx[0][2] = 0.0f; mtx[0][3] = 0.0f;
		mtx[1][0] = 0.0f; mtx[1][1] = 1.0f; mtx[1][2] = 0.0f; mtx[1][3] = 0.0f;
		mtx[2][0] = 0.0f; mtx[2][1] = 0.0f; mtx[2][2] = 1.0f; mtx[2][3] = 0.0f;
		mtx[3][0] = 0.0f; mtx[3][1] = 0.0f; mtx[3][2] = 0.0f; mtx[3][3] = 1.0f;

		prim->device->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&mtx));

		// #
		// unpack codemesh vertex data and place new data into the dynamic vertex buffer

		void* og_buffer_data;
		if (const auto hr = source->input.data->codeMesh.vb.buffer->Lock(0, source->input.data->codeMesh.vb.used, &og_buffer_data, D3DLOCK_READONLY);
			hr < 0)
		{
			//R_FatalLockError(hr);
			game::Com_Error(0, "Fatal lock error - codeMesh :: R_TessCodeMeshList_begin");
		}

		if ((int)(source->input.data->codeMesh.vb.used + game::gfx_buf->dynamicVertexBuffer->used) > game::gfx_buf->dynamicVertexBuffer->total)
		{
			game::gfx_buf->dynamicVertexBuffer->used = 0;
		}

		// R_SetVertexData
		void* buffer_data;
		if (const auto hr = game::gfx_buf->dynamicVertexBuffer->buffer->Lock(
			game::gfx_buf->dynamicVertexBuffer->used, source->input.data->codeMesh.vb.used, &buffer_data,
			game::gfx_buf->dynamicVertexBuffer->used != 0 ? 0x1000 : 0x2000);
			hr < 0)
		{
			//R_FatalLockError(hr);
			game::Com_Error(0, "Fatal lock error - dynamicVertexBuffer :: R_TessCodeMeshList_begin");
		}

		// #
		// unpack verts

		for (auto i = 0u; i * source->input.data->codeMesh.vertSize < (unsigned)source->input.data->codeMesh.vb.used && i < 0x4000; i++)
		{
			// position of vert within the vertex buffer
			const auto v_pos_in_buffer = i * source->input.data->codeMesh.vertSize; // size of GfxPackedVertex

			// interpret GfxPackedVertex as unpacked_model_vert
			const auto v = reinterpret_cast<unpacked_model_vert*>(((DWORD)buffer_data + v_pos_in_buffer));

			// interpret GfxPackedVertex as GfxPackedVertex 
			const auto src_vert = reinterpret_cast<game::GfxPackedVertex*>(((DWORD)og_buffer_data + v_pos_in_buffer));


			// vert pos
			v->pos[0] = src_vert->xyz[0];
			v->pos[1] = src_vert->xyz[1];
			v->pos[2] = src_vert->xyz[2];

			// normal unpacking in a cod4 hlsl shader:
			// temp0	 = i.normal * float4(0.007874016, 0.007874016, 0.007874016, 0.003921569) + float4(-1, -1, -1, 0.7529412);
			// temp0.xyz = temp0.www * temp0;

			const auto scale = static_cast<float>(static_cast<std::uint8_t>(src_vert->normal.array[3])) * (1.0f / 255.0f) + 0.7529412f;
			v->normal[0] = (static_cast<float>(static_cast<std::uint8_t>(src_vert->normal.array[0])) * (1.0f / 127.0f) + -1.0f) * scale;
			v->normal[1] = (static_cast<float>(static_cast<std::uint8_t>(src_vert->normal.array[1])) * (1.0f / 127.0f) + -1.0f) * scale;
			v->normal[2] = (static_cast<float>(static_cast<std::uint8_t>(src_vert->normal.array[2])) * (1.0f / 127.0f) + -1.0f) * scale;

			// uv's
			game::Vec2UnpackTexCoords(src_vert->texCoord.packed, v->texcoord);
		}

		source->input.data->codeMesh.vb.buffer->Unlock();
		game::gfx_buf->dynamicVertexBuffer->buffer->Unlock();

		const std::uint32_t vert_offset = game::gfx_buf->dynamicVertexBuffer->used;
		game::gfx_buf->dynamicVertexBuffer->used += source->input.data->codeMesh.vb.used;

		// #
		// #

		if (prim->streams[0].vb != game::gfx_buf->dynamicVertexBuffer->buffer || prim->streams[0].offset != vert_offset || prim->streams[0].stride != MODEL_VERTEX_STRIDE)
		{
			prim->streams[0].vb = game::gfx_buf->dynamicVertexBuffer->buffer;
			prim->streams[0].offset = vert_offset;
			prim->streams[0].stride = MODEL_VERTEX_STRIDE;
			prim->device->SetStreamSource(0, game::gfx_buf->dynamicVertexBuffer->buffer, vert_offset, MODEL_VERTEX_STRIDE);
		}

		// R_TessCodeMeshList :: game code will now render all codemesh drawsurfs - functions nop'd:
		// > R_UpdateVertexDecl - sets vertexshader
		// > R_SetStreamSource  - sets vertexbuffer (codeMesh.vb)
	}

	// reset ff at the end of R_TessCodeMeshList
	void R_TessCodeMeshList_end()
	{
		const auto dev = game::dx->device;
		//dev->SetVertexShader(_og_codemesh_vertex_shader);
		//dev->SetPixelShader(_og_codemesh_pixel_shader);
		dev->SetFVF(NULL);
	}

	__declspec(naked) void R_TessCodeMeshList_begin_stub()
	{
		const static uint32_t retn_addr = 0x645BB8;
		__asm
		{
			pushad;
			push	eax; // listArgs
			call	R_TessCodeMeshList_begin;
			add		esp, 4;
			popad;

			// og code
			mov     ebx, [ecx + 4];
			mov     ecx, [ecx];
			jmp		retn_addr;
		}
	}

	__declspec(naked) void R_TessCodeMeshList_end_stub()
	{
		const static uint32_t retn_addr = 0x645E1F;
		__asm
		{
			pushad;
			call	R_TessCodeMeshList_end;
			popad;

			// og code
			pop     edi;
			pop     esi;
			pop     ebx;
			mov     esp, ebp;
			jmp		retn_addr;
		}
	}


	// *
	// build buffers

	void build_gfxworld_buffers()
	{
		const auto dev = game::glob::d3d9_device;
		void* vertex_buffer_data = nullptr;

		if (game::gfx_world)
		{
			if (gfx_world_vertexbuffer)
			{
				gfx_world_vertexbuffer->Release();
				gfx_world_vertexbuffer = nullptr;

				__debugbreak();
				game::Com_Error(game::ERR_DISCONNECT, "build_gfxworld_buffers :: gfx_world_vertexbuffer != nullptr");
			}

			if (auto hr = dev->CreateVertexBuffer(WORLD_VERTEX_STRIDE * game::gfx_world->vertexCount, D3DUSAGE_WRITEONLY, WORLD_VERTEX_FORMAT, D3DPOOL_DEFAULT, &gfx_world_vertexbuffer, nullptr);
				hr >= 0)
			{	
				if (hr = gfx_world_vertexbuffer->Lock(0, 0, &vertex_buffer_data, 0);
					hr >= 0)
				{
					/*	struct GfxWorldVertex = 44 bytes
					{
						float xyz[3];
						float binormalSign;
						GfxColor color;
						float texCoord[2];
						float lmapCoord[2];
						PackedUnitVec normal;
						PackedUnitVec tangent;
					};*/

					// unpack normal so we can use fixed-function rendering with normals
					for (auto i = 0u; i < game::gfx_world->vertexCount; i++)
					{
						// packed source vertex
						const auto src_vert = game::gfx_world->vd.vertices[i];

						// position of our unpacked vert within the vertex buffer
						const auto v_pos_in_buffer = i * WORLD_VERTEX_STRIDE; // pos-xyz ; normal-xyz ; texcoords uv = 32 byte 
						const auto v = reinterpret_cast<unpacked_world_vert*>(((DWORD)vertex_buffer_data + v_pos_in_buffer));

						// vert pos
						v->pos[0] = src_vert.xyz[0];
						v->pos[1] = src_vert.xyz[1];
						v->pos[2] = src_vert.xyz[2];

						// unpack and assign vert normal

						// normal unpacking in a cod4 hlsl shader:
						// temp0	 = i.normal * float4(0.007874016, 0.007874016, 0.007874016, 0.003921569) + float4(-1, -1, -1, 0.7529412);
						// temp0.xyz = temp0.www * temp0;

						const auto scale =  static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[3])) * (1.0f/255.0f) + 0.7529412f;
						v->normal[0] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[0])) * (1.0f / 127.0f) + -1.0f) * scale;
						v->normal[1] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[1])) * (1.0f / 127.0f) + -1.0f) * scale;
						v->normal[2] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[2])) * (1.0f / 127.0f) + -1.0f) * scale;

						// packed vertex color : used for alpha blending of decals
						v->color = src_vert.color.packed;

						// uv's
						v->texcoord[0] = src_vert.texCoord[0];
						v->texcoord[1] = src_vert.texCoord[1];
					}

					gfx_world_vertexbuffer->Unlock();
				}
				else
				{
					gfx_world_vertexbuffer->Release();
					gfx_world_vertexbuffer = nullptr;
				}
			}
		}
	}

	void build_static_model_buffers()
	{
		// builds buffers at runtime if 'rtx_warm_smodels' is disabled
		if (dvars::rtx_warm_smodels && dvars::rtx_warm_smodels->current.enabled)
		{
			game::DB_EnumXAssets_FastFile(game::XAssetType::ASSET_TYPE_XMODEL, [](game::XAssetHeader header, [[maybe_unused]] void* data)
			{
				// ignore deformed (skinned) models because they use more then 2 partBits (partBit 3-4 are used for custom buffers)
				// vertex buffers for skinned models are created dynamically
				if (!header.model->surfs->deformed)
				{
					XModelOptimize(header.model, true);
				}
			}, nullptr, false);
		}
	}

	// called on map load (cg_init)
	__declspec(naked) void init_fixed_function_buffers_stub()
	{
		const static uint32_t stock_func = 0x431C80; // CG_NorthDirectionChanged
		const static uint32_t retn_addr = 0x440320;
		__asm
		{
			pushad;
			call	build_static_model_buffers;
			call	build_gfxworld_buffers;
			call	rtx::on_map_load;
			popad;

			call	stock_func;
			jmp		retn_addr;
		}
	}


	// *
	// cleanup buffers

	void free_fixed_function_buffers()
	{
		// #
		// cleanup world buffer

		if (gfx_world_vertexbuffer)
		{
			gfx_world_vertexbuffer->Release();
			gfx_world_vertexbuffer = nullptr;
		}

		// #
		// cleanup model buffer

		game::DB_EnumXAssets_FastFile(game::XAssetType::ASSET_TYPE_XMODEL, [](game::XAssetHeader header, [[maybe_unused]] void* data)
		{
			const auto dev = game::glob::d3d9_device;

			// ignore deformed (skinned) models because they might use more then 2 partBits (partBit 4 is used for custom vertex buffer)
			if (dev && !header.model->surfs->deformed && header.model->surfs->custom_vertexbuffer)
			{
				game::XSurface* surfaces;

				const auto lod_count = header.model->numLods;
				for (auto lod_index = 0; lod_index < lod_count; ++lod_index)
				{
					const auto surf_count = XModelGetSurfaces(header.model, &surfaces, lod_index);
					for (auto surf_index = 0; surf_index < surf_count; ++surf_index)
					{
						if (!surfaces[surf_index].deformed && surfaces[surf_index].custom_vertexbuffer)
						{
							surfaces[surf_index].custom_vertexbuffer->Release();
							surfaces[surf_index].custom_vertexbuffer = nullptr;
						}
					}
				}
			}
		}, nullptr, false);
	}

	// called on renderer shutdown (R_Shutdown)
	__declspec(naked) void free_fixed_function_buffers_stub()
	{
		const static uint32_t stock_func = 0x62F550; // R_ResetModelLighting
		const static uint32_t retn_addr = 0x5F5057;
		__asm
		{
			pushad;
			call	free_fixed_function_buffers;
			call	rtx::on_map_shutdown;
			popad;

			call	stock_func;
			jmp		retn_addr;
		}
	}


	// *
	// Tess techniques (2d, debug visualization etc.)

#ifdef TESS_TESTS
	namespace tess
	{
		int use_custom_tess_func()
		{
			if (const auto str = std::string_view(game::gfxCmdBufState->material->info.name);
				str == "$line" || str == "$line_nodepth" || str == "iw3xo_showcollision_wire" || str == "iw3xo_showcollision_fakelight")
			{
				return 1;
			}

			return 0;
		}

		void custom_tess_func(game::GfxDrawPrimArgs* args, [[maybe_unused]] game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state)
		{
			const int DEBUG_VERT_STRIDE = 16;
			const auto DEBUG_VERT_FORMAT = D3DFVF_XYZ | D3DFVF_DIFFUSE;

			if ((int)(DEBUG_VERT_STRIDE * game::tess->vertexCount + game::gfx_buf->dynamicVertexBuffer->used) > game::gfx_buf->dynamicVertexBuffer->total)
			{
				game::gfx_buf->dynamicVertexBuffer->used = 0;
			}

			// R_SetVertexData;
			void* buffer_data;
			if (const auto hr = game::gfx_buf->dynamicVertexBuffer->buffer->Lock(game::gfx_buf->dynamicVertexBuffer->used, DEBUG_VERT_STRIDE * game::tess->vertexCount, &buffer_data, game::gfx_buf->dynamicVertexBuffer->used != 0 ? 0x1000 : 0x2000);
				hr < 0)
			{
				game::Com_Error(game::ERR_DROP, "Fatal lock error :: R_DrawTessTechnique");
			}

			{
				struct debug_vert
				{
					game::vec3_t pos;
					DWORD color;
				};

				for (auto i = 0; i < game::tess->vertexCount; i++)
				{
					// packed source vertex
					const auto src_vert = game::tess->verts[i];

					// position of our unpacked vert within the vertex buffer
					const auto v_pos_in_buffer = i * DEBUG_VERT_STRIDE;
					const auto v = reinterpret_cast<debug_vert*>(((DWORD)buffer_data + v_pos_in_buffer));

					// vert pos
					v->pos[0] = src_vert.xyzw[0];
					v->pos[1] = src_vert.xyzw[1];
					v->pos[2] = src_vert.xyzw[2];

					v->color = src_vert.color.packed;
				}
			}

			game::gfx_buf->dynamicVertexBuffer->buffer->Unlock();
			const std::uint32_t vert_offset = game::gfx_buf->dynamicVertexBuffer->used;
			game::gfx_buf->dynamicVertexBuffer->used += (DEBUG_VERT_STRIDE * game::tess->vertexCount);


			// #
			// #

			if (state->prim.streams[0].vb != game::gfx_buf->dynamicVertexBuffer->buffer || state->prim.streams[0].offset != vert_offset || state->prim.streams[0].stride != DEBUG_VERT_STRIDE)
			{
				state->prim.streams[0].vb = game::gfx_buf->dynamicVertexBuffer->buffer;
				state->prim.streams[0].offset = vert_offset;
				state->prim.streams[0].stride = DEBUG_VERT_STRIDE;
				state->prim.device->SetStreamSource(0, game::gfx_buf->dynamicVertexBuffer->buffer, vert_offset, DEBUG_VERT_STRIDE);
			}

			IDirect3DVertexShader9* og_vertex_shader;
			IDirect3DPixelShader9* og_pixel_shader;

			// save shaders
			state->prim.device->GetVertexShader(&og_vertex_shader);
			state->prim.device->GetPixelShader(&og_pixel_shader);

			// needed or game renders mesh with shaders
			state->prim.device->SetVertexShader(nullptr);
			state->prim.device->SetPixelShader(nullptr);

			state->prim.device->SetRenderState(D3DRS_LIGHTING, FALSE);
			state->prim.device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

			// set random texture not used for anything else (remix-wise)
			const auto img = game::DB_FindXAssetHeader(game::ASSET_TYPE_IMAGE, "$identitynormalmap").image;
			state->prim.device->SetTexture(0, img->texture.basemap);

			// vertex format
			state->prim.device->SetFVF(DEBUG_VERT_FORMAT);

			// world matrix
			{
				float mtx[4][4] = {};
				mtx[0][0] = 1.0f;  mtx[0][1] = 0.0f; mtx[0][2] = 0.0f; mtx[0][3] = 0.0f;
				mtx[1][0] = 0.0f;  mtx[1][1] = 1.0f; mtx[1][2] = 0.0f; mtx[1][3] = 0.0f;
				mtx[2][0] = 0.0f;  mtx[2][1] = 0.0f; mtx[2][2] = 1.0f; mtx[2][3] = 0.0f;
				mtx[3][0] = 0.0f;  mtx[3][1] = 0.0f; mtx[3][2] = 0.0f; mtx[3][3] = 1.0f;

				mtx[3][0] = mtx[3][0]; // - source->eyeOffset[0];
				mtx[3][1] = mtx[3][1]; // - source->eyeOffset[1];
				mtx[3][2] = mtx[3][2]; // - source->eyeOffset[2];

				state->prim.device->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&mtx));
			}

			state->prim.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, args->vertexCount, args->baseIndex, args->triCount);

			state->prim.device->SetFVF(NULL);
			state->prim.device->SetVertexShader(og_vertex_shader);
			state->prim.device->SetPixelShader(og_pixel_shader);
		}

		__declspec(naked) void draw_tess_tech_stub()
		{
			const static uint32_t stock_func = 0x61A020; // R_DrawTessTechnique
			const static uint32_t retn_addr = 0x61A374;
			__asm
			{
				pushad;
				call	use_custom_tess_func;
				cmp		eax, 1;
				jne		OG_LOGIC;
				popad;
				call	custom_tess_func;
				jmp		retn_addr;

			OG_LOGIC:
				popad;
				call	stock_func;
				jmp		retn_addr;
			}
		}
	}
#endif
	// *
	// *


	rtx_fixed_function::rtx_fixed_function()
	{
		// fixed function rendering of static models and world geometry
		//if (flags::has_flag("fixed_function"))
		{
			// fixed-function rendering of static models (R_TessStaticModelRigidDrawSurfList)
			utils::hook(0x655A10, R_DrawStaticModelDrawSurfNonOptimized, HOOK_CALL).install()->quick();

			// fixed-function rendering of rigid xmodels - call 1 (RB_TessXModelRigidDrawSurfList-> R_DrawXModelSurfCamera-> R_DrawXModelRigidModelSurf1
			utils::hook(0x65722C, R_DrawXModelRigidModelSurf1_stub, HOOK_JUMP).install()->quick();

			// ^ call 2
			utils::hook(0x6575C5, R_DrawXModelRigidModelSurf2_stub, HOOK_JUMP).install()->quick();

			// fixed-function rendering of skinned (animated) models (R_TessXModelSkinnedDrawSurfList)
			utils::hook::nop(0x646ECE, 3); // nop eax overwrite before calling our stub (we need the ptr in eax)
			utils::hook(0x646ED4, R_DrawXModelSkinnedUncached_stub, HOOK_JUMP).install()->quick();

			// fixed-function rendering of static skinned models
			utils::hook(0x65618E, R_DrawStaticModelsSkinnedDrawSurf, HOOK_CALL).install()->quick();

			// fixed-function rendering of world surfaces (R_TessTrianglesPreTessList)
			utils::hook(0x6486F4, R_DrawBspDrawSurfsPreTess, HOOK_CALL).install()->quick();

			// ^ without batching
			utils::hook(0x648563, R_DrawBspDrawSurfs, HOOK_CALL).install()->quick();

			// fixed-function rendering of brushmodels
			utils::hook(0x648710, R_TessBModel, HOOK_JUMP).install()->quick();

#ifdef TESS_TESTS
			// fixed-function rendering of debug visualizations / 2d etc .. RB_EndTessSurface-> R_DrawTessTechnique
			// - currently only working "good" on debug collision polygons (lines-only are messed up - here to stay as reference)
			utils::hook(0x61A36F, tess::draw_tess_tech_stub, HOOK_JUMP).install()->quick();
#endif
			// ----

			// on map load :: build custom buffers for fixed-function rendering
			utils::hook(0x44031B, init_fixed_function_buffers_stub, HOOK_JUMP).install()->quick(); // CG_Init :: CG_NorthDirectionChanged call

			// on renderer shutdown :: release custom buffers used by fixed-function rendering
			utils::hook(0x5F5052, free_fixed_function_buffers_stub, HOOK_JUMP).install()->quick(); // R_Shutdown :: R_ResetModelLighting call

			dvars::rtx_warm_smodels = game::Dvar_RegisterBool(
				/* name		*/ "rtx_warm_smodels",
				/* desc		*/ "Build static model vertex buffers on map load (fixed-function rendering only)",
				/* default	*/ true,
				/* flags	*/ game::dvar_flags::saved);

#if DEBUG
			command::add("rtx_rebuild_world", "", "rebuilds the gfxworld vertex buffer (fixed-function)", [this](command::params)
			{
				gfx_world_vertexbuffer->Release();
				gfx_world_vertexbuffer = nullptr;
				build_gfxworld_buffers();
			});

			command::add("rtx_rebuild_all", "", "free all custom buffers and rebuilds them when needed + rebuilds the gfxworld vertex buffer", [this](command::params)
			{
				free_fixed_function_buffers();
				build_gfxworld_buffers();
			});
#endif
		}

		// fixed-function rendering of effects - R_TessCodeMeshList (particle clounds are still shader based)
		if (!flags::has_flag("stock_effects"))
		{
			utils::hook::set<BYTE>(0x645D31 + 2, 0x10); // change max verts from 0x4000 to 0x1000
			utils::hook::set<BYTE>(0x645E08 + 2, 0x10); // change max verts from 0x4000 to 0x1000 
			utils::hook::nop(0x645C71, 5); // R_UpdateVertexDecl
			utils::hook::nop(0x645CBB, 5); // R_SetStreamSource
			utils::hook(0x645BB3, R_TessCodeMeshList_begin_stub, HOOK_JUMP).install()->quick();
			utils::hook(0x645E1A, R_TessCodeMeshList_end_stub, HOOK_JUMP).install()->quick();
		}
	}
}