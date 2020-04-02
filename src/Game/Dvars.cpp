#include "STDInclude.hpp"

namespace Dvars
{
	// Radiant -------------------------------------------
	Game::dvar_s* radiant_live = nullptr;
	Game::dvar_s* radiant_livePort = nullptr;
	Game::dvar_s* radiant_liveDebug = nullptr;
	Game::dvar_s* radiant_syncCamera = nullptr;
	Game::dvar_s* radiant_brushCollision = nullptr;
	Game::dvar_s* radiant_brushColor = nullptr;
	Game::dvar_s* radiant_brushLit = nullptr;
	Game::dvar_s* radiant_brushWireframe = nullptr;
	Game::dvar_s* radiant_brushWireframeColor = nullptr;

	// Display -------------------------------------------
	Game::dvar_s* r_aspectRatio = nullptr;
	Game::dvar_s* ui_ultrawide = nullptr;
	Game::dvar_s* ui_button_highlight_radius = nullptr;
	Game::dvar_s* ui_button_highlight_brightness = nullptr;
	Game::dvar_s* ui_button_outline_radius = nullptr;
	Game::dvar_s* ui_button_outline_brightness = nullptr;
	Game::dvar_s* xo_menu_dbg = nullptr;
	Game::dvar_s* ui_subnavbar = nullptr;
	Game::dvar_s* ui_subnavbar_item = nullptr;
	Game::dvar_s* ui_renderWidth = nullptr;
	Game::dvar_s* ui_renderHeight = nullptr;

	Game::dvar_s* ui_devmap = nullptr;
	Game::dvar_s* ui_developer = nullptr;
	Game::dvar_s* ui_developer_script = nullptr;

	Game::dvar_s* ui_main_title = nullptr;

	// Console -------------------------------------------

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

	// Custom Movement -----------------------------------
	Game::dvar_s* pm_movementType = nullptr;
	Game::dvar_s* pm_crashland = nullptr;
	Game::dvar_s* pm_rocketJumpHeight = nullptr;
	Game::dvar_s* pm_disableSprint = nullptr;
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

	Game::dvar_s* pm_debug_prints = nullptr;
	Game::dvar_s* pm_debug_traceOrigin = nullptr;
	Game::dvar_s* pm_debug_traceVelocity = nullptr;
	Game::dvar_s* pm_debug_traceVelocityRealtime = nullptr;
	Game::dvar_s* pm_debug_lineColor = nullptr;
	Game::dvar_s* pm_debug_lineDepth = nullptr;
	Game::dvar_s* pm_debug_lineDuration = nullptr;


	// Debug Collision ------------------------------------
	Game::dvar_s* r_drawCollision = nullptr;
	Game::dvar_s* r_drawCollision_brushAmount = nullptr;
	Game::dvar_s* r_drawCollision_brushBegin = nullptr;
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

#if DEBUG
	Game::dvar_s* r_drawCollision_brushDebug = nullptr; 
#endif

#ifdef EXP_MAP_EXPORT
	
	Game::dvar_s* r_drawCollision_export = nullptr;
	Game::dvar_s* r_drawCollision_export_writeTriangles = nullptr;
	Game::dvar_s* r_drawCollision_export_writeQuads = nullptr;
	Game::dvar_s* r_drawCollision_export_writeEntities = nullptr;
	Game::dvar_s* r_drawCollision_export_writeModels = nullptr;
	Game::dvar_s* r_drawCollision_flickerBrushes = nullptr;
	Game::dvar_s* r_drawCollision_flickerOnTime = nullptr;
	Game::dvar_s* r_drawCollision_flickerOffTime = nullptr;
#endif

	Game::dvar_s* r_drawCollision_hud = nullptr;
	Game::dvar_s* r_drawCollision_hud_x = nullptr;
	Game::dvar_s* r_drawCollision_hud_y = nullptr;
	Game::dvar_s* r_drawCollision_hud_fontScale = nullptr;
	Game::dvar_s* r_drawCollision_hud_fontStyle = nullptr;
	Game::dvar_s* r_drawCollision_hud_fontColor = nullptr;


	// Shader Overlays ------------------------------------
	Game::dvar_s* xo_shaderoverlay = nullptr;
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
}