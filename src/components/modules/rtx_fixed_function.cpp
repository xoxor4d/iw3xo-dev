#include "std_include.hpp"

// notes:
// - r_pretess (surface batching) might cause some messed up normals (sometimes noticeable in water puddles)
// - ^ surface batching is def. needed with fixed-function rendering (fps)

// todo
// - disable normal and specularmaps so that remix does not pick them up (less clutter in remix ui)

namespace components
{
	bool XSurfaceOptimize(game::XSurface* surf)
	{
		const auto dev = game::glob::d3d9_device;
		bool allocated_any = false;

		// setup indexbuffer - actually unnecessary because the original one should work just fine - cba
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
					surf->custom_indexbuffer->Unlock();
				}
				else
				{
					surf->custom_indexbuffer->Release();
					surf->custom_indexbuffer = nullptr;
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

		// modifying the prim streams is most certainly unnecessary
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

		// not needed
		//dev->SetRenderState(D3DRS_ZENABLE, TRUE);
		//dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		//dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

		// #
		// draw prim

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

		// restore streams
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


	// *
	// world (bsp/terrain) drawing

	constexpr auto WORLD_VERTEX_STRIDE = 36u;
	constexpr auto WORLD_VERTEX_FORMAT = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	// (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1) = 32
	// (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1) = 36

	IDirect3DVertexBuffer9* gfx_world_vertexbuffer = nullptr;

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

		unsigned int base_index, count; game::GfxBspPreTessDrawSurf* list;
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

		dev->SetFVF(0);
	}


#if DEBUG
	/**
	 * @brief stub that could be used to override renderstates / textureargs
	 *		  - called after the games material-pass setup
	 *		  - called before 'R_DrawBspDrawSurfsPreTess'
	 *		  - 'cameraView' can be used to differentiate which drawlist (lit/decal/emissive) is being used
	 */
	void R_TessTrianglesPreTessList_begin(const game::GfxDrawSurfListArgs* context)
	{
		const auto dev = game::glob::d3d9_device;

		// are we rendering decals?
		if (context->info->cameraView == 2)
		{
			//dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			//dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

			//dev->SetRenderState(D3DRS_DEPTHBIAS, (DWORD)1.0f);
			//dev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, (DWORD)1.0f);

			//dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			//dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			//dev->SetTextureStageState(0, D3DTSS_ALPHAOP, rtx_gui::d3d_alpha_blend);

			//context->info->cameraView = 1;
		}
	}

	__declspec(naked) void R_TessTrianglesPreTessList_stub()
	{
		const static uint32_t retn_addr = 0x648595;
		__asm
		{
			pushad;
			push	edi;
			call	R_TessTrianglesPreTessList_begin;
			add		esp, 4;
			popad;

			// og ops
			mov     eax, [edi + 0xC];
			mov     esi, [edi + 4];
			jmp		retn_addr;
		}
	}
#endif


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

			// stride = 32 :: pos-xyz ; normal-xyz ; texcoords uv = 32 byte
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

					struct unpacked_vert
					{
						game::vec3_t pos;
						game::vec3_t normal;
						unsigned int color;
						game::vec2_t texcoord;
					};

					for (auto i = 0u; i < game::gfx_world->vertexCount; i++)
					{
						// packed source vertex
						const auto src_vert = game::gfx_world->vd.vertices[i];

						// position of our unpacked vert within the vertex buffer
						const auto v_pos_in_buffer = i * WORLD_VERTEX_STRIDE; // pos-xyz ; normal-xyz ; texcoords uv = 32 byte 
						const auto v = reinterpret_cast<unpacked_vert*>(((DWORD)vertex_buffer_data + v_pos_in_buffer));

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
				// HACK - ignore deformed (animated) models because they might use more then 2 partBits (partBit 3-4 are used for custom buffers)
				if (!header.model->surfs->deformed /*&& !header.model->surfs->custom_vertexbuffer && !header.model->surfs->custom_indexbuffer*/)
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

	// called on renderer shutdown (R_Shutdown)
	__declspec(naked) void free_fixed_function_buffers_stub()
	{
		const static uint32_t stock_func = 0x62F550; // R_ResetModelLighting
		const static uint32_t retn_addr = 0x5F5057;
		__asm
		{
			pushad;
			call	free_fixed_function_buffers;
			popad;

			call	stock_func;
			jmp		retn_addr;
		}
	}

	// *
	// *

	rtx_fixed_function::rtx_fixed_function()
	{
		// fixed function rendering of static models and world geometry
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

			// fixed-function rendering of static models
			utils::hook(0x655A10, R_DrawStaticModelDrawSurfNonOptimized, HOOK_CALL).install()->quick();

			// fixed-function rendering of world surfaces
			utils::hook(0x6486F4, R_DrawBspDrawSurfsPreTess, HOOK_CALL).install()->quick();

#if DEBUG
			utils::hook::set<BYTE>(0x5FA486 + 6, 0x02); // R_RenderScene :: set viewInfo->decalInfo.cameraView to 2 so we know when we render decals
			utils::hook::nop(0x64858F, 6); utils::hook(0x64858F, R_TessTrianglesPreTessList_stub, HOOK_JUMP).install()->quick();
#endif

			// ----

			// on map load :: build custom buffers for fixed-function rendering
			utils::hook(0x44031B, init_fixed_function_buffers_stub, HOOK_JUMP).install()->quick(); // CG_Init :: CG_NorthDirectionChanged call

			// on renderer shutdown :: release custom buffers used by fixed-function rendering
			utils::hook(0x5F5052, free_fixed_function_buffers_stub).install()->quick(); // R_Shutdown :: R_ResetModelLighting call

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
#endif
		}
	}
}