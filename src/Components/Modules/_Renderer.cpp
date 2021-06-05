#include "STDInclude.hpp"

//#define RELOC_GFX_SURFS_BUFFER

// BACKUP
//#define GFX_SKINNED_CACHE_VB_POOL_SIZE	0x2000000				// stock : 0x480000 : gfxBuf.skinnedCacheVbPool
//#define GFX_SMODEL_CACHE_VB_SIZE			0x1400000				// stock : 0x800000 : gfxBuf.smodelCacheVb
//#define GFX_SMODEL_CACHE_SIZE				0x250000				// stock : 0x100000 : gfxBuf.smodelCache
//#define GFX_DYN_IB_POOL_SIZE				0x250000				// stock : 0x100000 : gfxBuf.dynamicIndexBufferPool
//#define GFX_PRETESS_IB_POOL_SIZE			GFX_DYN_IB_POOL_SIZE	// stock : 0x100000 : gfxBuf.preTessIndexBufferPool

namespace Components
{
	/* ---------------------------------------------------------- */
	/* ------------ create dynamic rendering buffers ------------ */

	// Dynamic Indices
	int R_AllocDynamicIndexBuffer(IDirect3DIndexBuffer9** ib, int sizeInBytes, const char* buffer_name, bool loadForRenderer)
	{
		if (!loadForRenderer) {
			return 0;
		}

		HRESULT hr = Game::Globals::d3d9_device->CreateIndexBuffer(sizeInBytes, (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY), D3DFMT_INDEX16, D3DPOOL_DEFAULT, ib, 0);
		if (hr < 0)
		{
			const char* msg = Utils::function<const char* __stdcall(HRESULT)>(0x685F98)(hr); // R_ErrorDescription
			msg = Utils::VA("DirectX didn't create a 0x%.8x dynamic index buffer: %s\n", sizeInBytes, msg);

			Utils::function<void(const char*)>(0x576A30)(msg); // Sys_Error
		}

		Game::Com_PrintMessage(0, Utils::VA("D3D9: Created Indexbuffer (%s) of size: 0x%.8x\n", buffer_name, sizeInBytes), 0);
		return 0;
	}

	void R_InitDynamicIndexBufferState(Game::GfxIndexBufferState* ib, int indexCount, const char* buffer_name, bool loadForRenderer)
	{
		ib->used = 0;
		ib->total = indexCount;

		R_AllocDynamicIndexBuffer(&ib->buffer, 2 * indexCount, buffer_name, loadForRenderer);
	}

	// Dynamic Vertices
	char* R_AllocDynamicVertexBuffer(IDirect3DVertexBuffer9** vb, int sizeInBytes, const char* buffer_name, bool loadForRenderer)
	{
		if (!loadForRenderer)
		{
			return 0;
		}

		HRESULT hr = Game::Globals::d3d9_device->CreateVertexBuffer(sizeInBytes, (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY), 0, D3DPOOL_DEFAULT, vb, 0);
		if (hr < 0)
		{
			const char* msg = Utils::function<const char* __stdcall(HRESULT)>(0x685F98)(hr); // R_ErrorDescription
			msg = Utils::VA("DirectX didn't create a 0x%.8x dynamic vertex buffer: %s\n", sizeInBytes, msg);

			Utils::function<void(const char*)>(0x576A30)(msg); // Sys_Error
		}

		Game::Com_PrintMessage(0, Utils::VA("D3D9: Created Vertexbuffer (%s) of size: 0x%.8x\n", buffer_name, sizeInBytes), 0);
		return 0;
	}

	void R_InitDynamicVertexBufferState(Game::GfxVertexBufferState* vb, int bytes, const char* buffer_name, bool loadForRenderer)
	{
		vb->used = 0;
		vb->total = bytes;
		vb->verts = 0;

		R_AllocDynamicVertexBuffer(&vb->buffer, bytes, buffer_name, loadForRenderer);
	}

	// Temp skin buffer within backenddata
	void R_InitTempSkinBuf(int bytes)
	{
		auto* s_backEndData = reinterpret_cast<Game::GfxBackEndData*>(0xCC9F600);

		for (auto i = 0; i < 2; ++i)
		{
			s_backEndData[i].tempSkinBuf = (char*)VirtualAlloc(0, bytes, MEM_RESERVE, PAGE_READWRITE);
			Game::Com_PrintMessage(0, Utils::VA("Allocated tempSkinBuffer of size: 0x%.8x\n", bytes), 0);
		}
	}

	void R_CreateDynamicBuffers()
	{
		auto& gfxBuf = *reinterpret_cast<Game::GfxBuffers*>(0xD2B0840);
		const auto& r_loadForRenderer = Game::Dvar_FindVar("r_loadForRenderer")->current.enabled;

		// default size in bytes
		std::uint32_t dynamic_vb_size = 1 * 1048576;
		std::uint32_t skinned_cache_size = 4 * 1048576;
		std::uint32_t temp_skin_size = 4 * 1048576;
		std::uint32_t dynamic_ib_size = 2 * 1048576;
		std::uint32_t pretess_ib_size = 2 * 1048576;

		// get size in bytes from dvars
		if (Dvars::r_buf_dynamicVertexBuffer)
		{
			dynamic_vb_size = Dvars::r_buf_dynamicVertexBuffer->current.integer * 1048576;
		}

		if (Dvars::r_buf_skinnedCacheVb)
		{
			skinned_cache_size = Dvars::r_buf_skinnedCacheVb->current.integer * 1048576;
		}

		if (Dvars::r_buf_tempSkin)
		{
			temp_skin_size = Dvars::r_buf_tempSkin->current.integer * 1048576;
		}

		if (Dvars::r_buf_dynamicIndexBuffer)
		{
			dynamic_ib_size = Dvars::r_buf_dynamicIndexBuffer->current.integer * 1048576;
		}

		if (Dvars::r_buf_preTessIndexBuffer)
		{
			pretess_ib_size = Dvars::r_buf_preTessIndexBuffer->current.integer * 1048576;
		}


		for (auto i = 0; i != 1; ++i)
		{
			R_InitDynamicVertexBufferState(&gfxBuf.dynamicVertexBufferPool[i], dynamic_vb_size, "dynamicVertexBufferPool", r_loadForRenderer);
		}
		gfxBuf.dynamicVertexBuffer = gfxBuf.dynamicVertexBufferPool;


		for (auto i = 0; i != 2; ++i)
		{
			R_InitDynamicVertexBufferState(&gfxBuf.skinnedCacheVbPool[i], skinned_cache_size, "skinnedCacheVbPool", r_loadForRenderer);
		}

		R_InitTempSkinBuf(temp_skin_size);


		for (auto i = 0; i != 1; ++i)
		{
			R_InitDynamicIndexBufferState(&gfxBuf.dynamicIndexBufferPool[i], dynamic_ib_size, "dynamicIndexBufferPool", r_loadForRenderer);
		}
		gfxBuf.dynamicIndexBuffer = gfxBuf.dynamicIndexBufferPool;


		for (auto i = 0; i != 2; ++i)
		{
			R_InitDynamicIndexBufferState(&gfxBuf.preTessIndexBufferPool[i], pretess_ib_size, "preTessIndexBufferPool", r_loadForRenderer);
		}
		gfxBuf.preTessBufferFrame = 0;
		gfxBuf.preTessIndexBuffer = gfxBuf.preTessIndexBufferPool;
	}

	/* ---------------------------------------------------------- */
	/* ---------- Alloc dynamic vertices (smodelCache) ---------- */

	void R_AllocDynamicVertexBuffer()
	{
		auto& gfxBuf = *reinterpret_cast<Game::GfxBuffers*>(0xD2B0840);
		const auto& r_loadForRenderer = Game::Dvar_FindVar("r_loadForRenderer")->current.enabled;

		// default size in bytes
		std::uint32_t smodel_cache_vb_size = 8 * 1048576;

		// get size in bytes from dvar
		if (Dvars::r_buf_smodelCacheVb)
		{
			smodel_cache_vb_size = Dvars::r_buf_smodelCacheVb->current.integer * 1048576;
		}

		if (r_loadForRenderer)
		{
			HRESULT hr = Game::Globals::d3d9_device->CreateVertexBuffer(smodel_cache_vb_size, (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY), 0, D3DPOOL_DEFAULT, &gfxBuf.smodelCacheVb, 0);
			if (hr < 0)
			{
				const char* msg = Utils::function<const char* __stdcall(HRESULT)>(0x685F98)(hr); // R_ErrorDescription
				msg = Utils::VA("DirectX didn't create a 0x%.8x dynamic vertex buffer: %s\n", smodel_cache_vb_size, msg);

				Utils::function<void(const char*)>(0x576A30)(msg); // Sys_Error
			}

			Game::Com_PrintMessage(0, Utils::VA("D3D9: Created Vertexbuffer (smodelCacheVb) of size: 0x%.8x\n", smodel_cache_vb_size), 0);
		}
	}

	/* ---------------------------------------------------------- */
	/* ---------- Alloc dynamic indices (smodelCache) ----------- */

	void r_init_smodel_indices()
	{
		auto& gfxBuf = *reinterpret_cast<Game::GfxBuffers*>(0xD2B0840);

		// default size in bytes
		std::uint32_t smodel_cache_ib_size = 2 * 1048576;

		// get size in bytes from dvar
		if (Dvars::r_buf_smodelCacheIb)
		{
			smodel_cache_ib_size = Dvars::r_buf_smodelCacheIb->current.integer * 1048576;
		}

		gfxBuf.smodelCache.used = 0;
		gfxBuf.smodelCache.total = smodel_cache_ib_size / 2; // Why half?

		auto mem_reserve = VirtualAlloc(0, 0x200000u, MEM_RESERVE, PAGE_READWRITE);
		auto mem_commit = VirtualAlloc(mem_reserve, 0x200000u, MEM_COMMIT, PAGE_READWRITE);

		if (!mem_commit || !mem_reserve)
		{
			if (!mem_commit && mem_reserve)
			{
				VirtualFree(mem_reserve, 0, MEM_RESET);
			}

			const char* msg = Utils::VA("r_init_smodel_indices :: Unable to allocate 0x%.8x bytes. Out of memory?\n", smodel_cache_ib_size);
			Utils::function<void(const char*)>(0x576A30)(msg); // Sys_Error
		}

		Game::Com_PrintMessage(0, Utils::VA("Allocated smodelCache (smodelCacheIb) of size: 0x%.8x\n", smodel_cache_ib_size), 0);
		gfxBuf.smodelCache.indices = static_cast<unsigned __int16*>(mem_reserve);
	}

	__declspec(naked) void r_init_smodel_indices_stub()
	{
		const static uint32_t rtn_pt = 0x5F5E7D;
		__asm
		{
			call	r_init_smodel_indices;
			pop     edi;
			jmp		rtn_pt;
		}
	}

	/* ---------------------------------------------------------- */
	/* ------ change warning limits to fit new buffer size ------ */

	__declspec(naked) void r_warn_temp_skin_size_limit_stub()
	{
		const static uint32_t mb_size = 1048576;
		const static uint32_t rtn_pt = 0x643948;
		__asm
		{
			// replace 'cmp edx, 480000h'

			push	eax;
			mov		eax, Dvars::r_buf_tempSkin;
			mov		eax, dword ptr[eax + 12];	// current->integer
			imul	eax, mb_size;				// get size in bytes

			cmp		edx, eax;					// warning limit compare
			pop		eax;

			jmp		rtn_pt;
		}
	}

	__declspec(naked) void r_warn_max_skinned_cache_vertices_limit_stub()
	{
		const static uint32_t mb_size = 1048576;
		const static uint32_t rtn_pt = 0x643822;
		__asm
		{
			// replace 'cmp edx, 480000h'

			push	eax;
			mov		eax, Dvars::r_buf_skinnedCacheVb;
			mov		eax, dword ptr[eax + 12];	// current->integer
			imul	eax, mb_size;				// get size in bytes

			cmp		edx, eax;					// warning limit compare
			pop		eax;

			jmp		rtn_pt;
		}
	}

	// ---------------------

	// called from _Common::R_RegisterAdditionalDvars (R_Init->R_RegisterDvars)
	// these need to be re-registered on vid_restart because they are latched
	void _Renderer::R_RegisterBufferDvars()
	{
		// Do not register dvars with the "default" register functions here. (create duplicates)

		Dvars::r_buf_skinnedCacheVb = Game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_skinnedCacheVb",
			/* desc		*/ "Size of skinnedCache Vertexbuffer (Size * 2 will be allocated) in Megabytes. Default : 4 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 4, // 24
			/* minVal	*/ 4,
			/* maxVal	*/ 64,
			/* flags	*/ Game::dvar_flags::saved | Game::dvar_flags::latched);

		Dvars::r_buf_smodelCacheVb = Game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_smodelCacheVb",
			/* desc		*/ "Size of smodelCache Vertexbuffer in Megabytes. Default : 8 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 8, // 24
			/* minVal	*/ 8,
			/* maxVal	*/ 64,
			/* flags	*/ Game::dvar_flags::saved | Game::dvar_flags::latched);

		Dvars::r_buf_smodelCacheIb = Game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_smodelCacheIb",
			/* desc		*/ "Size of smodelCache Indexbuffer in Megabytes. Default : 2 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 2, // 4
			/* minVal	*/ 2,
			/* maxVal	*/ 64,
			/* flags	*/ Game::dvar_flags::saved | Game::dvar_flags::latched);

		Dvars::r_buf_tempSkin = Game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_tempSkin",
			/* desc		*/ "Size of tempSkin buffer in Megabytes. Default : 4 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 4, // 16
			/* minVal	*/ 4,
			/* maxVal	*/ 64,
			/* flags	*/ Game::dvar_flags::saved | Game::dvar_flags::latched);

		Dvars::r_buf_dynamicVertexBuffer = Game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_dynamicVertexBuffer",
			/* desc		*/ "Size of dynamic Vertexbuffer in Megabytes. Default : 1 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 1, // 2
			/* minVal	*/ 1,
			/* maxVal	*/ 64,
			/* flags	*/ Game::dvar_flags::saved | Game::dvar_flags::latched);

		Dvars::r_buf_dynamicIndexBuffer = Game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_dynamicIndexBuffer",
			/* desc		*/ "Size of dynamic Indexbuffer in Megabytes. Default : 2 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 2, // 4
			/* minVal	*/ 2,
			/* maxVal	*/ 64,
			/* flags	*/ Game::dvar_flags::saved | Game::dvar_flags::latched);

		Dvars::r_buf_preTessIndexBuffer = Game::Dvar_RegisterIntWrapper(
			/* name		*/ "r_buf_preTessIndexBuffer",
			/* desc		*/ "Size of preTess Indexbuffer (Size * 2 will be allocated) in Megabytes. Default : 2 \n! DISABLE r_fastSkin if you are changing this dvar or the client will crash when hitting the old limit.",
			/* default	*/ 2, // 4
			/* minVal	*/ 2,
			/* maxVal	*/ 64,
			/* flags	*/ Game::dvar_flags::saved | Game::dvar_flags::latched);

		Dvars::r_cullWorld = Game::Dvar_RegisterBoolWrapper(
			/* name		*/ "r_cullWorld",
			/* desc		*/ "Culls invisible world surfaces. Disabling this can be useful for vertex manipulating shaders.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::latched);

		Dvars::r_cullEntities = Game::Dvar_RegisterBoolWrapper(
			/* name		*/ "r_cullEntities",
			/* desc		*/ "Culls invisible entities. Disabling this can be useful for vertex manipulating shaders.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::latched);

		Dvars::r_drawDynents = Game::Dvar_RegisterBoolWrapper(
			/* name		*/ "r_drawDynents",
			/* desc		*/ "Draw dynamic entities.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::none);
	}


	/* ---------------------------------------------------------- */

	void codesampler_error(Game::MaterialShaderArgument* arg, Game::GfxCmdBufSourceState* source, Game::GfxCmdBufState* state, const char* sampler, int droptype, const char* msg, ...)
	{
		if (!sampler || !state || !state->material || !state->technique) 
		{
			return;
		}

		Game::Com_PrintMessage(0, Utils::VA(
			"^1Tried to use sampler <%s> when it isn't valid!\n"
			"^7[Passdump]\n"
			"|-> Material ----- %s\n"
			"|-> Technique ----	%s\n"
			"|-> RenderTarget - %s\n"
			"|-> Not setting sampler using R_SetSampler!\n",
			sampler, state->material->info.name, state->technique->name, Game::RendertargetStringFromID(state->renderTargetId)), 0);
	}

	// R_SetupPassPerObjectArgs
	__declspec(naked) void codesampler_error01_stub()
	{
		const static uint32_t rtnPt = 0x64BD0F; // offset after call to R_SetSampler
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

			jmp		rtnPt;
		}
	}

	// R_SetPassShaderStableArguments
	__declspec(naked) void codesampler_error02_stub()
	{
		const static uint32_t rtnPt = 0x64C36C; // offset after call to R_SetSampler
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
			jmp		rtnPt;
		}
	}

	void cubemap_shot_f_sync_msg()
	{
		if (const auto& r_smp_backend = Game::Dvar_FindVar("r_smp_backend"))
		{
			if (r_smp_backend->current.enabled)
			{
				Game::Com_PrintMessage(0, "^1Error: ^7r_smp_backend must be set to 0!", 0);
			}
		}

		if (const auto& r_aaSamples = Game::Dvar_FindVar("r_aaSamples"))
		{
			if (r_aaSamples->current.integer != 1)
			{
				Game::Com_PrintMessage(0, "^1Error: ^7r_aaSamples must be set to 1 (Disable AA)!", 0);
			}
		}
	}

	__declspec(naked) void cubemap_shot_f_stub()
	{
		const static uint32_t R_SyncRenderThread_Func = 0x5F6070;
		const static uint32_t rtnPt = 0x475411;
		__asm
		{
			call	R_SyncRenderThread_Func;

			pushad;
			call	cubemap_shot_f_sync_msg;
			popad;

			jmp		rtnPt;
		}
	}


	/* ---------------------------------------------------------- */

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

			push	eax;
			mov		eax, Dvars::r_cullWorld;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;
			jmp		rtn_pt_stock;

		SKIP:
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

			push	eax;
			mov		eax, Dvars::r_cullWorld;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;
			jmp		rtn_pt_stock;

		SKIP:
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

			push	eax;
			mov		eax, Dvars::r_cullWorld;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;
			jmp		rtn_pt_stock;

		SKIP:
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

			push	eax;
			mov		eax, Dvars::r_cullEntities;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;
			jmp		rtn_pt_stock;

		SKIP:
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
			push	eax;
			mov		eax, Dvars::r_drawDynents;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			// jump if not culling world
			jne		SKIP;
			call	R_AddCellDynModelSurfacesInFrustumCmd_Func;

		SKIP:
			jmp		rtn_pt_stock;
		}
	}


	/* ---------------------------------------------------------- */

	bool r_disable_prepass_material = false;

	// changing MaterialTechType at 0x633282 to 1C -> wireframe (look at enum MaterialTechniqueType)
	int R_SetMaterial(Game::MaterialTechniqueType techType, Game::GfxCmdBufSourceState* src, Game::GfxCmdBufState* state, Game::GfxDrawSurf drawSurf)
	{
		Game::Material*			 current_material;
		Game::MaterialTechnique* current_technique;

		Game::MaterialTechniqueType newTechType = techType;
		Game::Material*				debug_material = nullptr;

		auto& rg = *reinterpret_cast<Game::r_globals_t*>(0xCC9D150);
		auto rgp = reinterpret_cast<Game::r_global_permanent_t*>(0xCC98280);

		current_material  = rgp->sortedMaterials[(drawSurf.packed >> 29) & 0x7FF];
		current_technique = current_material->techniqueSet->remappedTechniqueSet->techniques[newTechType];

		bool any_tweaks			= false;
		bool change_material	= false;

		r_disable_prepass_material = false; // always reset

		if (current_material)
		{
			// wireframe xmodels
			if (Dvars::r_wireframe_xmodels && Dvars::r_wireframe_xmodels->current.integer)
			{
				if (Utils::StartsWith(current_material->info.name, "mc/"))
				{
					any_tweaks = true;

					switch (Dvars::r_wireframe_xmodels->current.integer)
					{
					case 1: // SHADED
						newTechType = Game::TECHNIQUE_WIREFRAME_SHADED;
						break;

					case 2: // SOLID
						newTechType = Game::TECHNIQUE_WIREFRAME_SOLID;
						break;

					case 3: // SHADED_WHITE
						newTechType		= Game::TECHNIQUE_UNLIT;
						debug_material	= reinterpret_cast<Game::Material*>(Game::Material_RegisterHandle("iw3xo_showcollision_wire", 3));

						r_disable_prepass_material = true;
						change_material = true;
						break;

					case 4: // SOLID_WHITE
						newTechType		= Game::TECHNIQUE_UNLIT;
						debug_material	= reinterpret_cast<Game::Material*>(Game::Material_RegisterHandle("iw3xo_showcollision_wire", 3));
						
						change_material = true;
						break;
					}
				}
			}

			if (Dvars::r_wireframe_world && Dvars::r_wireframe_world->current.integer)
			{
				if (Utils::StartsWith(current_material->info.name, "wc/") && !Utils::StartsWith(current_material->info.name, "wc/sky"))
				{
					any_tweaks = true;

					switch (Dvars::r_wireframe_world->current.integer)
					{
					case 1: // SHADED
						newTechType = Game::TECHNIQUE_WIREFRAME_SHADED;
						break;

					case 2: // SOLID
						newTechType = Game::TECHNIQUE_WIREFRAME_SOLID;
						break;

					case 3: // SHADED_WHITE
						newTechType		= Game::TECHNIQUE_UNLIT;
						debug_material	= reinterpret_cast<Game::Material*>(Game::Material_RegisterHandle("iw3xo_showcollision_wire", 3));

						r_disable_prepass_material = true;
						change_material = true;
						break;

					case 4: // SOLID_WHITE
						newTechType		= Game::TECHNIQUE_UNLIT;
						debug_material	= reinterpret_cast<Game::Material*>(Game::Material_RegisterHandle("iw3xo_showcollision_wire", 3));

						change_material = true;
						break;
					}
				}
			}

			// texcoord debugshader
			if (Dvars::r_debugShaderTexcoord && Dvars::r_debugShaderTexcoord->current.enabled)
			{
				r_disable_prepass_material = true;
				any_tweaks = true;

				//	if (Utils::StartsWith(current_material->info.name, "mc/") && !Utils::StartsWith(current_material->info.name, "mc/mtl_weapon")
				//		&& !Utils::Contains(current_material->info.name, "hands"s) && !Utils::Contains(current_material->info.name, "glove"s))
				if (Utils::StartsWith(current_material->info.name, "mc/"))
				{
					debug_material = reinterpret_cast<Game::Material*>(Game::Material_RegisterHandle("debug_texcoords_dtex", 3));
				}
				//	else if (Utils::StartsWith(current_material->info.name, "wc/") && !Utils::StartsWith(current_material->info.name, "wc/sky"))
				else if (Utils::StartsWith(current_material->info.name, "wc/"))
				{
					debug_material = reinterpret_cast<Game::Material*>(Game::Material_RegisterHandle("debug_texcoords", 3));
				}

				newTechType = Game::TECHNIQUE_UNLIT;
			}

			if (!any_tweaks)
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

		// if debug_material was set
		if (   debug_material 
			&& debug_material->techniqueSet 
			&& debug_material->techniqueSet->remappedTechniqueSet 
			&& debug_material->techniqueSet->remappedTechniqueSet->techniques
			&& debug_material->techniqueSet->remappedTechniqueSet->techniques[newTechType])
		{
			//current_material = debug_material;
			current_technique = debug_material->techniqueSet->remappedTechniqueSet->techniques[newTechType];

			if (change_material)
			{
				current_material = debug_material;
			}
		}

		state->origMaterial = state->material;
		state->material = current_material;
		state->technique = current_technique;

		if (!current_technique)
		{
			return 0;
		}

		if ((current_technique->flags & 1) != 0 && !rg.distortion)
		{
			return 0;
		}

		if (!any_tweaks)
		{
			if ((newTechType == Game::TECHNIQUE_EMISSIVE || newTechType == Game::TECHNIQUE_UNLIT) && (current_technique->flags & 0x10) != 0 && !src->constVersions[4])
			{
				return 0;
			}
		}

		const auto& r_logFile = Game::Dvar_FindVar("r_logFile");
		if (r_logFile && r_logFile->current.integer && current_material)
		{
			auto string = Utils::VA("R_SetMaterial( %s, %s, %i )\n", current_material->info.name, current_technique->name, newTechType);

			const static uint32_t RB_LogPrint_Funk = 0x63CF40;
			__asm
			{
				pushad;
				mov		edx, string;
				call	RB_LogPrint_Funk;
				popad;
			}
		}

		state->origTechType = state->techType;
		state->techType = newTechType;

		return 1;
	}

	__declspec(naked) void R_SetMaterial_stub()
	{
		const static uint32_t rtn_to_set_shadowable_light = 0x648F92;
		const static uint32_t rtn_to_rtn = 0x648F48;
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
			jmp		rtn_to_rtn;
		}
	}

	__declspec(naked) void R_SetPrepassMaterial_stub()
	{
		const static uint32_t R_SetPrepassMaterial_Func = 0x648DF0;
		const static uint32_t rtnPt = 0x648F41;
		__asm
		{
			push	eax;
			mov		al, r_disable_prepass_material;
			cmp		al, 1;
			pop		eax;

			// jump if true
			je		end;
			call	R_SetPrepassMaterial_Func;

		end:
			jmp		rtnPt;
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

	void pixelshader_custom_constants(Game::GfxCmdBufState* state)
	{
		// dump shaders at runtime ~> TODO: move that to its own function / hook
		if (Dvars::r_dumpShaders && Dvars::r_dumpShaders->current.enabled)
		{
			const auto basePath = Game::Dvar_FindVar("fs_basepath");

			if (!basePath) {
				return;
			}

			std::string filePath = basePath->current.string + "\\iw3xo\\shader_dump\\"s;

			if (state && state->pass)
			{
				if (state->pass->vertexShader && state->pass->vertexShader->name)
				{
					// check if shader was already dumped
					if (!dumpedshader_contains(r_dumped_shader_set, "vs_"s + state->pass->vertexShader->name))
					{
						if (!folder_vs_exists)
						{
							std::filesystem::create_directories(filePath + "vertexShader\\");
							folder_vs_exists = true;
						}

						std::uint16_t bin_size = state->pass->vertexShader->prog.loadDef.programSize;
						std::ofstream outfile(filePath + "vertexShader\\" + "vs_" + state->pass->vertexShader->name, std::ofstream::binary);

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
							std::filesystem::create_directories(filePath + "pixelShader\\");
							folder_ps_exists = true;
						}

						std::uint16_t bin_size = state->pass->pixelShader->prog.loadDef.programSize;
						std::ofstream outfile(filePath + "pixelShader\\" + "ps_" + state->pass->pixelShader->name, std::ofstream::binary);

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
#ifdef DEVGUI_XO_BLUR
					// dev stuff
					if (state->pass->pixelShader && !Utils::Q_stricmp(state->pass->pixelShader->name, "postfx_blur_overlay"))
					{
						float constant[4] = { Game::Globals::xo_blur_directions, Game::Globals::xo_blur_quality, Game::Globals::xo_blur_size, Game::Globals::xo_blur_alpha };

						if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0) {
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}
					}
#endif
#ifdef DEVGUI_OCEAN
					// ------------------------------------------------------------------------------------------------------------
					// ocean shader

					if (state->pass->pixelShader && !Utils::Q_stricmp(state->pass->pixelShader->name, "worldfx_ocean"))
					{
						// vertex + pixel
						if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0) {
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, Game::ocean::_WaveAmplitude, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_1) {
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, Game::ocean::_WavesIntensity, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2) {
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, Game::ocean::_WavesNoise, 1);
						}

						// pixel only
						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_3)
						{
							float constant[4] = { Game::ocean::_AmbientColor[0], Game::ocean::_AmbientColor[1], Game::ocean::_AmbientColor[2], Game::ocean::_AmbientDensity };
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_4)
						{
							float constant[4] = { Game::ocean::_ShoreColor[0], Game::ocean::_ShoreColor[1], Game::ocean::_ShoreColor[2], Game::ocean::_DiffuseDensity };
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_5)
						{
							float constant[4] = { Game::ocean::_SurfaceColor[0], Game::ocean::_SurfaceColor[1], Game::ocean::_SurfaceColor[2], Game::ocean::_NormalIntensity };
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_6)
						{
							float constant[4] = { Game::ocean::_DepthColor[0], Game::ocean::_DepthColor[1], Game::ocean::_DepthColor[2], Game::ocean::_ShoreFade };
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_7)
						{
							float constant[4] = { Game::ocean::_RefractionValues[0], Game::ocean::_RefractionValues[1], Game::ocean::_RefractionValues[2], Game::ocean::_RefractionScale };
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_COLOR_MATRIX_R)
						{
							float constant[4] = { Game::ocean::_HorizontalExtinction[0], Game::ocean::_HorizontalExtinction[1], Game::ocean::_HorizontalExtinction[2], Game::ocean::_Distortion };
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_COLOR_MATRIX_G)
						{
							float constant[4] = { Game::ocean::_WaterClarity, Game::ocean::_WaterTransparency, Game::ocean::_RadianceFactor, 0.0f };
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_COLOR_MATRIX_B)
						{
							float constant[4] = { Game::ocean::_SpecularValues[0], Game::ocean::_SpecularValues[1], Game::ocean::_SpecularValues[2], Game::ocean::_Shininess };
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_DOF_EQUATION_VIEWMODEL_AND_FAR_BLUR)
						{
							float constant[4] = { Game::ocean::_FoamTiling[0], Game::ocean::_FoamTiling[1], Game::ocean::_FoamTiling[2], Game::ocean::_FoamSpeed };
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_DOF_EQUATION_SCENE)
						{
							float constant[4] = { Game::ocean::_FoamRanges[0], Game::ocean::_FoamRanges[1], Game::ocean::_FoamRanges[2], Game::ocean::_FoamIntensity };
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, constant, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_DOF_LERP_SCALE) {
							(*Game::dx9_device_ptr)->SetPixelShaderConstantF(arg_def->dest, Game::ocean::_FoamNoise, 1);
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


	void vertexshader_custom_constants(Game::GfxCmdBufSourceState* source, Game::GfxCmdBufState* state)
	{
		// fixup cod4 code constants
		if (state && state->pass)
		{
			// loop through all argument defs to find custom codeconsts
			for (auto arg = 0; arg < state->pass->perObjArgCount + state->pass->perPrimArgCount + state->pass->stableArgCount; arg++)
			{
				const auto arg_def = &state->pass->args[arg];

				// reference
				/*if (arg_def && arg_def->type == 4)
				{
					if (state->pass->pixelShader && !Utils::Q_stricmp(state->pass->pixelShader->name, "xo_2d_blur"))
					{
						auto& rg = *reinterpret_cast<Game::r_globals_t*>(0xCC9D150);
						const char* sampler = rg.codeImageNames[arg_def->u.codeSampler];

						char samplerState = source->input.codeImageSamplerStates[arg_def->u.codeSampler];
						Game::GfxImage* mat = source->input.codeImages[arg_def->u.codeSampler];

						int x = 1;
					}
				}*/

				if (arg_def && arg_def->type == 3)
				{
#ifdef DEVGUI_OCEAN
					// ocean shader
					if (state->pass->vertexShader && !Utils::Q_stricmp(state->pass->vertexShader->name, "worldfx_ocean"))
					{
						// vertex + pixel
						if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0) {
							(*Game::dx9_device_ptr)->SetVertexShaderConstantF(arg_def->dest, Game::ocean::_WaveAmplitude, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_1) {
							(*Game::dx9_device_ptr)->SetVertexShaderConstantF(arg_def->dest, Game::ocean::_WavesIntensity, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2) {
							(*Game::dx9_device_ptr)->SetVertexShaderConstantF(arg_def->dest, Game::ocean::_WavesNoise, 1);
						}

						// vertex only
						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_3)
						{
							float constant[4] = { Game::ocean::_WindDirection[0], Game::ocean::_WindDirection[1], Game::ocean::_HeightIntensity, Game::ocean::_WaveAmplitudeFactor };
							(*Game::dx9_device_ptr)->SetVertexShaderConstantF(arg_def->dest, constant, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_4)
						{
							float constant[4] = { Game::ocean::_WaveSteepness, Game::ocean::_TextureTiling, Game::ocean::_WaveTiling, Game::ocean::_Time };
							(*Game::dx9_device_ptr)->SetVertexShaderConstantF(arg_def->dest, constant, 1);
						}

						else if (arg_def->u.codeConst.index == Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_5)
						{
							float constant[4] = { Game::ocean::_VisibleWaveDist, Game::ocean::_HeightMapScale, Game::ocean::_HeightMapScroll, 0.0f };
							(*Game::dx9_device_ptr)->SetVertexShaderConstantF(arg_def->dest, constant, 1);
						}

						// matrix if bigger then 58 (matrix fuckery :bad:)
						//if (arg_def->u.codeConst.index >= 58)
						//{
						//	float* matrix;
						//	// R_GetCodeMatrix
						//	matrix = Utils::function<float* (Game::GfxCmdBufSourceState * source, int argIndex, char row)>(0x631D90)(source, arg_def->u.codeConst.index, arg_def->u.codeConst.firstRow);
						//	
						//	if (matrix)
						//	{
						//		matrix[3] += 0.2f;
						//		matrix[6] += 0.2f;
						//		matrix[9] += 0.2f;
						//		matrix[12] += 0.2f;
						//		(*Game::dx9_device_ptr)->SetVertexShaderConstantF(arg_def->dest, matrix, arg_def->u.codeConst.rowCount);
						//	}
						//}
					}
#endif
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


	/* ---------------------------------------------------------- */

#if 0
	__declspec(naked) void r_xmodel_bones_stub_01()
	{
		const static uint32_t rtn_pt = 0x57DB5B;
		__asm
		{
			// we are above 128 bones
			// we would now call com_error (more then 128 bones ..)
			mov		[esp + 2Ch], 128; // lower bonecount cap keeps the game alive longer till it eventually crashes
			jmp		rtn_pt;
		}
	}

	void gfxsceneent_test(Game::GfxSceneEntity* s_entity)
	{
		if (s_entity->obj && s_entity->obj->numBones > 120)
		{
			s_entity->obj->numBones = 120;
		}

		int x = 1;
	}

	__declspec(naked) void r_xmodel_bones_stub_02()
	{
		const static uint32_t rtn_pt = 0x5F7D0D;
		__asm
		{
			// stock op's
			mov[ebx + 70h], edi;
			mov[ebx + 6Eh], cx;

			// ebx = GfxSceneEntity*
			pushad;
			push	ebx;
			call	gfxsceneent_test;
			add		esp, 4;
			popad;

			jmp		rtn_pt;
		}
	}
#endif

	_Renderer::_Renderer()
	{ 
		// *
		// error :: dobj for xmodel %s has more then 128 bones

		//Utils::Hook::Nop(0x57DB32, 7); Utils::Hook(0x57DB32, r_xmodel_bones_stub_01, HOOK_JUMP).install()->quick();
		//Utils::Hook::Nop(0x6292FE, 5); // (R_SkinGfxEntityCmd)
		//Utils::Hook::Nop(0x5F7D06, 7);  Utils::Hook(0x5F7D06, r_xmodel_bones_stub_02, HOOK_JUMP).install()->quick();

		// -----

		/*
		* Increase the amount of skinned vertices (bone controlled meshes) per frame.
		*      (R_MAX_SKINNED_CACHE_VERTICES | TEMP_SKIN_BUF_SIZE) Warnings
		*           'r_fastSkin' or 'r_skinCache' needs to be disabled or
		*			  the client will crash if you hit an unkown limit
		*/

		// Create dynamic rendering buffers
		Utils::Hook(0x5F3EC2, R_CreateDynamicBuffers, HOOK_CALL).install()->quick();
		Utils::Hook(0x5F3EA9, R_AllocDynamicVertexBuffer, HOOK_CALL).install()->quick();

		// Alloc dynamic indices (smodelCache)
		Utils::Hook::Nop(0x5F5D97, 7); // clear
		Utils::Hook(0x5F5D97, r_init_smodel_indices_stub, HOOK_JUMP).install()->quick();

		// Change 'R_WARN_TEMP_SKIN_BUF_SIZE' warning limit to new buffer size
		Utils::Hook::Nop(0x643942, 6); // clear
		Utils::Hook(0x643942, r_warn_temp_skin_size_limit_stub, HOOK_JUMP).install()->quick();

		// Change 'R_WARN_MAX_SKINNED_CACHE_VERTICES' warning limit to new buffer size
		Utils::Hook::Nop(0x64381C, 6); // clear
		Utils::Hook(0x64381C, r_warn_max_skinned_cache_vertices_limit_stub, HOOK_JUMP).install()->quick();


		/* ---------------------------------------------------------- */

		// Hook "Com_Error(1, "Tried to use '%s' when it isn't valid\n", codeSampler)" to skip the R_SetSampler call
		Utils::Hook(0x64BCF1, codesampler_error01_stub, HOOK_JUMP).install()->quick(); // R_SetupPassPerObjectArgs
		Utils::Hook(0x64C350, codesampler_error02_stub, HOOK_JUMP).install()->quick(); // R_SetPassShaderStableArguments (not really used)

		// fix cubemapshot_f (needs disabled AA, disabled r_smp_backend and game-resolution > then cubemapshot resolution)
		Utils::Hook::Nop(0x47549E, 3);				// start with suffix "_rt" and not with junk memory
		Utils::Hook::Set<BYTE>(0x4754D5 + 2, 0xB0); // end on "_dn" + 1 instead of "_bk" (6 images)
		Utils::Hook(0x47540C, cubemap_shot_f_stub, HOOK_JUMP).install()->quick(); // dvar info


		/* ---------------------------------------------------------- */

		// R_AddWorldSurfacesPortalWalk :: less culling
		// 0x60B02E -> jl to jmp // 0x7C -> 0xEB //Utils::Hook::Set<BYTE>(0x60B02E, 0xEB);
		Utils::Hook::Nop(0x60B028, 6); Utils::Hook(0x60B028, r_cull_world_stub_01, HOOK_JUMP).install()->quick();

		// R_AddAabbTreeSurfacesInFrustum_r :: disable all surface culling (bad fps)
		// 0x643B08 -> nop //Utils::Hook::Nop(0x643B08, 6);
		Utils::Hook(0x643B03, r_cull_world_stub_02, HOOK_JUMP).install()->quick();

		// 0x643B39 -> jmp ^ // 0x74 -> 0xEB //Utils::Hook::Set<BYTE>(0x643B39, 0xEB);
		Utils::Hook(0x643B34, r_cull_world_stub_03, HOOK_JUMP).install()->quick();

		// R_AddCellSceneEntSurfacesInFrustumCmd :: active ents like destructible cars / players (disable all culling)
		// 0x64D17A -> nop // 2 bytes //Utils::Hook::Nop(0x64D17A, 2);
		Utils::Hook::Nop(0x64D172, 8); Utils::Hook(0x64D172, r_cull_entities_stub, HOOK_JUMP).install()->quick();

		// R_AddWorkerCmd :: disable dynEnt models
		// 0x629328 -> nop
		Utils::Hook(0x629328, r_draw_dynents_stub, HOOK_JUMP).install()->quick();


		/* ---------------------------------------------------------- */

		static std::vector <const char*> r_wireframe_enum =
		{
			"NONE",
			"SHADED",
			"SOLID",
			"SHADED_WHITE",
			"SOLID_WHITE"
		};

		Dvars::r_wireframe_world = Game::Dvar_RegisterEnum(
			/* name		*/ "r_wireframe_world",
			/* desc		*/ "draw world objects using their wireframe technique",
			/* default	*/ 0,
			/* enumSize	*/ r_wireframe_enum.size(),
			/* enumData */ r_wireframe_enum.data(),
			/* flags	*/ Game::dvar_flags::none);

		Dvars::r_wireframe_xmodels = Game::Dvar_RegisterEnum(
			/* name		*/ "r_wireframe_xmodels",
			/* desc		*/ "draw xmodels using its wireframe technique",
			/* default	*/ 0,
			/* enumSize	*/ r_wireframe_enum.size(),
			/* enumData */ r_wireframe_enum.data(),
			/* flags	*/ Game::dvar_flags::none);

		Dvars::r_debugShaderTexcoord = Game::Dvar_RegisterBool(
			/* name		*/ "r_debugShaderTexcoord",
			/* desc		*/ "Show surface UV's / Texcoords",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		// not in use
		//Dvars::r_debugShaderToggle = Game::Dvar_RegisterBool(
		//	/* name		*/ "r_debugShaderToggle",
		//	/* desc		*/ "debugging purpose",
		//	/* default	*/ false,
		//	/* flags	*/ Game::dvar_flags::none);

		//// not in use
		//Dvars::r_setmaterial_hk = Game::Dvar_RegisterBool(
		//	/* name		*/ "r_setmaterial_hk",
		//	/* desc		*/ "debugging purpose",
		//	/* default	*/ false,
		//	/* flags	*/ Game::dvar_flags::none);


		// Hook R_SetMaterial
		Utils::Hook(0x648F86, R_SetMaterial_stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x648F3C, R_SetPrepassMaterial_stub, HOOK_JUMP).install()->quick();


		// custom pixelshader code constants
		Utils::Hook::Nop(0x64BEDB, 7);
		Utils::Hook(0x64BEDB, R_SetPassPixelShaderStableArguments_stub, HOOK_JUMP).install()->quick();

		// custom vertexshader code constants / per object
		Utils::Hook::Nop(0x64BD22, 7);
		Utils::Hook(0x64BD22, R_SetVertexShaderConstantFromCode_stub, HOOK_JUMP).install()->quick();
	}

	_Renderer::~_Renderer()
	{ }
}