#pragma once
#include "utils/function.hpp"

namespace game
{
	namespace glob
	{
		// general
		extern std::string loaded_modules;
		extern std::string loaded_libaries;
		
		// radiant
		extern game::cgs_addon_s				cgs_addons;
		extern game::saved_radiant_brushes_s	radiant_saved_brushes;
		extern game::dynBrushesArray_t		dynamic_brushes;
		extern game::dynBrushModelsArray_t	dynamic_brush_models;

		// movement
		extern bool lpmove_check_jump; // if Jumped in Check_Jump, reset after x frames in PmoveSingle

		extern glm::vec3 lpmove_velocity;	// grab local player velocity
		extern glm::vec3 lpmove_origin;		// grab local player origin
		extern glm::vec3 lpmove_angles;		// grab local player angles
		extern glm::vec3 lpmove_camera_origin; // grab local camera origin

		// ui / devgui
		extern bool loaded_main_menu;
		extern bool mainmenu_fade_done;

		extern game::gui_t gui;
		extern std::string changelog_html_body;

		// renderer
		extern IDirect3DDevice9* d3d9_device;

		// debug collision
		extern bool debug_collision_initialized; // debug collision was used
		extern int  debug_collision_rendered_brush_amount; // total amount of brushes used for calculations of planes 
		extern int  debug_collision_rendered_planes_amount;	// total amount of planes rendered 
		extern int  debug_collision_rendered_planes_counter; // total amount of planes rendered used to count while drawing

		extern std::string	r_drawCollision_materialList_string;

		// Frametime
		extern int lpmove_server_time;
		extern int lpmove_server_time_old;
		extern int lpmove_server_frame_time;
		extern int lpmove_pml_frame_time;
		
		// Misc
		extern int q3_last_projectile_weapon_used; // ENUM Q3WeaponNames :: this var holds the last proj. weapon that got fired

#ifdef DEVGUI_XO_BLUR
		extern float xo_blur_directions;
		extern float xo_blur_quality;
		extern float xo_blur_size;
		extern float xo_blur_alpha;
#endif
	}

	static inline float COLOR_WHITE[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static inline float COLOR_BLACK[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	extern game::TestLod g_testLods[4];

	// *
	// general structs

	extern game::clientActive_t*		clients;
	extern game::clientStatic_t*		cls;
	extern game::clientConnection_t&	clc;
	extern game::cg_s*					cgs;
	extern game::GfxBuffers*			gfx_buf;
	extern game::GfxScene*				scene;
	//extern Game::serverStatic_t* svs; // cba
	extern game::clipMap_t*				cm;
	extern game::ComWorld*				com;
	extern game::GfxWorld*				gfx_world;

	extern int*	com_frameTime;
	extern float* com_timescaleValue;


	// *
	// radiant / cgame

	extern const char* g_entityBeginParsePoint;
	extern const char* g_entityEndParsePoint;
	extern int* clientActive_cmdNumber;
	
	char* Com_Parse(const char **data_p /*edi*/); // ASM
	bool  CL_GetUserCmd(int cmdNumber /*eax*/, game::usercmd_s *ucmd);


	// *
	// filesystem

	extern HANDLE database_handle;

	extern const char** zone_code_post_gfx_mp;
	extern const char** zone_localized_code_post_gfx_mp;
	extern const char** zone_ui_mp;
	extern const char** zone_common_mp;
	extern const char** zone_localized_common_mp;
	extern const char** zone_mod;

	extern XAssetHeader* DB_XAssetPool;
	extern unsigned int* g_poolSize;

	typedef int(__cdecl* DB_GetXAssetSizeHandler_t)();
	extern DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandlers;

	XAssetHeader db_realloc_xasset_pool(XAssetType type, unsigned int new_size);

	static utils::function<void()> R_BeginRemoteScreenUpdate = 0x5F78A0;
	static utils::function<void()> R_EndRemoteScreenUpdate = 0x5F78F0;
	static utils::function<void()> DB_SyncXAssets = 0x48A120;

	extern bool DB_FileExists(const char* file_name, game::DB_FILE_EXISTS_PATH);
	extern void FS_DisplayPath(int bLanguageCull /*eax*/);

	extern const char* fs_gamedir;
	extern game::searchpath_s* fs_searchpaths;

	static utils::function<game::iwd_t* (const char* zipfile, const char* basename)> FS_LoadZipFile = 0x55C6F0;
	static utils::function<const char* (const char* lang)>							IwdFileLanguage = 0x55D700;
	static utils::function<BOOL(LPVOID lpAddress)>									FS_FreeFileList = 0x564520;
	static utils::function<void(void* lpMem)>										free = 0x670DA6;

	char** Sys_ListFiles(const char* filter, const char* directory, const char* extension, int* num_files); //ASM
	int	SEH_GetLanguageIndexForName(const char* psz_language_name, int* pi_language_index); //ASM
	int	unzClose(const char* file /*edi*/); //ASM


	// *
	// debug collision 

	static utils::function<bool()>
		CreateDebugStringsIfNeeded = 0x461EC0;

	static utils::function<void(game::DebugGlobals *debugGlobalsEntry, const float *origin, const float *color, float scale, const char *string)> 
		R_AddDebugString = 0x60DD10; // not working currently, or well, not displaying a thing because front/backend is empty?

	static utils::function<void(int count, int width, game::GfxPointVertex *verts, bool depthTest)>
		RB_DrawLines3D = 0x613040;

	static utils::function<void __fastcall (const float *colorFloat, char *colorBytes)>
		R_ConvertColorToBytes = 0x493530;


	// *
	// renderer

	extern game::MaterialLoadGlob* mtlLoadGlob;
	extern game::GfxRenderTarget* gfxRenderTargets;

	extern float& v_aspectRatioWindow;
	extern float& v_postProcessAspect;

	extern IDirect3DDevice9** dx9_device_ptr;

	extern game::Material* floatz_display;
	extern GfxCmdBufSourceState* gfxCmdBufSourceState;
	extern game::GfxCmdBufState* gfxCmdBufState;

	extern game::materialCommands_t* tess;
	extern game::GfxBackEndData* _frontEndDataOut;
	//extern game::GfxBackEndData* _backEndData;

	static DWORD* backEndDataOut_ptr = (DWORD*)(0xD0704BC);  // backendEndDataOut pointer
	extern game::GfxBackEndData* get_backenddata();

	extern game::r_globals_t* rg;
	extern game::r_global_permanent_t* rgp;

	extern game::clientDebugLineInfo_t* clientDebugLineInfo_client;
	extern game::clientDebugLineInfo_t* clientDebugLineInfo_server;

	static utils::function<void()> RB_ShowFbColorDebug_Screen = 0x64A5A0;
	static utils::function<void()> RB_ShowFbColorDebug_Feedback = 0x64A710;
	static utils::function<void()> RB_ShowFloatZDebug = 0x64AAA0;
	static utils::function<void()> RB_ShowShadowsDebug = 0x64AB60;

	static utils::function<void(float radius, int srcRenderTargetId)>
		RB_GaussianFilterImage = 0x6517A0;

	static utils::function<int(float radiusX, float radiusY, int srcWidth, int srcHeight, int dstWidth, int dstHeight, game::GfxImageFilterPass *filterPass)>
		RB_GenerateGaussianFilterChain = 0x651310;

	static utils::function<void(game::GfxImageFilter *filter)>
		RB_FilterImage = 0x6516A0;

	static utils::function<void(game::Material*, float s0, float t0, float s1, float t1, int color)>
		RB_DrawFullScreenColoredQuad = 0x6113E0;

	static utils::function<void(std::int32_t a1, std::int32_t a2, std::int32_t a3)>
		DrawXModelSkinnedCached = 0x646870;

	static utils::function<bool()>
		CreateDebugLinesIfNeeded = 0x462080;

	// get handle using DB_FindXAssetHeader
	static utils::function<Font_s* (const char* fontName, int fontSize)>
		R_RegisterFont = 0x5F1EC0;

	// get handle using DB_FindXAssetHeader
	static utils::function<Material* (const char* fontName, int fontSize)>
		Material_RegisterHandle = 0x5F2A80;

	typedef void(*RB_EndTessSurface_t)();
		extern RB_EndTessSurface_t RB_EndTessSurface;

	const char* get_rendertarget_string(game::GfxRenderTargetId id);

	game::MaterialTechnique* RB_BeginSurface(game::MaterialTechniqueType techType, game::Material* material); // ASM
	game::MaterialTechnique* RB_BeginSurface_CustomMaterial(game::MaterialTechniqueType techType, game::Material* material); // ASM

	void draw_text_with_engine(float x, float y, float scale_x, float scale_y, const char* font, const float *color, const char* text);
	void R_AddCmdDrawTextASM(const char *text, int max_chars, void *font, float x, float y, float x_scale, float y_scale, float rotation, const float *color, int style);
	void RB_DrawTextInSpace(const float* pixel_step_x, const float* pixel_step_y, const char* text, game::Font_s* font, const float* org, char* color_bytes); // ASM
	void RB_StandardDrawCommands(game::GfxViewInfo* view_info);																															
	void R_AddCmdDrawStretchPic(game::Material* material, float x, float y, float w, float h, float null1, float null2, float null3, float null4, float* color); // ASM
	void R_SetRenderTarget(int target); // ASM
	void R_Set2D(); // ASM
	void R_Set3D(); // ASM
	void RB_DrawStretchPic(game::Material* material, float x, float y, float w, float h, float a6, float a7, float a8, float a9 /*-1 pushed*/);
	void CG_DrawRotatedPicPhysical(ScreenPlacement* place, float a2, float a3, float a4, float a5, float a6, float *color, void* material);
	int  R_TextWidth(const char *text /*<eax*/, int maxChars, game::Font_s *font); // ASM


	// *
	// ui

	extern DWORD *ui_white_material_ptr;
	extern int*	 gameTypeEnum; 
	extern int*	 mapNameEnum;

	extern game::UiContext*	ui_cg_dc;
	extern game::UiContext*	ui_context;

	extern game::WinMouseVars_t* s_wmv;

	extern game::PlayerKeyState* playerKeys;
	extern game::clientUIActive_t* clientUI;

	extern ScreenPlacement* scrPlace;
	extern ScreenPlacement* scrPlaceFull;

	static utils::function<void(int clientNum, int menuNum)> UI_SetActiveMenu = 0x549540;
	static utils::function<void()> Key_SetCatcher = 0x4686A0;
	static utils::function<void*(const char* menufile, int imageTrack)> UI_LoadMenus_LoadObj = 0x558770;
	
	extern int	String_Parse(const char **p /*eax*/, char *out_str, int len);
	extern void Menus_OpenByName(const char* menu_name, game::UiContext *ui_dc);
	extern void Menus_CloseByName(const char* menu_name, game::UiContext *ui_dc);
	extern void Menus_CloseAll(game::UiContext *ui_dc);
	

	// *
	// gsc 

	extern DWORD*	gScrMemTreePub;
	extern DWORD*	scrVarPub;
	extern int*		scrVarPub_p4;
	extern char*	error_message;
	extern char*	errortype;
	extern int*		scr_numParam;

	extern game::gentity_s*	scr_g_entities;
	extern game::level_locals_t* level_locals;

	static utils::function<void()> Scr_Error_Internal = 0x51D1F0;
	void Scr_AddBool(bool value);

	static utils::function<void(int)> Scr_AddInt = 0x523AB0;
	static utils::function<game::gentity_s*()> G_Spawn = 0x4E37F0;
	static utils::function<bool(game::gentity_s*)> G_CallSpawnEntity = 0x4DFFA0;

	static utils::function<void(game::trajectory_t* pTr, const float* vPos, float fTotalTime, float fAccelTime, float fDecelTime, float* vCurrPos, float* pfSpeed, float* pfMidTime, float* pfDecelTime, float* vPos1, float* vPos2, float* vPos3)> 
		ScriptMover_SetupMove = 0x4D9440;

	void G_SetOrigin(game::gentity_s* ent, const float* origin);
	void G_SetAngles(game::gentity_s* ent, const float* angles);

	std::int16_t G_ModelIndex(const char* model_name /*eax*/); // ASM
	void SV_LinkEntity(game::gentity_s* ent /*edi*/); // ASM

	void Scr_ObjectError(const char* string /*eax*/); // ASM
	void Scr_GetVector(unsigned int arg_index /*eax*/, float* out /*edx*/); // ASM
	void Scr_AddVector(float* out /*esi*/); // ASM
	float Scr_GetFloat(unsigned int arg_index /*eax*/); // ASM

	int	is_button_pressed(int button, int button_data);


	// *
	// movement

	extern game::WeaponDef** BG_WeaponNames;

	extern int* g_entities;
	extern int* g_clients;

	static utils::function<void(game::pmove_t* pm)>									PmoveSingle = 0x4143A0;
	static utils::function<void(game::pmove_t* pm)>									PM_UpdateSprint = 0x40E6A0;
	static utils::function<void(game::pmove_t* pm, game::pml_t* pml)>				PM_WalkMove = 0x40F7A0;
	static utils::function<void(game::pmove_t* pm, game::pml_t* pml)>				PM_AirMove = 0x40F680;
	static utils::function<bool(game::pmove_t* pm, game::pml_t* pml, bool gravity)> PM_SlideMove = 0x414F40;
	static utils::function<void(game::pmove_t* pm, game::pml_t* pml, bool gravity)> PM_StepSlideMove = 0x4155C0;
	static utils::function<void(game::pmove_t* pm, game::pml_t* pml)>				PM_SetMovementDir = 0x40F2D0;
	static utils::function<bool(game::pmove_t* pm, game::pml_t* pml, int gravity)>	PM_SlideMove_Internal = 0x414F40;
	static utils::function<void(game::pmove_t* pm, game::pml_t* pml)>				PM_GroundTrace_Internal = 0x410660;
	static utils::function<void(std::int32_t nodeIndex, game::areaParms_t* ap)>		CM_AreaEntities = 0x4F7A80;

	//double PM_CmdScale_Walk(Game::pmove_t* pm, Game::usercmd_s* cmd /*ecx*/); // ASM // broken on release
	bool Jump_Check(game::pmove_t* pm /*eax*/, game::pml_t* pml); // ASM
	void PM_Friction(game::playerState_s* ps, game::pml_t* pml); // ASM
	bool PM_CorrectAllSolid(game::pmove_t* pm /*eax*/, game::pml_t* pml, game::trace_t* trace); // ASM
	void PM_GroundTraceMissed(game::pmove_t* pm /*eax*/, game::pml_t* pml); // ASM
	void PM_CrashLand(game::playerState_s* ps /*esi*/, game::pml_t* pml); // ASM
	void PM_AddTouchEnt(game::pmove_t* pm /*eax*/, int entity_num /*edi*/); // ASM
	void PM_playerTrace(game::pmove_t* pm /*esi*/, game::trace_t* results, const float* start, const float* mins, const float* maxs, const float* end, std::int32_t pass_entity_num, std::int32_t content_mask); // ASM
	
	void  G_Damage(float* dir /*eax*/, game::gentity_s* targ, game::gentity_s* inflictor, game::gentity_s* attacker, float* point, int damage, int flags, int mod, int self_client, int hitloc, /*hitLocation_t*/ int a11, unsigned int a12, unsigned int time_offset); // ASM
	float CanDamage(game::gentity_s* inflictor, float* center_pos, game::gentity_s *ent, float cone_angle_cos, float* cone_direction, int contentmask); // ASM
	void  Scr_PlayerDamage(float* dir, game::gentity_s *targ, game::gentity_s *inflictor, game::gentity_s *attacker, int damage, int dflags, int mod, int weapon, float* point, game::hitLocation_t hit_loc, int time_offset); // ASM
	game::gentity_s* G_FireRocket(float* kick_back, game::gentity_s* ent, signed int weap_index, float* dir, float* gun_vel, game::gentity_s* target, float* target_offset); // ASM


	// *
	// dvars 

	dvar_s* Dvar_FindVar(const char* dvar);

	// --- properly register integer dvars ---
	void	Dvar_ReregisterInt(dvar_s* dvar /*eax*/, std::uint32_t flags /*edi*/, const char* dvar_name, DvarType type, const char* description, int x, int y, int z, int w, int min, int max); // ASM
	dvar_s* Dvar_RegisterIntWrapper_r(const char* dvar_name, DvarType type, std::uint16_t flags, const char* description, int x, int y, int z, int w, int min, int max);
	
	inline dvar_s* Dvar_RegisterIntWrapper(const char* dvarName, const char* description, int defaultValue, int minValue, int maxValue, std::uint16_t flags) { 
		return Dvar_RegisterIntWrapper_r(dvarName, DVAR_TYPE_INT, flags, description, defaultValue, 0, 0, 0, minValue, maxValue); 
	}

	inline dvar_s* Dvar_RegisterBoolWrapper(const char* dvarName, const char* description, int defaultValue, std::uint16_t flags) { 
		return Dvar_RegisterIntWrapper_r(dvarName, DVAR_TYPE_BOOL, flags, description, defaultValue, 0, 0, 0, 0, 0); 
	}
	// -----------------------------------------

	static utils::function<void(dvar_s* dvar, float value, int source)> 
		Dvar_SetFloat = 0x56C960;

	static utils::function<dvar_s* (const char* dvar_name, float default_value, float min_value, float max_value, std::uint16_t flags, const char *description)> 
		Dvar_RegisterFloat_r = 0x56C460;

	inline dvar_s* Dvar_RegisterFloat(const char* dvarName, const char* description, float defaultValue, float minValue, float maxValue, std::uint16_t flags) {
		return Dvar_RegisterFloat_r(dvarName, defaultValue, minValue, maxValue, flags, description);
	}

	// using RegisterNew - FLOAT / VEC2 / VEC3 / VEC4 (01)
	static utils::function<dvar_s * (const char* dvar_name, DvarType type_float2, std::uint16_t flags, const char* description, float x, float y, std::int32_t null1, std::int32_t null2, float min, float max)>
		Dvar_RegisterVec2_r = 0x56C130;

	inline dvar_s* Dvar_RegisterVec2(const char* dvar_name, const char* description, float x, float y, float min_value, float max_value, std::uint16_t flags) {
		return Dvar_RegisterVec2_r(dvar_name, DvarType::DVAR_TYPE_FLOAT_2, flags, description, x, y, 0, 0, min_value, max_value);
	}

	// using RegisterNew - FLOAT / VEC2 / VEC3 / VEC4 (01)
	static utils::function<dvar_s * (const char* dvar_name, DvarType type_float3, std::uint16_t flags, const char* description, float x, float y, float z, std::int32_t null, float min, float max)>
		Dvar_RegisterVec3_r = 0x56C130;

	inline dvar_s* Dvar_RegisterVec3(const char* dvar_name, const char* description, float x, float y, float z, float min_value, float max_value, std::uint16_t flags) {
		return Dvar_RegisterVec3_r(dvar_name, DvarType::DVAR_TYPE_FLOAT_3, flags, description, x, y, z, 0, min_value, max_value);
	}

	// using RegisterNew - FLOAT / VEC2 / VEC3 / VEC4 (01)
	static utils::function<dvar_s* (const char *dvar_name, DvarType type_float4, std::uint16_t flags, const char *description, float x, float y, float z, float w, float min, float max)>
		Dvar_RegisterVec4_r = 0x56C130;

	inline dvar_s* Dvar_RegisterVec4(const char* dvar_name, const char* description, float x, float y, float z, float w, float min_value, float max_value, std::uint16_t flags) {
		return Dvar_RegisterVec4_r(dvar_name, DvarType::DVAR_TYPE_FLOAT_4, flags, description, x, y, z, w, min_value, max_value);
	}

	// using RegisterNew - INT / BOOL (02)
	static utils::function<dvar_s* (const char *dvarName, DvarType typeInt, std::uint16_t flags, const char *description, std::int32_t defaultValue, std::int32_t null1, std::int32_t null2, std::int32_t null3, std::int32_t minValue, std::int32_t maxValue)>
		Dvar_RegisterInt_r = 0x56C130;

	inline dvar_s* Dvar_RegisterInt(const char* dvar_name, const char* description, std::int32_t default_value, std::int32_t min_value, std::int32_t max_value, std::uint16_t flags) {
		return Dvar_RegisterInt_r(dvar_name, DvarType::DVAR_TYPE_INT, flags, description, default_value, 0, 0, 0, min_value, max_value);
	}

	// using RegisterNew - INT / BOOL (02)
	static utils::function<dvar_s* (const char *dvar_name, DvarType type_bool, std::uint16_t flags, const char *description, std::int32_t default_value, std::int32_t null1, std::int32_t null2, std::int32_t null3, std::int32_t null4, std::int32_t null5)>
		Dvar_RegisterBool_r = 0x56C130;

	inline dvar_s* Dvar_RegisterBool(const char* dvar_name, const char* description, std::int32_t default_value, std::uint16_t flags) {
		return Dvar_RegisterBool_r(dvar_name, DvarType::DVAR_TYPE_BOOL, flags, description, default_value, 0, 0, 0, 0, 0);
	}

	// using RegisterNew - ENUM (04)
	static utils::function<dvar_s* (const char *dvar_name, DvarType type_enum, std::uint16_t flags, const char *description, std::int32_t default_index, std::int32_t null1, std::int32_t null2, std::int32_t null3, std::int32_t enumSize, const char** enum_data)>
		Dvar_RegisterEnum_r = 0x56C130;

	inline dvar_s* Dvar_RegisterEnum(const char* dvar_name, const char* description, std::int32_t default_value, std::int32_t enum_size, const char** enum_data, std::uint16_t flags) {
		return Dvar_RegisterEnum_r(dvar_name, DvarType::DVAR_TYPE_ENUM, flags, description, default_value, 0, 0, 0, enum_size, enum_data);
	}

	// using RegisterNew - STRING (03) (cannot be used on module load - make sure to check if dvar exists already or it will be added again)
	static utils::function<dvar_s * (const char* dvar_name, DvarType type_string, std::uint16_t flags, const char* description, const char* default_value, std::int32_t null1, std::int32_t null2, std::int32_t null3, std::int32_t null4, std::int32_t null5)>
		Dvar_RegisterString_r = 0x56C130;

	// * do not use on module load (crash)
	inline dvar_s* Dvar_RegisterString(const char* dvar_name, const char* description, const char* default_value, std::uint16_t flags) 
	{
		const auto dvar = game::Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return Dvar_RegisterString_r(dvar_name, DvarType::DVAR_TYPE_STRING, flags, description, default_value, 0, 0, 0, 0, 0);
		}

		return dvar;
	}

	static utils::function<game::dvar_s* (const char* dvar_name, DvarType type, std::uint16_t flags, const char* description, int x, int y, int z, int w, int min, int max)>
		Dvar_RegisterNew = 0x56C130;

	void dvar_set_value_dirty(dvar_s* dvar, int value);
	void dvar_set_value_dirty(dvar_s* dvar, bool value);
	void dvar_set_value_dirty(dvar_s* dvar, float dvar_value);
	void dvar_set_value_dirty(dvar_s* dvar, const float* dvar_value, int size);
	void dvar_set_value_dirty(dvar_s* dvar, const char* dvar_value);

	game::dvar_s*	Dvar_RegisterString_hacky(const char* dvar_name, const char* dvar_value, const char* description);
	const char*		Dvar_EnumToString(const dvar_s* dvar);

	void Dvar_SetString(const char* text /*eax*/, dvar_s* dvar /*esi*/); //ASM
	

	// *
	// console 

	extern float* con_matchtxtColor_currentDvar;
	extern float* con_matchtxtColor_currentValue;
	extern float* con_matchtxtColor_defaultValue;
	extern float* con_matchtxtColor_dvarDescription;
	extern float* con_matchtxtColor_domainDescription;

	// console structs
	extern ConDrawInputGlob*	conDrawInputGlob;
	extern Console*				con;
	extern field_t*				g_consoleField;

	// console variables
	static DWORD* con_font_ptr = (DWORD*)(0xC5AE94);
	extern bool*  Key_IsCatcherActive;

	// con->screenMin / Max manual defines for use in asm
	extern float*	conScreenMin0; // left
	extern float*	conScreenMin1; // top
	extern float*	conScreenMax0; // right
	extern float*	conScreenMax1; // bottom

	extern bool*	extvar_con_ignoreMatchPrefixOnly;

	extern cmd_function_s* cmd_functions;

	static utils::function<bool(const char* query, const char* match_to_text, int match_text_len)>
		Con_IsAutoCompleteMatch = 0x45F990;

	typedef void(*SCR_DrawSmallStringExt_t)(int x, int y, const char* text);
		extern SCR_DrawSmallStringExt_t SCR_DrawSmallStringExt;

	typedef bool(*Sys_IsMainThread_t)();
		extern Sys_IsMainThread_t Sys_IsMainThread;

	typedef char*(*Con_TokenizeInput_t)();
		extern Con_TokenizeInput_t Con_TokenizeInput;

	typedef void(*Con_CancelAutoComplete_t)();
		extern Con_CancelAutoComplete_t Con_CancelAutoComplete;

	typedef void(*Con_DrawInputPrompt_t)();
		extern Con_DrawInputPrompt_t Con_DrawInputPrompt;

	typedef void(*Cmd_EndTokenizedString_t)();
		extern Cmd_EndTokenizedString_t Cmd_EndTokenizedString;

	typedef void(*ConDrawInput_IncrMatchCounter_t)(const char *);
		extern ConDrawInput_IncrMatchCounter_t ConDrawInput_IncrMatchCounter;

	typedef void(*Con_DrawInputPrompt_t)();
		extern Con_DrawInputPrompt_t Con_DrawInputPrompt;
	
	typedef bool(*Con_AnySpaceAfterCommand_t)();
		extern Con_AnySpaceAfterCommand_t Con_AnySpaceAfterCommand;

	typedef void(__cdecl *ConDrawInput_DetailedDvarMatch_t)(const char *);
		extern ConDrawInput_DetailedDvarMatch_t ConDrawInput_DetailedDvarMatch;
	
	typedef void(__cdecl *ConDrawInput_DetailedCmdMatch_t)(const char *);
		extern ConDrawInput_DetailedCmdMatch_t ConDrawInput_DetailedCmdMatch;

	typedef void(__cdecl *ConDrawInput_DvarMatch_t)(const char *);
		extern ConDrawInput_DvarMatch_t ConDrawInput_DvarMatch;

	typedef void(__cdecl *ConDrawInput_CmdMatch_t)(const char *);
		extern ConDrawInput_CmdMatch_t ConDrawInput_CmdMatch;

	typedef void(*Con_DrawOutputScrollBar_t)(float x, float y, float width, float height); 
		extern Con_DrawOutputScrollBar_t Con_DrawOutputScrollBar;

	typedef void(*Con_DrawOutputText_t)(float x, float y);
		extern Con_DrawOutputText_t Con_DrawOutputText;

	void Con_DrawMessageWindowOldToNew(DWORD* msg_window /*esi*/, int local_client_num, int x_pos, int y_pos, int char_height, int horz_align, int vert_align, int mode, Font_s* font, const float* color, int text_style, float msgwnd_scale, int text_align_mode); // ASM
	void Cmd_ForEachXO(void(__cdecl* callback)(const char *));
	void Cmd_ForEach_PassCmd(void(__cdecl* callback)(cmd_function_s*));

	void AddBaseDrawConsoleTextCmd(int char_count /*eax*/, const float* color_float /*ecx*/, const char* text_pool, int pool_size, int first_char, game::Font_s* font, float x, float y, float x_scale, float y_scale, int style); // ASM
	void ConDrawInput_Box(float* color, int line_height_multi); // ASM
	void ConDrawInput_TextAndOver(char* text); // ASM
	void ConDraw_Box(float* color, float x, float y, float width, float height); // ASM
	void Con_DrawAutoCompleteChoice(int is_dvar_cmd, char* text); // ASM

	char* Cmd_Argv(int arg_index); // ASM
	void  Dvar_ForEachName(void(__cdecl* func)(const char *)); // ASM
	void  Cmd_ForEach(void(__cdecl* func)(const char *)); // ASM
	

	// *
	// anims
	
	void BG_AnimScriptEvent(scriptAnimEventTypes_t event, game::playerState_s* ps, int force); // ASM
	

	// *
	// common

	extern game::playerState_s* ps_loc;
	extern game::pmove_t* pmove;

	extern game::CmdArgs*	cmd_args;
	extern cmd_function_s** cmd_ptr;
	extern XZone*			g_zones;
	extern XAssetEntry*		g_assetEntryPool;
	extern unsigned short*	db_hashTable;
	extern infoParm_t*		infoParams;

	typedef void(*Cmd_ExecuteSingleCommand_t)(int controller, int a2, const char* cmd);
		extern Cmd_ExecuteSingleCommand_t Cmd_ExecuteSingleCommand;

	typedef void(*Com_Error_t)(int type, const char* message, ...);
		extern Com_Error_t Com_Error;

	typedef void(*Com_PrintMessage_t)(int, const char*, char);
		extern Com_PrintMessage_t Com_PrintMessage;

	typedef XAssetHeader(*DB_FindXAssetHeader_t)(XAssetType type, const char* name);
		extern DB_FindXAssetHeader_t DB_FindXAssetHeader;

	typedef void(*DB_EnumXAssets_FastFile_t)(XAssetType type, void(*)(XAssetHeader, void*), void* userdata, bool overrides);
		extern DB_EnumXAssets_FastFile_t DB_EnumXAssets_FastFile;

	typedef const char* (*DB_GetXAssetNameHandler_t)(XAssetHeader* asset);
		extern DB_GetXAssetNameHandler_t* DB_GetXAssetNameHandlers;

    typedef void(*DB_LoadXAssets_t)(XZoneInfo *zoneInfo, unsigned int zone_count, int sync);
		extern DB_LoadXAssets_t DB_LoadXAssets;

	void Cbuf_AddText(const char* text /*eax*/, int local_client_num /*ecx*/);
	void Cmd_AddCommand(const char* name, void(*callback)(), cmd_function_s* data, char);
	void Cmd_AddCommand(const char* name, const char* args, const char* description, void(*callback)(), cmd_function_s* data, char);
	const char* SL_ConvertToString(int idx);

	game::PackedUnitVec Vec3PackUnitVec(const float *unitVec);
	void Vec3UnpackUnitVec(game::PackedUnitVec in, const float *out);

	void Byte4UnpackRgba(unsigned __int8* from, float* to);
	char Byte1PackClamp(const float from);
	void Byte4PackRgba(const float* from, char* to);
	void Byte4PackPixelColor(const float* from, char* to);

	void AxisToAngles(float* angles /*eax*/, const float(*axis)[3] /*ecx*/);


	// *
	// cgaz - Port from mDd client Proxymod (https://github.com/Jelvan1/cgame_proxymod)

	void CG_FillAngleYaw(float start, float end, float yaw, float y, float h, float color[4]);
	void CG_DrawLineYaw(float angle, float yaw, float y, float w, float h, float color[4]);
}
