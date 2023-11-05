#include "std_include.hpp"

//#define RELOC_GFX_SURFS_BUFFER

// BACKUP
//#define GFX_SKINNED_CACHE_VB_POOL_SIZE	0x2000000				// stock : 0x480000 : gfxBuf.skinnedCacheVbPool
//#define GFX_SMODEL_CACHE_VB_SIZE			0x1400000				// stock : 0x800000 : gfxBuf.smodelCacheVb
//#define GFX_SMODEL_CACHE_SIZE				0x250000				// stock : 0x100000 : gfxBuf.smodelCache
//#define GFX_DYN_IB_POOL_SIZE				0x250000				// stock : 0x100000 : gfxBuf.dynamicIndexBufferPool
//#define GFX_PRETESS_IB_POOL_SIZE			GFX_DYN_IB_POOL_SIZE	// stock : 0x100000 : gfxBuf.preTessIndexBufferPool

namespace components
{
	/* ---------------------------------------------------------- */
	/* ------------ create dynamic rendering buffers ------------ */

	IDirect3DDevice9* get_d3d_device()
	{
		if (game::glob::d3d9_device)
		{
			return game::glob::d3d9_device;
		}
		else
		{
			return *game::dx9_device_ptr;
		}
	}

	// R_AllocDynamicIndexBuffer
	int alloc_dynamic_index_buffer(IDirect3DIndexBuffer9** ib, int size_in_bytes, const char* buffer_name, bool load_for_renderer)
	{
		if (!load_for_renderer) 
		{
			return 0;
		}

		HRESULT hr = get_d3d_device()->CreateIndexBuffer(size_in_bytes, (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY), D3DFMT_INDEX16, D3DPOOL_DEFAULT, ib, nullptr);
		if (hr < 0)
		{
			const char* msg = utils::function<const char* __stdcall(HRESULT)>(0x685F98)(hr); // R_ErrorDescription
			msg = utils::va("DirectX didn't create a 0x%.8x dynamic index buffer: %s\n", size_in_bytes, msg);

			utils::function<void(const char*)>(0x576A30)(msg); // Sys_Error
		}

		game::Com_PrintMessage(0, utils::va("D3D9: Created Indexbuffer (%s) of size: 0x%.8x\n", buffer_name, size_in_bytes), 0);
		return 0;
	}

	// R_InitDynamicIndexBufferState
	void init_dynamic_index_buffer_state(game::GfxIndexBufferState* ib, int indexCount, const char* buffer_name, bool loadForRenderer)
	{
		ib->used = 0;
		ib->total = indexCount;

		alloc_dynamic_index_buffer(&ib->buffer, 2 * indexCount, buffer_name, loadForRenderer);
	}

	// R_AllocDynamicVertexBuffer
	char* alloc_dynamic_vertex_buffer(IDirect3DVertexBuffer9** vb, int size_in_bytes, const char* buffer_name, bool load_for_renderer)
	{
		if (!load_for_renderer)
		{
			return nullptr;
		}

		HRESULT hr = get_d3d_device()->CreateVertexBuffer(size_in_bytes, (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY), 0, D3DPOOL_DEFAULT, vb, 0);
		if (hr < 0)
		{
			const char* msg = utils::function<const char* __stdcall(HRESULT)>(0x685F98)(hr); // R_ErrorDescription
			msg = utils::va("DirectX didn't create a 0x%.8x dynamic vertex buffer: %s\n", size_in_bytes, msg);

			utils::function<void(const char*)>(0x576A30)(msg); // Sys_Error
		}

		game::Com_PrintMessage(0, utils::va("D3D9: Created Vertexbuffer (%s) of size: 0x%.8x\n", buffer_name, size_in_bytes), 0);
		return nullptr;
	}

	// R_InitDynamicVertexBufferState
	void init_dynamic_vertex_buffer_state(game::GfxVertexBufferState* vb, int bytes, const char* buffer_name, bool load_for_renderer)
	{
		vb->used = 0;
		vb->total = bytes;
		vb->verts = 0;

		alloc_dynamic_vertex_buffer(&vb->buffer, bytes, buffer_name, load_for_renderer);
	}

	// R_InitTempSkinBuf :: Temp skin buffer within backenddata
	void init_temp_skin_buf(int bytes)
	{
		auto* back_end_data = reinterpret_cast<game::GfxBackEndData*>(0xCC9F600);

		for (auto i = 0; i < 2; ++i)
		{
			back_end_data[i].tempSkinBuf = (char*)VirtualAlloc(nullptr, bytes, MEM_RESERVE, PAGE_READWRITE);
			game::Com_PrintMessage(0, utils::va("Allocated tempSkinBuffer of size: 0x%.8x\n", bytes), 0);
		}
	}

	// R_CreateDynamicBuffers
	void create_dynamic_buffers()
	{
		auto& gfx_buf = *reinterpret_cast<game::GfxBuffers*>(0xD2B0840);
		const auto& r_loadForRenderer = game::Dvar_FindVar("r_loadForRenderer")->current.enabled;

		// default size in bytes
		std::uint32_t dynamic_vb_size = 1 * 1048576;
		std::uint32_t skinned_cache_size = 4 * 1048576;
		std::uint32_t temp_skin_size = 4 * 1048576;
		std::uint32_t dynamic_ib_size = 2 * 1048576;
		std::uint32_t pretess_ib_size = 2 * 1048576;

		// get size in bytes from dvars
		if (dvars::r_buf_dynamicVertexBuffer)
		{
			dynamic_vb_size = dvars::r_buf_dynamicVertexBuffer->current.integer * 1048576;
		}

		if (dvars::r_buf_skinnedCacheVb)
		{
			skinned_cache_size = dvars::r_buf_skinnedCacheVb->current.integer * 1048576;
		}

		if (dvars::r_buf_tempSkin)
		{
			temp_skin_size = dvars::r_buf_tempSkin->current.integer * 1048576;
		}

		if (dvars::r_buf_dynamicIndexBuffer)
		{
			dynamic_ib_size = dvars::r_buf_dynamicIndexBuffer->current.integer * 1048576;
		}

		if (dvars::r_buf_preTessIndexBuffer)
		{
			pretess_ib_size = dvars::r_buf_preTessIndexBuffer->current.integer * 1048576;
		}


		for (auto i = 0; i != 1; ++i)
		{
			init_dynamic_vertex_buffer_state(&gfx_buf.dynamicVertexBufferPool[i], dynamic_vb_size, "dynamicVertexBufferPool", r_loadForRenderer);
		}
		gfx_buf.dynamicVertexBuffer = gfx_buf.dynamicVertexBufferPool;


		for (auto i = 0; i != 2; ++i)
		{
			init_dynamic_vertex_buffer_state(&gfx_buf.skinnedCacheVbPool[i], skinned_cache_size, "skinnedCacheVbPool", r_loadForRenderer);
		}

		init_temp_skin_buf(temp_skin_size);


		for (auto i = 0; i != 1; ++i)
		{
			init_dynamic_index_buffer_state(&gfx_buf.dynamicIndexBufferPool[i], dynamic_ib_size, "dynamicIndexBufferPool", r_loadForRenderer);
		}
		gfx_buf.dynamicIndexBuffer = gfx_buf.dynamicIndexBufferPool;


		for (auto i = 0; i != 2; ++i)
		{
			init_dynamic_index_buffer_state(&gfx_buf.preTessIndexBufferPool[i], pretess_ib_size, "preTessIndexBufferPool", r_loadForRenderer);
		}
		gfx_buf.preTessBufferFrame = 0;
		gfx_buf.preTessIndexBuffer = gfx_buf.preTessIndexBufferPool;
	}

	/* ---------------------------------------------------------- */
	/* ---------- Alloc dynamic vertices (smodelCache) ---------- */

	// R_AllocDynamicVertexBuffer
	void alloc_dynamic_vertex_buffer()
	{
		auto& gfx_buf = *reinterpret_cast<game::GfxBuffers*>(0xD2B0840);
		const auto& r_loadForRenderer = game::Dvar_FindVar("r_loadForRenderer")->current.enabled;

		// default size in bytes
		std::uint32_t smodel_cache_vb_size = 8 * 1048576;

		// get size in bytes from dvar
		if (dvars::r_buf_smodelCacheVb)
		{
			smodel_cache_vb_size = dvars::r_buf_smodelCacheVb->current.integer * 1048576;
		}

		if (r_loadForRenderer)
		{
			HRESULT hr = get_d3d_device()->CreateVertexBuffer(smodel_cache_vb_size, (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY), 0, D3DPOOL_DEFAULT, &gfx_buf.smodelCacheVb, 0);
			if (hr < 0)
			{
				const char* msg = utils::function<const char* __stdcall(HRESULT)>(0x685F98)(hr); // R_ErrorDescription
				msg = utils::va("DirectX didn't create a 0x%.8x dynamic vertex buffer: %s\n", smodel_cache_vb_size, msg);

				utils::function<void(const char*)>(0x576A30)(msg); // Sys_Error
			}
			

			game::Com_PrintMessage(0, utils::va("D3D9: Created Vertexbuffer (smodelCacheVb) of size: 0x%.8x\n", smodel_cache_vb_size), 0);
		}
	}

	/* ---------------------------------------------------------- */
	/* ---------- Alloc dynamic indices (smodelCache) ----------- */

	void init_smodel_indices()
	{
		auto& gfx_buf = *reinterpret_cast<game::GfxBuffers*>(0xD2B0840);

		// default size in bytes
		std::uint32_t smodel_cache_ib_size = 2 * 1048576;

		// get size in bytes from dvar
		if (dvars::r_buf_smodelCacheIb)
		{
			smodel_cache_ib_size = dvars::r_buf_smodelCacheIb->current.integer * 1048576;
		}

		gfx_buf.smodelCache.used = 0;
		gfx_buf.smodelCache.total = static_cast<int>(smodel_cache_ib_size / 2); // why half?

		const auto mem_reserve = VirtualAlloc(nullptr, 0x200000u, MEM_RESERVE, PAGE_READWRITE);
		const auto mem_commit = VirtualAlloc(mem_reserve, 0x200000u, MEM_COMMIT, PAGE_READWRITE);

		if (!mem_commit || !mem_reserve)
		{
			if (!mem_commit && mem_reserve)
			{
				VirtualFree(mem_reserve, 0, MEM_RESET);
			}

			const char* msg = utils::va("r_init_smodel_indices :: Unable to allocate 0x%.8x bytes. Out of memory?\n", smodel_cache_ib_size);
			utils::function<void(const char*)>(0x576A30)(msg); // Sys_Error
		}

		game::Com_PrintMessage(0, utils::va("Allocated smodelCache (smodelCacheIb) of size: 0x%.8x\n", smodel_cache_ib_size), 0);
		gfx_buf.smodelCache.indices = static_cast<unsigned __int16*>(mem_reserve);
	}

	__declspec(naked) void init_smodel_indices_stub()
	{
		const static uint32_t retn_addr = 0x5F5E7D;
		__asm
		{
			call	init_smodel_indices;
			pop     edi;
			jmp		retn_addr;
		}
	}

	/* ---------------------------------------------------------- */
	/* ------ change warning limits to fit new buffer size ------ */

	__declspec(naked) void r_warn_temp_skin_size_limit_stub()
	{
		const static uint32_t mb_size = 1048576;
		const static uint32_t retn_addr = 0x643948;
		__asm
		{
			// replace 'cmp edx, 480000h'

			push	eax;
			mov		eax, dvars::r_buf_tempSkin;
			mov		eax, dword ptr[eax + 12];	// current->integer
			imul	eax, mb_size;				// get size in bytes

			cmp		edx, eax;					// warning limit compare
			pop		eax;

			jmp		retn_addr;
		}
	}

	__declspec(naked) void r_warn_max_skinned_cache_vertices_limit_stub()
	{
		const static uint32_t mb_size = 1048576;
		const static uint32_t retn_addr = 0x643822;
		__asm
		{
			// replace 'cmp edx, 480000h'

			push	eax;
			mov		eax, dvars::r_buf_skinnedCacheVb;
			mov		eax, dword ptr[eax + 12];	// current->integer
			imul	eax, mb_size;				// get size in bytes

			cmp		edx, eax;					// warning limit compare
			pop		eax;

			jmp		retn_addr;
		}
	}

	// ---------------------

	// called from _common::register_additional_dvars (R_Init->R_RegisterDvars)
	// these need to be re-registered on vid_restart because they are latched
	void _renderer::register_dvars()
	{
		// do not register dvars with the "default" register functions here. (creates duplicates)

		dvars::r_buf_skinnedCacheVb = game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_skinnedCacheVb",
			/* desc		*/ "Size of skinnedCache Vertexbuffer (Size * 2 will be allocated) in Megabytes. Default : 4 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 4, // 24
			/* minVal	*/ 4,
			/* maxVal	*/ 64,
			/* flags	*/ game::dvar_flags::saved | game::dvar_flags::latched);

		dvars::r_buf_smodelCacheVb = game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_smodelCacheVb",
			/* desc		*/ "Size of smodelCache Vertexbuffer in Megabytes. Default : 8 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 8, // 24
			/* minVal	*/ 8,
			/* maxVal	*/ 64,
			/* flags	*/ game::dvar_flags::saved | game::dvar_flags::latched);

		dvars::r_buf_smodelCacheIb = game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_smodelCacheIb",
			/* desc		*/ "Size of smodelCache Indexbuffer in Megabytes. Default : 2 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 2, // 4
			/* minVal	*/ 2,
			/* maxVal	*/ 64,
			/* flags	*/ game::dvar_flags::saved | game::dvar_flags::latched);

		dvars::r_buf_tempSkin = game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_tempSkin",
			/* desc		*/ "Size of tempSkin buffer in Megabytes. Default : 4 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 4, // 16
			/* minVal	*/ 4,
			/* maxVal	*/ 64,
			/* flags	*/ game::dvar_flags::saved | game::dvar_flags::latched);

		dvars::r_buf_dynamicVertexBuffer = game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_dynamicVertexBuffer",
			/* desc		*/ "Size of dynamic Vertexbuffer in Megabytes. Default : 1 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 1, // 2
			/* minVal	*/ 1,
			/* maxVal	*/ 64,
			/* flags	*/ game::dvar_flags::saved | game::dvar_flags::latched);

		dvars::r_buf_dynamicIndexBuffer = game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_dynamicIndexBuffer",
			/* desc		*/ "Size of dynamic Indexbuffer in Megabytes. Default : 2 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 2, // 4
			/* minVal	*/ 2,
			/* maxVal	*/ 64,
			/* flags	*/ game::dvar_flags::saved | game::dvar_flags::latched);

		dvars::r_buf_preTessIndexBuffer = game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_preTessIndexBuffer",
			/* desc		*/ "Size of preTess Indexbuffer (Size * 2 will be allocated) in Megabytes. Default : 2 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 2, // 4
			/* minVal	*/ 2,
			/* maxVal	*/ 64,
			/* flags	*/ game::dvar_flags::saved | game::dvar_flags::latched);

		//dvars::r_cullWorld = Game::Dvar_RegisterBoolWrapper(
		//	/* name		*/ "r_cullWorld",
		//	/* desc		*/ "Culls invisible world surfaces. Disabling this can be useful for vertex manipulating shaders.",
		//	/* default	*/ true,
		//	/* flags	*/ Game::dvar_flags::latched);

		//dvars::r_cullEntities = Game::Dvar_RegisterBoolWrapper(
		//	/* name		*/ "r_cullEntities",
		//	/* desc		*/ "Culls invisible entities. Disabling this can be useful for vertex manipulating shaders.",
		//	/* default	*/ true,
		//	/* flags	*/ Game::dvar_flags::latched);

		//dvars::r_drawDynents = Game::Dvar_RegisterBoolWrapper(
		//	/* name		*/ "r_drawDynents",
		//	/* desc		*/ "Draw dynamic entities.",
		//	/* default	*/ true,
		//	/* flags	*/ Game::dvar_flags::none);
	}


	/* ---------------------------------------------------------- */

	void codesampler_error([[maybe_unused]] game::MaterialShaderArgument* arg, [[maybe_unused]] game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, const char* sampler, [[maybe_unused]] int droptype, [[maybe_unused]] const char* msg, ...)
	{
		if (!sampler || !state || !state->material || !state->technique) 
		{
			return;
		}

		game::Com_PrintMessage(0, utils::va(
			"^1Tried to use sampler <%s> when it isn't valid!\n"
			"^7[Passdump]\n"
			"|-> Material ----- %s\n"
			"|-> Technique ----	%s\n"
			"|-> RenderTarget - %s\n"
			"|-> Not setting sampler using R_SetSampler!\n",
			sampler, state->material->info.name, state->technique->name, game::get_rendertarget_string(state->renderTargetId)), 0);
	}

	// R_SetupPassPerObjectArgs
	__declspec(naked) void codesampler_error01_stub()
	{
		const static uint32_t retn_addr = 0x64BD0F; // offset after call to R_SetSampler
		__asm
		{
			// - already pushed args -
			// push    fmt
			// push    msg
			// push    dropType

			mov     eax, [esp + 8h];	// move sampler string into eax
			push	eax;				// decreased esp by 4
			mov     eax, [esp + 14h];	// move GfxCmdBufState* into eax (now at 14h)
			push	eax;				// GfxCmdBufState*
			push	ebx;				// GfxCmdBufSourceState*
			push	edi;				// MaterialShaderArgument*

#if DEBUG
			Call	codesampler_error;	// only dump info on debug builds
#endif
			add		esp, 28;

			mov     eax, [esp + 14h];
			mov     ecx, [esp + 24h];
			movzx   esi, word ptr[edi + 2];
			push    eax;
			push    ecx;
			push    ebx;
			mov     eax, ebp;

			jmp		retn_addr;
		}
	}

	// R_SetPassShaderStableArguments
	__declspec(naked) void codesampler_error02_stub()
	{
		const static uint32_t retn_addr = 0x64C36C; // offset after call to R_SetSampler
		__asm
		{
			// skip error and R_SetSampler
			add		esp, 12;

			mov     eax, [esp + 20h];
			movzx   esi, word ptr[edi + 2];
			push    eax;
			mov     eax, [esp + 18h];
			push    ebx;
			push    ebp;

			//call    R_SetSampler
			jmp		retn_addr;
		}
	}

	void cubemap_shot_f_sync_msg()
	{
		if (const auto& r_smp_backend = game::Dvar_FindVar("r_smp_backend");
						r_smp_backend && r_smp_backend->current.enabled)
		{
			game::Com_PrintMessage(0, "^1Error: ^7r_smp_backend must be set to 0!", 0);
		}

		if (const auto& r_aaSamples = game::Dvar_FindVar("r_aaSamples");
						r_aaSamples && r_aaSamples->current.integer != 1)
		{
			game::Com_PrintMessage(0, "^1Error: ^7r_aaSamples must be set to 1 (Disable AA)!", 0);
		}
	}

	__declspec(naked) void cubemap_shot_f_stub()
	{
		const static uint32_t R_SyncRenderThread_func = 0x5F6070;
		const static uint32_t retn_addr = 0x475411;
		__asm
		{
			call	R_SyncRenderThread_func;

			pushad;
			call	cubemap_shot_f_sync_msg;
			popad;

			jmp		retn_addr;
		}
	}


	/* ---------------------------------------------------------- */

#if 0 // disabled for now
	// R_AddWorldSurfacesPortalWalk
	__declspec(naked) void r_cull_world_stub_01()
	{
		const static uint32_t rtn_pt_skip = 0x60B095;
		const static uint32_t rtn_pt_stock = 0x60B02E;
		__asm
		{
			// stock op's
			cmp     esi, ebp;
			mov		[esp + 10h], eax;

			pushad;
			push	eax;
			mov		eax, dvars::r_cullWorld;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;

			popad;
			jmp		rtn_pt_stock;

		SKIP:
			popad;
			jmp		rtn_pt_skip;
		}
	}

	// R_AddAabbTreeSurfacesInFrustum_r
	__declspec(naked) void r_cull_world_stub_02()
	{
		const static uint32_t rtn_pt_skip = 0x643B0E;
		const static uint32_t rtn_pt_stock = 0x643B08;
		__asm
		{
			// stock op's
			fnstsw  ax;
			test    ah, 41h;

			pushad;
			push	eax;
			mov		eax, dvars::r_cullWorld;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;

			popad;
			jmp		rtn_pt_stock;

		SKIP:
			popad;
			jmp		rtn_pt_skip;
		}
	}

	// R_AddAabbTreeSurfacesInFrustum_r
	__declspec(naked) void r_cull_world_stub_03()
	{
		const static uint32_t rtn_pt_skip = 0x643B48;
		const static uint32_t rtn_pt_stock = 0x643B39;
		__asm
		{
			// stock op's
			fnstsw  ax;
			test    ah, 1;

			pushad;
			push	eax;
			mov		eax, dvars::r_cullWorld;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;

			popad;
			jmp		rtn_pt_stock;

		SKIP:
			popad;
			jmp		rtn_pt_skip;
		}
	}

	// R_AddCellSceneEntSurfacesInFrustumCmd
	__declspec(naked) void r_cull_entities_stub()
	{
		const static uint32_t rtn_pt_skip = 0x64D17C;
		const static uint32_t rtn_pt_stock = 0x64D17A;
		__asm
		{
			// stock op's
			and		[esp + 18h], edx;
			cmp     byte ptr[esi + eax], 0;

			pushad;
			push	eax;
			mov		eax, dvars::r_cullEntities;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;

			popad;
			jmp		rtn_pt_stock;

		SKIP:
			popad;
			jmp		rtn_pt_skip;
		}
	}

	// R_AddWorkerCmd
	__declspec(naked) void r_draw_dynents_stub()
	{
		const static uint32_t R_AddCellDynModelSurfacesInFrustumCmd_Func = 0x64D4C0;
		const static uint32_t rtn_pt_stock = 0x62932D;
		__asm
		{
			pushad;
			push	eax;
			mov		eax, dvars::r_drawDynents;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;

			popad;
			call	R_AddCellDynModelSurfacesInFrustumCmd_Func;

		SKIP:
			popad;
			jmp		rtn_pt_stock;
		}
	}
#endif


	/* ---------------------------------------------------------- */

	bool disable_prepass = false;

	// *
	// mat->techniqueSet->remappedTechniqueSet->techniques[type]
	bool _renderer::is_valid_technique_for_type(const game::Material* mat, const game::MaterialTechniqueType type)
	{
		if (	mat
			 && mat->techniqueSet
			 && mat->techniqueSet->remappedTechniqueSet
			 && mat->techniqueSet->remappedTechniqueSet->techniques
			 && mat->techniqueSet->remappedTechniqueSet->techniques[type])
		{
			return true;
		}

		return false;
	}


	// *
	// return remappedTechnique for technique_type if valid, stock technique otherwise
	void _renderer::switch_technique(game::switch_material_t* swm, game::Material* material)
	{
		if (material)
		{
			swm->technique = nullptr;

			if (is_valid_technique_for_type(material, swm->technique_type))
			{
				swm->technique = material->techniqueSet->remappedTechniqueSet->techniques[swm->technique_type];
			}

			swm->switch_technique = true;
			return;
		}

		// return stock technique if the above failed
		swm->technique = swm->current_technique;
	}


	// *
	// return remappedTechnique for technique_type if valid, stock technique otherwise
	void _renderer::switch_technique(game::switch_material_t* swm, const char* material_name)
	{
		

		if (const auto	material = game::Material_RegisterHandle(material_name, 3); 
						material)
		{
			swm->technique = nullptr;

			if (is_valid_technique_for_type(material, swm->technique_type))
			{
				swm->technique = material->techniqueSet->remappedTechniqueSet->techniques[swm->technique_type];
			}

			swm->switch_technique = true;
			return;
		}

		// return stock technique if the above failed
		swm->technique = swm->current_technique;
	}


	//*
	// return new material if valid, stock material otherwise
	void _renderer::switch_material(game::switch_material_t* swm, const char* material_name)
	{
		
		
		if (const auto	material = game::Material_RegisterHandle(material_name, 3); 
						material)
		{
			swm->material = material;
			swm->technique = nullptr;

			if (is_valid_technique_for_type(material, swm->technique_type))
			{
				swm->technique = material->techniqueSet->remappedTechniqueSet->techniques[swm->technique_type];
			}

			swm->switch_material = true;
			return;
		}

		// return stock material if the above failed
		swm->material = swm->current_material;
	}

	// memes
	// bool rendered_scope = false;

	// *
	// :>
	int R_SetMaterial(game::MaterialTechniqueType techType, game::GfxCmdBufSourceState* src, game::GfxCmdBufState* state, game::GfxDrawSurf drawSurf)
	{
		auto& rg = *reinterpret_cast<game::r_globals_t*>(0xCC9D150);
		auto rgp =  reinterpret_cast<game::r_global_permanent_t*>(0xCC98280);

		game::switch_material_t mat = {};

		mat.current_material  = rgp->sortedMaterials[(drawSurf.packed >> 29) & 0x7FF];
		mat.current_technique = mat.current_material->techniqueSet->remappedTechniqueSet->techniques[techType];

		mat.material		  = mat.current_material;
		mat.technique		  = mat.current_technique;
		mat.technique_type	  = techType;

		disable_prepass		  = false; // always reset

		if (mat.current_material)
		{
			if (components::active.daynight_cycle)
			{
				daynight_cycle::overwrite_sky_material(&mat);
			}

			// wireframe xmodels
			if (dvars::r_wireframe_xmodels && dvars::r_wireframe_xmodels->current.integer)
			{
				if (utils::starts_with(mat.current_material->info.name, "mc/"))
				{
					switch (dvars::r_wireframe_xmodels->current.integer)
					{
					case 1: // SHADED
						mat.technique_type = game::TECHNIQUE_WIREFRAME_SHADED;
						mat.switch_technique_type = true;
						break;

					case 2: // SOLID
						mat.technique_type = game::TECHNIQUE_WIREFRAME_SOLID;
						mat.switch_technique_type = true;
						break;

					case 3: // SHADED_WHITE
						disable_prepass = true;

						mat.technique_type	= game::TECHNIQUE_UNLIT;
						_renderer::switch_material(&mat, "iw3xo_showcollision_wire");
						break;

					case 4: // SOLID_WHITE
						mat.technique_type	= game::TECHNIQUE_UNLIT;
						_renderer::switch_material(&mat, "iw3xo_showcollision_wire");
						break;
					}
				}
			}

			// wireframe world
			if (dvars::r_wireframe_world && dvars::r_wireframe_world->current.integer)
			{
				if (utils::starts_with(mat.current_material->info.name, "wc/") && !utils::starts_with(mat.current_material->info.name, "wc/sky"))
				{
					switch (dvars::r_wireframe_world->current.integer)
					{
					case 1: // SHADED
						mat.technique_type = game::TECHNIQUE_WIREFRAME_SHADED;
						mat.switch_technique_type = true;
						break;

					case 2: // SOLID
						mat.technique_type = game::TECHNIQUE_WIREFRAME_SOLID;
						mat.switch_technique_type = true;
						break;

					case 3: // SHADED_WHITE
						disable_prepass = true;

						mat.technique_type	= game::TECHNIQUE_UNLIT;
						_renderer::switch_material(&mat, "iw3xo_showcollision_wire");
						break;

					case 4: // SOLID_WHITE
						mat.technique_type	= game::TECHNIQUE_UNLIT;
						_renderer::switch_material(&mat, "iw3xo_showcollision_wire");
						break;
					}
				}
			}


			// texcoord debugshader
			if (dvars::r_debugShaderTexcoord && dvars::r_debugShaderTexcoord->current.enabled)
			{
				if (utils::starts_with(mat.current_material->info.name, "mc/"))
				{
					disable_prepass = true;

					mat.technique_type = game::TECHNIQUE_UNLIT;
					_renderer::switch_technique(&mat, "debug_texcoords_dtex");
				}
				else if (utils::starts_with(mat.current_material->info.name, "wc/"))
				{
					disable_prepass = true;

					mat.technique_type = game::TECHNIQUE_UNLIT;
					_renderer::switch_technique(&mat, "debug_texcoords");
				}
			}

			if (!mat.switch_material && !mat.switch_technique && !mat.switch_technique_type)
			{
				if (state->origMaterial)
				{
					state->material = state->origMaterial;
				}
				if (state->origTechType)
				{
					state->techType = state->origTechType;
				}
			}
		}

		// save the original material
		state->origMaterial = state->material;

		// only switch to a different technique_type
		if (mat.switch_technique_type)
		{
			if (_renderer::is_valid_technique_for_type(mat.current_material, mat.technique_type))
			{
				_renderer::switch_technique(&mat, mat.current_material);
			}
		}

		// set stock or new material & technique
		state->material = mat.material;
		state->technique = mat.technique;


		if (!state->technique || (state->technique->flags & 1) != 0 && !rg.distortion)
		{
			return 0;
		}

		if (!mat.switch_material && !mat.switch_technique && !mat.switch_technique_type)
		{
			if ((mat.technique_type == game::TECHNIQUE_EMISSIVE || mat.technique_type == game::TECHNIQUE_UNLIT) && (state->technique->flags & 0x10) != 0 && !src->constVersions[4])
			{
				return 0;
			}
		}

		const auto& r_logFile = game::Dvar_FindVar("r_logFile");
		if (r_logFile && r_logFile->current.integer && mat.current_material)
		{
			const auto string = utils::va("R_SetMaterial( %s, %s, %i )\n", state->material->info.name, state->technique->name, mat.technique_type);
			const static uint32_t RB_LogPrint_func = 0x63CF40;
			__asm
			{
				pushad;
				mov		edx, string;
				call	RB_LogPrint_func;
				popad;
			}
		}

		state->origTechType = state->techType;
		state->techType = mat.technique_type;

		return 1;
	}


	__declspec(naked) void R_SetMaterial_stub()
	{
		const static uint32_t rtn_to_set_shadowable_light = 0x648F92;
		const static uint32_t retn_to_retn = 0x648F48;
		__asm
		{
			push	esi;		// techType
			call	R_SetMaterial;
			pop		esi;
			add     esp, 10h;

			test    eax, eax;	// do not return BOOL if you test 4 byte sized registers :>
			jz      memes;
			jmp		rtn_to_set_shadowable_light;

		memes:
			jmp		retn_to_retn;
		}
	}

	__declspec(naked) void R_SetPrepassMaterial_stub()
	{
		const static uint32_t R_SetPrepassMaterial_func = 0x648DF0;
		const static uint32_t retn_addr = 0x648F41;
		__asm
		{
			push	eax;
			mov		al, disable_prepass;
			cmp		al, 1;
			pop		eax;

			// jump if true
			je		disable;
			call	R_SetPrepassMaterial_func;

		disable:
			jmp		retn_addr;
		}
	}

	__declspec(naked) void R_SetMaterial_Emissive_stub()
	{
		const static uint32_t retn_to_retn = 0x6490BF;
		__asm
		{
			push	esi;		// techType
			call	R_SetMaterial;
			pop		esi;
			add     esp, 10h;
			jmp		retn_to_retn;
		}
	}


	// no reason to dump the same shader multiple times over the lifespan of the current session
	std::unordered_set<std::string> r_dumped_shader_set;

	bool folder_ps_exists = false;
	bool folder_vs_exists = false;

	bool dumpedshader_contains(const std::unordered_set<std::string>& set, const std::string& s)
	{
		return set.find(s) != set.end();
	}

	void pixelshader_custom_constants(game::GfxCmdBufState* state)
	{
		// dump shaders at runtime ~> TODO: move that to its own function / hook
		if (dvars::r_dumpShaders && dvars::r_dumpShaders->current.enabled)
		{
			const auto base_path = game::Dvar_FindVar("fs_basepath");
			if (!base_path) 
			{
				return;
			}

			const std::string file_path = base_path->current.string + "\\iw3xo\\shader_dump\\"s;

			if (state && state->pass)
			{
				if (state->pass->vertexShader && state->pass->vertexShader->name)
				{
					// check if shader was already dumped
					if (!dumpedshader_contains(r_dumped_shader_set, "vs_"s + state->pass->vertexShader->name))
					{
						if (!folder_vs_exists)
						{
							std::filesystem::create_directories(file_path + "vertexShader\\");
							folder_vs_exists = true;
						}

						const std::uint16_t bin_size = state->pass->vertexShader->prog.loadDef.programSize;
						std::ofstream outfile(file_path + "vertexShader\\" + "vs_" + state->pass->vertexShader->name, std::ofstream::binary);

						outfile.write(reinterpret_cast<char*>(state->pass->vertexShader->prog.loadDef.program), bin_size * 4);
						outfile.close();

						r_dumped_shader_set.emplace("vs_"s + state->pass->vertexShader->name);
					}
				}

				if (state->pass->pixelShader && state->pass->pixelShader->name)
				{
					// check if shader was already dumped
					if (!dumpedshader_contains(r_dumped_shader_set, "ps_"s + state->pass->pixelShader->name))
					{
						if (!folder_ps_exists)
						{
							std::filesystem::create_directories(file_path + "pixelShader\\");
							folder_ps_exists = true;
						}

						const std::uint16_t bin_size = state->pass->pixelShader->prog.loadDef.programSize;
						std::ofstream outfile(file_path + "pixelShader\\" + "ps_" + state->pass->pixelShader->name, std::ofstream::binary);

						outfile.write(reinterpret_cast<char*>(state->pass->pixelShader->prog.loadDef.program), bin_size * 4);
						outfile.close();

						r_dumped_shader_set.emplace("ps_"s + state->pass->pixelShader->name);
					}
				}
			}
		}

		if (state && state->pass)
		{
			// loop through all argument defs to find custom codeconsts
			for (auto arg = 0; arg < state->pass->perObjArgCount + state->pass->perPrimArgCount + state->pass->stableArgCount; arg++)
			{
				const auto arg_def = &state->pass->args[arg];

				if (arg_def && arg_def->type == 5)
				{
					if (components::active.daynight_cycle)
					{
						daynight_cycle::set_pixelshader_constants(state, arg_def);
					}

					if (components::active.ocean)
					{
						ocean::set_pixelshader_constants(state, arg_def);
					}

#ifdef DEVGUI_XO_BLUR
					// dev stuff
					if (state->pass->pixelShader && !utils::q_stricmp(state->pass->pixelShader->name, "sm2_blur"))
					{
						float constant[4] = { game::glob::xo_blur_directions, game::glob::xo_blur_quality, game::glob::xo_blur_size, game::glob::xo_blur_alpha };

						if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0) {
							(*game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}

						if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_MATERIAL_COLOR) {
							(*game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}
					}
#endif
				}
			}
		}
	}

	__declspec(naked) void R_SetPassPixelShaderStableArguments_stub()
	{
		__asm
		{
			// stock op's
			pop     edi;
			pop     esi;
			pop     ebp;
			pop     ebx;
			add     esp, 8;

			// GfxCmdBufState
			mov		edx, [esp + 0Ch];

			pushad;
			push	edx;
			call	pixelshader_custom_constants;
			add		esp, 4;
			popad;

			retn;
		}
	}

	
	void vertexshader_custom_constants([[maybe_unused]] game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state)
	{
		// fixup cod4 code constants
		if (state && state->pass)
		{
			// loop through all argument defs to find custom codeconsts
			for (auto arg = 0; arg < state->pass->perObjArgCount + state->pass->perPrimArgCount + state->pass->stableArgCount; arg++)
			{
				const auto arg_def = &state->pass->args[arg];

				if (arg_def && arg_def->type == 3)
				{
					if (components::active.ocean)
					{
						ocean::set_vertexshader_constants(state, arg_def);
					}
				}
			}
		}
	}

	__declspec(naked) void R_SetVertexShaderConstantFromCode_stub()
	{
		__asm
		{
			// stock op's
			pop     edi;
			pop     esi;
			pop     ebp;
			pop     ebx;
			add     esp, 8;

			// GfxCmdBufState
			mov		ecx, [esp + 8h];
			mov		edx, [esp + 0Ch];

			pushad;
			push	edx; // state
			push	ecx; // source
			call	vertexshader_custom_constants;
			add		esp, 8;
			popad;

			retn;
		}
	}

	// *
	// *

#pragma warning(push)
#pragma warning(disable: 6385)
#pragma warning(disable: 6386)
	void InfinitePerspectiveMatrix(const float tan_half_fov_x, const float tan_half_fov_y, const float z_near, float(*mtx)[4])
	{
		(*mtx)[0]  = 0.99951172f / tan_half_fov_x;
		(*mtx)[5]  = 0.99951172f / tan_half_fov_y;
		(*mtx)[10] = 0.99951172f;
		(*mtx)[11] = 1.0f;
		(*mtx)[14] = 0.99951171875f * -z_near;
	}
#pragma warning(pop)
	
	// rewrite of CG_GetViewFov()
	float calculate_gunfov_with_zoom(float fov_val)
	{
		float calc_fov;
		const auto& cg_fovMin = game::Dvar_FindVar("cg_fovMin");

		unsigned int offhand_index = game::cgs->predictedPlayerState.offHandIndex;
		if ((game::cgs->predictedPlayerState.weapFlags & 2) == 0)
		{
			offhand_index = game::cgs->predictedPlayerState.weapon;
		}

		// #
		auto check_flags_and_fovmin = [&]() -> float
		{
			if ((game::cgs->predictedPlayerState.eFlags & 0x300) != 0)
			{
				calc_fov = 55.0f;
			}

			if (cg_fovMin->current.value - calc_fov >= 0.0f)
			{
				calc_fov = cg_fovMin->current.value;
			}

			return calc_fov;
		};

		
		const auto weapon = game::BG_WeaponNames[offhand_index];
		if (game::cgs->predictedPlayerState.pm_type == 5)
		{
			return check_flags_and_fovmin();
		}
		
		calc_fov = fov_val;
		if (weapon->aimDownSight)
		{
			if (game::cgs->predictedPlayerState.fWeaponPosFrac == 1.0f)
			{
				calc_fov = weapon->fAdsZoomFov;
				return check_flags_and_fovmin();
			}
			
			if (0.0f != game::cgs->predictedPlayerState.fWeaponPosFrac)
			{
				float ads_factor;
				
				if (game::cgs->playerEntity.bPositionToADS)
				{
					const float w_pos_frac = game::cgs->predictedPlayerState.fWeaponPosFrac - (1.0f - weapon->fAdsZoomInFrac);
					if (w_pos_frac <= 0.0f)
					{
						return check_flags_and_fovmin();
					}
					ads_factor = w_pos_frac / weapon->fAdsZoomInFrac;
				}
				else
				{
					const float w_pos_frac = game::cgs->predictedPlayerState.fWeaponPosFrac - (1.0f - weapon->fAdsZoomOutFrac);
					if (w_pos_frac <= 0.0f)
					{
						return check_flags_and_fovmin();
					}
					ads_factor = w_pos_frac / weapon->fAdsZoomOutFrac;
				}
				
				if (ads_factor > 0.0f)
				{
					calc_fov = calc_fov - ads_factor * (calc_fov - weapon->fAdsZoomFov);
				}
			}
		}
		
		return check_flags_and_fovmin();
	}

	void set_gunfov(game::GfxViewParms* view_parms)
	{
		if(dvars::cg_fov_tweaks && dvars::cg_fov_tweaks->current.enabled)
		{
			// calc gun fov (includes weapon zoom)
			const float gun_fov = calculate_gunfov_with_zoom(dvars::cg_fov_gun->current.value);
			const float w_fov = 0.75f * tanf(gun_fov * 0.01745329238474369f * 0.5f);

			const float tan_half_x = (static_cast<float>(game::cgs->refdef.width) / static_cast<float>(game::cgs->refdef.height)) * w_fov;
			const float tan_half_y = w_fov;

			// calc projection matrix
			float proj_mtx[4][4] = {};
			InfinitePerspectiveMatrix(tan_half_x, tan_half_y, view_parms->zNear, proj_mtx);

			// only overwrite the projection matrix ;)
			memcpy(view_parms->projectionMatrix.m, proj_mtx, sizeof(game::GfxMatrix));
		}
	}
	
	__declspec(naked) void R_SetViewParmsForScene_stub()
	{
		const static uint32_t retn_addr = 0x5FAA0B;
		__asm
		{
			pushad;
			push	edi; // viewParms
			call	set_gunfov;
			add		esp, 4;
			popad;
			
			// stock op's
			lea     ecx, [edi + 0C0h];
			jmp		retn_addr;
		}
	}

	#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)
	// D3DCOLOR_XRGB(255, 255, 255) is white(full colour)
	struct CUSTOMVERTEX {

		D3DXVECTOR3 position; // The transformed position for the vertex.
		//DWORD colour; // The vertex colour.
		D3DXVECTOR3 normal;
		FLOAT tu, tv;
	};



	VOID SetupLights()
	{
		// Set up a material. The material here just has the diffuse and ambient
		// colors set to yellow. Note that only one material can be used at a time.
		D3DMATERIAL9 mtrl;
		ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
		mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
		mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
		mtrl.Diffuse.b = mtrl.Ambient.b = 0.0f;
		mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
		//game::glob::d3d9_device->SetMaterial(&mtrl);

		// Set up a white, directional light, with an oscillating direction.
		// Note that many lights may be active at a time (but each one slows down
		// the rendering of our scene). However, here we are just using one. Also,
		// we need to set the D3DRS_LIGHTING renderstate to enable lighting
		D3DXVECTOR3 vecDir;
		D3DLIGHT9 light;
		ZeroMemory(&light, sizeof(D3DLIGHT9));
		light.Type = D3DLIGHT_POINT;
		light.Diffuse.r = gui_devgui::dev_vec_04[0];
		light.Diffuse.g = gui_devgui::dev_vec_04[1];
		light.Diffuse.b = gui_devgui::dev_vec_04[2];

		light.Position.x = gui_devgui::dev_vec_03[0] /*+ cosf(timeGetTime() / 350.0f)*/;
		light.Position.y = gui_devgui::dev_vec_03[1];
		light.Position.z = gui_devgui::dev_vec_03[2];

		/*vecDir = D3DXVECTOR3(cosf(timeGetTime() / 350.0f),
			1.0f,
			sinf(timeGetTime() / 350.0f));
		D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);*/

		light.Falloff = gui_devgui::dev_vec_02[0];
		light.Range = gui_devgui::dev_vec_02[2]; //400.0f;
		game::glob::d3d9_device->SetLight(0, &light);
		game::glob::d3d9_device->LightEnable(0, TRUE);
		game::glob::d3d9_device->SetRenderState(D3DRS_LIGHTING, TRUE);

		// Finally, turn on some ambient light.
		//game::glob::d3d9_device->SetRenderState(D3DRS_AMBIENT, 0x00202020);
	}

	LPDIRECT3DTEXTURE9      g_pTexture = NULL; // Our texture
	bool init_once_test = false;

	void r_set_3d()
	{
		const auto state = game::gfxCmdBufSourceState;
		float v1;
		game::GfxViewParms* v2;

		if (state->viewMode != game::VIEW_MODE_3D)
		{
			v1 = 0.0f;
			v2 = state->viewParms3D;
			state->viewMode = game::VIEW_MODE_3D;
			memcpy(&state->viewParms, v2, sizeof(state->viewParms));

			if (0.0f == state->viewParms.origin[3])
			{
				state->eyeOffset[0] = 0.0f;
				state->eyeOffset[1] = 0.0f;
			}
			else
			{
				state->eyeOffset[0] = state->viewParms.origin[0];
				state->eyeOffset[1] = state->viewParms.origin[1];
				v1 = state->viewParms.origin[2];
			}
			state->eyeOffset[2] = v1;
			state->eyeOffset[3] = 1.0f;

			//R_CmdBufSet3D(a1);

			++state->matrixVersions[0];
			++state->matrixVersions[1];
			++state->matrixVersions[2];
			++state->matrixVersions[4];
			++state->matrixVersions[3];
			++state->matrixVersions[5];
			++state->matrixVersions[7];
			state->constVersions[58] = state->matrixVersions[0];

			//memcpy(state, &identity_matrix, 0x40u);
			const auto mtx = state->matrices.matrix[0].m;
			mtx[0][0] = 1.0f;  mtx[0][1] = 0.0f; mtx[0][2] = 0.0f; mtx[0][3] = 0.0f;
			mtx[1][0] = 0.0f;  mtx[1][1] = 1.0f; mtx[1][2] = 0.0f; mtx[1][3] = 0.0f;
			mtx[2][0] = 0.0f;  mtx[2][1] = 0.0f; mtx[2][2] = 1.0f; mtx[2][3] = 0.0f;
			mtx[3][0] = 0.0f;  mtx[3][1] = 0.0f; mtx[3][2] = 0.0f; mtx[3][3] = 1.0f;

			mtx[3][0] = mtx[3][0] - state->eyeOffset[0];
			mtx[3][1] = mtx[3][1] - state->eyeOffset[1];
			mtx[3][2] = mtx[3][2] - state->eyeOffset[2];
		}
	}

	void fixed_drawing_test()
	{
		const auto dev = game::glob::d3d9_device;
		const auto data = game::get_backenddata();
		// R_CheckLostDevice
		//if (utils::hook::call<bool(__cdecl)()>(0x5F5530)())
		{
			// rg.inFrame
			//game::rg->inFrame;
			//if (SUCCEEDED(dev->BeginScene()) && data->viewInfoCount)
			{
				//game::R_SetRenderTarget(game::R_RENDERTARGET_SCENE);

				//game::glob::d3d9_device->Clear(0, NULL, D3DCLEAR_TARGET /*| D3DCLEAR_ZBUFFER*/,
				//	D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

				//dev->Clear(0, NULL, D3DCLEAR_TARGET /*| D3DCLEAR_ZBUFFER*/,
				//	D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

				//game::R_Set3D();
				game::gfxCmdBufSourceState->viewParms3D = &data->viewInfo->viewParms;
				r_set_3d();

				//game::gfxCmdBufSourceState;
				//D3DMATRIX viewMtx = {};
				//memcpy(&viewMtx.m, game::gfxCmdBufSourceState->viewParms.viewMatrix.m)

				//D3DXMATRIXA16 matWorld;
				//D3DXMatrixIdentity(&matWorld);
				//dev->SetTransform(D3DTS_WORLD, &matWorld);



				// part of R_DrawLit
				// R_InitCmdBufSourceState
				/*game::GfxCmdBufSourceState source;
				utils::hook::call<void(__cdecl)(game::GfxCmdBufSourceState*, game::GfxCmdBufInput*, int)>(0x633430)(&source, &data->viewInfo->input, 1);
				source.renderTargetWidth = data->viewInfo->displayViewport.width;
				source.sceneViewport.x = data->viewInfo->sceneViewport.x;
				source.sceneViewport.width = data->viewInfo->sceneViewport.width;
				source.renderTargetHeight = data->viewInfo->displayViewport.height;
				source.sceneViewport.y = data->viewInfo->sceneViewport.y;
				source.viewportBehavior = game::GFX_USE_VIEWPORT_FOR_VIEW;
				source.sceneViewport.height = data->viewInfo->sceneViewport.height;
				source.viewMode = game::VIEW_MODE_NONE;
				source.viewportIsDirty = true;*/

				// part of R_DrawCall


				// -------------------------

				dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->matrices.matrix[0].m));
				dev->SetTransform(D3DTS_VIEW, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->viewParms.viewMatrix.m));
				dev->SetTransform(D3DTS_PROJECTION, reinterpret_cast<D3DMATRIX*>(&game::gfxCmdBufSourceState->viewParms.projectionMatrix.m));

				if (!init_once_test)
				{
					init_once_test = true;

					if (FAILED(D3DXCreateTextureFromFile(dev, L"banana.bmp", &g_pTexture)))
					{
						// If texture is not in current folder, try parent folder
						if (FAILED(D3DXCreateTextureFromFile(dev, L"..\\banana.bmp", &g_pTexture)))
						{
							MessageBox(NULL, L"Could not find banana.bmp", L"dx9textures.exe", MB_OK);
						}
					}
				}

				const float multi = gui_devgui::dev_vec_02[1]; //5.0f;

				const float x_offset = gui_devgui::dev_vec_01[0];
				const float y_offset = gui_devgui::dev_vec_01[1];
				const float z_offset = gui_devgui::dev_vec_01[2]; //150.0f;

				//CUSTOMVERTEX cvVertices[] = 
				//{
				//	{-100.0f * multi, -100.0f * multi, z_offset, 1.0f, 0xffff0000,}, //Vertex 1 – Red (250, 100)
				//	{ 100.0f * multi, -100.0f * multi, z_offset, 1.0f, 0xff00ff00,}, //Vertex 2 – Green (400, 350)
				//	{  50.0f,          100.0f * multi, z_offset, 1.0f, 0xff00ffff,}, //Vertex 3 – Blue (100, 350) D3DCOLOR_XRGB(0, 0, 255)
				//};

				CUSTOMVERTEX cvVertices[3] = {};
				cvVertices[0].position = D3DXVECTOR3(-100.0f * multi + x_offset, -100.0f * multi + y_offset, z_offset);
				cvVertices[0].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
				cvVertices[0].tu = 0.0f;
				cvVertices[0].tv = 0.0f;

				cvVertices[1].position = D3DXVECTOR3(100.0f * multi + x_offset, -100.0f * multi + y_offset, z_offset);
				cvVertices[1].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
				cvVertices[1].tu = 0.0f;
				cvVertices[0].tv = 1.0f;

				cvVertices[2].position = D3DXVECTOR3(50.0f + x_offset, 100.0f * multi + y_offset, z_offset);
				cvVertices[2].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
				cvVertices[1].tu = 1.0f;
				cvVertices[0].tv = 0.5f;

				SetupLights();

				dev->SetTexture(0, g_pTexture);
				dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

				dev->SetFVF(D3DFVF_CUSTOMVERTEX);
				dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, &cvVertices, sizeof(CUSTOMVERTEX));

				//dev->EndScene();

				/*utils::hook::call<void(__cdecl)(game::GfxCmdBufInput*, game::GfxViewInfo*, game::GfxCmdBufSourceState*, game::GfxCmdBufState*)>(0x6495A0)
					(&data->viewInfo->input, data->viewInfo, game::gfxCmdBufSourceState, game::gfxCmdBufState);*/
			}

			//dev->Present(NULL, NULL, NULL, NULL);

			/*game::glob::d3d9_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
				D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);*/
		}
	}

	__declspec(naked) void fixed_drawing_test_stub()
	{
		const static uint32_t retn_addr = 0x655621;
		__asm
		{
			mov     ecx, [esp + 0x10];
			movzx   eax, word ptr[ecx];

			pushad;
			call	fixed_drawing_test;
			popad;

			jmp		retn_addr;
		}
	}

	__declspec(naked) void fixed_drawing_test_stub2()
	{
		const static uint32_t retn_addr = 0x64B20D;
		__asm
		{
			pushad;
			call	fixed_drawing_test;
			popad;

			jmp		retn_addr;
		}
	}

	_renderer::_renderer()
	{
		// fixed function test

		//utils::hook::nop(0x655604, 22); // nop DrawIndexedPrimitive call (+ stack pushes) in R_DrawBspDrawSurfsLitPreTess (general world)

		// 65561A
		//utils::hook::nop(0x65561A, 7);
		//utils::hook(0x65561A, fixed_drawing_test_stub, HOOK_JUMP).install()->quick();

		//utils::hook(0x64B7CB, fixed_drawing_test_stub2, HOOK_JUMP).install()->quick();
		//utils::hook(0x64B1F9, fixed_drawing_test_stub2, HOOK_JUMP).install()->quick(); // works kinda because of depth prepass
		utils::hook(0x64B208, fixed_drawing_test_stub2, HOOK_JUMP).install()->quick();

		/*
		* Increase the amount of skinned vertices (bone controlled meshes) per frame.
		*      (R_MAX_SKINNED_CACHE_VERTICES | TEMP_SKIN_BUF_SIZE) Warnings
		*           'r_fastSkin' or 'r_skinCache' needs to be disabled or
		*			  the client will crash if you hit an unkown limit
		*/

		// Create dynamic rendering buffers
		utils::hook(0x5F3EC2, create_dynamic_buffers, HOOK_CALL).install()->quick();
		utils::hook(0x5F3EA9, alloc_dynamic_vertex_buffer, HOOK_CALL).install()->quick();

		// Alloc dynamic indices (smodelCache)
		utils::hook::nop(0x5F5D97, 7); // clear
		utils::hook(0x5F5D97, init_smodel_indices_stub, HOOK_JUMP).install()->quick();

		// Change 'R_WARN_TEMP_SKIN_BUF_SIZE' warning limit to new buffer size
		utils::hook::nop(0x643942, 6); // clear
		utils::hook(0x643942, r_warn_temp_skin_size_limit_stub, HOOK_JUMP).install()->quick();

		// Change 'R_WARN_MAX_SKINNED_CACHE_VERTICES' warning limit to new buffer size
		utils::hook::nop(0x64381C, 6); // clear
		utils::hook(0x64381C, r_warn_max_skinned_cache_vertices_limit_stub, HOOK_JUMP).install()->quick();


		/* ---------------------------------------------------------- */

		// hook "Com_Error(1, "Tried to use '%s' when it isn't valid\n", codeSampler)" to skip the R_SetSampler call
		utils::hook(0x64BCF1, codesampler_error01_stub, HOOK_JUMP).install()->quick(); // R_SetupPassPerObjectArgs
		utils::hook(0x64C350, codesampler_error02_stub, HOOK_JUMP).install()->quick(); // R_SetPassShaderStableArguments (not really used)

		// fix cubemapshot_f (needs disabled AA, disabled r_smp_backend and game-resolution > then cubemapshot resolution)
		utils::hook::nop(0x47549E, 3);				// start with suffix "_rt" and not with junk memory
		utils::hook::set<BYTE>(0x4754D5 + 2, 0xB0); // end on "_dn" + 1 instead of "_bk" (6 images)
		utils::hook(0x47540C, cubemap_shot_f_stub, HOOK_JUMP).install()->quick(); // dvar info


		/* ---------------------------------------------------------- */

		// R_AddWorldSurfacesPortalWalk :: less culling
		// 0x60B02E -> jl to jmp // 0x7C -> 0xEB //utils::hook::set<BYTE>(0x60B02E, 0xEB);
		//utils::hook::nop(0x60B028, 6); utils::hook(0x60B028, r_cull_world_stub_01, HOOK_JUMP).install()->quick(); // crashes on release

		// R_AddAabbTreeSurfacesInFrustum_r :: disable all surface culling (bad fps)
		// 0x643B08 -> nop //utils::hook::nop(0x643B08, 6);
		//utils::hook(0x643B03, r_cull_world_stub_02, HOOK_JUMP).install()->quick();

		// 0x643B39 -> jmp ^ // 0x74 -> 0xEB //utils::hook::set<BYTE>(0x643B39, 0xEB);
		//utils::hook(0x643B34, r_cull_world_stub_03, HOOK_JUMP).install()->quick();

		// R_AddCellSceneEntSurfacesInFrustumCmd :: active ents like destructible cars / players (disable all culling)
		// 0x64D17A -> nop // 2 bytes //utils::hook::nop(0x64D17A, 2);
		//utils::hook::nop(0x64D172, 8); utils::hook(0x64D172, r_cull_entities_stub, HOOK_JUMP).install()->quick();

		// R_AddWorkerCmd :: disable dynEnt models
		// 0x629328 -> nop
		//utils::hook(0x629328, r_draw_dynents_stub, HOOK_JUMP).install()->quick(); // popad makes it worse


		/* ---------------------------------------------------------- */

		static std::vector <const char*> r_wireframe_enum =
		{
			"NONE",
			"SHADED",
			"SOLID",
			"SHADED_WHITE",
			"SOLID_WHITE"
		};

		dvars::r_wireframe_world = game::Dvar_RegisterEnum(
			/* name		*/ "r_wireframe_world",
			/* desc		*/ "Draw world objects using their wireframe technique",
			/* default	*/ 0,
			/* enumSize	*/ r_wireframe_enum.size(),
			/* enumData */ r_wireframe_enum.data(),
			/* flags	*/ game::dvar_flags::none);

		dvars::r_wireframe_xmodels = game::Dvar_RegisterEnum(
			/* name		*/ "r_wireframe_xmodels",
			/* desc		*/ "Draw xmodels using their wireframe technique",
			/* default	*/ 0,
			/* enumSize	*/ r_wireframe_enum.size(),
			/* enumData */ r_wireframe_enum.data(),
			/* flags	*/ game::dvar_flags::none);

		dvars::r_debugShaderTexcoord = game::Dvar_RegisterBool(
			/* name		*/ "r_debugShaderTexcoord",
			/* desc		*/ "Show surface UV's / Texcoords",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none);

		// not in use
		//dvars::r_debugShaderToggle = Game::Dvar_RegisterBool(
		//	/* name		*/ "r_debugShaderToggle",
		//	/* desc		*/ "debugging purpose",
		//	/* default	*/ false,
		//	/* flags	*/ Game::dvar_flags::none);

		//// not in use
		//dvars::r_setmaterial_hk = Game::Dvar_RegisterBool(
		//	/* name		*/ "r_setmaterial_hk",
		//	/* desc		*/ "debugging purpose",
		//	/* default	*/ false,
		//	/* flags	*/ Game::dvar_flags::none);


		// hook R_SetMaterial
		utils::hook(0x648F86, R_SetMaterial_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x648F3C, R_SetPrepassMaterial_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x6490B7, R_SetMaterial_Emissive_stub, HOOK_JUMP).install()->quick();


		// custom pixelshader code constants
		utils::hook::nop(0x64BEDB, 7);
		utils::hook(0x64BEDB, R_SetPassPixelShaderStableArguments_stub, HOOK_JUMP).install()->quick();

		// custom vertexshader code constants / per object :: R_SetPassShaderObjectArguments
		utils::hook::nop(0x64BD22, 7);
		utils::hook(0x64BD22, R_SetVertexShaderConstantFromCode_stub, HOOK_JUMP).install()->quick();

		// seperate world and viewmodel fov
		utils::hook::nop(0x5FAA05, 6);
		utils::hook(0x5FAA05, R_SetViewParmsForScene_stub, HOOK_JUMP).install()->quick();

		dvars::cg_fov_tweaks = game::Dvar_RegisterBool(
			/* name		*/ "cg_fov_tweaks",
			/* desc		*/ "Enable gun fov tweaks",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);
		
		dvars::cg_fov_gun = game::Dvar_RegisterFloat(
			/* name		*/ "cg_fov_gun",
			/* desc		*/ "Adjust gun fov separately (wont effect world fov)",
			/* default	*/ 65.0f,
			/* minVal	*/ 20.0f,
			/* maxVal	*/ 160.0f,
			/* flags	*/ game::dvar_flags::saved);


		// increase fps cap to 125 for menus and loadscreen
		utils::hook::set<BYTE>(0x500174 + 2, 8);
		utils::hook::set<BYTE>(0x500177 + 2, 8);


		command::add("dumpreflections", "", "", [this](command::params)
		{
			const auto gfx = game::DB_FindXAssetHeader(game::ASSET_TYPE_GFXWORLD, utils::va("maps/mp/%s.d3dbsp", game::Dvar_FindVar("ui_mapname")->current.string)).gfxWorld;

			for (size_t i = 0; i < gfx->reflectionProbeCount; i++)
			{
				auto* probe = gfx->reflectionProbes[i].reflectionImage->texture.cubemap;
				for (auto j = 0; j < 6; j++)
				{
					const auto& base_path = game::Dvar_FindVar("fs_basepath");
					std::string file_path = base_path->current.string + "\\iw3xo\\reflection_cubes\\"s;
					std::filesystem::create_directories(file_path);

					IDirect3DSurface9* surface = nullptr;
					probe->GetCubeMapSurface((D3DCUBEMAP_FACES)j, 0, &surface);

					const char* str = utils::va("%s\\probe%d_side%d.png", file_path.c_str(), i, j);

					D3DXSaveSurfaceToFileA(str, D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG, surface, nullptr, nullptr);
				}
			}
		});
	}
}