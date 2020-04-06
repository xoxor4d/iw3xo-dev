#include "STDInclude.hpp"

namespace Components
{
	void RB_GaussianFilterImage(float radius, Game::GfxRenderTargetId srcRenderTargetId, Game::GfxRenderTargetId dstRenderTargetId, float _dstWidth = 0.0, float _dstHeight = 0.0)
	{
		int srcWidth, srcHeight, dstWidth, dstHeight;
		float radiusX, radiusY;

		radiusY = *Game::wnd_SceneHeight * radius / 480.0f; // sceneHeight
		radiusX = *Game::wnd_SceneAspect * radiusY; // sceneHeight * AspectRatio(1.7778) / sceneWidth

		srcWidth = Game::RenderTargetWidth[5 * srcRenderTargetId];
		srcHeight = Game::RenderTargetHeight[5 * srcRenderTargetId];

		dstWidth = (std::int32_t)(Game::RenderTargetWidth[5 * dstRenderTargetId] * 0.25);
		dstHeight = (std::int32_t)(Game::RenderTargetHeight[5 * dstRenderTargetId] * 0.25);

		Game::GfxImageFilter filter;
		filter.sourceImage = (Game::GfxImage *)Game::RenderTargetArray[5 * srcRenderTargetId];
		filter.finalTarget = dstRenderTargetId;

		// generates 1 pass if src is 2x bigger then dest
		filter.passCount = Game::RB_GenerateGaussianFilterChain( radiusX, radiusY, srcWidth, srcHeight, dstWidth, dstHeight, filter.passes );

		if (filter.passCount) 
		{
			Game::RB_FilterImage(&filter);
		}
	}

	void DrawToRendertarget(Game::GfxRenderTargetId Rendertarget, char *materialName, float x, float y, float width, float height)
	{
		Game::R_Set2D();
		Game::R_SetRenderTarget(Rendertarget);

		Game::Material *material = reinterpret_cast<Game::Material *>(Game::Material_RegisterHandle(materialName, 3));
		Game::RB_DrawStretchPic(material, x, y, width, height, 0.0, 0.0, 1.0, 1.0);

		Game::RB_EndTessSurface();
	}

	void RB_DrawCustomShaders(char *shader)
	{
		if (*Game::tessSurface)
		{
			Game::RB_EndTessSurface();
		}

		float _SSAO_DBG_MATSCALE = 0.33f;

		float fullscreenX = Game::scrPlace->realViewableMax[0];
		float fullscreenY = Game::scrPlace->realViewableMax[1];
		
		// POSTFX_SSAO || POSTFX_SSAO_NORMAL
		if (Dvars::xo_shaderoverlay->current.integer == Game::XO_SHADEROVERLAY::Z_SHADER_SSAO || Dvars::xo_ssao_debugnormal->current.enabled)
		{
			// SSAO DVARS WITHIN SHADER CONSTANT FILTERTAP

			// FilterTap 0
			Game::gfxCmdBufSourceState->input.consts[21][0] = Dvars::xo_ssao_noisescale->current.value;
			Game::gfxCmdBufSourceState->input.consts[21][1] = Dvars::xo_ssao_quality->current.value;
			Game::gfxCmdBufSourceState->input.consts[21][2] = fullscreenX; // DONT USE FilterTap[0][2] for dvars -> Used as TextureSize.x
			Game::gfxCmdBufSourceState->input.consts[21][3] = fullscreenY; // DONT USE FilterTap[0][3] for dvars -> Used as TextureSize.y

			// FilterTap 1
			Game::gfxCmdBufSourceState->input.consts[22][0] = Dvars::xo_ssao_radius->current.value;
			Game::gfxCmdBufSourceState->input.consts[22][2] = Game::Dvar_FindVar("cg_fov")->current.value / Game::Dvar_FindVar("cg_fovScale")->current.value;

			// FilterTap 2
			Game::gfxCmdBufSourceState->input.consts[23][0] = Dvars::xo_ssao_contrast->current.value;
			Game::gfxCmdBufSourceState->input.consts[23][1] = Dvars::xo_ssao_attenuation->current.value;
			Game::gfxCmdBufSourceState->input.consts[23][2] = Dvars::xo_ssao_angleBias->current.value;

			// Camera Angles / Axis
			Dvars::xo_camDir0->current.value = Game::gfxCmdBufSourceState->input.data->viewParms->axis[0][0];
			Dvars::xo_camDir1->current.value = Game::gfxCmdBufSourceState->input.data->viewParms->axis[0][1];
			Dvars::xo_camDir2->current.value = Game::gfxCmdBufSourceState->input.data->viewParms->axis[0][2];

			// PASS 1 :: CREATE DEPTH
			DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FLOAT_Z, "z_shader_ssao_depth", 0.0f, 0.0f, fullscreenX, fullscreenY);

			// DEBUG :: Fullscreen Normals
			if (Dvars::xo_ssao_debugnormal->current.enabled)
			{
				DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_ssao_normal", 0.0f, 0.0f, fullscreenX, fullscreenY);
				return;

				// only here if we want to look what values are within BufState
				// const auto GfxCmdBufState = reinterpret_cast<Game::GfxCmdBufState*>(0xD5404F0); 
			}

			// Draw SSAO if not using fullscreen Normals
			else
			{
				// PASS 2 :: CALCULATE AO :: CREATE NORMALS FROM DEPTH
				DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_RESOLVED_POST_SUN, "z_shader_ssao", 0.0f, 0.0f, fullscreenX, fullscreenY);

				// DEBUG :: Draw Normals & Non Blured AO
				if (Dvars::xo_ssao_debugTargets->current.enabled)
				{
					Game::gfxCmdBufSourceState->input.consts[21][2] = _SSAO_DBG_MATSCALE * fullscreenX; // DONT USE FilterTap[0][2] for dvars -> Used as TextureSize.x
					Game::gfxCmdBufSourceState->input.consts[21][3] = _SSAO_DBG_MATSCALE * fullscreenY; // DONT USE FilterTap[0][3] for dvars -> Used as TextureSize.y

					// DEBUG :: Draw Normals
					DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_ssao_normal", 0.0f, 0.0f, _SSAO_DBG_MATSCALE * fullscreenX, _SSAO_DBG_MATSCALE * fullscreenY);

					// DEBUG :: Non Blured AO
					DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_ssao", 0.0f, _SSAO_DBG_MATSCALE * fullscreenY, _SSAO_DBG_MATSCALE * fullscreenX, _SSAO_DBG_MATSCALE * fullscreenY);

					Game::gfxCmdBufSourceState->input.consts[21][2] = fullscreenX; // DONT USE FilterTap[0][2] for dvars -> Used as TextureSize.x
					Game::gfxCmdBufSourceState->input.consts[21][3] = fullscreenY; // DONT USE FilterTap[0][3] for dvars -> Used as TextureSize.y
				}

				// PASS 3 :: BLUR AO
				DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FLOAT_Z, "z_shader_ssao_blur", 0.0f, 0.0f, fullscreenX, fullscreenY);

				// DEBUG :: Blured AO
				if (Dvars::xo_ssao_debugTargets->current.enabled)
				{
					Game::gfxCmdBufSourceState->input.consts[21][2] = _SSAO_DBG_MATSCALE * fullscreenX; // DONT USE FilterTap[0][2] for dvars -> Used as TextureSize.x
					Game::gfxCmdBufSourceState->input.consts[21][3] = _SSAO_DBG_MATSCALE * fullscreenY; // DONT USE FilterTap[0][3] for dvars -> Used as TextureSize.y

					DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_ssao_blur", 0.0f, 2.0f * _SSAO_DBG_MATSCALE * fullscreenY, _SSAO_DBG_MATSCALE * fullscreenX, _SSAO_DBG_MATSCALE * fullscreenY);

					Game::gfxCmdBufSourceState->input.consts[21][2] = fullscreenX; // DONT USE FilterTap[0][2] for dvars -> Used as TextureSize.x
					Game::gfxCmdBufSourceState->input.consts[21][3] = fullscreenY; // DONT USE FilterTap[0][3] for dvars -> Used as TextureSize.y
				}

				// PASS 4 :: APPLY AO MULTIPLY MATERIAL
				else
				{
					DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_ssao_apply", 0.0f, 0.0f, fullscreenX, fullscreenY);
				}
				
			}
		}

		// POSTFX_OUTLINER
		else if (Dvars::xo_shaderoverlay->current.integer == Game::XO_SHADEROVERLAY::Z_SHADER_POSTFX_OUTLINER)
		{
			// Depth prepass
			DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FLOAT_Z, "floatz_display", 0.0f, 0.0f, fullscreenX, fullscreenY);

			// FilterTap 0
			Game::gfxCmdBufSourceState->input.consts[21][0] = Dvars::xo_outliner_scale->current.value;
			Game::gfxCmdBufSourceState->input.consts[21][1] = Dvars::xo_outliner_depthDiffScale->current.value;
			Game::gfxCmdBufSourceState->input.consts[21][2] = Dvars::xo_outliner_depthThreshold->current.value;

			// FilterTap 2
			Game::gfxCmdBufSourceState->input.consts[23][0] = Dvars::xo_outliner_toonEnable->current.enabled ? 1.0f : 0.0f;
			Game::gfxCmdBufSourceState->input.consts[23][1] = Dvars::xo_outliner_toonShades->current.value;

			// Draw Outliner
			DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_outliner", 0.0f, 0.0f, fullscreenX, fullscreenY);
		}

		else 
		{
			// If using other overlay shaders
			DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, shader, 0.0f, 0.0f, fullscreenX, fullscreenY);
		}
	}

	void RB_DrawDebugPostEffects()
	{
		std::int32_t r_showFbColorDebug = Game::Dvar_FindVar("r_showFbColorDebug")->current.integer;
		bool r_showFloatZDebug			= Game::Dvar_FindVar("r_showFloatZDebug")->current.enabled;
		bool sc_showDebug				= Game::Dvar_FindVar("sc_showDebug")->current.enabled;

		// force depthbuffer
		if (!Game::Dvar_FindVar("r_zFeather")->current.enabled)
		{
			Game::Cmd_ExecuteSingleCommand(0, 0, "r_zFeather 1\n");
		}

		if (Dvars::xo_shaderoverlay->current.integer != 0 || Dvars::xo_ssao_debugnormal->current.enabled) 
		{
			RB_DrawCustomShaders(Game::Dvar_EnumToString(Dvars::xo_shaderoverlay));
		}

		else if (r_showFbColorDebug == 1) 
		{
			Game::RB_ShowFbColorDebug_Screen();
		}

		else if (r_showFbColorDebug == 2) 
		{
			Game::RB_ShowFbColorDebug_Feedback();
		}

		else if (r_showFloatZDebug ) 
		{
			Game::RB_ShowFloatZDebug();
		}

		else if (sc_showDebug) 
		{
			Game::RB_ShowShadowsDebug();
		}
	}

	RB_ShaderOverlays::RB_ShaderOverlays()
	{
		// -----
		// Dvars

		static std::vector <char*> xo_shaderoverlayEnum =
		{
			"NONE",
			"Z_SHADER_SSAO",
			"Z_SHADER_POSTFX_CELLSHADING",
			"Z_SHADER_POSTFX_OUTLINER",
			"FLOATZ_DISPLAY",
		};

		Dvars::xo_shaderoverlay = Game::Dvar_RegisterEnum(
			/* name		*/ "xo_shaderoverlay",
			/* desc		*/ "fullscreen shaderoverlays",
			/* default	*/ 0,
			/* enumSize	*/ xo_shaderoverlayEnum.size(),
			/* enumData */ xo_shaderoverlayEnum.data(),
			/* flags	*/ Game::dvar_flags::none);

		Dvars::xo_ssao_debugnormal = Game::Dvar_RegisterBool(
			/* name		*/ "xo_ssao_debugnormal",
			/* desc		*/ "draw reconstructed normals from depth",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::xo_ssao_debugTargets = Game::Dvar_RegisterBool(
			/* name		*/ "xo_ssao_debugTargets",
			/* desc		*/ "draw each ssao pass",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::xo_ssao_depthprepass = Game::Dvar_RegisterBool(
			/* name		*/ "xo_ssao_depthprepass",
			/* desc		*/ "use a custom floatz pass for ssao",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::none);

		// SSAO FilterTap 0
		Dvars::xo_ssao_noisescale	= Game::Dvar_RegisterFloat("xo_ssao_noisescale", "hlsl constant filtertap[0][0] :: _NOISESCALE :: scale of noisemap", 4.0f, -1000.0f, 1000.0f, Game::dvar_flags::none);
		Dvars::xo_ssao_quality		= Game::Dvar_RegisterFloat("xo_ssao_quality", "hlsl constant filtertap[0][1] :: _QUALITY :: 0 = Low, 1 = High", 0.0f, -1000.0f, 1000.0f, Game::dvar_flags::none);

		// SSAO FilterTap 1
		Dvars::xo_ssao_radius = Game::Dvar_RegisterFloat("xo_ssao_radius", "hlsl constant filtertap[1][0] :: _RADIUS :: sample radius", 0.2f, -1000.0f, 1000.0f, Game::dvar_flags::none);

		// SSAO FilterTap 2
		Dvars::xo_ssao_contrast		= Game::Dvar_RegisterFloat("xo_ssao_contrast", "hlsl constant filtertap[2][0] :: _CONTRAST :: ao contrast", 0.75f, -1000.0f, 1000.0f, Game::dvar_flags::none);
		Dvars::xo_ssao_attenuation	= Game::Dvar_RegisterFloat("xo_ssao_attenuation", "hlsl constant filtertap[2][1] :: _ATTENUATION :: ao attenuation", 4.0f, -1000.0f, 1000.0f, Game::dvar_flags::none);
		Dvars::xo_ssao_angleBias	= Game::Dvar_RegisterFloat("xo_ssao_angleBias", "hlsl constant filtertap[2][2] :: _ANGLEBIAS :: in degrees", 25.0f, -1000.0f, 1000.0f, Game::dvar_flags::none);

		// SSAO Misc
		Dvars::xo_camDir0 = Game::Dvar_RegisterFloat("xo_camDir0", "viewMatrix camera dir x :: read-only", 0.0f, -1000.0f, 1000.0f, Game::dvar_flags::read_only);
		Dvars::xo_camDir1 = Game::Dvar_RegisterFloat("xo_camDir1", "viewMatrix camera dir y :: read-only", 0.0f, -1000.0f, 1000.0f, Game::dvar_flags::read_only);
		Dvars::xo_camDir2 = Game::Dvar_RegisterFloat("xo_camDir2", "viewMatrix camera dir z :: read-only", 0.0f, -1000.0f, 1000.0f, Game::dvar_flags::read_only);

		// Outliner FilterTap 0
		Dvars::xo_outliner_scale			= Game::Dvar_RegisterFloat("xo_outliner_scale", "hlsl constant filtertap[0][0] :: edge sample radius / scale", 2.0f, -1000.0f, 1000.0f, Game::dvar_flags::none);
		Dvars::xo_outliner_depthDiffScale	= Game::Dvar_RegisterFloat("xo_outliner_depthDiffScale", "hlsl constant filtertap[0][1] :: scale when edges with x depth difference get detected", 45.0f, -1000.0f, 1000.0f, Game::dvar_flags::none);
		Dvars::xo_outliner_depthThreshold	= Game::Dvar_RegisterFloat("xo_outliner_depthThreshold", "hlsl constant filtertap[0][2] :: Edges are drawn between areas where the sampled depth is greater than depthThreshold", 6.0f, -1000.0f, 1000.0f, Game::dvar_flags::none);

		// Outliner FilterTap 2
		Dvars::xo_outliner_toonEnable = Game::Dvar_RegisterBool("xo_outliner_toonEnable", "hlsl constant filtertap[2][0] :: toggle use of a simple toon shader", false, Game::dvar_flags::none);
		Dvars::xo_outliner_toonShades = Game::Dvar_RegisterFloat("xo_outliner_toonShades", "hlsl constant filtertap[2][1] :: Max amount of shades / colors used by the toon shader", 6.0f, -1000.0f, 1000.0f, Game::dvar_flags::none);

		// -----
		// Hooks

		// Disable Change from FLOATZ to SHADOWCOOKIE, to prevent AO lag or freezing
		Utils::Hook::Nop(0x658FB2, 2);

		// Rewrite RB_DrawDebugPostEffects (Entry for custom post-effects)
		Utils::Hook(0x64AD70, RB_DrawDebugPostEffects, HOOK_CALL).install()->quick();
	}

	RB_ShaderOverlays::~RB_ShaderOverlays()
	{ }
}
