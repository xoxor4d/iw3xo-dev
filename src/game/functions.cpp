#include "std_include.hpp"

namespace game
{
	namespace glob
	{
		// general
		std::string loaded_modules;
		std::string loaded_libaries;

		// radiant
		game::cgs_addon_s cgs_addons = {};
		game::saved_radiant_brushes_s radiant_saved_brushes = {};
		game::dynBrushesArray_t	dynamic_brushes	= {};
		game::dynBrushModelsArray_t dynamic_brush_models = {};

		// movement
		bool lpmove_check_jump = false;
		glm::vec3 lpmove_velocity = {};
		glm::vec3 lpmove_origin	= {};
		glm::vec3 lpmove_angles	= {};
		glm::vec3 lpmove_camera_origin = {};

		// ui / devgui
		bool loaded_main_menu = false;
		bool mainmenu_fade_done = false;
		bool mainmenu_snd_played = false;

		game::gui_t gui = {};
		std::string changelog_html_body;

		// renderer
		IDirect3DDevice9* d3d9_device = nullptr;

		// debug collision
		bool debug_collision_initialized = false; 
		int  debug_collision_rendered_brush_amount = 0;
		int  debug_collision_rendered_planes_amount	= 0;
		int  debug_collision_rendered_planes_counter = 0; 
		
		std::string	r_drawCollision_materialList_string;

		// frametime
		int lpmove_server_time = 0;
		int lpmove_server_time_old = 0;
		int lpmove_server_frame_time = 0;
		int lpmove_pml_frame_time = 0;

		// misc
		int q3_last_projectile_weapon_used = 0; // ENUM Q3WeaponNames :: this var holds the last proj. weapon that got fired

#ifdef DEVGUI_XO_BLUR
		float xo_blur_directions = 32.0f;
		float xo_blur_quality = 16.0f;
		float xo_blur_size = 32.0f;
		float xo_blur_alpha = 1.0f;
#endif
	}

	game::TestLod g_testLods[4] = {};

	// *
	// general structs

	game::clientActive_t*		clients = reinterpret_cast<game::clientActive_t*>(0xC5F930);
	game::clientStatic_t*		cls = reinterpret_cast<game::clientStatic_t*>(0x956D80);
	game::clientConnection_t&	clc = *reinterpret_cast<game::clientConnection_t*>(0x8F4CE0);
	game::cg_s*					cgs = reinterpret_cast<game::cg_s*>(0x74E338);
	game::GfxBuffers*			gfx_buf = reinterpret_cast<game::GfxBuffers*>(0xD2B0840);
	game::GfxScene*				scene = reinterpret_cast<game::GfxScene*>(0xCF10280);
	//Game::serverStatic_t* svs = reinterpret_cast<Game::serverStatic_t*>(0x185C480); // cba
	game::clipMap_t*			cm = reinterpret_cast<game::clipMap_t*>(0x14098C0);
	game::ComWorld*				com = reinterpret_cast<game::ComWorld*>(0x1435CB8);
	game::GfxWorld*				gfx_world = reinterpret_cast<game::GfxWorld*>(0xD0701E0);

	game::DObj_s* objBuf = reinterpret_cast<game::DObj_s*>(0x1477F30);
	std::uint16_t* clientObjMap = reinterpret_cast<std::uint16_t*>(0x14A9F30);
	game::centity_s* cg_entitiesArray = reinterpret_cast<game::centity_s*>(0x84F2D8);
	game::weaponInfo_s* cg_weaponsArray = reinterpret_cast<game::weaponInfo_s*>(0x748658);

	int* com_frameTime = reinterpret_cast<int*>(0x1476EFC);
	float* com_timescaleValue = reinterpret_cast<float*>(0x1435D68);


	// *
	// radiant / cgame

	const char* g_entityBeginParsePoint	= reinterpret_cast<const char*>(0x1113674);
	const char* g_entityEndParsePoint	= reinterpret_cast<const char*>(0x1113678);
	int*		clientActive_cmdNumber	= reinterpret_cast<int*>(0xCC5FF8); // part of clientActive_t
	
	char* Com_Parse(const char **data_p /*edi*/)
	{
		const static uint32_t Com_Parse_func = 0x571380;
		__asm
		{
			mov     edi, data_p;
			Call	Com_Parse_func;
		}
	}

	bool CL_GetUserCmd(int cmdNumber /*eax*/, game::usercmd_s *ucmd)
	{
		const static uint32_t CL_GetUserCmd_func = 0x45AB40;
		__asm
		{
			mov     eax, cmdNumber;
			push	ucmd;

			Call	CL_GetUserCmd_func;
			add     esp, 4;
		}
	}


	// *
	// filesystem

	HANDLE database_handle = reinterpret_cast<HANDLE>(0x14E89A4);

	const char** zone_code_post_gfx_mp				= reinterpret_cast<const char**>(0xCC9D128);
	const char** zone_localized_code_post_gfx_mp	= reinterpret_cast<const char**>(0xCC9D134);
	const char** zone_ui_mp							= reinterpret_cast<const char**>(0xCC9D12C);
	const char** zone_common_mp						= reinterpret_cast<const char**>(0xCC9D130);
	const char** zone_localized_common_mp			= reinterpret_cast<const char**>(0xCC9D138);
	const char** zone_mod							= reinterpret_cast<const char**>(0xCC9D13C);

	XAssetHeader* DB_XAssetPool = reinterpret_cast<XAssetHeader*>(0x7265E0);
	unsigned int* g_poolSize	= reinterpret_cast<unsigned int*>(0x7263A0);

	DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers = reinterpret_cast<DB_GetXAssetSizeHandler_t*>(0x726A10);

	XAssetHeader db_realloc_xasset_pool(XAssetType type, unsigned int new_size)
	{
		const XAssetHeader pool_entry = 
		{ 
			utils::memory::get_allocator()->allocate(new_size * DB_GetXAssetSizeHandlers[type]())
		};

		DB_XAssetPool[type] = pool_entry;
		g_poolSize[type] = new_size;

		return pool_entry;
	}

	bool DB_FileExists(const char* file_name, game::DB_FILE_EXISTS_PATH source)
	{
		const static uint32_t DB_FileExists_func = 0x48B9B0;
		__asm
		{
			push	source;
			mov		eax, file_name;

			Call	DB_FileExists_func;
			add     esp, 4h;
		}
	}

	void FS_DisplayPath(int bLanguageCull /*eax*/)
	{
		const static uint32_t FS_DisplayPath_func = 0x55D510;
		__asm
		{
			mov		eax, bLanguageCull;
			Call	FS_DisplayPath_func;
		}
	}

	const char* fs_gamedir = reinterpret_cast<const char*>(0xCB19898);
	game::searchpath_s* fs_searchpaths = reinterpret_cast<game::searchpath_s*>(0xD5EC4DC);

	char** Sys_ListFiles(const char* filter /*eax*/, const char* directory, const char* extension, int* num_files)
	{
		const static uint32_t Sys_ListFiles_func = 0x572F00;
		__asm
		{
			push	0;				// wantsubs
			push	num_files;
			push[extension];
			push	directory;
			xor eax, eax;		// filter

			call	Sys_ListFiles_func;
			add     esp, 10h;
		}
	}

	int SEH_GetLanguageIndexForName(const char* psz_language_name /*edi*/, int* pi_language_index)
	{
		const static uint32_t SEH_GetLanguageIndexForName_func = 0x539250;
		__asm
		{
			lea		ecx, [pi_language_index];
			push	ecx;						// langIndex
			mov		edi, [psz_language_name];	// langName

			call	SEH_GetLanguageIndexForName_func;
			add     esp, 4;
		}
	}

	int unzClose(const char* file /*edi*/)
	{
		const static uint32_t unzClose_func = 0x596A50;
		__asm
		{
			mov		edi, [file];
			call	unzClose_func;
		}
	}


	// *
	// debug collision 


	// *
	// renderer

	game::MaterialLoadGlob* mtlLoadGlob = reinterpret_cast<game::MaterialLoadGlob*>(0xD541330);
	game::GfxRenderTarget* gfxRenderTargets = reinterpret_cast<game::GfxRenderTarget*>(0xD573EB0);

	float& v_aspectRatioWindow = *reinterpret_cast<float*>(0xCC9D0E4);
	float& v_postProcessAspect = *reinterpret_cast<float*>(0xCC9D0FC);

	IDirect3DDevice9** dx9_device_ptr = reinterpret_cast<IDirect3DDevice9**>(0xCC9A408);
	DxGlobals* dx = reinterpret_cast<DxGlobals*>(0xCC9A400);

	game::Material*			floatz_display = reinterpret_cast<game::Material*>(0xFA5378);
	GfxCmdBufSourceState*	gfxCmdBufSourceState = reinterpret_cast<GfxCmdBufSourceState*>(0xD53F5F0);
	game::GfxCmdBufState*	gfxCmdBufState = reinterpret_cast<game::GfxCmdBufState*>(0xD5404F0);

	game::materialCommands_t* tess = reinterpret_cast<game::materialCommands_t*>(0xD085EE0);
	game::GfxBackEndData* _frontEndDataOut = reinterpret_cast<game::GfxBackEndData*>(0xCC9827C);
	//game::GfxBackEndData* _backEndData = reinterpret_cast<game::GfxBackEndData*>(0xD0704BC);

	game::GfxBackEndData* get_backenddata()
	{
		const auto out = reinterpret_cast<game::GfxBackEndData*>(*game::backEndDataOut_ptr);
		return out;
	}

	game::r_globals_t* rg = reinterpret_cast<game::r_globals_t*>(0xCC9D150);
	game::r_global_permanent_t* rgp = reinterpret_cast<game::r_global_permanent_t*>(0xCC98280);

	game::clientDebugLineInfo_t* clientDebugLineInfo_client = reinterpret_cast<game::clientDebugLineInfo_t*>(0xC5B054);
	game::clientDebugLineInfo_t* clientDebugLineInfo_server = reinterpret_cast<game::clientDebugLineInfo_t*>(0xC5B074);

	RB_EndTessSurface_t RB_EndTessSurface = (RB_EndTessSurface_t)0x61A2F0;

	const char* get_rendertarget_string(game::GfxRenderTargetId id)
	{
		switch (id)
		{
			case R_RENDERTARGET_SAVED_SCREEN: return "R_RENDERTARGET_SAVED_SCREEN";
			case R_RENDERTARGET_FRAME_BUFFER: return "R_RENDERTARGET_FRAME_BUFFER";
			case R_RENDERTARGET_SCENE: return "R_RENDERTARGET_SCENE";
			case R_RENDERTARGET_RESOLVED_POST_SUN: return "R_RENDERTARGET_RESOLVED_POST_SUN";
			case R_RENDERTARGET_RESOLVED_SCENE: return "R_RENDERTARGET_RESOLVED_SCENE";
			case R_RENDERTARGET_FLOAT_Z: return "R_RENDERTARGET_FLOAT_Z";
			case R_RENDERTARGET_DYNAMICSHADOWS: return "R_RENDERTARGET_DYNAMICSHADOWS";
			case R_RENDERTARGET_PINGPONG_0: return "R_RENDERTARGET_PINGPONG_0";
			case R_RENDERTARGET_PINGPONG_1: return "R_RENDERTARGET_PINGPONG_1";
			case R_RENDERTARGET_SHADOWCOOKIE: return "R_RENDERTARGET_SHADOWCOOKIE";
			case R_RENDERTARGET_SHADOWCOOKIE_BLUR: return "R_RENDERTARGET_SHADOWCOOKIE_BLUR";
			case R_RENDERTARGET_POST_EFFECT_0: return "R_RENDERTARGET_POST_EFFECT_0";
			case R_RENDERTARGET_POST_EFFECT_1: return "R_RENDERTARGET_POST_EFFECT_1";
			case R_RENDERTARGET_SHADOWMAP_SUN: return "R_RENDERTARGET_SHADOWMAP_SUN";
			case R_RENDERTARGET_SHADOWMAP_SPOT: return "R_RENDERTARGET_SHADOWMAP_SPOT";
			case R_RENDERTARGET_COUNT: return "R_RENDERTARGET_COUNT";
			case R_RENDERTARGET_NONE: return "R_RENDERTARGET_NONE";
			default: return "R_RENDERTARGET_UNKOWN";
		}
	}

	game::MaterialTechnique* RB_BeginSurface(game::MaterialTechniqueType techType, game::Material* material)
	{
		const static uint32_t RB_BeginSurface_func = 0x61A220;
		__asm
		{
			pushad;
			mov		edi, techType;
			mov		esi, material;
			call	RB_BeginSurface_func;
			popad;
		}
	}

	game::MaterialTechnique* RB_BeginSurface_CustomMaterial(game::MaterialTechniqueType techType, game::Material* material)
	{
		const static uint32_t RB_BeginSurface_func = 0x61A220;
		__asm
		{
			pushad;
			mov		edi, techType;
			mov		esi, material;
			call	RB_BeginSurface_func;
			popad;
		}
	}

	void draw_text_with_engine(float x, float y, float scale_x, float scale_y, const char* font, const float* color, const char* text)
	{
		void* fontHandle = R_RegisterFont(font, sizeof(font));
		R_AddCmdDrawTextASM(text, 0x7FFFFFFF, fontHandle, x, y, scale_x, scale_y, 0.0f, color, 0);
	}

	// wrapper not correct ?!
	void R_AddCmdDrawTextASM(const char* text, int max_chars, void* font, float x, float y, float x_scale, float y_scale, float rotation, const float* color, int style)
	{
		const static uint32_t R_AddCmdDrawText_func = 0x5F6B00;
		__asm
		{
			push	style;
			sub     esp, 14h;

			fld		rotation;
			fstp	[esp + 10h];

			fld		y_scale;
			fstp	[esp + 0Ch];

			fld		x_scale;
			fstp	[esp + 8];

			fld		y;
			fstp	[esp + 4];

			fld		x;
			fstp	[esp];

			push	font;
			push	max_chars;
			push	text;
			mov		ecx, [color];

			call	R_AddCmdDrawText_func;
			add		esp, 24h;
		}
	}

	void RB_DrawTextInSpace(const float* pixel_step_x /*eax*/, const float* pixel_step_y /*esi*/, const char* text, game::Font_s* font, const float* org, char* color_bytes)
	{
		const static uint32_t RB_DrawTextInSpace_func = 0x614D80;
		__asm
		{
			push	color_bytes;
			push	org;
			push	font;
			push	text;
			mov		esi, [pixel_step_y];
			mov		eax, [pixel_step_x];

			call	RB_DrawTextInSpace_func;
			add		esp, 10h;
		}
	}

	void RB_StandardDrawCommands(game::GfxViewInfo* view_info)
	{
		const static uint32_t RB_StandardDrawCommands_func = 0x64AFB0;
		__asm
		{
			mov		eax, view_info;
			call	RB_StandardDrawCommands_func;
		}
	}

	void R_AddCmdDrawStretchPic(game::Material* material, float x, float y, float w, float h, float null1, float null2, float null3, float null4, float* color)
	{
		const static uint32_t R_AddCmdDrawStretchPic_func = 0x5F65F0;
		__asm
		{
			pushad;
			push	color;
			mov		eax, [material];
			sub		esp, 20h;

			fld		null4;
			fstp	[esp + 1Ch];

			fld		null3;
			fstp	[esp + 18h];

			fld		null2;
			fstp	[esp + 14h];

			fld		null1;
			fstp	[esp + 10h];

			fld		h;
			fstp	[esp + 0Ch];

			fld		w;
			fstp	[esp + 8h];

			fld		y;
			fstp	[esp + 4h];

			fld		x;
			fstp	[esp];

			call	R_AddCmdDrawStretchPic_func;
			add		esp, 24h;
			popad;
		}
	}

	void R_SetRenderTarget(int target)
	{
		const static uint32_t R_SetRenderTarget_func = 0x632B60;
		const static uint32_t _gfxCmdBufSourceState  = 0xD53F5F0; // State
		const static uint32_t _gfxCmdBufSourceSource = 0xD5404F0; // Source // should be switched? using Struct "GfxCmdBufState" on 0xD5404F0 works

		__asm
		{
			pushad;
			push	_gfxCmdBufSourceSource; // eax
			push	_gfxCmdBufSourceState; // ecx
			mov     eax, target;

			call	R_SetRenderTarget_func;
			add     esp, 8;
			popad;
		}
	}

	void R_Set2D()
	{
		const static uint32_t R_Set2D_func = 0x6336E0;
		const static uint32_t _gfxCmdBufSourceState = 0xD53F5F0;

		__asm
		{
			pushad;
			mov		edi, [_gfxCmdBufSourceState];
			call	R_Set2D_func;
			popad;
		}
	}

	void R_Set3D()
	{
		const static uint32_t R_Set3D_func = 0x6337C0;
		//const static uint32_t _gfxCmdBufSourceState = 0xD53F5F0;

		__asm
		{
			pushad;
			mov		edx, game::gfxCmdBufSourceState;
			//mov		edx		offset game::gfxCmdBufSourceState;
			call	R_Set3D_func;
			popad;
		}
	}

	void RB_DrawStretchPic(game::Material* material, float x, float y, float w, float h, float texcoord0, float texcoord1, float texcoord2, float texcoord3 /*-1 pushed*/)
	{
		const static uint32_t RB_DrawStretchPic_func = 0x610E10;
		__asm
		{
			pushad;
			mov		eax, material;
			push    0FFFFFFFFh; // -1
			sub     esp, 20h;

			fld		texcoord3;
			fstp	[esp + 1Ch];

			fld		texcoord2;
			fstp	[esp + 18h];

			fld		texcoord1;
			fstp	[esp + 14h];

			fld		texcoord0;
			fstp	[esp + 10h];

			fld		h;
			fstp	[esp + 0Ch];

			fld		w;
			fstp	[esp + 8h];

			fld		y;
			fstp	[esp + 4h];

			fld		x;
			fstp	[esp];

			call	RB_DrawStretchPic_func;
			add     esp, 24h;
			popad;
		}
	}

	void CG_DrawRotatedPicPhysical(ScreenPlacement* place, float a2, float a3, float a4, float a5, float a6, float* color, void* material)
	{
		const static uint32_t CG_DrawRotatedPicPhysical_func = 0x431490;
		__asm
		{
			pushad;
			push	[material];
			push	[color];
			sub     esp, 14h;

			fld		a6;
			fstp	[esp + 10h];

			mov     edx, place;

			fld		a5;
			fstp	[esp + 0Ch];

			fld		a4;
			fstp	[esp + 8h];

			fld		a3;
			fstp	[esp + 4h];

			fld		a2;
			fstp	[esp];

			call	CG_DrawRotatedPicPhysical_func;
			add     esp, 1Ch;
			popad;
		}
	}

	int R_TextWidth(const char* text /*<eax*/, int maxChars, game::Font_s* font)
	{
		const static uint32_t R_TextWidth_func = 0x5F1EE0;
		__asm
		{
			push	font;
			push	maxChars;
			mov		eax, [text];
			call	R_TextWidth_func;
			add		esp, 8;
		}
	}


	// *
	// ui

	DWORD*	ui_white_material_ptr = reinterpret_cast<DWORD*>(0xCAF06F0);

	int*	gameTypeEnum = reinterpret_cast<int*>(0xCAF1820);
	int*	mapNameEnum = reinterpret_cast<int*>(0xCAF2330);

	game::UiContext* ui_cg_dc = reinterpret_cast<game::UiContext*>(0x746FA8);
	game::UiContext* ui_context = reinterpret_cast<game::UiContext*>(0xCAEE200);

	game::WinMouseVars_t* s_wmv = reinterpret_cast<game::WinMouseVars_t*>(0xCC147C4);

	game::PlayerKeyState* playerKeys = reinterpret_cast<game::PlayerKeyState*>(0x8F1DB8);
	game::clientUIActive_t* clientUI = reinterpret_cast<game::clientUIActive_t*>(0xC5F8F4);

	ScreenPlacement* scrPlace = reinterpret_cast<ScreenPlacement*>(0xE34420);
	ScreenPlacement* scrPlaceFull = reinterpret_cast<ScreenPlacement*>(0xE343D8);

	int String_Parse(const char** p /*eax*/, char* out_str, int len)
	{
		const static uint32_t String_Parse_func = 0x54B510;
		__asm
		{
			push	len;
			lea		eax, [out_str];
			push	eax;
			mov		eax, p;
			call	String_Parse_func;
			add		esp, 8;
		}
	}

	void Menus_OpenByName(const char* menu_name, game::UiContext* ui_dc)
	{
		const static uint32_t Menus_OpenByName_func = 0x550B50;
		__asm
		{
			pushad;
			mov		esi, ui_dc;
			mov		edi, menu_name;
			call	Menus_OpenByName_func;
			popad;
		}
	}

	void Menus_CloseByName(const char* menu_name, game::UiContext* ui_dc)
	{
		const static uint32_t Menus_CloseByName_func = 0x54C520;
		__asm
		{
			pushad;
			mov		eax, menu_name;
			mov		esi, ui_dc;
			call	Menus_CloseByName_func;
			popad;
		}
	}

	void Menus_CloseAll(game::UiContext* ui_dc)
	{
		const static uint32_t Menus_CloseAll_func = 0x54C540;
		__asm
		{
			pushad;
			mov		esi, ui_dc;
			Call	Menus_CloseAll_func;
			popad;
		}
	}
	
	
	// *
	// gsc  

	DWORD*	gScrMemTreePub					= reinterpret_cast<DWORD*>(0x14E8A04);
	DWORD*	scrVarPub /*char[1024]*/		= reinterpret_cast<DWORD*>(0x15CA61C);
	int*	scrVarPub_p4 /*scrVarpub+4*/	= reinterpret_cast<int*>(0x15CA620);
	char*	error_message /*char[1023]*/	= reinterpret_cast<char*>(0x1798378);
	char*	errortype /*char[1023]*/		= reinterpret_cast<char*>(0x1798777);
	int*	scr_numParam					= reinterpret_cast<int*>(0x1794074);
	scr_const_t* scr_const					= reinterpret_cast<scr_const_t*>(0x1406E90);

	game::gentity_s*		scr_g_entities	= reinterpret_cast<game::gentity_s*>(0x1288500);
	game::level_locals_t*	level_locals	= reinterpret_cast<game::level_locals_t*>(0x13EB6A8);

	void G_SetOrigin(game::gentity_s* ent, const float* origin)
	{
		if (ent)
		{
			ent->s.lerp.pos.trBase[0]	= origin[0];
			ent->s.lerp.pos.trBase[1]	= origin[1];
			ent->s.lerp.pos.trBase[2]	= origin[2];
			ent->s.lerp.pos.trType		= game::trType_t::TR_STATIONARY;
			ent->s.lerp.pos.trTime		= 0;
			ent->s.lerp.pos.trDuration	= 0;
			ent->s.lerp.pos.trDelta[0]	= 0.0f;
			ent->s.lerp.pos.trDelta[1]	= 0.0f;
			ent->s.lerp.pos.trDelta[2]	= 0.0f;
			ent->r.currentOrigin[0]		= origin[0];
			ent->r.currentOrigin[1]		= origin[1];
			ent->r.currentOrigin[2]		= origin[2];
		}
	}

	void G_SetAngles(game::gentity_s* ent, const float *angles)
	{
		if (ent)
		{
			ent->s.lerp.apos.trBase[0]	= angles[0];
			ent->s.lerp.apos.trBase[1]	= angles[1];
			ent->s.lerp.apos.trBase[2]	= angles[2];
			ent->s.lerp.apos.trType		= game::trType_t::TR_STATIONARY;
			ent->s.lerp.apos.trTime		= 0;
			ent->s.lerp.apos.trDuration = 0;
			ent->s.lerp.apos.trDelta[0] = 0.0f;
			ent->s.lerp.apos.trDelta[1] = 0.0f;
			ent->s.lerp.apos.trDelta[2] = 0.0f;
			ent->r.currentAngles[0]		= angles[0];
			ent->r.currentAngles[1]		= angles[1];
			ent->r.currentAngles[2]		= angles[2];
		}
	}

	std::int16_t G_ModelIndex(const char* model_name /*eax*/)
	{
		const static uint32_t G_ModelIndex_func = 0x4E21F0;
		__asm
		{
			mov		eax, model_name;
			call	G_ModelIndex_func;
		}
	}

	void SV_LinkEntity(game::gentity_s* ent /*edi*/)
	{
		const static uint32_t SV_LinkEntity_func = 0x536D80;
		__asm
		{
			mov		edi, ent;
			call	SV_LinkEntity_func;
		}
	}

	void Scr_ObjectError(const char *string /*eax*/)
	{
		const static uint32_t Scr_ObjectError_func = 0x523F90;
		__asm
		{
			mov		eax, string;
			call	Scr_ObjectError_func;
		}
	}

	void Scr_GetVector(unsigned int arg_index /*eax*/, float* float_out /*edx*/ )
	{
		const static uint32_t Scr_GetVector_func = 0x5236E0;
		__asm
		{
			mov		edx, [float_out];
			mov		eax, arg_index;
			xor		eax, eax;

			call	Scr_GetVector_func;
		}
	}

	// :<
	void Scr_AddBool(bool value)
	{
		auto some_scr_counter = reinterpret_cast<int*>(0x1794070);
		auto scr_script_inst = reinterpret_cast<DWORD*>(0x1794068);
		auto scr_script_inst_ptr = reinterpret_cast<bool**>(0x1794068);

		// Scr_ClearOutParams
		utils::function<void()>(0x51D260)();

		if (*scr_script_inst == *(DWORD*)0x179405C)
		{
			game::Com_Error(0, "Internal script stack overflow");
		}

		++ *some_scr_counter;

		uintptr_t v0 = *scr_script_inst + 8;
		*scr_script_inst = v0;

		*(DWORD*)(v0 + 4) = 6;
		**scr_script_inst_ptr = value;
	}

	void Scr_AddVector(float* float_out /*esi*/)
	{
		const static uint32_t Scr_AddVector_func = 0x523D10;
		__asm
		{
			mov		esi, [float_out];
			call	Scr_AddVector_func;
		}
	}

	float Scr_GetFloat(unsigned int arg_index /*eax*/)
	{
		const static uint32_t Scr_GetFloat_func = 0x523360;
		__asm
		{
			mov		eax, arg_index;
			xor		eax, eax;
			call	Scr_GetFloat_func;
		}
	}

	int GetTagPos(std::uint16_t tag, game::centity_s* ent, float* origin_out)
	{
		const static uint32_t func_addr = 0x4024B0;
		int return_val = 0;

		__asm
		{
			push	origin_out;
			movzx	esi, tag;
			mov		ecx, ent;
			movzx	eax, byte ptr[ecx + 4];
			call	func_addr;
			add		esp, 4;
			mov		return_val, eax;
		}

		return return_val;
	}

	int DObjGetBoneIndex(DObj_s* obj /*ecx*/, int tag_name, BYTE* bone_index)
	{
		const static uint32_t func_addr = 0x57F2B0;
		int result = 0;
		__asm
		{
			push	bone_index;
			push	tag_name;
			mov		ecx, obj;
			call	func_addr;
			add		esp, 8;
			mov		result, eax;
		}

		return result;
	}

	int CG_GetBoneIndex(int local_client_num /*eax*/, int tag_name /*edx*/, int nextstate_num, char* bone)
	{
		const static uint32_t func_addr = 0x435A50;
		int result = 0;
		__asm
		{
			push	bone;
			push	nextstate_num;
			mov		edx, tag_name;
			mov		eax, local_client_num;
			call	func_addr;
			add		esp, 8;
			mov		result, eax;
		}

		return result;
	}

	int CG_DObjGetWorldBoneMatrix(cpose_t* pose /*eax*/, int bone_index /*ecx*/, float* axis /*esi*/, DObj_s* obj, float* origin)
	{
		const static uint32_t func_addr = 0x433F00;
		int result = 0;
		__asm
		{
			push	origin;
			push	obj;
			mov		esi, axis;
			mov		ecx, bone_index;
			mov		eax, pose;
			call	func_addr;
			add		esp, 8;
			mov		result, eax;
		}

		return result;
	}

	int is_button_pressed(int button, int button_data) 
	{
		int tmp, i = 0;
		int pwrs[20] = {}; // Never going to need more than 20

		if (button_data < button) 
		{
			return 0;
		}

		if (button_data == button) 
		{
			return 1;
		}

		while (button_data != 0) 
		{
			tmp = 1;
			while (tmp * 2 <= button_data) 
			{
				tmp *= 2;
			}

			button_data -= tmp;
			pwrs[i] = tmp;
			i++;
		}

		for (tmp = 0; tmp < i; tmp++) 
		{
			if (pwrs[tmp] == button)
			{
				return 1;
			}
		}

		return 0;
	}

	
	// *
	// filesystem

	

	
	// *
	// movement

	game::WeaponDef** BG_WeaponNames = reinterpret_cast<game::WeaponDef**>(0x736DB8);

	game::gentity_s* g_entities = reinterpret_cast<game::gentity_s*>(0x12885C4);
	game::gclient_s* g_clients = reinterpret_cast<game::gclient_s*>(0x13255A8);

	bool Jump_Check(game::pmove_t* pm /*eax*/, game::pml_t* pml)
	{
		const static uint32_t Jump_Check_func = 0x407D90;
		__asm
		{
			push	pml;
			mov		eax, pm;
			call	Jump_Check_func;
			add     esp, 4h;
		}
	}

	void PM_Friction(game::playerState_s* ps, game::pml_t* pml)
	{
		const static uint32_t PM_Friction_func = 0x40E860;
		__asm
		{
			pushad;
			push	pml;
			mov		esi, ps;
			call	PM_Friction_func;
			add     esp, 4h;
			popad;
		}
	}

	bool PM_CorrectAllSolid(game::pmove_t* pm /*eax*/, game::pml_t* pml, game::trace_t* trace)
	{
		const static uint32_t PM_CorrectAllSolid_func = 0x410370;
		__asm
		{
			push    trace;
			push	pml;
			mov		eax, pm;
			call	PM_CorrectAllSolid_func;
			add		esp, 8h;
		}
	}

	void PM_GroundTraceMissed(game::pmove_t* pm /*eax*/, game::pml_t* pml)
	{
		const static uint32_t PM_GroundTraceMissed_func = 0x4104E0;
		__asm
		{
			push	pml;
			mov		eax, pm;
			call	PM_GroundTraceMissed_func;
			add		esp, 4h;
		}
	}

	void PM_CrashLand(game::playerState_s* ps /*esi*/, game::pml_t* pml) 
	{
		const static uint32_t PM_CrashLand_func = 0x40FFB0;
		__asm
		{
			push	pml;
			mov		esi, ps;
			call	PM_CrashLand_func;
			add		esp, 4h;
		}
	}

	void PM_AddTouchEnt(game::pmove_t* pm /*eax*/, int entity_num /*edi*/)
	{
		const static uint32_t PM_AddTouchEnt_func = 0x40E270;
		__asm
		{
			movzx	edi, entity_num;
			mov		eax, pm;
			call	PM_AddTouchEnt_func;
		}
	}

	void PM_playerTrace(game::pmove_t* pm /*esi*/, game::trace_t* results, const float* start, const float* mins, const float* maxs, const float* end, std::int32_t pass_entity_num, std::int32_t content_mask)
	{
		const static uint32_t PM_playerTrace_func = 0x40E160;
		__asm
		{
			push	content_mask;
			push	pass_entity_num;
			push	[end];
			push	[maxs];
			push	[mins];
			push	[start];
			push	results;
			mov		esi, pm;

			call	PM_playerTrace_func;
			add     esp, 1Ch;
		}
	}

	void G_Damage(float *dir /*eax*/, game::gentity_s *targ, game::gentity_s *inflictor, game::gentity_s *attacker, float *point, int damage, int flags, int _mod, int self_client, int hitloc, /*hitLocation_t*/ int a11, unsigned int a12, unsigned int time_offset)
	{
		const static uint32_t G_Damage_func = 0x4B5560;
		__asm
		{
			push	time_offset;
			push	a12;
			push	a11;
			push	hitloc;
			push	self_client;
			push	_mod;
			push	flags;
			push	[damage];
			push	[point];
			push	attacker;
			push	inflictor;
			push	targ;
			mov		eax, [dir];

			call	G_Damage_func;
			add     esp, 30h;
		}
	}

	float CanDamage(game::gentity_s *inflictor /*eax*/, float *center_pos /*ecx*/, game::gentity_s *ent, float cone_angle_cos, float *cone_direction, int contentmask)
	{
		const static uint32_t CanDamage_func = 0x4B5770;
		__asm
		{
			push	contentmask;
			push	[cone_direction];
			push	0; // ??

			fld		cone_angle_cos;
			fstp	[esp];

			push	ent;
			mov		ecx, [center_pos];
			mov		eax, inflictor;

			call	CanDamage_func;
			add     esp, 10h;
		}
	}
	
	void Scr_PlayerDamage(float* dir, game::gentity_s* targ, game::gentity_s* inflictor, game::gentity_s* attacker, int damage, int dflags, int _mod, int weapon, float* point, game::hitLocation_t hit_loc, int time_offset)
	{
		const static uint32_t Scr_PlayerDamage_func = 0x4D8B80;
		__asm
		{
			pushad;

			push	time_offset;
			push	hit_loc;
			push	[point];
			push	weapon;
			push	_mod;
			push	dflags;
			push	damage;
			push	attacker;
			push	inflictor;
			push	targ;
			mov		eax, [dir];

			call	Scr_PlayerDamage_func;
			add		esp, 28h;
			popad;
		}
	}
	
	game::gentity_s* G_FireRocket(float* kick_back, game::gentity_s* ent, signed int weap_index, float* dir, float* gun_vel, game::gentity_s* target, float* target_offset)
	{
		const static uint32_t G_FireRocket_func = 0x4C7C30;
		__asm
		{
			push	target_offset;
			push	target;
			push	gun_vel;
			push	dir;
			push	weap_index;
			push	ent;
			mov		eax, [kick_back];

			call	G_FireRocket_func;
			add     esp, 18h;
		}
	}
	

	// *
	// dvars 

	void dvar_set_value_dirty(dvar_s* dvar, int value)
	{
		if (!dvar)
		{
			return;
		}

		dvar->current.integer = value;
		dvar->latched.integer = value;
		//_dvar->modified = false;
	}

	void dvar_set_value_dirty(dvar_s* dvar, bool value)
	{
		if (!dvar)
		{
			return;
		}

		dvar->current.enabled = value;
		dvar->latched.enabled = value;
		//_dvar->modified = false;
	}

	void dvar_set_value_dirty(dvar_s* dvar, const float dvar_value)
	{
		if (!dvar)
		{
			return;
		}

		dvar->current.value = dvar_value;
		dvar->latched.value = dvar_value;
		//_dvar->modified = false;
	}

	void dvar_set_value_dirty(dvar_s* dvar, const char *dvar_value)
	{
		if (!dvar)
		{
			return;
		}

		dvar->current.string = dvar_value;
		dvar->latched.string = dvar_value;
		dvar->modified = false;
	}

	void dvar_set_value_dirty(dvar_s* dvar, const float* dvar_value, int size)
	{
		if (!dvar)
		{
			return;
		}

		for (auto i = 0; i < size && i < 4; i++)
		{
			dvar->current.vector[i] = dvar_value[i];
			dvar->latched.vector[i] = dvar_value[i];
		}
	}

	// Registering StringDvars is a pain, so heres a workaround
	game::dvar_s* Dvar_RegisterString_hacky(const char* dvar_name, const char* dvar_value, const char* description)
	{
		// register external dvar
		game::Cmd_ExecuteSingleCommand(0, 0, utils::va("set %s %s\n", dvar_name, dvar_value));

		// find the dvar
		game::dvar_s* string_dvar = game::Dvar_FindVar(dvar_name);

		if (!string_dvar)
		{
			return nullptr;
		}

		// description will be "External Dvar" if dvar was created successfully 
		if (string_dvar->description == "External Dvar"s)
		{
			// success
			string_dvar->description = description;
			string_dvar->flags = game::dvar_flags::none;

			return string_dvar;
		}

		return nullptr;
	}

	const char* Dvar_EnumToString(const dvar_s* dvar)
	{
		auto result = "";

		if (dvar->domain.enumeration.stringCount) 
		{
			result = *(const char **)(dvar->domain.integer.max + 4 * dvar->current.integer);
		}

		return result;
	}

	void Dvar_SetString(const char* text /*eax*/, dvar_s* dvar /*esi*/)
	{
		const static uint32_t Dvar_SetString_func = 0x56CA90;
		__asm
		{
			pushad;
			mov		eax, [text];
			mov		esi, [dvar];
			call	Dvar_SetString_func;
			popad;
		}
	}

	__declspec(naked) dvar_s* Dvar_FindVar(const char* /*dvar*/)
	{
		__asm
		{
			push	eax;
			pushad;

			mov		edi, [esp + 28h];
			mov		eax, 0x56B5D0;
			call	eax;

			mov		[esp + 20h], eax;
			popad;

			pop eax;
			retn;
		}
	}

	void Dvar_ReregisterInt(dvar_s* dvar /*eax*/, std::uint32_t flags /*edi*/, const char* dvar_name, DvarType dvar_type, const char* description, int x, int y, int z, int w, int min, int max)
	{
		const static uint32_t Dvar_Reregister_func = 0x56BFF0;
		__asm
		{
			pushad;
			push	max;
			push	min;
			push	w;
			push	z;
			push	y;
			push	x;
			push	description;
			push	dvar_type;
			push	dvar_name;

			mov		edi, flags;
			mov		eax, [dvar];

			call	Dvar_Reregister_func;
			add		esp, 24h;
			popad;
		}
	}
	
	game::dvar_s* Dvar_RegisterIntWrapper_r(const char* dvar_name, DvarType type, std::uint16_t flags, const char* description, int x, int y, int z, int w, int min, int max)
	{
		const auto dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return game::Dvar_RegisterNew(dvar_name, type, flags, description, x, y, z, w, min, max);
		}

		Dvar_ReregisterInt(dvar, flags, dvar_name, type, description, x, y, z, w, min, max);

		return dvar;
	}

	// *
	// console 

	float* con_matchtxtColor_currentDvar = reinterpret_cast<float*>(0x6BDF14);
	float* con_matchtxtColor_currentValue = reinterpret_cast<float*>(0x6BDF24);
	float* con_matchtxtColor_defaultValue = reinterpret_cast<float*>(0x6BDF34);
	float* con_matchtxtColor_dvarDescription = reinterpret_cast<float*>(0x6BDF54);
	float* con_matchtxtColor_domainDescription = reinterpret_cast<float*>(0x6BDF44);

	// console structs
	ConDrawInputGlob*	conDrawInputGlob = reinterpret_cast<ConDrawInputGlob*>(0x8CC2C8);
	Console*			con = reinterpret_cast<Console*>(0x8DC8C0);
	field_t*			g_consoleField = reinterpret_cast<field_t*>(0x8F1B88);

	// console variables
	bool*	Key_IsCatcherActive = reinterpret_cast<bool*>(0xC5F8F8);

	// con->screenMin / Max manual defines for use in asm
	float*	conScreenMin0 = reinterpret_cast<float*>(0x8ECB14); // left
	float*	conScreenMin1 = reinterpret_cast<float*>(0x8ECB18); // top
	float*	conScreenMax0 = reinterpret_cast<float*>(0x8ECB1C); // right
	float*	conScreenMax1 = reinterpret_cast<float*>(0x8ECB20); // bottom

	bool*	extvar_con_ignoreMatchPrefixOnly = reinterpret_cast<bool*>(0x736BB1);

	cmd_function_s* cmd_functions = reinterpret_cast<cmd_function_s*>(0x1410B3C);

	SCR_DrawSmallStringExt_t	SCR_DrawSmallStringExt = (SCR_DrawSmallStringExt_t)0x474C30;
	Sys_IsMainThread_t			Sys_IsMainThread = (Sys_IsMainThread_t)0x50B5D0;
	Con_TokenizeInput_t			Con_TokenizeInput = (Con_TokenizeInput_t)0x45F350;
	Con_CancelAutoComplete_t	Con_CancelAutoComplete = (Con_CancelAutoComplete_t)0x460A90;
	Con_DrawInputPrompt_t		Con_DrawInputPrompt = (Con_DrawInputPrompt_t)0x460510;
	Cmd_EndTokenizedString_t	Cmd_EndTokenizedString = (Cmd_EndTokenizedString_t)0x4F98C0;

	ConDrawInput_IncrMatchCounter_t		ConDrawInput_IncrMatchCounter = (ConDrawInput_IncrMatchCounter_t)0x45FA40;
	Con_AnySpaceAfterCommand_t			Con_AnySpaceAfterCommand = (Con_AnySpaceAfterCommand_t)0x45F3D0;
	ConDrawInput_DetailedDvarMatch_t	ConDrawInput_DetailedDvarMatch = (ConDrawInput_DetailedDvarMatch_t)0x45FFB0;
	ConDrawInput_DetailedCmdMatch_t		ConDrawInput_DetailedCmdMatch = (ConDrawInput_DetailedCmdMatch_t)0x460370;

	ConDrawInput_DvarMatch_t	ConDrawInput_DvarMatch = (ConDrawInput_DvarMatch_t)0x45FAB0;
	ConDrawInput_CmdMatch_t		ConDrawInput_CmdMatch = (ConDrawInput_CmdMatch_t)0x460440;
	Con_DrawOutputScrollBar_t	Con_DrawOutputScrollBar = (Con_DrawOutputScrollBar_t)0x461860;
	Con_DrawOutputText_t		Con_DrawOutputText = (Con_DrawOutputText_t)0x4619E0;

	void Con_DrawMessageWindowOldToNew(DWORD* msg_window /*esi*/, int local_client_num, int x_pos, int y_pos, int char_height, int horz_align, int vert_align, int mode, Font_s* font, const float* color, int text_style, float msgwnd_scale, int text_align_mode)
	{
		const static uint32_t Con_DrawMessageWindowOldToNew_func = 0x461150;
		__asm
		{
			push	text_align_mode;
			push	0;

			fld		msgwnd_scale;
			fstp	dword ptr[esp];

			push	text_style;
			push	color;
			push	font;
			push	mode;
			push	vert_align;
			push	horz_align;
			push	char_height;
			push	y_pos;
			push	x_pos;
			push	local_client_num;
			mov		esi, msg_window;

			call	Con_DrawMessageWindowOldToNew_func;
			add     esp, 30h;
		}
	}

	void Cmd_ForEachXO(void(__cdecl* callback)(const char*))
	{
		// skip the first cmd (nullptr)?
		for (auto cmd = cmd_functions->next; cmd; cmd = cmd->next)
		{
			if (cmd->name)
			{
				callback(cmd->name);
			}
		}
	}

	void Cmd_ForEach_PassCmd(void(__cdecl* callback)(cmd_function_s*))
	{
		// skip the first cmd (nullptr)?
		for (auto cmd = cmd_functions->next; cmd; cmd = cmd->next)
		{
			if (cmd->name)
			{
				callback(cmd);
			}
		}
	}

	void AddBaseDrawConsoleTextCmd(int char_count /*eax*/, const float *color_float /*ecx*/, const char *text_pool, int pool_size, int first_char, game::Font_s *font, float x, float y, float x_scale, float y_scale, int style)
	{
		const static uint32_t AddBaseDrawConsoleTextCmd_func = 0x5F6F60;
		__asm
		{
			push    style;
			sub     esp, 10h;

			fld		[y_scale];
			fstp	[esp + 0Ch];

			fld		[x_scale];
			fstp	[esp + 8];

			fld		[y];
			fstp	[esp + 4];

			fld		[x];
			fstp	[esp];

			push    font;
			push    first_char;
			push    pool_size;
			push    text_pool;
			mov     ecx, [color_float];
			mov     eax, [char_count];

			call	AddBaseDrawConsoleTextCmd_func;
			add     esp, 24h;
		}
	}

	void ConDrawInput_Box(float* color, int line_height_multi)
	{
		const static uint32_t ConDrawInput_Box_func = 0x45F700;
		__asm
		{
			pushad;
			mov		esi, [color];
			push	line_height_multi;
			call	ConDrawInput_Box_func;
			add		esp, 4h;
			popad;
		}
	}
	
	void ConDrawInput_TextAndOver(char* text)
	{
		const static uint32_t ConDrawInput_TextAndOver_func = 0x45F500;
		__asm
		{
			pushad;
			mov		esi, [text];
			call	ConDrawInput_TextAndOver_func;
			popad;
		}
	}

	void ConDraw_Box(float* color, float x, float y, float width, float height)
	{
		const static uint32_t ConDraw_Box_func = 0x45F540;
		__asm
		{
			pushad;
			mov		esi, [color];
			sub		esp, 10h;

			fld		height;
			fstp	[esp + 0Ch];

			fld		width;
			fstp	[esp + 8h];

			fld		y;
			fstp	[esp + 4h];

			fld		x;
			fstp	[esp];

			call	ConDraw_Box_func;
			add		esp, 10h;
			popad;
		}
	}

	void Con_DrawAutoCompleteChoice(int is_dvar_cmd, char* text)
	{
		const static uint32_t Con_DrawAutoCompleteChoice_func = 0x460490;
		__asm
		{
			pushad;
			mov		eax, is_dvar_cmd;
			push	text;

			call	Con_DrawAutoCompleteChoice_func;
			add		esp, 4h;
			popad;
		}
	}

	char* Cmd_Argv(int arg_index)
	{
		const static uint32_t Cmd_Argv_func = 0x42A950;
		__asm
		{
			mov		eax, arg_index;
			call	Cmd_Argv_func;
		}
	}

	void Dvar_ForEachName(void(__cdecl* func)(const char *))
	{
		const static uint32_t Dvar_ForEachName_func = 0x569D30;
		__asm
		{
			pushad;
			mov		edi, [func];
			call	Dvar_ForEachName_func;
			popad;
		}
	}

	void Cmd_ForEach(void(__cdecl *func)(const char *))
	{
		const static uint32_t Cmd_ForEach_Func = 0x4F9A40;
		__asm
		{
			pushad;
			mov		edi, [func];
			call	Cmd_ForEach_Func;
			popad;
		}
	}

	
	// *
	// anims

	// prob. broken af
	void BG_AnimScriptEvent(scriptAnimEventTypes_t script_event, game::playerState_s* ps, int force)
	{
		const static uint32_t BG_AnimScriptEvent_func = 0x405720;
		__asm
		{
			pushad;
			mov     edi, [ps];
			push    force;
			mov     eax, script_event;

			call	BG_AnimScriptEvent_func;
			add		esp, 4h;
			popad;
		}
	}


	// *
	// common

	game::playerState_s*	ps_loc = reinterpret_cast<game::playerState_s*>(0x13255A8);
	game::pmove_t*			pmove = reinterpret_cast<game::pmove_t*>(0x8C9C90);

	game::CmdArgs* cmd_args = reinterpret_cast<game::CmdArgs*>(0x1410B40);
	cmd_function_s** cmd_ptr = reinterpret_cast<cmd_function_s**>(0x1410B3C);

	XZone*			g_zones = reinterpret_cast<XZone*>(0xFFEFD0);
	XAssetEntry*	g_assetEntryPool = reinterpret_cast<XAssetEntry*>(0xF0D640);
	unsigned short* db_hashTable = reinterpret_cast<unsigned short*>(0xE62A80);
	infoParm_t*		infoParams = reinterpret_cast<game::infoParm_t*>(0x71FBD0); // Count 0x1C

	Cmd_ExecuteSingleCommand_t	Cmd_ExecuteSingleCommand = Cmd_ExecuteSingleCommand_t(0x4F9AB0);
	Com_Error_t					Com_Error = Com_Error_t(0x4FD330);
	Com_PrintMessage_t			Com_PrintMessage = Com_PrintMessage_t(0x4FCA50);
	DB_FindXAssetHeader_t		DB_FindXAssetHeader = DB_FindXAssetHeader_t(0x489570);
	DB_EnumXAssets_FastFile_t	DB_EnumXAssets_FastFile = DB_EnumXAssets_FastFile_t(0x489120);
	DB_GetXAssetNameHandler_t*	DB_GetXAssetNameHandlers = reinterpret_cast<DB_GetXAssetNameHandler_t*>(0x7268D0);
    DB_LoadXAssets_t			DB_LoadXAssets = DB_LoadXAssets_t(0x48A2B0);

	void Cbuf_AddText(const char *text /*eax*/, int local_client_num /*ecx*/)
	{
		const static uint32_t Cbuf_AddText_func = 0x4F8D90;
		__asm
		{
			mov		ecx, local_client_num;
			mov		eax, text;
			call	Cbuf_AddText_func;
		}
	}

	void Cmd_AddCommand(const char* name, void(*callback)(), cmd_function_s* data, char)
	{
		data->name = name;
		data->args = nullptr;		 //data->autoComplargseteExt;
		data->description = nullptr; //data->autoCompleteDir;
		data->function = callback;
		data->next = *cmd_ptr;

		*cmd_ptr = data;
	}

	void Cmd_AddCommand(const char* name, const char* args, const char* description, void(*callback)(), cmd_function_s* data, char)
	{
		data->name = name;
		data->args = args;
		data->description = description;
		data->function = callback;
		data->next = *cmd_ptr;

		*cmd_ptr = data;
	}

	const char* SL_ConvertToString(int idx)
	{
		struct stringList
		{
			int unk;
			char string[1];
			int unk2;
		};

		return (*reinterpret_cast<stringList**>(0x14E8A04))[idx & 0xFFFF].string;
	}

	__declspec(naked) game::PackedUnitVec Vec3PackUnitVec(const float*)
	{
		const static uint32_t func_addr = 0x5645A0;
		__asm
		{
			mov		eax, [esp + 4h];
			call	func_addr;
			retn;

			//push	0x5645A0;
			//retn;
		}
	}

	void Vec3UnpackUnitVec(unsigned int packed, float* texcoord_out /*ecx*/)
	{
		const static uint32_t func_addr = 0x5647D0;
		__asm
		{
			mov		ecx, texcoord_out;
			push	packed;
			call	func_addr;
			add		esp, 4;
		}
	}

	void Vec2UnpackTexCoords(unsigned int packed, float* texcoord_out /*ecx*/)
	{
		const static uint32_t func_addr = 0x5648E0;
		__asm
		{
			mov		ecx, texcoord_out;
			push	packed;
			call	func_addr;
			add		esp, 4;
		}
	}


	void Byte4UnpackRgba(unsigned __int8* from, float* to)
	{
		if (from && to)
		{
			to[0] = static_cast<float>((double)from[0] * 0.003921568859368563);
			to[1] = static_cast<float>((double)from[1] * 0.003921568859368563);
			to[2] = static_cast<float>((double)from[2] * 0.003921568859368563);
			to[3] = static_cast<float>((double)from[3] * 0.003921568859368563);
		}
	}

#pragma warning( push )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4309 )
	char Byte1PackClamp(const float from)
	{
		char val;

		if ((255.0 * from + 9.313225746154785e-10) < 255)
		{
			val = 255 * from + 9.313225746154785e-10;
		}
		else
		{
			val = 255;
		}

		if (val > 0)
		{
			return val;
		}

		return 0;
	}
#pragma warning( pop ) 

	void Byte4PackRgba(const float* from, char* to)
	{
		to[0] = Byte1PackClamp(from[0]);
		to[1] = Byte1PackClamp(from[1]);
		to[2] = Byte1PackClamp(from[2]);
		to[3] = Byte1PackClamp(from[3]);
	}

	void Byte4PackPixelColor(const float* from, char* to)
	{
		to[2] = Byte1PackClamp(from[0]);
		to[1] = Byte1PackClamp(from[1]);
		to[0] = Byte1PackClamp(from[2]);
		to[3] = Byte1PackClamp(from[3]);
	}

	void AxisToAngles(float* angles /*eax*/, const float(*axis)[3] /*ecx*/)
	{
		const static uint32_t func_addr = 0x561B50;
		__asm
		{
			pushad;
			mov     eax, angles;
			mov     ecx, axis;
			call	func_addr;
			popad;
		}
	}

	// *
	// cgaz - Port from mDd client Proxymod (https://github.com/Jelvan1/cgame_proxymod)

	// Adjusted for resolution and screen aspect ratio
	void CG_AdjustFrom640(float* x, float* y, float* w, float* h)
	{
		assert(x);
		assert(y);
		assert(w);
		assert(h);

		const float screen_x_scale = static_cast<float>(game::cgs->refdef.width) / 640.0f;

		// scale for screen sizes
		*x *= screen_x_scale;
		*y *= screen_x_scale; // Note that screenXScale is used to avoid widescreen stretching.
		*w *= screen_x_scale;
		*h *= screen_x_scale; // Note that screenXScale is used to avoid widescreen stretching.
	}

	// Coordinates are 640*480 virtual values
	void CG_FillRect(float x, float y, float w, float h, float color[4])
	{
		if (!w || !h)
		{
			return;
		}

		CG_AdjustFrom640(&x, &y, &w, &h);

		const auto material = Material_RegisterHandle("white", 3);
		game::R_AddCmdDrawStretchPic(material, x, y, w, h, 0.0f, 0.0f, 0.0f, 0.0f, color);
	}

	static bool AngleInFov(float angle)
	{
		float const half_fov_x = atanf(game::cgs->refdef.tanHalfFovX);
		return angle > -half_fov_x && angle < half_fov_x;
	}

	static float AngleScreenProjection(float angle)
	{
		float const half_fov_x = atanf(game::cgs->refdef.tanHalfFovX);

		if (angle >= half_fov_x)
		{
			return 0;
		}
			
		if (angle <= -half_fov_x)
		{
			return SCREEN_WIDTH;
		}
			
		return SCREEN_WIDTH / 2 * (1 - tanf(angle) / tanf(half_fov_x));
	}

	typedef struct
	{
		float	x1;
		float	x2;
		bool	split;
	} range_t;

	static range_t AnglesToRange(float start, float end, float yaw)
	{
		if (fabsf(end - start) > 2 * M_PI)
		{
			range_t const ret = { 0, SCREEN_WIDTH, false };
			return ret;
		}

		bool split = end > start;

		start	= utils::vector::angle_normalize_pi(start - yaw);
		end		= utils::vector::angle_normalize_pi(end - yaw);

		if (end > start)
		{
			split = !split;
			float const tmp = start;

			start	= end;
			end		= tmp;
		}

		range_t const ret = { AngleScreenProjection(start), AngleScreenProjection(end), split };
		return ret;
	}

	void CG_FillAngleYaw(float start, float end, float yaw, float y, float h, float color[4])
	{
		range_t const range = AnglesToRange(start, end, yaw);
		if (!range.split)
		{
			CG_FillRect(range.x1, y, range.x2 - range.x1, h, color);
		}
		else
		{
			CG_FillRect(0, y, range.x1, h, color);
			CG_FillRect(range.x2, y, SCREEN_WIDTH - range.x2, h, color);
		}
	}

	void CG_DrawLineYaw(float angle, float yaw, float y, float w, float h, float color[4])
	{
		angle = utils::vector::angle_normalize_pi(angle - yaw);
		if (!AngleInFov(angle))
		{
			return;
		}

		float const x = AngleScreenProjection(angle);
		CG_FillRect(x - w / 2, y, w, h, color);
	}
}
