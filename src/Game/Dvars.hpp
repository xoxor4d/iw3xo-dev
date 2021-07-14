#pragma once

namespace Dvars
{
	// Client
	extern Game::dvar_s* cl_avidemo_streams;
	extern Game::dvar_s* cl_avidemo_streams_greenscreen;
	extern Game::dvar_s* cl_avidemo_streams_viewmodel;
	extern Game::dvar_s* cl_avidemo_streams_depth;
	extern Game::dvar_s* cl_avidemo_streams_hud;
	extern Game::dvar_s* cl_pause_demo;
	extern Game::dvar_s* load_iw3mvm;


	// Radiant
	extern Game::dvar_s* radiant_live;
	extern Game::dvar_s* radiant_livePort;
	extern Game::dvar_s* radiant_liveDebug;
	extern Game::dvar_s* radiant_syncCamera;
	extern Game::dvar_s* radiant_brushCollision;
	extern Game::dvar_s* radiant_brushColor;
	extern Game::dvar_s* radiant_brushLit;
	extern Game::dvar_s* radiant_brushWireframe;
	extern Game::dvar_s* radiant_brushWireframeColor;


	// Menu
	extern Game::dvar_s* ui_ultrawide;
	extern Game::dvar_s* ui_button_highlight_radius;
	extern Game::dvar_s* ui_button_highlight_brightness;
	extern Game::dvar_s* ui_button_outline_radius;
	extern Game::dvar_s* ui_button_outline_brightness;
	extern Game::dvar_s* ui_subnavbar;
	extern Game::dvar_s* ui_subnavbar_item;
	extern Game::dvar_s* ui_renderWidth;
	extern Game::dvar_s* ui_renderHeight;
	extern Game::dvar_s* ui_devmap;
	extern Game::dvar_s* ui_developer;
	extern Game::dvar_s* ui_developer_script;
	extern Game::dvar_s* ui_main_title;
	extern Game::dvar_s* ui_eyes_position;
	extern Game::dvar_s* ui_eyes_size;
	extern Game::dvar_s* ui_eyes_alpha;
	extern Game::dvar_s* xo_menu_dbg;


	// ImGui
	extern Game::dvar_s* _imgui_saving;
	extern Game::dvar_s* _imgui_window_alpha;


	// Devgui
	extern Game::dvar_s* _imgui_devgui_pos;
	extern Game::dvar_s* _imgui_devgui_size;


	// Console 
	extern Game::dvar_s* con_minicon_position;
	extern Game::dvar_s* con_minicon_font;
	extern Game::dvar_s* con_minicon_fontHeight;
	extern Game::dvar_s* con_minicon_fontColor;
	extern Game::dvar_s* con_minicon_fontStyle;

	extern Game::dvar_s* xo_con_fltCon;
	extern Game::dvar_s* xo_con_fltConLeft;
	extern Game::dvar_s* xo_con_fltConTop;
	extern Game::dvar_s* xo_con_fltConRight;
	extern Game::dvar_s* xo_con_fltConBottom;

	extern Game::dvar_s* xo_con_outputHeight;
	extern Game::dvar_s* xo_con_maxMatches;
	extern Game::dvar_s* xo_con_useDepth;
	extern Game::dvar_s* xo_con_fontSpacing;
	extern Game::dvar_s* xo_con_padding;

	extern Game::dvar_s* xo_con_cursorOverdraw;
	extern Game::dvar_s* xo_con_cursorState;

	extern Game::dvar_s* xo_con_hintBoxTxtColor_currentDvar;
	extern Game::dvar_s* xo_con_hintBoxTxtColor_currentValue;
	extern Game::dvar_s* xo_con_hintBoxTxtColor_defaultValue;
	extern Game::dvar_s* xo_con_hintBoxTxtColor_dvarDescription;
	extern Game::dvar_s* xo_con_hintBoxTxtColor_domainDescription;


	// Custom movement
	extern Game::dvar_s* pm_movementType;
	extern Game::dvar_s* pm_crashland;
	extern Game::dvar_s* pm_rocketJumpHeight;
	extern Game::dvar_s* pm_disableSprint;
	extern Game::dvar_s* pm_terrainEdgeBounces;
	extern Game::dvar_s* pm_bhop_auto;
	extern Game::dvar_s* pm_bhop_slowdown;
	extern Game::dvar_s* pm_cpm_useQuakeDamage;
	extern Game::dvar_s* pm_cpm_damageKnockback;
	extern Game::dvar_s* pm_cpm_useBouncing;
	extern Game::dvar_s* pm_cpm_airControl;
	extern Game::dvar_s* pm_cpm_airAccelerate;
	extern Game::dvar_s* pm_cpm_airstopAccelerate;
	extern Game::dvar_s* pm_cpm_strafeAccelerate;
	extern Game::dvar_s* pm_cs_airAccelerate;
	extern Game::dvar_s* pm_cs_airspeedCap;

	extern Game::dvar_s* pm_hud_enable;
	extern Game::dvar_s* pm_hud_x;
	extern Game::dvar_s* pm_hud_y;
	extern Game::dvar_s* pm_hud_fontScale;
	extern Game::dvar_s* pm_hud_fontStyle;
	extern Game::dvar_s* pm_hud_fontColor;

	extern Game::dvar_s* pm_origin_hud;
	extern Game::dvar_s* pm_origin_hud_x;
	extern Game::dvar_s* pm_origin_hud_y;
	extern Game::dvar_s* pm_origin_hud_fontScale;
	extern Game::dvar_s* pm_origin_hud_fontStyle;
	extern Game::dvar_s* pm_origin_hud_fontColor;

	extern Game::dvar_s* pm_debug_prints;
	extern Game::dvar_s* pm_debug_traceOrigin;
	extern Game::dvar_s* pm_debug_traceVelocity;
	extern Game::dvar_s* pm_debug_lineColor;
	extern Game::dvar_s* pm_debug_lineDepth;
	extern Game::dvar_s* pm_debug_lineDuration;
	extern Game::dvar_s* pm_debug_lineWidth;
	extern Game::dvar_s* pm_debug_drawAxis;
	extern Game::dvar_s* pm_debug_drawAxis_radius;
	extern Game::dvar_s* pm_debug_drawAxis_height;
	extern Game::dvar_s* pm_debug_drawAxis_col125;
	extern Game::dvar_s* pm_debug_drawAxis_col250;
	extern Game::dvar_s* pm_debug_drawAxis_col333;


	// Renderer
	extern Game::dvar_s* r_aspectRatio_custom;
	extern Game::dvar_s* r_aspectRatio;
	extern Game::dvar_s* r_noborder;
	extern Game::dvar_s* r_d3d9ex;
	extern Game::dvar_s* r_dumpShaders;

	extern Game::dvar_s* r_buf_skinnedCacheVb;
	extern Game::dvar_s* r_buf_smodelCacheVb;
	extern Game::dvar_s* r_buf_smodelCacheIb;
	extern Game::dvar_s* r_buf_tempSkin;
	extern Game::dvar_s* r_buf_dynamicVertexBuffer;
	extern Game::dvar_s* r_buf_dynamicIndexBuffer;
	extern Game::dvar_s* r_buf_preTessIndexBuffer;

	extern Game::dvar_s* r_debugShaderToggle;
	extern Game::dvar_s* r_setmaterial_hk;

	extern Game::dvar_s* r_wireframe_world;
	extern Game::dvar_s* r_wireframe_xmodels;

	extern Game::dvar_s* r_cullWorld;
	extern Game::dvar_s* r_cullEntities;
	extern Game::dvar_s* r_drawDynents;

	extern Game::dvar_s* r_debugShaderTexcoord;
	extern Game::dvar_s* r_dayAndNight;

	extern Game::dvar_s* r_fogTweaks;
	extern Game::dvar_s* r_fogTweaksColor;
	extern Game::dvar_s* r_fogTweaksStart;
	extern Game::dvar_s* r_fogTweaksDensity;

	extern Game::dvar_s* cg_fov_gun;
	extern Game::dvar_s* cg_fov_tweaks;

	// Debug collision
	extern Game::dvar_s* r_drawCollision;
	extern Game::dvar_s* r_drawCollision_brushAmount;
	extern Game::dvar_s* r_drawCollision_brushDist;
	extern Game::dvar_s* r_drawCollision_brushSorting;
	extern Game::dvar_s* r_drawCollision_brushIndexFilter;
	extern Game::dvar_s* r_drawCollision_brushIndexVisible;
	extern Game::dvar_s* r_drawCollision_material;
	extern Game::dvar_s* r_drawCollision_materialInclude;
	extern Game::dvar_s* r_drawCollision_materialList;
	extern Game::dvar_s* r_drawCollision_lineColor;
	extern Game::dvar_s* r_drawCollision_lineWidth;
	extern Game::dvar_s* r_drawCollision_polyAlpha;
	extern Game::dvar_s* r_drawCollision_polyDepth;
	extern Game::dvar_s* r_drawCollision_polyFace;
	extern Game::dvar_s* r_drawCollision_polyLit;
	extern Game::dvar_s* r_drawCollision_flickerBrushes;
	extern Game::dvar_s* r_drawCollision_flickerOnTime;
	extern Game::dvar_s* r_drawCollision_flickerOffTime;
	extern Game::dvar_s* r_drawCollision_hud;
	extern Game::dvar_s* r_drawCollision_hud_position;
	extern Game::dvar_s* r_drawCollision_hud_fontScale;
	extern Game::dvar_s* r_drawCollision_hud_fontStyle;
	extern Game::dvar_s* r_drawCollision_hud_fontColor;

#if DEBUG
	extern Game::dvar_s* r_drawCollision_brushDebug;
#endif

	// Map exporting
	extern Game::dvar_s* mapexport_brushEpsilon1;
	extern Game::dvar_s* mapexport_brushEpsilon2;
	extern Game::dvar_s* mapexport_brushMinSize;
	extern Game::dvar_s* mapexport_brush5Sides;
	extern Game::dvar_s* mapexport_selectionMode;
	extern Game::dvar_s* mapexport_writeTriangles;
	extern Game::dvar_s* mapexport_writeQuads;
	extern Game::dvar_s* mapexport_writeEntities;
	extern Game::dvar_s* mapexport_writeModels;


	// Shader overlays
	extern Game::dvar_s* xo_shaderoverlay;
	extern Game::dvar_s* xo_shaderoverlay_custom;
	extern Game::dvar_s* xo_ssao_noisescale;
	extern Game::dvar_s* xo_ssao_quality;
	extern Game::dvar_s* xo_ssao_radius;
	extern Game::dvar_s* xo_ssao_debugnormal;
	extern Game::dvar_s* xo_ssao_debugTargets;
	extern Game::dvar_s* xo_ssao_depthprepass;
	extern Game::dvar_s* xo_ssao_contrast;
	extern Game::dvar_s* xo_ssao_attenuation;
	extern Game::dvar_s* xo_ssao_angleBias;
	extern Game::dvar_s* xo_outliner_scale;
	extern Game::dvar_s* xo_outliner_depthDiffScale;
	extern Game::dvar_s* xo_outliner_depthThreshold;
	extern Game::dvar_s* xo_outliner_toonEnable;
	extern Game::dvar_s* xo_outliner_toonShades;

	extern Game::dvar_s* xo_camDir0;
	extern Game::dvar_s* xo_camDir1;
	extern Game::dvar_s* xo_camDir2;

	extern Game::dvar_s* xo_shaderdbg_matrix;

	// Random stock
	extern Game::dvar_s* snaps;
	extern Game::dvar_s* cg_fovScale;

	// Compass
	extern Game::dvar_s* mdd_compass;
	extern Game::dvar_s* mdd_compass_yh;
	extern Game::dvar_s* mdd_compass_quadrant_rgbas0;
	extern Game::dvar_s* mdd_compass_quadrant_rgbas1;
	extern Game::dvar_s* mdd_compass_quadrant_rgbas2;
	extern Game::dvar_s* mdd_compass_quadrant_rgbas3;
	extern Game::dvar_s* mdd_compass_ticks_rgba;

	// CGaz
	extern Game::dvar_s* mdd_cgaz;
	extern Game::dvar_s* mdd_cgaz_ground;
	extern Game::dvar_s* mdd_cgaz_min_speed;
	extern Game::dvar_s* mdd_cgaz_yh;
	extern Game::dvar_s* mdd_cgaz_rgbaNoAccel;
	extern Game::dvar_s* mdd_cgaz_rgbaPartialAccel;
	extern Game::dvar_s* mdd_cgaz_rgbaFullAccel;
	extern Game::dvar_s* mdd_cgaz_rgbaTurnZone;
}