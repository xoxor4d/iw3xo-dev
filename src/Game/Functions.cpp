#include "STDInclude.hpp"

namespace Game
{
	namespace Globals
	{
		// INIT
		std::string loadedModules;
		bool loaded_MainMenu;

		// RADIANT ----------------------------------------------------------------
		Game::cgsAddon cgsAddons = Game::cgsAddon();
		Game::savedRadiantBrushes rad_savedBrushes = Game::savedRadiantBrushes();
		Game::dynBrushesArray_t dynBrushes = Game::dynBrushesArray_t();
		Game::dynBrushModelsArray_t dynBrushModels = Game::dynBrushModelsArray_t();

		// MOVEMENT ---------------------------------------------------------------
		bool locPmove_checkJump = false; // if Jumped in Check_Jump, reset after x frames in PmoveSingle

		glm::vec3 locPmove_playerVelocity = { 0.0f, 0.0f, 0.0f };	// grab local player velocity
		glm::vec3 locPmove_playerOrigin = { 0.0f, 0.0f, 0.0f };		// grab local player origin
		glm::vec3 locPmove_playerAngles = { 0.0f, 0.0f, 0.0f };		// grab local player angles
		glm::vec3 locPmove_cameraOrigin = { 0.0f, 0.0f, 0.0f };		// grab local camera origin

		// COLLISION ---------------------------------------------------------------
		bool dbgColl_initialized = false;	// debug collision was used
		int  dbgColl_drawnBrushAmount = 0;	// total amount of brushes used for calculations of planes 
		int  dbgColl_drawnPlanesAmount = 0; // total amount of planes rendered for hud
		int  dbgColl_drawnPlanesAmountTemp = 0; // total amount of planes rendered used to count while drawing 

		// FAMETIME ---------------------------------------------------------------
		int serverTime = 0;
		int serverTimeOld = 0;
		int serverFrameTime = 0;
		int pmlFrameTime = 0;

		// MISC ---------------------------------------------------------------
		int Q3_LastProjectileWeaponUsed = 0; // ENUM Q3WeaponNames :: this var holds the last proj. weapon that got fired
	}

	float COLOR_WHITE[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float COLOR_BLACK[4] = { 0.0f, 0.0f, 0.0f, 1.0f };


	// ---------------
	// RADIANT / CGAME

	const char* g_entityBeginParsePoint		= reinterpret_cast<const char*>(0x1113674);
	const char* g_entityEndParsePoint		= reinterpret_cast<const char*>(0x1113678);
	int* clientActive_cmdNumber				= reinterpret_cast<int*>(0xCC5FF8); // part of clientActive_t
	Game::cg_s *cgs					= reinterpret_cast<Game::cg_s*>(0x74E338);

	char* Com_Parse(const char **data_p /*edi*/)
	{
		const static uint32_t Com_Parse_Func = 0x571380;
		__asm
		{
			mov     edi, data_p
			Call	Com_Parse_Func
		}
	}

	bool CL_GetUserCmd(int cmdNumber /*eax*/, Game::usercmd_s *ucmd)
	{
		const static uint32_t CL_GetUserCmd_Func = 0x45AB40;
		__asm
		{
			mov     eax, cmdNumber
			push	ucmd

			Call	CL_GetUserCmd_Func
			add     esp, 4
		}
	}

	// ---------
	// FASTFILES

	extern HANDLE dbHandle							= reinterpret_cast<HANDLE>(0x14E89A4);

	const char** zone_code_post_gfx_mp				= reinterpret_cast<const char**>(0xCC9D128);
	const char** zone_localized_code_post_gfx_mp	= reinterpret_cast<const char**>(0xCC9D134);
	const char** zone_ui_mp							= reinterpret_cast<const char**>(0xCC9D12C);
	const char** zone_common_mp						= reinterpret_cast<const char**>(0xCC9D130);
	const char** zone_localized_common_mp			= reinterpret_cast<const char**>(0xCC9D138);
	const char** zone_mod							= reinterpret_cast<const char**>(0xCC9D13C);

	XAssetHeader* DB_XAssetPool = reinterpret_cast<XAssetHeader*>(0x7265E0);
	unsigned int* g_poolSize = reinterpret_cast<unsigned int*>(0x7263A0);

	DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers = reinterpret_cast<DB_GetXAssetSizeHandler_t*>(0x726A10);

	XAssetHeader DB_ReallocXAssetPool(XAssetType type, unsigned int newSize)
	{
		int elSize = DB_GetXAssetSizeHandlers[type]();
		XAssetHeader poolEntry = { Utils::Memory::GetAllocator()->allocate(newSize * elSize) };
		DB_XAssetPool[type] = poolEntry;
		g_poolSize[type] = newSize;
		return poolEntry;
	}

	bool DB_FileExists(const char* fileName, Game::DB_FILE_EXISTS_PATH source)
	{
		const static uint32_t DB_FileExists_Func = 0x48B9B0;
		__asm
		{
			push	source
			mov		eax, fileName

			Call	DB_FileExists_Func
			add     esp, 4h
		}
	}


	// ---------
	// COLLISION 

	int* vertexCount = reinterpret_cast<int*>(0xD2B082C);
	void* frontEndDataOut = reinterpret_cast<void*>(0xCC9827C);

	int* polyVertColor_Minus_0x4 = reinterpret_cast<int*>(0xD085EEC);
	int* polyVertColor = reinterpret_cast<int*>(0xD085EF0);
	int* polyVertColor_Plus_0xC = reinterpret_cast<int*>(0xD085EFC);
	char* indices = reinterpret_cast<char*>(0xD0B0820);
	int* indices_plus_0x2 = reinterpret_cast<int*>(0xD0B0822);
	int* indices_plus_0x4 = reinterpret_cast<int*>(0xD0B0824);
	char* initStringDvarValue = reinterpret_cast<char*>(0x6BFEA7);

	Game::clipMap_t* cm = reinterpret_cast<Game::clipMap_t*>(0x14098C0);
	Game::ComWorld* com = reinterpret_cast<Game::ComWorld*>(0x1435CB8);

	Game::MaterialTechniqueType* OverflowTessTech = reinterpret_cast<Game::MaterialTechniqueType*>(0xD540EFC);
	Game::Material* OverflowTessSurf = reinterpret_cast<Game::Material*>(0xD540EF8);
	Game::Material* builtIn_material_unlit = reinterpret_cast<Game::Material*>(0xCC9A2C4); // 0xCC9A2B4
	Game::Material* builtIn_material_unlit_depth = reinterpret_cast<Game::Material*>(0xCC9A2C0);

	Game::materialCommands_t* tess = reinterpret_cast<Game::materialCommands_t*>(0xD085EE0);
	
	Game::DebugGlobals* debugGlob = reinterpret_cast<Game::DebugGlobals*>((((char*)frontEndDataOut) + 0x11E71C));
	Game::GfxBackEndData *_frontEndDataOut = reinterpret_cast<Game::GfxBackEndData *>(0xCC9827C);
	Game::GfxBackEndData *_backEndData = reinterpret_cast<Game::GfxBackEndData *>(0xD0704BC);
	Game::GfxWorld * _gfxWorld = reinterpret_cast<Game::GfxWorld *>(0xD0701E0);

	// print3d // DebugStrings / Lines
	int* clsDebugFromServer = reinterpret_cast<int*>(0xC5B020);
	bool* ifRendererStarted = reinterpret_cast<bool*>(0x956E88);
	Game::clientStatic_t *cls = reinterpret_cast<Game::clientStatic_t *>(0x956D80);
	Game::clientDebugStringInfo_t *clsDebugSV_Strings = reinterpret_cast<Game::clientDebugStringInfo_t*>(0xC5B044);
	Game::clientDebugStringInfo_t *clsDebugCL_Strings = reinterpret_cast<Game::clientDebugStringInfo_t*>(0xC5B024);

	void R_AddDebugPolygon(int pointCount, const float(*points)[3])
	{
		const static uint32_t R_AddDebugPolygon_Func = 0x60DAC0;
		__asm
		{
			mov		esi, debugGlob
			push	points
			push	pointCount

			Call	R_AddDebugPolygon_Func
			add     esp, 8
		}
	}

	void R_AddDebugPolygonNew(Game::DebugGlobals* debugGlobalsEntry, int pointCount, const float(*points)[3])
	{
		const static uint32_t R_AddDebugPolygon_Func = 0x60DAC0;
		__asm
		{
			mov		esi, debugGlobalsEntry
			push	points
			push	pointCount

			Call	R_AddDebugPolygon_Func
			add     esp, 8
		}
	}

	
	// --------
	// RENDERER

	Game::MaterialLoadGlob* mtlLoadGlob = reinterpret_cast<Game::MaterialLoadGlob*>(0xD541330);

	bool* gfxRenderTargets = reinterpret_cast<bool*>(0xD573F18);
	bool* tessSurface = reinterpret_cast<bool*>(0xD2B0828);
	int* tessSurfaceInt = reinterpret_cast<int*>(0xD2B0828);

	int* RenderTargetWidth = reinterpret_cast<int*>(0xD573EBC); //D573EBC
	int* RenderTargetHeight = reinterpret_cast<int*>(0xD573EC0); // D573EC0
	int* RenderTargetArray = reinterpret_cast<int*>(0xD573EB0); // D573EC0

	int* wnd_SceneHeight = reinterpret_cast<int*>(0xCC9D0E4); // CC9D0E4
	float* wnd_SceneAspect = reinterpret_cast<float*>(0xCC9D0FC); // CC9D0FC

	//const char** code_textures_string_array

	Game::Material* floatz_display = reinterpret_cast<Game::Material*>(0xFA5378);
	GfxCmdBufSourceState* gfxCmdBufSourceState = reinterpret_cast<GfxCmdBufSourceState*>(0xD53F5F0);
	Game::clientDebugLineInfo_t* clientDebugLineInfo_client = reinterpret_cast<Game::clientDebugLineInfo_t*>(0xC5B054);
	Game::clientDebugLineInfo_t* clientDebugLineInfo_server = reinterpret_cast<Game::clientDebugLineInfo_t*>(0xC5B074);

	// get handle using DB_FindXAssetHeader
	R_RegisterFont_t R_RegisterFont = (R_RegisterFont_t)0x5F1EC0;
	Material_RegisterHandle_t Material_RegisterHandle = (Material_RegisterHandle_t)0x5F2A80;

	RB_EndTessSurface_t RB_EndTessSurface = (RB_EndTessSurface_t)0x61A2F0;

	const char* RendertargetStringFromID(Game::GfxRenderTargetId id)
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

	void DrawTextWithEngine(float x, float y, float scaleX, float scaleY, char* font, const float *color, const char* text)
	{
		void* fontHandle = R_RegisterFont(font, sizeof(font));
		R_AddCmdDrawTextASM(text, 0x7FFFFFFF, fontHandle, x, y, scaleX, scaleY, 0.0f, color, 0);
	}
	
	Game::MaterialTechnique * RB_BeginSurface(Game::MaterialTechniqueType techType, Game::Material material)
	{
		const static uint32_t RB_BeginSurface_Func = 0x61A220;
		__asm
		{
			pushad

			mov		edi, techType
			mov		esi, [material]

			Call	RB_BeginSurface_Func

			popad
		}
	}

	Game::MaterialTechnique * RB_BeginSurface_CustomMaterial(Game::MaterialTechniqueType techType, Game::Material *material)
	{
		const static uint32_t RB_BeginSurface_Func = 0x61A220;
		__asm
		{
			pushad

			mov		edi, techType
			mov		esi, material

			Call	RB_BeginSurface_Func

			popad
		}
	}

	// wrapper not correct ?!
	void R_AddCmdDrawTextASM(const char *text, int maxChars, void *font, float x, float y, float xScale, float yScale, float rotation, const float *color, int style)
	{
		const static uint32_t R_AddCmdDrawText_Func = 0x5F6B00;
		__asm
		{
			push	style
			sub     esp, 14h

			fld		rotation
			fstp	[esp + 10h]

			fld		yScale
			fstp	[esp + 0Ch]

			fld		xScale
			fstp	[esp + 8]

			fld		y
			fstp	[esp + 4]

			fld		x
			fstp	[esp]

			push	font
			push	maxChars
			push	text
			mov		ecx, [color]

			Call	R_AddCmdDrawText_Func
			add		esp, 24h
		}
	}

	// fails after a few seconds
	void RB_StandardDrawCommands(Game::GfxViewInfo *viewInfo)
	{
		const static uint32_t RB_StandardDrawCommands_Func = 0x64AFB0;
		__asm
		{
			mov		eax, viewInfo
			Call	RB_StandardDrawCommands_Func
		}
	}

	void R_AddCmdDrawStretchPic(void *material, float x, float y, float w, float h, float null1, float null2, float null3, float null4, float *color)
	{
		const static uint32_t R_AddCmdDrawStretchPic_Func = 0x5F65F0;
		__asm
		{
			pushad

			push	color
			mov		eax, [material]
			sub		esp, 20h

			fld		null4
			fstp	[esp + 1Ch]

			fld		null3
			fstp	[esp + 18h]

			fld		null2
			fstp	[esp + 14h]

			fld		null1
			fstp	[esp + 10h]

			fld		h
			fstp	[esp + 0Ch]

			fld		w
			fstp	[esp + 8h]

			fld		y
			fstp	[esp + 4h]

			fld		x
			fstp	[esp]

			call	R_AddCmdDrawStretchPic_Func
			add		esp, 24h

			popad
		}
	}

	void R_SetRenderTarget(int target)
	{
		const static uint32_t R_SetRenderTarget_Func = 0x632B60;
		const static uint32_t _gfxCmdBufSourceState = 0xD53F5F0; // State
		const static uint32_t _gfxCmdBufSourceSource = 0xD5404F0; // Source // should be switched? using Struct "GfxCmdBufState" on 0xD5404F0 works

		__asm
		{
			pushad

			push	_gfxCmdBufSourceSource // eax
			push	_gfxCmdBufSourceState // ecx
			mov     eax, target

			call	R_SetRenderTarget_Func
			add     esp, 8

			popad
		}
	}

	void R_Set2D()
	{
		const static uint32_t R_Set2D_Func = 0x6336E0;
		const static uint32_t _gfxCmdBufSourceState = 0xD53F5F0;

		__asm
		{
			pushad

			mov		edi, [_gfxCmdBufSourceState]
			call	R_Set2D_Func

			popad
		}
	}

	void RB_DrawStretchPic(Game::Material *material, float x, float y, float w, float h, float texcoord0, float texcoord1, float texcoord2, float texcoord3 /*-1 pushed*/)
	{
		const static uint32_t RB_DrawStretchPic_Func = 0x610E10;
		__asm
		{
			pushad

			mov		eax, material
			push    0FFFFFFFFh // -1
			sub     esp, 20h

			fld		texcoord3
			fstp	[esp + 1Ch]

			fld		texcoord2
			fstp	[esp + 18h]

			fld		texcoord1
			fstp	[esp + 14h]

			fld		texcoord0
			fstp	[esp + 10h]

			fld		h
			fstp	[esp + 0Ch]

			fld		w
			fstp	[esp + 8h]

			fld		y
			fstp	[esp + 4h]

			fld		x
			fstp	[esp]

			Call	RB_DrawStretchPic_Func
			add     esp, 24h

			popad
		}
	}

	void CG_DrawRotatedPicPhysical(ScreenPlacement* place, float a2, float a3, float a4, float a5, float a6, float *color, void *material)
	{
		const static uint32_t CG_DrawRotatedPicPhysical_Func = 0x431490;
		__asm
		{
			pushad

			push	[material]
			push	[color]
			sub     esp, 14h

			fld		a6
			fstp	[esp + 10h]

			mov     edx, place

			fld		a5
			fstp	[esp + 0Ch]

			fld		a4
			fstp	[esp + 8h]

			fld		a3
			fstp	[esp + 4h]

			fld		a2
			fstp	[esp]

			call	CG_DrawRotatedPicPhysical_Func
			add     esp, 1Ch

			popad
		}
	}

	int R_TextWidth(const char *text /*<eax*/, int maxChars, Game::Font_s *font)
	{
		const static uint32_t R_TextWidth_Func = 0x5F1EE0;
		__asm
		{
			push	font
			push	maxChars
			mov		eax, [text]

			Call	R_TextWidth_Func
			add		esp,8
		}
	}

	// ---------
	// UI / MENU

	DWORD* ui_white_material_ptr = reinterpret_cast<DWORD*>(0xCAF06F0);
	int* gameTypeEnum = reinterpret_cast<int*>(0xCAF1820);
	int* mapNameEnum = reinterpret_cast<int*>(0xCAF2330);
	Game::UiContext* _uiContext = reinterpret_cast<Game::UiContext*>(0xCAEE200);
	Game::PlayerKeyState* playerKeys = reinterpret_cast<Game::PlayerKeyState*>(0x8F1DB8);
	Game::clientUIActive_t* clientUI = reinterpret_cast<Game::clientUIActive_t*>(0xC5F8F4);

	ScreenPlacement* scrPlace = reinterpret_cast<ScreenPlacement*>(0xE34420);
	ScreenPlacement* scrPlaceFull = reinterpret_cast<ScreenPlacement*>(0xE343D8);

	int String_Parse(const char **p /*eax*/, char *outStr, int len)
	{
		const static uint32_t String_Parse_Func = 0x54B510;
		__asm
		{
			push	len
			lea		eax, [outStr]
			push	eax
			mov		eax, p

			Call	String_Parse_Func
			add		esp, 8
		}
	}

	void Menus_OpenByName(const char* menuName, Game::UiContext *uiDC)
	{
		const static uint32_t Menus_OpenByName_Func = 0x550B50;
		__asm
		{
			pushad

			mov		esi, uiDC
			mov		edi, menuName

			Call	Menus_OpenByName_Func

			popad
		}
	}

	void Menus_CloseByName(const char* menuName, Game::UiContext *uiDC)
	{
		const static uint32_t Menus_CloseByName_Func = 0x54C520;
		__asm
		{
			pushad

			mov		eax, menuName
			mov		esi, uiDC

			Call	Menus_CloseByName_Func

			popad
		}
	}

	void Menus_CloseAll(Game::UiContext *uiDC)
	{
		const static uint32_t Menus_CloseAll_Func = 0x54C540;
		__asm
		{
			pushad

			mov		esi, uiDC
			Call	Menus_CloseAll_Func

			popad
		}
	}
	
	
	// ------------------------------------------------------
	// GSC 

	DWORD* gScrMemTreePub				= reinterpret_cast<DWORD*>(0x14E8A04);
	DWORD* scrVarPub /*char[1024]*/		= reinterpret_cast<DWORD*>(0x15CA61C);
	int* scrVarPub_p4 /*scrVarpub+4*/	= reinterpret_cast<int*>(0x15CA620);
	char* error_message /*char[1023]*/	= reinterpret_cast<char*>(0x1798378);
	char* errortype /*char[1023]*/		= reinterpret_cast<char*>(0x1798777);
	int* scr_numParam					= reinterpret_cast<int*>(0x1794074);

	Game::gentity_s* scr_g_entities = reinterpret_cast<Game::gentity_s*>(0x1288500);
	Game::level_locals_t* level_locals = reinterpret_cast<Game::level_locals_t*>(0x13EB6A8);

	void G_SetOrigin(Game::gentity_s* ent, float *origin)
	{
		if (ent)
		{
			ent->s.lerp.pos.trBase[0] = origin[0];
			ent->s.lerp.pos.trBase[1] = origin[1];
			ent->s.lerp.pos.trBase[2] = origin[2];
			ent->s.lerp.pos.trType = Game::trType_t::TR_STATIONARY;
			ent->s.lerp.pos.trTime = 0;
			ent->s.lerp.pos.trDuration = 0;
			ent->s.lerp.pos.trDelta[0] = 0.0;
			ent->s.lerp.pos.trDelta[1] = 0.0;
			ent->s.lerp.pos.trDelta[2] = 0.0;
			ent->r.currentOrigin[0] = origin[0];
			ent->r.currentOrigin[1] = origin[1];
			ent->r.currentOrigin[2] = origin[2];
		}
	}

	void G_SetAngles(Game::gentity_s* ent, float *angles)
	{
		if (ent)
		{
			ent->s.lerp.apos.trBase[0] = angles[0];
			ent->s.lerp.apos.trBase[1] = angles[1];
			ent->s.lerp.apos.trBase[2] = angles[2];
			ent->s.lerp.apos.trType = Game::trType_t::TR_STATIONARY;
			ent->s.lerp.apos.trTime = 0;
			ent->s.lerp.apos.trDuration = 0;
			ent->s.lerp.apos.trDelta[0] = 0.0;
			ent->s.lerp.apos.trDelta[1] = 0.0;
			ent->s.lerp.apos.trDelta[2] = 0.0;
			ent->r.currentAngles[0] = angles[0];
			ent->r.currentAngles[1] = angles[1];
			ent->r.currentAngles[2] = angles[2];
		}
	}

	std::int16_t G_ModelIndex(const char *modelName /*eax*/)
	{
		const static uint32_t G_ModelIndex_Func = 0x4E21F0;
		__asm
		{
			mov		eax, modelName
			Call	G_ModelIndex_Func
		}
	}

	void SV_LinkEntity(Game::gentity_s* ent /*edi*/)
	{
		const static uint32_t SV_LinkEntity_Func = 0x536D80;
		__asm
		{
			mov		edi, ent
			Call	SV_LinkEntity_Func
		}
	}

	void Scr_ObjectError(const char *string /*eax*/)
	{
		const static uint32_t Scr_ObjectError_Func = 0x523F90;
		__asm
		{
			mov		eax, string
			Call	Scr_ObjectError_Func
		}
	}

	void Scr_GetVector(unsigned int argIndex /*eax*/, float *floatOut /*edx*/ )
	{
		const static uint32_t Scr_GetVector_Func = 0x5236E0;
		__asm
		{
			mov		edx, [floatOut]
			mov		eax, argIndex
			xor		eax, eax

			Call	Scr_GetVector_Func
		}
	}

	int isButtonPressed(int button, int buttonData) 
	{
		int tmp, i = 0;
		int pwrs[20] = { 0 }; // Never going to need more than 20

		if (buttonData < button) 
		{
			return 0;
		}

		if (buttonData == button) 
		{
			return 1;
		}

		while (buttonData != 0) 
		{
			tmp = 1;
			while (tmp * 2 <= buttonData) 
			{
				tmp *= 2;
			}

			buttonData -= tmp;
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

	
	// ----
	// IWDs

	const char* fs_gamedir = reinterpret_cast<const char*>(0xCB19898);
	Game::searchpath_s* fs_searchpaths = reinterpret_cast<Game::searchpath_s*>(0xD5EC4DC);

	char ** Sys_ListFiles(const char *filter /*eax*/, const char *directory, const char *extension, int *numfiles, int wantsubs)
	{
		const static uint32_t Sys_ListFiles_Func = 0x572F00;
		__asm
		{
			push	0 // wantsubs
			push	numfiles
			push	[extension] // ext
			push	directory
			xor		eax, eax // filter

			Call	Sys_ListFiles_Func
			add     esp, 10h
		}
	}

	int SEH_GetLanguageIndexForName(const char *pszLanguageName /*edi*/, int *piLanguageIndex)
	{
		const static uint32_t SEH_GetLanguageIndexForName_Func = 0x539250;
		__asm
		{
			lea		ecx, [piLanguageIndex]
			push	ecx // langIndex
			mov		edi, [pszLanguageName] //langName

			Call	SEH_GetLanguageIndexForName_Func
			add     esp, 4
		}
	}

	int unzClose(const char *file /*edi*/)
	{
		const static uint32_t unzClose_Func = 0x596A50;
		__asm
		{
			mov		edi, [file]
			Call	unzClose_Func
		}
	}

	
	// --------
	// MOVEMENT

	int* g_entities = reinterpret_cast<int*>(0x12885C4);
	int* g_clients = reinterpret_cast<int*>(0x13255A8);
	int* currentTime = reinterpret_cast<int*>(0x13EB894);
	int* CanDamageContentMask = reinterpret_cast<int*>(0x802011);

	void PM_Friction(Game::playerState_s *ps, Game::pml_t *pml)
	{
		const static uint32_t PM_Friction_Func = 0x40E860;

		__asm
		{
			pushad

			push	pml
			mov		esi, ps

			Call	PM_Friction_Func
			add     esp, 4h

			popad
		}
	}

	void PM_ClipVelocity_Call(const float *velocityIn, const float *traceNormal, float *velocityOut)
	{
		const static uint32_t PM_ClipVelocity_Adr = 0x40E2B0;
		__asm
		{
			pushad

			push	velocityOut
			mov		esi, traceNormal
			mov		edi, velocityIn

			call	PM_ClipVelocity_Adr
			add		esp, 4h

			popad
			retn

		}
	}

	bool PM_CorrectAllSolid(Game::pmove_t *pm /*eax*/, Game::pml_t *pml, Game::trace_t *trace)
	{
		const static uint32_t PM_CorrectAllSolid_Func = 0x410370;
		__asm
		{
			push    trace
			push	pml
			mov		eax, pm

			Call	PM_CorrectAllSolid_Func
			add		esp, 8h
		}
	}

	void PM_GroundTraceMissed(Game::pmove_t *pm /*eax*/, Game::pml_t *pml)
	{
		const static uint32_t PM_GroundTraceMissed_Func = 0x4104E0;
		__asm
		{
			push	pml
			mov		eax, pm

			Call	PM_GroundTraceMissed_Func
			add		esp, 4h
		}
	}

#pragma warning( push )
#pragma warning( disable : 4100 )
	void PM_CrashLand(Game::pmove_t *pm /*bl-no-push-needed*/, Game::playerState_s *ps /*esi*/, Game::pml_t *pml) 
	{
		const static uint32_t PM_CrashLand_Func = 0x40FFB0;
		__asm
		{
			push	pml
			mov		esi, ps

			Call	PM_CrashLand_Func
			add		esp, 4h
		}
	}
#pragma warning( pop ) 

	void PM_AddTouchEnt(Game::pmove_t *pm /*eax*/, int entityNum /*edi*/)
	{
		const static uint32_t PM_AddTouchEnt_Func = 0x40E270;
		__asm
		{
			movzx	edi, entityNum
			mov		eax, pm

			Call	PM_AddTouchEnt_Func
		}
	}

	void PM_playerTrace(Game::pmove_t *pm /*esi*/, Game::trace_t *results, const float *start, const float *mins, const float *maxs, const float *end, std::int32_t passEntityNum, std::int32_t contentMask)
	{
		const static uint32_t PM_playerTrace_Func = 0x40E160;
		__asm
		{
			push	contentMask
			push	passEntityNum
			push	[end]
			push	[maxs]
			push	[mins]
			push	[start]
			push	results
			mov		esi, pm

			Call	PM_playerTrace_Func
			add     esp, 1Ch
		}
	}

	void G_Damage(float *dir /*eax*/, Game::gentity_s *targ, Game::gentity_s *inflictor, Game::gentity_s *attacker, float *point, int damage, int flags, int _mod, int self_client, int hitloc, /*hitLocation_t*/ int a11, unsigned int a12, unsigned int timeOffset)
	{
		const static uint32_t G_Damage_Func = 0x4B5560;
		__asm
		{
			push	timeOffset
			push	a12
			push	a11
			push	hitloc
			push	self_client
			push	_mod
			push	flags
			push	[damage]
			push	[point]
			push	attacker
			push	inflictor
			push	targ

			mov		eax, [dir]

			Call	G_Damage_Func
			add     esp, 30h
		}
	}

	float CanDamage(Game::gentity_s *inflictor /*eax*/, float *centerPos /*ecx*/, Game::gentity_s *ent, float coneAngleCos, float *coneDirection, int contentmask)
	{
		const static uint32_t CanDamage_Func = 0x4B5770;
		__asm
		{
			push	contentmask
			push	[coneDirection]
			push	0 // ??

			fld		coneAngleCos
			fstp	[esp]

			push	ent

			mov		ecx, [centerPos]
			mov		eax, inflictor

			Call	CanDamage_Func
			add     esp, 10h
		}
	}
	
	void Scr_PlayerDamage(float *dir, Game::gentity_s *targ, Game::gentity_s *inflictor, Game::gentity_s *attacker, int damage, int dflags, int _mod, int weapon, float *point, Game::hitLocation_t hitLoc, int timeOffset)
	{
		const static uint32_t Scr_PlayerDamage_Func = 0x4D8B80;
		__asm
		{
			pushad

			push	timeOffset
			push	hitLoc
			push	[point]
			push	weapon
			push	_mod
			push	dflags
			push	damage
			push	attacker
			push	inflictor
			push	targ
			mov		eax, [dir]

			call Scr_PlayerDamage_Func
			add		esp, 28h
			popad
		}
	}
	
	Game::gentity_s* G_FireRocket(float *kickBack, Game::gentity_s *ent, signed int weapindex, float *dir, float *gunVel, Game::gentity_s *target, float *targetOffset)
	{
		const static uint32_t G_FireRocket_Func = 0x4C7C30;
		__asm
		{
			push	targetOffset;
			push	target;
			push	gunVel
			push	dir
			push	weapindex
			push	ent
			mov		eax, [kickBack]

			Call	G_FireRocket_Func
			add     esp, 18h
		}
	}
	
	// -----
	// DVARs 

	void Dvar_SetValue(dvar_s* _dvar, int _dvarValue)
	{
		_dvar->current.integer = _dvarValue;
		_dvar->latched.integer = _dvarValue;
		//_dvar->modified = false;
	}

	void Dvar_SetValue(dvar_s* _dvar, bool _dvarValue)
	{
		_dvar->current.enabled = _dvarValue;
		_dvar->latched.enabled = _dvarValue;
		//_dvar->modified = false;
	}

	void Dvar_SetValue(dvar_s* _dvar, const float _dvarValue)
	{
		_dvar->current.value = _dvarValue;
		_dvar->latched.value = _dvarValue;
		//_dvar->modified = false;
	}

	void Dvar_SetValue(dvar_s* _dvar, const char *_dvarValue)
	{
		_dvar->current.string = _dvarValue;
		_dvar->latched.string = _dvarValue;
		_dvar->modified = false;
	}

	//dvar_s* Dvar_RegisterVariant(const char* dvarName /*eax*/, DvarType typeInt, std::uint16_t flags, const char* description, float defaultValue, float y, float z, float w, float min, int max)
	//{
	//	const static uint32_t Dvar_RegisterVariant_Func = 0x56C350;
	//	__asm
	//	{
	//		mov		eax, [text]
	//		mov		esi, [dvar]

	//		push	max
	//		push	min

	//		sub		esp, 10h
	//		mov		ecx, 

	//		push	description
	//		push	flags
	//		push	dvarType
	//		mov		eax, dvarName

	//		Call	Dvar_RegisterVariant_Func
	//	}
	//}

	// Registering StringDvars is a pain, so heres a workaround
	Game::dvar_s* Dvar_RegisterString_hacky(const char *dvarName, const char *dvarValue, const char *description)
	{
		Game::dvar_s *dvarToRegister;

		auto registerString = Utils::VA("set %s %s\n", dvarName, dvarValue);
		Game::Cmd_ExecuteSingleCommand(0, 0, registerString);

		// find the dvar
		dvarToRegister = Game::Dvar_FindVar(dvarName);

		if (!dvarToRegister)
		{
			return nullptr;
		}

		// if our dvar was created successfully, our description will be "External Dvar"
		std::string external = dvarToRegister->description;
		if (external == "External Dvar")
		{
			// success
			dvarToRegister->description = description;
			dvarToRegister->flags = Game::dvar_flags::none;

			return dvarToRegister;
		}

		return nullptr;
	}

	char* Dvar_EnumToString(const dvar_s *dvar)
	{
		char *result;

		if (dvar->domain.enumeration.stringCount) 
		{
			result = *(char **)(dvar->domain.integer.max + 4 * dvar->current.integer);
		}
		else 
		{
			result = "";
		}

		return result;
	}

	void Dvar_SetString(const char *text /*eax*/, dvar_s *dvar /*esi*/)
	{
		const static uint32_t Dvar_SetString_Func = 0x56CA90;
		__asm
		{
			pushad

			mov		eax, [text]
			mov		esi, [dvar]

			Call	Dvar_SetString_Func

			popad
		}
	}

	__declspec(naked) dvar_s* Dvar_FindVar(const char* /*dvar*/)
	{
		__asm
		{
			push	eax
			pushad

			mov		edi, [esp + 28h]
			mov		eax, 56B5D0h
			call	eax

			mov		[esp + 20h], eax
			popad

			pop eax
			retn
		}
	}


	// -------
	// CONSOLE 

	float* con_matchtxtColor_currentDvar = reinterpret_cast<float*>(0x6BDF14); // 0x6BDF14
	float* con_matchtxtColor_currentValue = reinterpret_cast<float*>(0x6BDF24); // 0x6BDF24
	float* con_matchtxtColor_defaultValue = reinterpret_cast<float*>(0x6BDF34); // 0x6BDF34
	float* con_matchtxtColor_dvarDescription = reinterpret_cast<float*>(0x6BDF54); // 0x6BDF54
	float* con_matchtxtColor_domainDescription = reinterpret_cast<float*>(0x6BDF44); // 0x6BDF44

	// console structs
	ConDrawInputGlob* conDrawInputGlob = reinterpret_cast<ConDrawInputGlob*>(0x8CC2C8);
	Console* con = reinterpret_cast<Console*>(0x8DC8C0);
	field_t* g_consoleField = reinterpret_cast<field_t*>(0x8F1B88);

	// console variables
	//Font_s* con_font = reinterpret_cast<Font_s*>(0xC5AE94));
	bool*	Key_IsCatcherActive = reinterpret_cast<bool*>(0xC5F8F8);

	// con->screenMin / Max manual defines for use in asm
	float*	conScreenMin0 = reinterpret_cast<float*>(0x8ECB14); // left
	float*	conScreenMin1 = reinterpret_cast<float*>(0x8ECB18); // top
	float*	conScreenMax0 = reinterpret_cast<float*>(0x8ECB1C); // right
	float*	conScreenMax1 = reinterpret_cast<float*>(0x8ECB20); // bottom

	bool*	extvar_con_ignoreMatchPrefixOnly = reinterpret_cast<bool*>(0x736BB1);

	// cmd args
	int* argc_1410B84 = reinterpret_cast<int*>(0x1410B84);
	int* argc_1410B40 = reinterpret_cast<int*>(0x1410B40);
	char* argv_6BFEA7 = reinterpret_cast<char*>(0x6BFEA7);
	int* argv_1410BA4 = reinterpret_cast<int*>(0x1410BA4);
	cmd_function_s* cmd_functions = reinterpret_cast<cmd_function_s*>(0x1410B3C);

	SCR_DrawSmallStringExt_t SCR_DrawSmallStringExt = (SCR_DrawSmallStringExt_t)0x474C30;
	Sys_IsMainThread_t Sys_IsMainThread = (Sys_IsMainThread_t)0x50B5D0;
	Con_TokenizeInput_t Con_TokenizeInput = (Con_TokenizeInput_t)0x45F350;
	Con_CancelAutoComplete_t Con_CancelAutoComplete = (Con_CancelAutoComplete_t)0x460A90;
	Con_DrawInputPrompt_t Con_DrawInputPrompt = (Con_DrawInputPrompt_t)0x460510;
	Cmd_EndTokenizedString_t Cmd_EndTokenizedString = (Cmd_EndTokenizedString_t)0x4F98C0;
	ConDrawInput_IncrMatchCounter_t ConDrawInput_IncrMatchCounter = (ConDrawInput_IncrMatchCounter_t)0x45FA40;
	Con_AnySpaceAfterCommand_t Con_AnySpaceAfterCommand = (Con_AnySpaceAfterCommand_t)0x45F3D0;
	ConDrawInput_DetailedDvarMatch_t ConDrawInput_DetailedDvarMatch = (ConDrawInput_DetailedDvarMatch_t)0x45FFB0;
	ConDrawInput_DetailedCmdMatch_t ConDrawInput_DetailedCmdMatch = (ConDrawInput_DetailedCmdMatch_t)0x460370;
	ConDrawInput_DvarMatch_t ConDrawInput_DvarMatch = (ConDrawInput_DvarMatch_t)0x45FAB0;
	ConDrawInput_CmdMatch_t ConDrawInput_CmdMatch = (ConDrawInput_CmdMatch_t)0x460440;
	Con_DrawOutputScrollBar_t Con_DrawOutputScrollBar = (Con_DrawOutputScrollBar_t)0x461860;
	Con_DrawOutputText_t Con_DrawOutputText = (Con_DrawOutputText_t)0x4619E0;
	Con_IsAutoCompleteMatch_t Con_IsAutoCompleteMatch = (Con_IsAutoCompleteMatch_t)0x45F990;

	void Con_DrawMessageWindowOldToNew(DWORD* msgWindow /*esi*/, int localClientNum, int xPos, int yPos, int charHeight, int horzAlign, int vertAlign, int mode, Font_s* font, const float* color, int textStyle, float msgwndScale, int textAlignMode)
	{
		const static uint32_t Con_DrawMessageWindowOldToNew_Func = 0x461150;
		__asm
		{
			push	textAlignMode
			push	0

			fld		msgwndScale
			fstp	dword ptr[esp]

			push	textStyle
			push	color
			push	font
			push	mode
			push	vertAlign
			push	horzAlign
			push	charHeight
			push	yPos
			push	xPos
			push	localClientNum

			mov		esi, msgWindow

			Call	Con_DrawMessageWindowOldToNew_Func
			add     esp, 30h
		}
	}

	void Cmd_ForEachXO(void(__cdecl *callback)(const char *))
	{
		cmd_function_s *cmd;

		// skip the first cmd (nullptr)?
		for (cmd = cmd_functions->next; cmd; cmd = cmd->next)
		{
			if (cmd->name)
			{
				callback(cmd->name);
			}
		}
	}

	void AddBaseDrawConsoleTextCmd(int charCount /*eax*/, const float *colorFloat /*ecx*/, const char *textPool, int poolSize, int firstChar, Game::Font_s *font, float x, float y, float xScale, float yScale, int style)
	{
		const static uint32_t AddBaseDrawConsoleTextCmd_Func = 0x5F6F60;
		__asm
		{
			push    style
			sub     esp, 10h

			fld		[yScale]
			fstp	[esp + 0Ch]

			fld		[xScale]
			fstp	[esp + 8]

			fld		[y]
			fstp	[esp + 4]

			fld		[x]
			fstp	[esp]

			push    font
			push    firstChar
			push    poolSize
			push    textPool

			mov     ecx, [colorFloat]
			mov     eax, [charCount]

			Call	AddBaseDrawConsoleTextCmd_Func
			add     esp, 24h
		}
	}

	void ConDrawInput_Box(float *color, int lineHeightMulti)
	{
		const static uint32_t ConDrawInput_Box_Func = 0x45F700;
		__asm
		{
			pushad

			mov		esi, [color]
			push	lineHeightMulti

			call	ConDrawInput_Box_Func
			add		esp, 4h

			popad
		}
	}
	
	void ConDrawInput_TextAndOver(char *text)
	{
		const static uint32_t ConDrawInput_TextAndOver_Func = 0x45F500;
		__asm
		{
			pushad

			mov		esi, [text]
			call	ConDrawInput_TextAndOver_Func

			popad
		}
	}

	void ConDraw_Box(float *color, float x, float y, float width, float height)
	{
		const static uint32_t ConDraw_Box_Func = 0x45F540;
		__asm
		{
			pushad

			mov		esi, [color]
			sub		esp, 10h

			fld		height
			fstp	[esp + 0Ch]

			fld		width
			fstp	[esp + 8h]

			fld		y
			fstp	[esp + 4h]

			fld		x
			fstp	[esp]

			call	ConDraw_Box_Func
			add		esp, 10h

			popad
		}
	}

	void Con_DrawAutoCompleteChoice(int a1, char *a2)
	{
		const static uint32_t Con_DrawAutoCompleteChoice_Func = 0x460490;
		__asm
		{
			pushad

			mov eax, a1
			push a2

			call Con_DrawAutoCompleteChoice_Func
			add esp, 4h

			popad
		}
	}

	char* Cmd_Argv(int argIndex)
	{
		const static uint32_t Cmd_Argv_Func = 0x42A950;
		__asm
		{
			mov		eax, argIndex
			call	Cmd_Argv_Func
		}
	}

	void Dvar_ForEachName(void(__cdecl *func)(const char *))
	{
		const static uint32_t Dvar_ForEachName_Func = 0x569D30;
		__asm
		{
			pushad

			mov		edi, [func]
			call	Dvar_ForEachName_Func

			popad
		}
	}

	void Cmd_ForEach(void(__cdecl *func)(const char *))
	{
		const static uint32_t Cmd_ForEach_Func = 0x4F9A40;
		__asm
		{
			pushad

			mov		edi, [func]
			call	Cmd_ForEach_Func

			popad
		}
	}

	
	// -----
	// ANIMS

	// prob. broken af
	void BG_AnimScriptEvent(scriptAnimEventTypes_t event, Game::playerState_s *ps, int force)
	{
		const static uint32_t BG_AnimScriptEvent_Func = 0x405720;
		__asm
		{
			pushad

			mov     edi, [ps]
			push    force
			mov     eax, event

			call BG_AnimScriptEvent_Func
			add esp, 4h

			popad
		}
	}


	// ------
	// COMMON

	Game::playerState_s* ps_loc = reinterpret_cast<Game::playerState_s*>(0x13255A8);

	DWORD* cmd_id = reinterpret_cast<DWORD*>(0x1410B40);
	DWORD* cmd_argc = reinterpret_cast<DWORD*>(0x1410B84);
	char*** cmd_argv = reinterpret_cast<char***>(0x1410BA4);
	cmd_function_s** cmd_ptr = reinterpret_cast<cmd_function_s**>(0x1410B3C);
	XZone* g_zones = reinterpret_cast<XZone*>(0xFFEFD0);
	XAssetEntry* g_assetEntryPool = reinterpret_cast<XAssetEntry*>(0xF0D640);
	unsigned short* db_hashTable = reinterpret_cast<unsigned short*>(0xE62A80);
	infoParm_t* infoParams = reinterpret_cast<Game::infoParm_t*>(0x71FBD0); // Count 0x1C

	Cmd_ExecuteSingleCommand_t Cmd_ExecuteSingleCommand = Cmd_ExecuteSingleCommand_t(0x4F9AB0);
	Com_Error_t Com_Error = Com_Error_t(0x4FD330);
	Com_PrintMessage_t Com_PrintMessage = Com_PrintMessage_t(0x4FCA50);
	DB_FindXAssetHeader_t DB_FindXAssetHeader = DB_FindXAssetHeader_t(0x489570);
	DB_EnumXAssets_FastFile_t DB_EnumXAssets_FastFile = DB_EnumXAssets_FastFile_t(0x489120);
	DB_GetXAssetNameHandler_t* DB_GetXAssetNameHandlers = reinterpret_cast<DB_GetXAssetNameHandler_t*>(0x7268D0);
    DB_LoadXAssets_t DB_LoadXAssets = DB_LoadXAssets_t(0x48A2B0);

	void Cbuf_AddText(const char *text /*eax*/, int localClientNum /*ecx*/)
	{
		const static uint32_t Cbuf_AddText_Func = 0x4F8D90;
		__asm
		{
			mov		ecx, localClientNum
			mov		eax, text
			Call	Cbuf_AddText_Func
		}
	}

	void Cmd_AddCommand(const char* name, void(*callback)(), cmd_function_s* data, char)
	{
		data->name = name;
		data->autoCompleteDir = NULL;
		data->autoCompleteExt = NULL;
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

	__declspec(naked) Game::PackedUnitVec Vec3PackUnitVec(const float*)
	{
		__asm
		{
			mov eax, [esp + 4h]
			push 5645A0h
			retn
		}
	}

	__declspec(naked) void Vec3UnpackUnitVec(Game::PackedUnitVec, const float*)
	{
		__asm
		{
			push ecx
			mov ecx, [esp + 0Ch]
			push [esp + 08h]

			mov eax, 5647D0h
			call eax

			add esp, 4h
			pop ecx
			retn
		}
	}
}
