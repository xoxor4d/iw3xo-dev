#pragma once

namespace dvars
{
	// Client
	extern game::dvar_s* cl_avidemo_streams;
	extern game::dvar_s* cl_avidemo_streams_greenscreen;
	extern game::dvar_s* cl_avidemo_streams_viewmodel;
	extern game::dvar_s* cl_avidemo_streams_depth;
	extern game::dvar_s* cl_avidemo_streams_hud;
	extern game::dvar_s* cl_pause_demo;
	extern game::dvar_s* load_iw3mvm;


	// Radiant
	extern game::dvar_s* radiant_live;
	extern game::dvar_s* radiant_livePort;
	extern game::dvar_s* radiant_liveDebug;
	extern game::dvar_s* radiant_syncCamera;
	extern game::dvar_s* radiant_brushCollision;
	extern game::dvar_s* radiant_brushColor;
	extern game::dvar_s* radiant_brushLit;
	extern game::dvar_s* radiant_brushWireframe;
	extern game::dvar_s* radiant_brushWireframeColor;


	// Menu
	extern game::dvar_s* ui_ultrawide;
	extern game::dvar_s* ui_button_highlight_radius;
	extern game::dvar_s* ui_button_highlight_brightness;
	extern game::dvar_s* ui_button_outline_radius;
	extern game::dvar_s* ui_button_outline_brightness;
	extern game::dvar_s* ui_subnavbar;
	extern game::dvar_s* ui_subnavbar_item;
	extern game::dvar_s* ui_renderWidth;
	extern game::dvar_s* ui_renderHeight;
	extern game::dvar_s* ui_devmap;
	extern game::dvar_s* ui_developer;
	extern game::dvar_s* ui_developer_script;
	extern game::dvar_s* ui_main_title;
	extern game::dvar_s* ui_eyes_position;
	extern game::dvar_s* ui_eyes_size;
	extern game::dvar_s* ui_eyes_alpha;
	extern game::dvar_s* xo_menu_dbg;


	// ImGui
	extern game::dvar_s* _imgui_saving;
	extern game::dvar_s* _imgui_window_alpha;


	// Devgui
	extern game::dvar_s* _imgui_devgui_pos;
	extern game::dvar_s* _imgui_devgui_size;


	// Console 
	extern game::dvar_s* con_minicon_position;
	extern game::dvar_s* con_minicon_font;
	extern game::dvar_s* con_minicon_fontHeight;
	extern game::dvar_s* con_minicon_fontColor;
	extern game::dvar_s* con_minicon_fontStyle;

	extern game::dvar_s* xo_con_fltCon;
	extern game::dvar_s* xo_con_fltConLeft;
	extern game::dvar_s* xo_con_fltConTop;
	extern game::dvar_s* xo_con_fltConRight;
	extern game::dvar_s* xo_con_fltConBottom;

	extern game::dvar_s* xo_con_outputHeight;
	extern game::dvar_s* xo_con_maxMatches;
	extern game::dvar_s* xo_con_useDepth;
	extern game::dvar_s* xo_con_fontSpacing;
	extern game::dvar_s* xo_con_padding;

	extern game::dvar_s* xo_con_cursorOverdraw;
	extern game::dvar_s* xo_con_cursorState;

	extern game::dvar_s* xo_con_hintBoxTxtColor_currentDvar;
	extern game::dvar_s* xo_con_hintBoxTxtColor_currentValue;
	extern game::dvar_s* xo_con_hintBoxTxtColor_defaultValue;
	extern game::dvar_s* xo_con_hintBoxTxtColor_dvarDescription;
	extern game::dvar_s* xo_con_hintBoxTxtColor_domainDescription;


	// Custom movement
	extern game::dvar_s* pm_movementType;
	extern game::dvar_s* pm_crashland;
	extern game::dvar_s* pm_rocketJumpHeight;
	extern game::dvar_s* pm_disableSprint;
	extern game::dvar_s* pm_terrainEdgeBounces;
	extern game::dvar_s* pm_bhop_auto;
	extern game::dvar_s* pm_bhop_slowdown;
	extern game::dvar_s* pm_cpm_useQuakeDamage;
	extern game::dvar_s* pm_cpm_damageKnockback;
	extern game::dvar_s* pm_cpm_useBouncing;
	extern game::dvar_s* pm_cpm_airControl;
	extern game::dvar_s* pm_cpm_airAccelerate;
	extern game::dvar_s* pm_cpm_airstopAccelerate;
	extern game::dvar_s* pm_cpm_strafeAccelerate;
	extern game::dvar_s* pm_cs_airAccelerate;
	extern game::dvar_s* pm_cs_airspeedCap;

	extern game::dvar_s* pm_hud_enable;
	extern game::dvar_s* pm_hud_x;
	extern game::dvar_s* pm_hud_y;
	extern game::dvar_s* pm_hud_fontScale;
	extern game::dvar_s* pm_hud_fontStyle;
	extern game::dvar_s* pm_hud_fontColor;

	extern game::dvar_s* pm_origin_hud;
	extern game::dvar_s* pm_origin_hud_x;
	extern game::dvar_s* pm_origin_hud_y;
	extern game::dvar_s* pm_origin_hud_fontScale;
	extern game::dvar_s* pm_origin_hud_fontStyle;
	extern game::dvar_s* pm_origin_hud_fontColor;

	extern game::dvar_s* pm_debug_prints;
	extern game::dvar_s* pm_debug_traceOrigin;
	extern game::dvar_s* pm_debug_traceVelocity;
	extern game::dvar_s* pm_debug_lineColor;
	extern game::dvar_s* pm_debug_lineDepth;
	extern game::dvar_s* pm_debug_lineDuration;
	extern game::dvar_s* pm_debug_lineWidth;
	extern game::dvar_s* pm_debug_drawAxis;
	extern game::dvar_s* pm_debug_drawAxis_radius;
	extern game::dvar_s* pm_debug_drawAxis_height;
	extern game::dvar_s* pm_debug_drawAxis_col125;
	extern game::dvar_s* pm_debug_drawAxis_col250;
	extern game::dvar_s* pm_debug_drawAxis_col333;


	// Renderer
	extern game::dvar_s* r_aspectRatio_custom;
	extern game::dvar_s* r_aspectRatio;
	extern game::dvar_s* r_noborder;
	extern game::dvar_s* r_d3d9ex;
	extern game::dvar_s* r_dumpShaders;

	extern game::dvar_s* r_buf_skinnedCacheVb;
	extern game::dvar_s* r_buf_smodelCacheVb;
	extern game::dvar_s* r_buf_smodelCacheIb;
	extern game::dvar_s* r_buf_tempSkin;
	extern game::dvar_s* r_buf_dynamicVertexBuffer;
	extern game::dvar_s* r_buf_dynamicIndexBuffer;
	extern game::dvar_s* r_buf_preTessIndexBuffer;

	extern game::dvar_s* r_debugShaderToggle;
	extern game::dvar_s* r_setmaterial_hk;

	extern game::dvar_s* r_wireframe_world;
	extern game::dvar_s* r_wireframe_xmodels;

	extern game::dvar_s* r_debugShaderTexcoord;
	extern game::dvar_s* r_dayAndNight;

	extern game::dvar_s* rtx_hacks;
	extern game::dvar_s* rtx_sky_hacks;
	extern game::dvar_s* rtx_sky_follow_player;
	extern game::dvar_s* rtx_sky_materials;
	extern game::dvar_s* rtx_warm_smodels;
	extern game::dvar_s* rtx_extend_smodel_drawing;
	extern game::dvar_s* r_forceLod_second_lowest;
	extern game::dvar_s* rtx_disable_world_culling;
	extern game::dvar_s* rtx_disable_entity_culling;
	extern game::dvar_s* rtx_draw_dynents;

	extern game::dvar_s* r_fogTweaks;
	extern game::dvar_s* r_fogTweaksColor;
	extern game::dvar_s* r_fogTweaksStart;
	extern game::dvar_s* r_fogTweaksDensity;

	extern game::dvar_s* cg_fov_gun;
	extern game::dvar_s* cg_fov_tweaks;

	// Debug collision
	extern game::dvar_s* r_drawCollision;
	extern game::dvar_s* r_drawCollision_brushAmount;
	extern game::dvar_s* r_drawCollision_brushDist;
	extern game::dvar_s* r_drawCollision_brushSorting;
	extern game::dvar_s* r_drawCollision_brushIndexFilter;
	extern game::dvar_s* r_drawCollision_brushIndexVisible;
	extern game::dvar_s* r_drawCollision_material;
	extern game::dvar_s* r_drawCollision_materialInclude;
	extern game::dvar_s* r_drawCollision_materialList;
	extern game::dvar_s* r_drawCollision_lineColor;
	extern game::dvar_s* r_drawCollision_lineWidth;
	extern game::dvar_s* r_drawCollision_polyAlpha;
	extern game::dvar_s* r_drawCollision_polyDepth;
	extern game::dvar_s* r_drawCollision_polyFace;
	extern game::dvar_s* r_drawCollision_polyLit;
	extern game::dvar_s* r_drawCollision_hud;
	extern game::dvar_s* r_drawCollision_hud_position;
	extern game::dvar_s* r_drawCollision_hud_fontScale;
	extern game::dvar_s* r_drawCollision_hud_fontStyle;
	extern game::dvar_s* r_drawCollision_hud_fontColor;

#if DEBUG
	extern game::dvar_s* r_drawCollision_brushDebug;
#endif

	// Map exporting
	extern game::dvar_s* mapexport_brushEpsilon1;
	extern game::dvar_s* mapexport_brushEpsilon2;
	extern game::dvar_s* mapexport_brushMinSize;
	extern game::dvar_s* mapexport_brush5Sides;
	extern game::dvar_s* mapexport_selectionMode;
	extern game::dvar_s* mapexport_writeTriangles;
	extern game::dvar_s* mapexport_writeQuads;
	extern game::dvar_s* mapexport_writeEntities;
	extern game::dvar_s* mapexport_writeModels;
	extern game::dvar_s* mapexport_writeDynModels;

	extern game::dvar_s* mpsp_require_gsc;

	// Shader overlays
	extern game::dvar_s* xo_shaderoverlay;
	extern game::dvar_s* xo_shaderoverlay_custom;
	extern game::dvar_s* xo_ssao_noisescale;
	extern game::dvar_s* xo_ssao_quality;
	extern game::dvar_s* xo_ssao_radius;
	extern game::dvar_s* xo_ssao_debugnormal;
	extern game::dvar_s* xo_ssao_contrast;
	extern game::dvar_s* xo_ssao_attenuation;
	extern game::dvar_s* xo_ssao_angleBias;
	extern game::dvar_s* xo_outliner_scale;
	extern game::dvar_s* xo_outliner_depthDiffScale;
	extern game::dvar_s* xo_outliner_depthThreshold;
	extern game::dvar_s* xo_outliner_toonEnable;
	extern game::dvar_s* xo_outliner_toonShades;

	// Random stock
	extern game::dvar_s* snaps;
	extern game::dvar_s* cg_fovScale;

	// Compass
	extern game::dvar_s* mdd_compass;
	extern game::dvar_s* mdd_compass_yh;
	extern game::dvar_s* mdd_compass_quadrant_rgbas0;
	extern game::dvar_s* mdd_compass_quadrant_rgbas1;
	extern game::dvar_s* mdd_compass_quadrant_rgbas2;
	extern game::dvar_s* mdd_compass_quadrant_rgbas3;
	extern game::dvar_s* mdd_compass_ticks_rgba;

	// CGaz
	extern game::dvar_s* mdd_cgaz;
	extern game::dvar_s* mdd_cgaz_ground;
	extern game::dvar_s* mdd_cgaz_min_speed;
	extern game::dvar_s* mdd_cgaz_yh;
	extern game::dvar_s* mdd_cgaz_rgbaNoAccel;
	extern game::dvar_s* mdd_cgaz_rgbaPartialAccel;
	extern game::dvar_s* mdd_cgaz_rgbaFullAccel;
	extern game::dvar_s* mdd_cgaz_rgbaTurnZone;

	//
	//

	void bool_override(const char* dvarName, const bool& value, const game::dvar_flags& flags = game::saved);
	void int_override(const char* dvarName, const int& value, const game::dvar_flags& flags = game::saved, const bool& limits = false, const int& mins = 0, const int& maxs = 0);
	void float_override(const char* dvarName, const float& value, const game::dvar_flags& flags = game::saved, const bool& limits = false, const float& mins = 0.0f, const float& maxs = 0.0f);

}