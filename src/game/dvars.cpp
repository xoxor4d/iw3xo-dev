#include "std_include.hpp"

namespace dvars
{
	// Client
	game::dvar_s* cl_avidemo_streams = nullptr;
	game::dvar_s* cl_avidemo_streams_greenscreen = nullptr;
	game::dvar_s* cl_avidemo_streams_viewmodel = nullptr;
	game::dvar_s* cl_avidemo_streams_depth = nullptr;
	game::dvar_s* cl_avidemo_streams_hud = nullptr;
	game::dvar_s* cl_pause_demo = nullptr;
	game::dvar_s* load_iw3mvm = nullptr;


	// Radiant
	game::dvar_s* radiant_live = nullptr;
	game::dvar_s* radiant_livePort = nullptr;
	game::dvar_s* radiant_liveDebug = nullptr;
	game::dvar_s* radiant_syncCamera = nullptr;
	game::dvar_s* radiant_brushCollision = nullptr;
	game::dvar_s* radiant_brushColor = nullptr;
	game::dvar_s* radiant_brushLit = nullptr;
	game::dvar_s* radiant_brushWireframe = nullptr;
	game::dvar_s* radiant_brushWireframeColor = nullptr;


	// Menu
	game::dvar_s* ui_ultrawide = nullptr;
	game::dvar_s* ui_button_highlight_radius = nullptr;
	game::dvar_s* ui_button_highlight_brightness = nullptr;
	game::dvar_s* ui_button_outline_radius = nullptr;
	game::dvar_s* ui_button_outline_brightness = nullptr;
	game::dvar_s* ui_subnavbar = nullptr;
	game::dvar_s* ui_subnavbar_item = nullptr;
	game::dvar_s* ui_renderWidth = nullptr;
	game::dvar_s* ui_renderHeight = nullptr;
	game::dvar_s* ui_devmap = nullptr;
	game::dvar_s* ui_developer = nullptr;
	game::dvar_s* ui_developer_script = nullptr;
	game::dvar_s* ui_main_title = nullptr;
	game::dvar_s* ui_eyes_position = nullptr;
	game::dvar_s* ui_eyes_size = nullptr;
	game::dvar_s* ui_eyes_alpha = nullptr;
	game::dvar_s* xo_menu_dbg = nullptr;

	// ImGui
	game::dvar_s* _imgui_saving = nullptr;
	game::dvar_s* _imgui_window_alpha = nullptr;

	// Devgui
	game::dvar_s* _imgui_devgui_pos = nullptr;
	game::dvar_s* _imgui_devgui_size = nullptr;

	// Console 
	game::dvar_s* con_minicon_position = nullptr;
	game::dvar_s* con_minicon_font = nullptr;
	game::dvar_s* con_minicon_fontHeight = nullptr;
	game::dvar_s* con_minicon_fontColor = nullptr;
	game::dvar_s* con_minicon_fontStyle = nullptr;
	game::dvar_s* xo_con_fltCon = nullptr;
	game::dvar_s* xo_con_fltConLeft = nullptr;
	game::dvar_s* xo_con_fltConTop = nullptr;
	game::dvar_s* xo_con_fltConRight = nullptr;
	game::dvar_s* xo_con_fltConBottom = nullptr;
	game::dvar_s* xo_con_outputHeight = nullptr;
	game::dvar_s* xo_con_maxMatches = nullptr;
	game::dvar_s* xo_con_useDepth = nullptr;
	game::dvar_s* xo_con_fontSpacing = nullptr;
	game::dvar_s* xo_con_padding = nullptr;
	game::dvar_s* xo_con_cursorOverdraw = nullptr;
	game::dvar_s* xo_con_cursorState = nullptr;
	game::dvar_s* xo_con_hintBoxTxtColor_currentDvar = nullptr;
	game::dvar_s* xo_con_hintBoxTxtColor_currentValue = nullptr;
	game::dvar_s* xo_con_hintBoxTxtColor_defaultValue = nullptr;
	game::dvar_s* xo_con_hintBoxTxtColor_dvarDescription = nullptr;
	game::dvar_s* xo_con_hintBoxTxtColor_domainDescription = nullptr; 


	// Custom movement
	game::dvar_s* pm_movementType = nullptr;
	game::dvar_s* pm_crashland = nullptr;
	game::dvar_s* pm_rocketJumpHeight = nullptr;
	game::dvar_s* pm_disableSprint = nullptr;
	game::dvar_s* pm_terrainEdgeBounces = nullptr;
	game::dvar_s* pm_bhop_auto = nullptr;
	game::dvar_s* pm_bhop_slowdown = nullptr;
	game::dvar_s* pm_cpm_useQuakeDamage = nullptr;
	game::dvar_s* pm_cpm_damageKnockback = nullptr;
	game::dvar_s* pm_cpm_useBouncing = nullptr;
	game::dvar_s* pm_cpm_airControl = nullptr;
	game::dvar_s* pm_cpm_airAccelerate = nullptr;
	game::dvar_s* pm_cpm_airstopAccelerate = nullptr;
	game::dvar_s* pm_cpm_strafeAccelerate = nullptr;
	game::dvar_s* pm_cs_airAccelerate = nullptr;
	game::dvar_s* pm_cs_airspeedCap = nullptr;

	game::dvar_s* pm_hud_enable = nullptr;
	game::dvar_s* pm_hud_x = nullptr;
	game::dvar_s* pm_hud_y = nullptr;
	game::dvar_s* pm_hud_fontScale = nullptr;
	game::dvar_s* pm_hud_fontStyle = nullptr;
	game::dvar_s* pm_hud_fontColor = nullptr;

	game::dvar_s* pm_origin_hud = nullptr;
	game::dvar_s* pm_origin_hud_x = nullptr;
	game::dvar_s* pm_origin_hud_y = nullptr;
	game::dvar_s* pm_origin_hud_fontScale = nullptr;
	game::dvar_s* pm_origin_hud_fontStyle = nullptr;
	game::dvar_s* pm_origin_hud_fontColor = nullptr;

	game::dvar_s* pm_debug_prints = nullptr;
	game::dvar_s* pm_debug_traceOrigin = nullptr;
	game::dvar_s* pm_debug_traceVelocity = nullptr;
	game::dvar_s* pm_debug_lineColor = nullptr;
	game::dvar_s* pm_debug_lineDepth = nullptr;
	game::dvar_s* pm_debug_lineDuration = nullptr;
	game::dvar_s* pm_debug_lineWidth = nullptr;
	game::dvar_s* pm_debug_drawAxis = nullptr;
	game::dvar_s* pm_debug_drawAxis_radius = nullptr;
	game::dvar_s* pm_debug_drawAxis_height = nullptr;
	game::dvar_s* pm_debug_drawAxis_col125 = nullptr;
	game::dvar_s* pm_debug_drawAxis_col250 = nullptr;
	game::dvar_s* pm_debug_drawAxis_col333 = nullptr;


	// Renderer
	game::dvar_s* r_aspectRatio_custom = nullptr;
	game::dvar_s* r_aspectRatio = nullptr;
	game::dvar_s* r_noborder = nullptr;
	game::dvar_s* r_d3d9ex = nullptr;
	game::dvar_s* r_dumpShaders = nullptr;

	game::dvar_s* r_buf_skinnedCacheVb = nullptr;
	game::dvar_s* r_buf_smodelCacheVb = nullptr;
	game::dvar_s* r_buf_smodelCacheIb = nullptr;
	game::dvar_s* r_buf_tempSkin = nullptr;
	game::dvar_s* r_buf_dynamicVertexBuffer = nullptr;
	game::dvar_s* r_buf_dynamicIndexBuffer = nullptr;
	game::dvar_s* r_buf_preTessIndexBuffer = nullptr;

	game::dvar_s* r_debugShaderToggle = nullptr;
	game::dvar_s* r_setmaterial_hk = nullptr;

	game::dvar_s* r_wireframe_world = nullptr;
	game::dvar_s* r_wireframe_xmodels = nullptr;

	game::dvar_s* r_cullWorld = nullptr;
	game::dvar_s* r_cullEntities = nullptr;
	game::dvar_s* r_drawDynents = nullptr;

	game::dvar_s* r_debugShaderTexcoord = nullptr;
	game::dvar_s* r_dayAndNight = nullptr;

	game::dvar_s* rtx_hacks = nullptr;

	game::dvar_s* r_fogTweaks = nullptr;
	game::dvar_s* r_fogTweaksColor = nullptr;
	game::dvar_s* r_fogTweaksStart = nullptr;
	game::dvar_s* r_fogTweaksDensity = nullptr;

	game::dvar_s* cg_fov_gun = nullptr;
	game::dvar_s* cg_fov_tweaks = nullptr;
	
	// Debug Collision
	game::dvar_s* r_drawCollision = nullptr;
	game::dvar_s* r_drawCollision_brushAmount = nullptr;
	game::dvar_s* r_drawCollision_brushDist = nullptr;
	game::dvar_s* r_drawCollision_brushSorting = nullptr;
	game::dvar_s* r_drawCollision_brushIndexFilter = nullptr;
	game::dvar_s* r_drawCollision_brushIndexVisible = nullptr;
	game::dvar_s* r_drawCollision_material = nullptr;
	game::dvar_s* r_drawCollision_materialInclude = nullptr;
	game::dvar_s* r_drawCollision_materialList = nullptr;
	game::dvar_s* r_drawCollision_lineColor = nullptr;
	game::dvar_s* r_drawCollision_lineWidth = nullptr;
	game::dvar_s* r_drawCollision_polyAlpha = nullptr;
	game::dvar_s* r_drawCollision_polyDepth = nullptr;
	game::dvar_s* r_drawCollision_polyFace = nullptr;
	game::dvar_s* r_drawCollision_polyLit = nullptr;
	game::dvar_s* r_drawCollision_hud = nullptr;
	game::dvar_s* r_drawCollision_hud_position = nullptr;
	game::dvar_s* r_drawCollision_hud_fontScale = nullptr;
	game::dvar_s* r_drawCollision_hud_fontStyle = nullptr;
	game::dvar_s* r_drawCollision_hud_fontColor = nullptr;

#if DEBUG
	game::dvar_s* r_drawCollision_brushDebug = nullptr; 
#endif


	// Map exporting
	game::dvar_s* mapexport_brushEpsilon1 = nullptr;
	game::dvar_s* mapexport_brushEpsilon2 = nullptr;
	game::dvar_s* mapexport_brushMinSize = nullptr;
	game::dvar_s* mapexport_brush5Sides = nullptr;
	game::dvar_s* mapexport_selectionMode = nullptr;
	game::dvar_s* mapexport_writeTriangles = nullptr;
	game::dvar_s* mapexport_writeQuads = nullptr;
	game::dvar_s* mapexport_writeEntities = nullptr;
	game::dvar_s* mapexport_writeModels = nullptr;
	game::dvar_s* mapexport_writeDynModels = nullptr;

	game::dvar_s* mpsp_require_gsc = nullptr;

	// Shader Overlays
	game::dvar_s* xo_shaderoverlay = nullptr;
	game::dvar_s* xo_shaderoverlay_custom = nullptr;
	game::dvar_s* xo_ssao_noisescale = nullptr;
	game::dvar_s* xo_ssao_quality = nullptr;
	game::dvar_s* xo_ssao_radius = nullptr;
	game::dvar_s* xo_ssao_debugnormal = nullptr;
	game::dvar_s* xo_ssao_contrast = nullptr;
	game::dvar_s* xo_ssao_attenuation = nullptr;
	game::dvar_s* xo_ssao_angleBias = nullptr;
	game::dvar_s* xo_outliner_scale = nullptr;
	game::dvar_s* xo_outliner_depthDiffScale = nullptr;
	game::dvar_s* xo_outliner_depthThreshold = nullptr;
	game::dvar_s* xo_outliner_toonEnable = nullptr;
	game::dvar_s* xo_outliner_toonShades = nullptr;

	// Random stock
	game::dvar_s* snaps = nullptr;
	game::dvar_s* cg_fovScale = nullptr;

	// Compass
	game::dvar_s* mdd_compass = nullptr;
	game::dvar_s* mdd_compass_yh = nullptr;
	game::dvar_s* mdd_compass_quadrant_rgbas0 = nullptr;
	game::dvar_s* mdd_compass_quadrant_rgbas1 = nullptr;
	game::dvar_s* mdd_compass_quadrant_rgbas2 = nullptr;
	game::dvar_s* mdd_compass_quadrant_rgbas3 = nullptr;
	game::dvar_s* mdd_compass_ticks_rgba = nullptr;

	// CGaz
	game::dvar_s* mdd_cgaz = nullptr;
	game::dvar_s* mdd_cgaz_ground = nullptr;
	game::dvar_s* mdd_cgaz_min_speed = nullptr;
	game::dvar_s* mdd_cgaz_yh = nullptr;
	game::dvar_s* mdd_cgaz_rgbaNoAccel = nullptr;
	game::dvar_s* mdd_cgaz_rgbaPartialAccel = nullptr;
	game::dvar_s* mdd_cgaz_rgbaFullAccel = nullptr;
	game::dvar_s* mdd_cgaz_rgbaTurnZone = nullptr;
}