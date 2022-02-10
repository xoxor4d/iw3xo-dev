#include "STDInclude.hpp"

namespace Dvars
{
	// Client
	Game::dvar_s* cl_avidemo_streams = nullptr;
	Game::dvar_s* cl_avidemo_streams_greenscreen = nullptr;
	Game::dvar_s* cl_avidemo_streams_viewmodel = nullptr;
	Game::dvar_s* cl_avidemo_streams_depth = nullptr;
	Game::dvar_s* cl_avidemo_streams_hud = nullptr;
	Game::dvar_s* cl_pause_demo = nullptr;
	Game::dvar_s* load_iw3mvm = nullptr;


	// Radiant
	Game::dvar_s* radiant_live = nullptr;
	Game::dvar_s* radiant_livePort = nullptr;
	Game::dvar_s* radiant_liveDebug = nullptr;
	Game::dvar_s* radiant_syncCamera = nullptr;
	Game::dvar_s* radiant_brushCollision = nullptr;
	Game::dvar_s* radiant_brushColor = nullptr;
	Game::dvar_s* radiant_brushLit = nullptr;
	Game::dvar_s* radiant_brushWireframe = nullptr;
	Game::dvar_s* radiant_brushWireframeColor = nullptr;


	// Menu
	Game::dvar_s* ui_ultrawide = nullptr;
	Game::dvar_s* ui_button_highlight_radius = nullptr;
	Game::dvar_s* ui_button_highlight_brightness = nullptr;
	Game::dvar_s* ui_button_outline_radius = nullptr;
	Game::dvar_s* ui_button_outline_brightness = nullptr;
	Game::dvar_s* ui_subnavbar = nullptr;
	Game::dvar_s* ui_subnavbar_item = nullptr;
	Game::dvar_s* ui_renderWidth = nullptr;
	Game::dvar_s* ui_renderHeight = nullptr;
	Game::dvar_s* ui_devmap = nullptr;
	Game::dvar_s* ui_developer = nullptr;
	Game::dvar_s* ui_developer_script = nullptr;
	Game::dvar_s* ui_main_title = nullptr;
	Game::dvar_s* ui_eyes_position = nullptr;
	Game::dvar_s* ui_eyes_size = nullptr;
	Game::dvar_s* ui_eyes_alpha = nullptr;
	Game::dvar_s* xo_menu_dbg = nullptr;

	// ImGui
	Game::dvar_s* _imgui_saving = nullptr;
	Game::dvar_s* _imgui_window_alpha = nullptr;

	// Devgui
	Game::dvar_s* _imgui_devgui_pos = nullptr;
	Game::dvar_s* _imgui_devgui_size = nullptr;

	// Console 
	Game::dvar_s* con_minicon_position = nullptr;
	Game::dvar_s* con_minicon_font = nullptr;
	Game::dvar_s* con_minicon_fontHeight = nullptr;
	Game::dvar_s* con_minicon_fontColor = nullptr;
	Game::dvar_s* con_minicon_fontStyle = nullptr;
	Game::dvar_s* xo_con_fltCon = nullptr;
	Game::dvar_s* xo_con_fltConLeft = nullptr;
	Game::dvar_s* xo_con_fltConTop = nullptr;
	Game::dvar_s* xo_con_fltConRight = nullptr;
	Game::dvar_s* xo_con_fltConBottom = nullptr;
	Game::dvar_s* xo_con_outputHeight = nullptr;
	Game::dvar_s* xo_con_maxMatches = nullptr;
	Game::dvar_s* xo_con_useDepth = nullptr;
	Game::dvar_s* xo_con_fontSpacing = nullptr;
	Game::dvar_s* xo_con_padding = nullptr;
	Game::dvar_s* xo_con_cursorOverdraw = nullptr;
	Game::dvar_s* xo_con_cursorState = nullptr;
	Game::dvar_s* xo_con_hintBoxTxtColor_currentDvar = nullptr;
	Game::dvar_s* xo_con_hintBoxTxtColor_currentValue = nullptr;
	Game::dvar_s* xo_con_hintBoxTxtColor_defaultValue = nullptr;
	Game::dvar_s* xo_con_hintBoxTxtColor_dvarDescription = nullptr;
	Game::dvar_s* xo_con_hintBoxTxtColor_domainDescription = nullptr; 


	// Custom movement
	Game::dvar_s* pm_movementType = nullptr;
	Game::dvar_s* pm_crashland = nullptr;
	Game::dvar_s* pm_rocketJumpHeight = nullptr;
	Game::dvar_s* pm_disableSprint = nullptr;
	Game::dvar_s* pm_terrainEdgeBounces = nullptr;
	Game::dvar_s* pm_bhop_auto = nullptr;
	Game::dvar_s* pm_bhop_slowdown = nullptr;
	Game::dvar_s* pm_cpm_useQuakeDamage = nullptr;
	Game::dvar_s* pm_cpm_damageKnockback = nullptr;
	Game::dvar_s* pm_cpm_useBouncing = nullptr;
	Game::dvar_s* pm_cpm_airControl = nullptr;
	Game::dvar_s* pm_cpm_airAccelerate = nullptr;
	Game::dvar_s* pm_cpm_airstopAccelerate = nullptr;
	Game::dvar_s* pm_cpm_strafeAccelerate = nullptr;
	Game::dvar_s* pm_cs_airAccelerate = nullptr;
	Game::dvar_s* pm_cs_airspeedCap = nullptr;

	Game::dvar_s* pm_hud_enable = nullptr;
	Game::dvar_s* pm_hud_x = nullptr;
	Game::dvar_s* pm_hud_y = nullptr;
	Game::dvar_s* pm_hud_fontScale = nullptr;
	Game::dvar_s* pm_hud_fontStyle = nullptr;
	Game::dvar_s* pm_hud_fontColor = nullptr;

	Game::dvar_s* pm_origin_hud = nullptr;
	Game::dvar_s* pm_origin_hud_x = nullptr;
	Game::dvar_s* pm_origin_hud_y = nullptr;
	Game::dvar_s* pm_origin_hud_fontScale = nullptr;
	Game::dvar_s* pm_origin_hud_fontStyle = nullptr;
	Game::dvar_s* pm_origin_hud_fontColor = nullptr;

	Game::dvar_s* pm_debug_prints = nullptr;
	Game::dvar_s* pm_debug_traceOrigin = nullptr;
	Game::dvar_s* pm_debug_traceVelocity = nullptr;
	Game::dvar_s* pm_debug_lineColor = nullptr;
	Game::dvar_s* pm_debug_lineDepth = nullptr;
	Game::dvar_s* pm_debug_lineDuration = nullptr;
	Game::dvar_s* pm_debug_lineWidth = nullptr;
	Game::dvar_s* pm_debug_drawAxis = nullptr;
	Game::dvar_s* pm_debug_drawAxis_radius = nullptr;
	Game::dvar_s* pm_debug_drawAxis_height = nullptr;
	Game::dvar_s* pm_debug_drawAxis_col125 = nullptr;
	Game::dvar_s* pm_debug_drawAxis_col250 = nullptr;
	Game::dvar_s* pm_debug_drawAxis_col333 = nullptr;


	// Renderer
	Game::dvar_s* r_aspectRatio_custom = nullptr;
	Game::dvar_s* r_aspectRatio = nullptr;
	Game::dvar_s* r_noborder = nullptr;
	Game::dvar_s* r_d3d9ex = nullptr;
	Game::dvar_s* r_dumpShaders = nullptr;

	Game::dvar_s* r_buf_skinnedCacheVb = nullptr;
	Game::dvar_s* r_buf_smodelCacheVb = nullptr;
	Game::dvar_s* r_buf_smodelCacheIb = nullptr;
	Game::dvar_s* r_buf_tempSkin = nullptr;
	Game::dvar_s* r_buf_dynamicVertexBuffer = nullptr;
	Game::dvar_s* r_buf_dynamicIndexBuffer = nullptr;
	Game::dvar_s* r_buf_preTessIndexBuffer = nullptr;

	Game::dvar_s* r_debugShaderToggle = nullptr;
	Game::dvar_s* r_setmaterial_hk = nullptr;

	Game::dvar_s* r_wireframe_world = nullptr;
	Game::dvar_s* r_wireframe_xmodels = nullptr;

	Game::dvar_s* r_cullWorld = nullptr;
	Game::dvar_s* r_cullEntities = nullptr;
	Game::dvar_s* r_drawDynents = nullptr;

	Game::dvar_s* r_debugShaderTexcoord = nullptr;
	Game::dvar_s* r_dayAndNight = nullptr;

	Game::dvar_s* r_fogTweaks = nullptr;
	Game::dvar_s* r_fogTweaksColor = nullptr;
	Game::dvar_s* r_fogTweaksStart = nullptr;
	Game::dvar_s* r_fogTweaksDensity = nullptr;

	Game::dvar_s* cg_fov_gun = nullptr;
	Game::dvar_s* cg_fov_tweaks = nullptr;
	
	// Debug Collision
	Game::dvar_s* r_drawCollision = nullptr;
	Game::dvar_s* r_drawCollision_brushAmount = nullptr;
	Game::dvar_s* r_drawCollision_brushDist = nullptr;
	Game::dvar_s* r_drawCollision_brushSorting = nullptr;
	Game::dvar_s* r_drawCollision_brushIndexFilter = nullptr;
	Game::dvar_s* r_drawCollision_brushIndexVisible = nullptr;
	Game::dvar_s* r_drawCollision_material = nullptr;
	Game::dvar_s* r_drawCollision_materialInclude = nullptr;
	Game::dvar_s* r_drawCollision_materialList = nullptr;
	Game::dvar_s* r_drawCollision_lineColor = nullptr;
	Game::dvar_s* r_drawCollision_lineWidth = nullptr;
	Game::dvar_s* r_drawCollision_polyAlpha = nullptr;
	Game::dvar_s* r_drawCollision_polyDepth = nullptr;
	Game::dvar_s* r_drawCollision_polyFace = nullptr;
	Game::dvar_s* r_drawCollision_polyLit = nullptr;
	Game::dvar_s* r_drawCollision_flickerBrushes = nullptr;
	Game::dvar_s* r_drawCollision_flickerOnTime = nullptr;
	Game::dvar_s* r_drawCollision_flickerOffTime = nullptr;
	Game::dvar_s* r_drawCollision_hud = nullptr;
	Game::dvar_s* r_drawCollision_hud_position = nullptr;
	Game::dvar_s* r_drawCollision_hud_fontScale = nullptr;
	Game::dvar_s* r_drawCollision_hud_fontStyle = nullptr;
	Game::dvar_s* r_drawCollision_hud_fontColor = nullptr;

#if DEBUG
	Game::dvar_s* r_drawCollision_brushDebug = nullptr; 
#endif


	// Map exporting
	Game::dvar_s* mapexport_brushEpsilon1 = nullptr;
	Game::dvar_s* mapexport_brushEpsilon2 = nullptr;
	Game::dvar_s* mapexport_brushMinSize = nullptr;
	Game::dvar_s* mapexport_brush5Sides = nullptr;
	Game::dvar_s* mapexport_selectionMode = nullptr;
	Game::dvar_s* mapexport_writeTriangles = nullptr;
	Game::dvar_s* mapexport_writeQuads = nullptr;
	Game::dvar_s* mapexport_writeEntities = nullptr;
	Game::dvar_s* mapexport_writeModels = nullptr;

	Game::dvar_s* mpsp_require_gsc = nullptr;

	// Shader Overlays
	Game::dvar_s* xo_shaderoverlay = nullptr;
	Game::dvar_s* xo_shaderoverlay_custom = nullptr;
	Game::dvar_s* xo_ssao_noisescale = nullptr;
	Game::dvar_s* xo_ssao_quality = nullptr;
	Game::dvar_s* xo_ssao_radius = nullptr;
	Game::dvar_s* xo_ssao_debugnormal = nullptr;
	Game::dvar_s* xo_ssao_debugTargets = nullptr;
	Game::dvar_s* xo_ssao_depthprepass = nullptr;
	Game::dvar_s* xo_ssao_contrast = nullptr;
	Game::dvar_s* xo_ssao_attenuation = nullptr;
	Game::dvar_s* xo_ssao_angleBias = nullptr;
	Game::dvar_s* xo_outliner_scale = nullptr;
	Game::dvar_s* xo_outliner_depthDiffScale = nullptr;
	Game::dvar_s* xo_outliner_depthThreshold = nullptr;
	Game::dvar_s* xo_outliner_toonEnable = nullptr;
	Game::dvar_s* xo_outliner_toonShades = nullptr;

	Game::dvar_s* xo_camDir0 = nullptr;
	Game::dvar_s* xo_camDir1 = nullptr;
	Game::dvar_s* xo_camDir2 = nullptr;

	Game::dvar_s* xo_shaderdbg_matrix = nullptr;


	// Random stock
	Game::dvar_s* snaps = nullptr;
	Game::dvar_s* cg_fovScale = nullptr;

	// Compass
	Game::dvar_s* mdd_compass = nullptr;
	Game::dvar_s* mdd_compass_yh = nullptr;
	Game::dvar_s* mdd_compass_quadrant_rgbas0 = nullptr;
	Game::dvar_s* mdd_compass_quadrant_rgbas1 = nullptr;
	Game::dvar_s* mdd_compass_quadrant_rgbas2 = nullptr;
	Game::dvar_s* mdd_compass_quadrant_rgbas3 = nullptr;
	Game::dvar_s* mdd_compass_ticks_rgba = nullptr;

	// CGaz
	Game::dvar_s* mdd_cgaz = nullptr;
	Game::dvar_s* mdd_cgaz_ground = nullptr;
	Game::dvar_s* mdd_cgaz_min_speed = nullptr;
	Game::dvar_s* mdd_cgaz_yh = nullptr;
	Game::dvar_s* mdd_cgaz_rgbaNoAccel = nullptr;
	Game::dvar_s* mdd_cgaz_rgbaPartialAccel = nullptr;
	Game::dvar_s* mdd_cgaz_rgbaFullAccel = nullptr;
	Game::dvar_s* mdd_cgaz_rgbaTurnZone = nullptr;
}