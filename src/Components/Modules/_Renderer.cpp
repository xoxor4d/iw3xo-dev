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

	IDirect3DDevice9* get_d3d_device()
	{
		if (Game::Globals::d3d9_device)
		{
			return Game::Globals::d3d9_device;
		}
		else
		{
			return *Game::dx9_device_ptr;
		}
	}

	// Dynamic Indices
	int R_AllocDynamicIndexBuffer(IDirect3DIndexBuffer9** ib, int sizeInBytes, const char* buffer_name, bool loadForRenderer)
	{
		if (!loadForRenderer) {
			return 0;
		}

		HRESULT hr = get_d3d_device()->CreateIndexBuffer(sizeInBytes, (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY), D3DFMT_INDEX16, D3DPOOL_DEFAULT, ib, 0);
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

		HRESULT hr = get_d3d_device()->CreateVertexBuffer(sizeInBytes, (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY), 0, D3DPOOL_DEFAULT, vb, 0);
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
			HRESULT hr = get_d3d_device()->CreateVertexBuffer(smodel_cache_vb_size, (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY), 0, D3DPOOL_DEFAULT, &gfxBuf.smodelCacheVb, 0);
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

		//Dvars::r_cullWorld = Game::Dvar_RegisterBoolWrapper(
		//	/* name		*/ "r_cullWorld",
		//	/* desc		*/ "Culls invisible world surfaces. Disabling this can be useful for vertex manipulating shaders.",
		//	/* default	*/ true,
		//	/* flags	*/ Game::dvar_flags::latched);

		//Dvars::r_cullEntities = Game::Dvar_RegisterBoolWrapper(
		//	/* name		*/ "r_cullEntities",
		//	/* desc		*/ "Culls invisible entities. Disabling this can be useful for vertex manipulating shaders.",
		//	/* default	*/ true,
		//	/* flags	*/ Game::dvar_flags::latched);

		//Dvars::r_drawDynents = Game::Dvar_RegisterBoolWrapper(
		//	/* name		*/ "r_drawDynents",
		//	/* desc		*/ "Draw dynamic entities.",
		//	/* default	*/ true,
		//	/* flags	*/ Game::dvar_flags::none);
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
			mov		eax, Dvars::r_cullWorld;
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
			mov		eax, Dvars::r_cullWorld;
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
			mov		eax, Dvars::r_cullWorld;
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
			mov		eax, Dvars::r_cullEntities;
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
			mov		eax, Dvars::r_drawDynents;
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
	bool _Renderer::is_valid_technique_for_type(const Game::Material* mat, const Game::MaterialTechniqueType type)
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
	void _Renderer::switch_technique(Game::switch_material_t* swm, Game::Material* material)
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
	void _Renderer::switch_technique(Game::switch_material_t* swm, const char* material_name)
	{
		Game::Material* temp_mat = reinterpret_cast<Game::Material*>(Game::Material_RegisterHandle(material_name, 3));

		if (temp_mat)
		{
			swm->technique = nullptr;

			if (is_valid_technique_for_type(temp_mat, swm->technique_type))
			{
				swm->technique = temp_mat->techniqueSet->remappedTechniqueSet->techniques[swm->technique_type];
			}

			swm->switch_technique = true;
			return;
		}

		// return stock technique if the above failed
		swm->technique = swm->current_technique;
	}


	//*
	// return new material if valid, stock material otherwise
	void _Renderer::switch_material(Game::switch_material_t* swm, const char* material_name)
	{
		Game::Material* temp_mat = reinterpret_cast<Game::Material*>(Game::Material_RegisterHandle(material_name, 3));
		
		if (temp_mat)
		{
			swm->material = temp_mat;
			swm->technique = nullptr;

			if (is_valid_technique_for_type(temp_mat, swm->technique_type))
			{
				swm->technique = temp_mat->techniqueSet->remappedTechniqueSet->techniques[swm->technique_type];
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
	int R_SetMaterial(Game::MaterialTechniqueType techType, Game::GfxCmdBufSourceState* src, Game::GfxCmdBufState* state, Game::GfxDrawSurf drawSurf)
	{
		auto& rg = *reinterpret_cast<Game::r_globals_t*>(0xCC9D150);
		auto rgp =  reinterpret_cast<Game::r_global_permanent_t*>(0xCC98280);

		Game::switch_material_t mat = { 0 };

		mat.current_material  = rgp->sortedMaterials[(drawSurf.packed >> 29) & 0x7FF];
		mat.current_technique = mat.current_material->techniqueSet->remappedTechniqueSet->techniques[techType];

		mat.material		  = mat.current_material;
		mat.technique		  = mat.current_technique;
		mat.technique_type	  = techType;

		disable_prepass		  = false; // always reset

		if (mat.current_material)
		{
			if (Components::active.DayNightCycle)
			{
				DayNightCycle::overwrite_sky_material(&mat);
			}

			// wireframe xmodels
			if (Dvars::r_wireframe_xmodels && Dvars::r_wireframe_xmodels->current.integer)
			{
				if (Utils::StartsWith(mat.current_material->info.name, "mc/"))
				{
					switch (Dvars::r_wireframe_xmodels->current.integer)
					{
					case 1: // SHADED
						mat.technique_type = Game::TECHNIQUE_WIREFRAME_SHADED;
						mat.switch_technique_type = true;
						break;

					case 2: // SOLID
						mat.technique_type = Game::TECHNIQUE_WIREFRAME_SOLID;
						mat.switch_technique_type = true;
						break;

					case 3: // SHADED_WHITE
						disable_prepass = true;

						mat.technique_type	= Game::TECHNIQUE_UNLIT;
						_Renderer::switch_material(&mat, "iw3xo_showcollision_wire");
						break;

					case 4: // SOLID_WHITE
						mat.technique_type	= Game::TECHNIQUE_UNLIT;
						_Renderer::switch_material(&mat, "iw3xo_showcollision_wire");
						break;
					}
				}
			}

			// wireframe world
			if (Dvars::r_wireframe_world && Dvars::r_wireframe_world->current.integer)
			{
				if (Utils::StartsWith(mat.current_material->info.name, "wc/") && !Utils::StartsWith(mat.current_material->info.name, "wc/sky"))
				{
					switch (Dvars::r_wireframe_world->current.integer)
					{
					case 1: // SHADED
						mat.technique_type = Game::TECHNIQUE_WIREFRAME_SHADED;
						mat.switch_technique_type = true;
						break;

					case 2: // SOLID
						mat.technique_type = Game::TECHNIQUE_WIREFRAME_SOLID;
						mat.switch_technique_type = true;
						break;

					case 3: // SHADED_WHITE
						disable_prepass = true;

						mat.technique_type	= Game::TECHNIQUE_UNLIT;
						_Renderer::switch_material(&mat, "iw3xo_showcollision_wire");
						break;

					case 4: // SOLID_WHITE
						mat.technique_type	= Game::TECHNIQUE_UNLIT;
						_Renderer::switch_material(&mat, "iw3xo_showcollision_wire");
						break;
					}
				}
			}


			// texcoord debugshader
			if (Dvars::r_debugShaderTexcoord && Dvars::r_debugShaderTexcoord->current.enabled)
			{
				if (Utils::StartsWith(mat.current_material->info.name, "mc/"))
				{
					disable_prepass = true;

					mat.technique_type = Game::TECHNIQUE_UNLIT;
					_Renderer::switch_technique(&mat, "debug_texcoords_dtex");
				}
				else if (Utils::StartsWith(mat.current_material->info.name, "wc/"))
				{
					disable_prepass = true;

					mat.technique_type = Game::TECHNIQUE_UNLIT;
					_Renderer::switch_technique(&mat, "debug_texcoords");
				}
			}

			// memes
			//{
			//	if (Utils::Contains(mat.current_material->info.name, "mtl_weapon_acog_reticle"))
			//	{
			//		// float* matrix = Utils::function<float* (Game::GfxCmdBufSourceState* source, int argIndex, char row)>(0x631D90)(source, arg_def->u.codeConst.index, arg_def->u.codeConst.firstRow);
			//		//Utils::function<void(Game::GfxViewInfo* view, float blur)>(0x63BFD0)(Game::_frontEndDataOut->viewInfo, 2.0f);

			//		//mat.technique_type = Game::TECHNIQUE_UNLIT;
			//		_Renderer::switch_technique(&mat, "reticle_blur");

			//		rendered_scope = true;
			//	}
			//}


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
			if (_Renderer::is_valid_technique_for_type(mat.current_material, mat.technique_type))
			{
				_Renderer::switch_technique(&mat, mat.current_material);
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
			if ((mat.technique_type == Game::TECHNIQUE_EMISSIVE || mat.technique_type == Game::TECHNIQUE_UNLIT) && (state->technique->flags & 0x10) != 0 && !src->constVersions[4])
			{
				return 0;
			}
		}

		const auto& r_logFile = Game::Dvar_FindVar("r_logFile");
		if (r_logFile && r_logFile->current.integer && mat.current_material)
		{
			auto string = Utils::VA("R_SetMaterial( %s, %s, %i )\n", state->material->info.name, state->technique->name, mat.technique_type);

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
		state->techType = mat.technique_type;

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
			mov		al, disable_prepass;
			cmp		al, 1;
			pop		eax;

			// jump if true
			je		disable;
			call	R_SetPrepassMaterial_Func;

		disable:
			jmp		rtnPt;
		}
	}

	__declspec(naked) void R_SetMaterial_Emissive_stub()
	{
		const static uint32_t rtn_to_rtn = 0x6490BF;
		__asm
		{
			push	esi;		// techType
			call	R_SetMaterial;
			pop		esi;
			add     esp, 10h;
			jmp		rtn_to_rtn;
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
					if (Components::active.DayNightCycle)
					{
						DayNightCycle::set_pixelshader_constants(state, arg_def);
					}

					if (Components::active.Ocean)
					{
						Ocean::set_pixelshader_constants(state, arg_def);
					}

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

				if (arg_def && arg_def->type == 3)
				{
					if (Components::active.Ocean)
					{
						Ocean::set_vertexshader_constants(state, arg_def);
					}
				}

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
	void MatrixMultiply44(const float(*in1)[4], const float(*in2)[4], float(*out)[4])
	{
		(*out)[0]  = ((((*in1)[0]  * (*in2)[0]) + ((*in1)[1]  * (*in2)[4])) + ((*in1)[2]  * (*in2)[8]))  + ((*in1)[3]  * (*in2)[12]);
		(*out)[1]  = ((((*in1)[0]  * (*in2)[1]) + ((*in1)[1]  * (*in2)[5])) + ((*in1)[2]  * (*in2)[9]))  + ((*in1)[3]  * (*in2)[13]);
		(*out)[2]  = ((((*in1)[0]  * (*in2)[2]) + ((*in1)[1]  * (*in2)[6])) + ((*in1)[2]  * (*in2)[10])) + ((*in1)[3]  * (*in2)[14]);
		(*out)[3]  = ((((*in1)[0]  * (*in2)[3]) + ((*in1)[1]  * (*in2)[7])) + ((*in1)[2]  * (*in2)[11])) + ((*in1)[3]  * (*in2)[15]);
		(*out)[4]  = ((((*in1)[4]  * (*in2)[0]) + ((*in1)[5]  * (*in2)[4])) + ((*in1)[6]  * (*in2)[8]))  + ((*in1)[7]  * (*in2)[12]);
		(*out)[5]  = ((((*in1)[4]  * (*in2)[1]) + ((*in1)[5]  * (*in2)[5])) + ((*in1)[6]  * (*in2)[9]))  + ((*in1)[7]  * (*in2)[13]);
		(*out)[6]  = ((((*in1)[4]  * (*in2)[2]) + ((*in1)[5]  * (*in2)[6])) + ((*in1)[6]  * (*in2)[10])) + ((*in1)[7]  * (*in2)[14]);
		(*out)[7]  = ((((*in1)[4]  * (*in2)[3]) + ((*in1)[5]  * (*in2)[7])) + ((*in1)[6]  * (*in2)[11])) + ((*in1)[7]  * (*in2)[15]);
		(*out)[8]  = ((((*in1)[8]  * (*in2)[0]) + ((*in1)[9]  * (*in2)[4])) + ((*in1)[10] * (*in2)[8]))  + ((*in1)[11] * (*in2)[12]);
		(*out)[9]  = ((((*in1)[8]  * (*in2)[1]) + ((*in1)[9]  * (*in2)[5])) + ((*in1)[10] * (*in2)[9]))  + ((*in1)[11] * (*in2)[13]);
		(*out)[10] = ((((*in1)[8]  * (*in2)[2]) + ((*in1)[9]  * (*in2)[6])) + ((*in1)[10] * (*in2)[10])) + ((*in1)[11] * (*in2)[14]);
		(*out)[11] = ((((*in1)[8]  * (*in2)[3]) + ((*in1)[9]  * (*in2)[7])) + ((*in1)[10] * (*in2)[11])) + ((*in1)[11] * (*in2)[15]);
		(*out)[12] = ((((*in1)[12] * (*in2)[0]) + ((*in1)[13] * (*in2)[4])) + ((*in1)[14] * (*in2)[8]))  + ((*in1)[15] * (*in2)[12]);
		(*out)[13] = ((((*in1)[12] * (*in2)[1]) + ((*in1)[13] * (*in2)[5])) + ((*in1)[14] * (*in2)[9]))  + ((*in1)[15] * (*in2)[13]);
		(*out)[14] = ((((*in1)[12] * (*in2)[2]) + ((*in1)[13] * (*in2)[6])) + ((*in1)[14] * (*in2)[10])) + ((*in1)[15] * (*in2)[14]);
		(*out)[15] = ((((*in1)[12] * (*in2)[3]) + ((*in1)[13] * (*in2)[7])) + ((*in1)[14] * (*in2)[11])) + ((*in1)[15] * (*in2)[15]);
	}

	void InfinitePerspectiveMatrix(const float tanHalfFovX, const float tanHalfFovY, const float zNear, float(*mtx)[4])
	{
		(*mtx)[0]  = 0.99951172f / tanHalfFovX;
		(*mtx)[5]  = 0.99951172f / tanHalfFovY;
		(*mtx)[10] = 0.99951172f;
		(*mtx)[11] = 1.0f;
		(*mtx)[14] = 0.99951171875f * -zNear;
	}

	void MatrixForViewer(const float* origin, const float(*axis)[3], float(*mtx)[4])
	{
		// [0][0] to [3][0]
		(*mtx)[0]  = -(*axis)[3];
		(*mtx)[4]  = -(*axis)[4];
		(*mtx)[8]  = -(*axis)[5];
		(*mtx)[12] = -((*origin * (*mtx)[0]) + (origin[1] * (*mtx)[4]) + (origin[2] * (*mtx)[8]));
		
		// [0][1] to [3][1]
		(*mtx)[1]  = (*axis)[6];
		(*mtx)[5]  = (*axis)[7];
		(*mtx)[9]  = (*axis)[8];
		(*mtx)[13] = -((*origin * (*mtx)[1]) + (origin[1] * (*mtx)[5]) + (origin[2] * (*mtx)[9]));

		// [0][2] to [3][2]
		(*mtx)[2]  = (*axis)[0];
		(*mtx)[6]  = (*axis)[1];
		(*mtx)[10] = (*axis)[2];
		(*mtx)[14] = -((*origin * (*mtx)[2]) + (origin[1] * (*mtx)[6]) + (origin[2] * (*mtx)[10]));

		// [0][3] to [3][3]
		(*mtx)[3]  = 0.0f;
		(*mtx)[7]  = 0.0f;
		(*mtx)[11] = 0.0f;
		(*mtx)[15] = 1.0f;
	}
#pragma warning(pop)
	
	// rewrite of CG_GetViewFov()
	float calculate_worldfov_with_zoom(float fov_val)
	{
		float calc_fov;

		const auto& cg_fovScale = Game::Dvar_FindVar("cg_fovScale");
		const auto& cg_fovMin = Game::Dvar_FindVar("cg_fovMin");

		unsigned int offhand_index = Game::cgs->predictedPlayerState.offHandIndex;
		if ((Game::cgs->predictedPlayerState.weapFlags & 2) == 0)
		{
			offhand_index = Game::cgs->predictedPlayerState.weapon;
		}
		
		Game::WeaponDef* weapon = Game::BG_WeaponNames[offhand_index];
		if (Game::cgs->predictedPlayerState.pm_type == 5)
		{
			calc_fov = 90.0f;
			goto LABEL_16;
		}
		
		calc_fov = fov_val;
		if (weapon->aimDownSight)
		{
			if (Game::cgs->predictedPlayerState.fWeaponPosFrac == 1.0f)
			{
				calc_fov = weapon->fAdsZoomFov;
				goto LABEL_16;
			}
			
			if (0.0f != Game::cgs->predictedPlayerState.fWeaponPosFrac)
			{
				float ads_factor;
				
				if (Game::cgs->playerEntity.bPositionToADS)
				{
					const float w_pos_frac = Game::cgs->predictedPlayerState.fWeaponPosFrac - (1.0f - weapon->fAdsZoomInFrac);
					if (w_pos_frac <= 0.0f)
					{
						goto LABEL_16;
					}
					ads_factor = w_pos_frac / weapon->fAdsZoomInFrac;
				}
				else
				{
					const float w_pos_frac = Game::cgs->predictedPlayerState.fWeaponPosFrac - (1.0f - weapon->fAdsZoomOutFrac);
					if (w_pos_frac <= 0.0f)
					{
						goto LABEL_16;
					}
					ads_factor = w_pos_frac / weapon->fAdsZoomOutFrac;
				}
				
				if (ads_factor > 0.0f)
				{
					calc_fov = calc_fov - ads_factor * (calc_fov - weapon->fAdsZoomFov);
				}
			}
		}
		
	LABEL_16:
		if ((Game::cgs->predictedPlayerState.eFlags & 0x300) != 0)
		{
			calc_fov = 55.0f;
		}

		float calc_fov_out = cg_fovScale->current.value * calc_fov;

		if (cg_fovMin->current.value - calc_fov_out >= 0.0f)
		{
			calc_fov_out = cg_fovMin->current.value;
		}
		
		return calc_fov_out;
	}

	void set_worldfov(Game::GfxViewParms* viewParms)
	{
		if(Dvars::cg_fov_world_tweaks && Dvars::cg_fov_world_tweaks->current.enabled)
		{
			// calc world fov + weapon zoom
			const float world_fov = calculate_worldfov_with_zoom(Dvars::cg_fov_world->current.value);
			const float w_fov = 0.75f * tanf(world_fov * 0.01745329238474369f * 0.5f);

			const float tanHalfX = ((float)Game::cgs->refdef.width / (float)Game::cgs->refdef.height) * w_fov;
			const float tanHalfY = w_fov;

			// calc viewmatrix
			float view_mtx[4][4] = { 0.0f };
			MatrixForViewer(viewParms->origin, viewParms->axis, view_mtx);

			// calc projection matrix
			float proj_mtx[4][4] = { 0.0f };
			InfinitePerspectiveMatrix(tanHalfX, tanHalfY, viewParms->zNear, proj_mtx);

			// calc viewprojection matrix
			float viewproj_mtx[4][4] = { 0.0f };
			MatrixMultiply44(view_mtx, proj_mtx, viewproj_mtx);

			// only overwrite the projection matrix ;)
			memcpy(viewParms->viewProjectionMatrix.m, viewproj_mtx, sizeof(Game::GfxMatrix));
		}
	}
	
	__declspec(naked) void R_SetViewParmsForScene_stub()
	{
		const static uint32_t retn_pt = 0x5FAA0B;
		__asm
		{
			pushad;
			push	edi; // viewParms
			call	set_worldfov;
			add		esp, 4;
			popad;
			
			// stock op's
			lea     ecx, [edi + 0C0h];
			jmp		retn_pt;
		}
	}


	// memes
	//void dosth(Game::GfxViewInfo* view)
	//{
	//	if(rendered_scope)
	//	{
	//		Utils::function<void(Game::GfxViewInfo*, float)>(0x63BFD0)(view, 16.0f);
	//		rendered_scope = false;
	//	}
	//}

	//__declspec(naked) void do_sth_stub()
	//{
	//	const static uint32_t R_DrawEmissive_Func = 0x64AEF0;
	//	const static uint32_t retn_pt = 0x64B377;
	//	__asm
	//	{
	//		pushad;
	//		push	edi; // viewParms
	//		call	dosth;
	//		add		esp, 4;
	//		popad;
	//		
	//		call	R_DrawEmissive_Func;
	//		//add     esp, 10h;

	//		jmp		retn_pt;
	//	}
	//}

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
		//Utils::Hook::Nop(0x60B028, 6); Utils::Hook(0x60B028, r_cull_world_stub_01, HOOK_JUMP).install()->quick(); // crashes on release

		// R_AddAabbTreeSurfacesInFrustum_r :: disable all surface culling (bad fps)
		// 0x643B08 -> nop //Utils::Hook::Nop(0x643B08, 6);
		//Utils::Hook(0x643B03, r_cull_world_stub_02, HOOK_JUMP).install()->quick();

		// 0x643B39 -> jmp ^ // 0x74 -> 0xEB //Utils::Hook::Set<BYTE>(0x643B39, 0xEB);
		//Utils::Hook(0x643B34, r_cull_world_stub_03, HOOK_JUMP).install()->quick();

		// R_AddCellSceneEntSurfacesInFrustumCmd :: active ents like destructible cars / players (disable all culling)
		// 0x64D17A -> nop // 2 bytes //Utils::Hook::Nop(0x64D17A, 2);
		//Utils::Hook::Nop(0x64D172, 8); Utils::Hook(0x64D172, r_cull_entities_stub, HOOK_JUMP).install()->quick();

		// R_AddWorkerCmd :: disable dynEnt models
		// 0x629328 -> nop
		//Utils::Hook(0x629328, r_draw_dynents_stub, HOOK_JUMP).install()->quick(); // popad makes it worse


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
			/* desc		*/ "Draw world objects using their wireframe technique",
			/* default	*/ 0,
			/* enumSize	*/ r_wireframe_enum.size(),
			/* enumData */ r_wireframe_enum.data(),
			/* flags	*/ Game::dvar_flags::none);

		Dvars::r_wireframe_xmodels = Game::Dvar_RegisterEnum(
			/* name		*/ "r_wireframe_xmodels",
			/* desc		*/ "Draw xmodels using their wireframe technique",
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
		Utils::Hook(0x6490B7, R_SetMaterial_Emissive_stub, HOOK_JUMP).install()->quick();


		// custom pixelshader code constants
		Utils::Hook::Nop(0x64BEDB, 7);
		Utils::Hook(0x64BEDB, R_SetPassPixelShaderStableArguments_stub, HOOK_JUMP).install()->quick();

		// custom vertexshader code constants / per object :: R_SetPassShaderObjectArguments
		Utils::Hook::Nop(0x64BD22, 7);
		Utils::Hook(0x64BD22, R_SetVertexShaderConstantFromCode_stub, HOOK_JUMP).install()->quick();

		// seperate world and viewmodel fov
		Utils::Hook::Nop(0x5FAA05, 6);
		Utils::Hook(0x5FAA05, R_SetViewParmsForScene_stub, HOOK_JUMP).install()->quick();

		Dvars::cg_fov_world_tweaks = Game::Dvar_RegisterBool(
			/* name		*/ "cg_fov_world_tweaks",
			/* desc		*/ "Enable world fov tweaks",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);
		
		Dvars::cg_fov_world = Game::Dvar_RegisterFloat(
			/* name		*/ "cg_fov_world",
			/* desc		*/ "Adjust world fov separately (wont effect viewmodel fov)",
			/* default	*/ 65.0f,
			/* minVal	*/ 20.0f,
			/* maxVal	*/ 160.0f,
			/* flags	*/ Game::dvar_flags::saved);

		// memes
		//Utils::Hook(0x64B372, do_sth_stub, HOOK_JUMP).install()->quick();

		// increase fps cap to 125 for menus and loadscreen
		Utils::Hook::Set<BYTE>(0x500174 + 2, 8);
		Utils::Hook::Set<BYTE>(0x500177 + 2, 8);
	}

	_Renderer::~_Renderer()
	{ }
}