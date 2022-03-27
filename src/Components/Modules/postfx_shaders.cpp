#include "STDInclude.hpp"

namespace components
{
	// RB_GaussianFilterImage
	void gaussian_filter_image(float radius, Game::GfxRenderTargetId src_render_target, Game::GfxRenderTargetId dest_render_target, [[maybe_unused]] float dest_width = 0.0, [[maybe_unused]] float dest_height = 0.0)
	{
		float radius_y = *Game::wnd_SceneHeight * radius / 480.0f; // sceneHeight
		float radius_x = *Game::wnd_SceneAspect * radius_y; // sceneHeight * AspectRatio(1.7778) / sceneWidth

		int src_width = Game::RenderTargetWidth[5 * src_render_target];
		int src_height = Game::RenderTargetHeight[5 * src_render_target];

		int dst_width = (std::int32_t)(Game::RenderTargetWidth[5 * dest_render_target] * 0.25);
		int dst_height = (std::int32_t)(Game::RenderTargetHeight[5 * dest_render_target] * 0.25);

		Game::GfxImageFilter filter = {};
		filter.sourceImage = (Game::GfxImage *)Game::RenderTargetArray[5 * src_render_target];
		filter.finalTarget = dest_render_target;

		// generates 1 pass if src is 2x bigger then dest
		filter.passCount = Game::RB_GenerateGaussianFilterChain( radius_x, radius_y, src_width, src_height, dst_width, dst_height, filter.passes );

		if (filter.passCount) 
		{
			Game::RB_FilterImage(&filter);
		}
	}

	void draw_to_rendertarget(Game::GfxRenderTargetId render_target, const char *material_name, const float x, const float y, const float width, const float height)
	{
		Game::R_Set2D();
		Game::R_SetRenderTarget(render_target);

		if (const auto	material = Game::Material_RegisterHandle(material_name, 3); 
						material)
		{
			Game::RB_DrawStretchPic(material, x, y, width, height, 0.0, 0.0, 1.0, 1.0);
		}

		Game::RB_EndTessSurface();
	}

	void draw_custom_postfx_shaders(const char *shader)
	{
		if (!Game::gfxCmdBufSourceState)
		{
			return;
		}

		if (Game::tess->indexCount)
		{
			Game::RB_EndTessSurface();
		}

		const float fullscreen_x = Game::scrPlace->realViewableMax[0];
		const float fullscreen_y = Game::scrPlace->realViewableMax[1];
		
		// POSTFX_OUTLINER
		if (dvars::xo_shaderoverlay->current.integer == Game::XO_SHADEROVERLAY::Z_SHADER_POSTFX_OUTLINER)
		{
			// Depth prepass
			draw_to_rendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FLOAT_Z, "floatz_display", 0.0f, 0.0f, fullscreen_x, fullscreen_y);

			// FilterTap 0
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][0] = dvars::xo_outliner_scale->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][1] = dvars::xo_outliner_depthDiffScale->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][2] = dvars::xo_outliner_depthThreshold->current.value;

			// FilterTap 2
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][0] = dvars::xo_outliner_toonEnable->current.enabled ? 1.0f : 0.0f;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][1] = dvars::xo_outliner_toonShades->current.value;

			// Draw Outliner
			draw_to_rendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_outliner", 0.0f, 0.0f, fullscreen_x, fullscreen_y);
		}

		else 
		{
			if(dvars::xo_shaderoverlay->current.integer != Game::XO_SHADEROVERLAY::Z_SHADER_SSAO)
			{
				// If using other overlay shaders
				draw_to_rendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, shader, 0.0f, 0.0f, fullscreen_x, fullscreen_y);
			}
		}
	}

	// rewrite of RB_DrawDebugPostEffects
	void draw_debug_postfx(const Game::GfxViewInfo* view_info)
	{
		const auto& r_showFbColorDebug = Game::Dvar_FindVar("r_showFbColorDebug");
		const auto& r_showFloatZDebug = Game::Dvar_FindVar("r_showFloatZDebug");
		const auto& sc_showDebug	= Game::Dvar_FindVar("sc_showDebug");
		
		if (r_showFbColorDebug && r_showFbColorDebug->current.integer == 1)
		{
			Game::RB_ShowFbColorDebug_Screen();
		}

		else if (r_showFbColorDebug && r_showFbColorDebug->current.integer == 2)
		{
			Game::RB_ShowFbColorDebug_Feedback();
		}

		else if (r_showFloatZDebug && r_showFloatZDebug->current.enabled)
		{
			Game::RB_ShowFloatZDebug();
		}

		else if (sc_showDebug && sc_showDebug->current.enabled)
		{
			Game::RB_ShowShadowsDebug();
		}

		else if (dvars::xo_shaderoverlay && dvars::xo_ssao_debugnormal && (dvars::xo_shaderoverlay->current.integer != 0 || dvars::xo_ssao_debugnormal->current.enabled))
		{
			const auto& r_zFeather = Game::Dvar_FindVar("r_zFeather");
			const auto& r_distortion	= Game::Dvar_FindVar("r_distortion");
			const auto& r_glow_allowed = Game::Dvar_FindVar("r_glow_allowed");
			
			// force depthbuffer
			if (r_zFeather && !r_zFeather->current.enabled)
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "r_zFeather 1\n");
			}

			// enable distortion (its rendertarget is needed)
			if (r_distortion && !r_distortion->current.enabled)
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "r_distortion 1\n");
			}

			// disable glow as it uses the backbuffer exclusively 
			if (r_glow_allowed && r_glow_allowed->current.enabled)
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "r_glow_allowed 0\n");
			}

			// use a custom material
			if (dvars::xo_shaderoverlay_custom && dvars::xo_shaderoverlay->current.integer == 5)
			{
				draw_custom_postfx_shaders(dvars::xo_shaderoverlay_custom->current.string);
			}

			// use pre-defined materials
			else
			{
				draw_custom_postfx_shaders(Game::Dvar_EnumToString(dvars::xo_shaderoverlay));
			}
		}

		// *
		// overwrite resolvedPostSun to include filmTweaks

		Game::R_Set2D();
		Game::R_SetRenderTarget(Game::GfxRenderTargetId::R_RENDERTARGET_RESOLVED_POST_SUN);

		const auto rgp = reinterpret_cast<Game::r_global_permanent_t*>(0xCC98280);

		if (const auto	material = rgp->postFxColorMaterial;
						material)
		{
			Game::RB_DrawStretchPic(material, 0.0f, 0.0f, Game::scrPlace->realViewableMax[0], Game::scrPlace->realViewableMax[1], 0.0, 0.0, 1.0, 1.0);
		}

		Game::RB_EndTessSurface();
	}

	__declspec(naked) void RB_DrawDebugPostEffects_stub()
	{
		const static uint32_t retn_addr = 0x64AD75;
		__asm
		{
			pushad;
			push	esi; // GfxViewInfo
			call	draw_debug_postfx;
			add		esp, 4;
			popad;

			jmp		retn_addr;
		}
	}

	void draw_ssao()
	{
		if (dvars::xo_shaderoverlay->current.integer == Game::XO_SHADEROVERLAY::Z_SHADER_SSAO)
		{
			Game::R_Set2D();

			const float fullscreen_x = Game::scrPlace->realViewableMax[0];
			const float fullscreen_y = Game::scrPlace->realViewableMax[1];

			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][0] = dvars::xo_ssao_noisescale->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][1] = dvars::xo_ssao_quality->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][2] = fullscreen_x; // DONT USE FilterTap[0][2] for dvars -> Used as TextureSize.x
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][3] = fullscreen_y; // DONT USE FilterTap[0][3] for dvars -> Used as TextureSize.y

			// FilterTap 1
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_1][0] = dvars::xo_ssao_radius->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_1][2] = Game::Dvar_FindVar("cg_fov")->current.value / Game::Dvar_FindVar("cg_fovScale")->current.value;

			// FilterTap 2
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][0] = dvars::xo_ssao_contrast->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][1] = dvars::xo_ssao_attenuation->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][2] = dvars::xo_ssao_angleBias->current.value;

			// DEBUG :: fullscreen normals
			if (dvars::xo_ssao_debugnormal->current.enabled)
			{
				draw_to_rendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_ssao_normal", 0.0f, 0.0f, fullscreen_x, fullscreen_y);
				return;
			}

			Game::R_SetRenderTarget(Game::GfxRenderTargetId::R_RENDERTARGET_RESOLVED_SCENE);

			auto material = Game::Material_RegisterHandle("z_shader_ssao", 3);
			Game::RB_DrawFullScreenColoredQuad(material, 0.0f, 0.0f, 1.0f, 1.0f, -1);

			Game::GfxRenderTarget& resolved_scene = *reinterpret_cast<Game::GfxRenderTarget*>(0xD573F00);

			// needs r_depthPrepass or the zBuffer might freeze
			resolved_scene.image->texture.map->GetSurfaceLevel(0, &resolved_scene.surface.color);
			Game::gfxCmdBufState->prim.device->StretchRect(resolved_scene.surface.color, 0, resolved_scene.surface.color, 0, D3DTEXF_LINEAR);


			Game::gfxCmdBufSourceState->input.codeImages[11] = resolved_scene.image;

			material = Game::Material_RegisterHandle("z_shader_ssao_blur", 3);
			Game::RB_DrawFullScreenColoredQuad(material, 0.0f, 0.0f, 1.0f, 1.0f, -1);

			Game::R_SetRenderTarget(Game::GfxRenderTargetId::R_RENDERTARGET_SCENE);

			material = Game::Material_RegisterHandle("z_shader_ssao_apply", 3);
			Game::RB_DrawFullScreenColoredQuad(material, 0.0f, 0.0f, 1.0f, 1.0f, -1);
		}
	}

	__declspec(naked) void ssao_draw_stub()
	{
		const static uint32_t og_func_addr = 0x634820;
		const static uint32_t retn_addr = 0x64B2EB;
		__asm
		{
			pushad;
			call	draw_ssao;
			popad;

			call	og_func_addr;
			jmp		retn_addr;
		}
	}

	void postfx_shaders::register_dvars()
	{
		dvars::xo_shaderoverlay_custom = Game::Dvar_RegisterString(
			/* name		*/ "xo_shaderoverlay_custom",
			/* desc		*/ "<postfx material name>",
			/* default	*/ "default",
			/* flags	*/ Game::dvar_flags::none);
	}

	postfx_shaders::postfx_shaders()
	{
		// -----
		// dvars

		 dvars::r_dumpShaders = Game::Dvar_RegisterBool(
			/* name		*/ "r_dumpShaders",
			/* desc		*/ "dump loaded shaders at runtime",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		static std::vector <const char*> xo_shaderoverlay_enum =
		{
			"NONE",
			"Z_SHADER_SSAO",
			"Z_SHADER_POSTFX_CELLSHADING",
			"Z_SHADER_POSTFX_OUTLINER",
			"FLOATZ_DISPLAY",
			"CUSTOM",
		};

		dvars::xo_shaderoverlay = Game::Dvar_RegisterEnum(
			/* name		*/ "xo_shaderoverlay",
			/* desc		*/ "fullscreen shaderoverlays. <CUSTOM> uses the material defined with \"xo_shaderoverlay_custom\"",
			/* default	*/ 0,
			/* enumSize	*/ xo_shaderoverlay_enum.size(),
			/* enumData */ xo_shaderoverlay_enum.data(),
			/* flags	*/ Game::dvar_flags::none);

		dvars::xo_ssao_debugnormal = Game::Dvar_RegisterBool(
			/* name		*/ "xo_ssao_debugnormal",
			/* desc		*/ "draw reconstructed normals from depth",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		// SSAO FilterTap 0
		dvars::xo_ssao_noisescale	= Game::Dvar_RegisterFloat("xo_ssao_noisescale", "hlsl constant filtertap[0][0] :: _NOISESCALE :: scale of noisemap", 0.0f, 0.0f, 100.0f, Game::dvar_flags::none);
		dvars::xo_ssao_quality		= Game::Dvar_RegisterFloat("xo_ssao_quality", "hlsl constant filtertap[0][1] :: _QUALITY :: 0 = Low, 1 = High", 1.0f, 0.0f, 1.0f, Game::dvar_flags::none);

		// SSAO FilterTap 1
		dvars::xo_ssao_radius		= Game::Dvar_RegisterFloat("xo_ssao_radius", "hlsl constant filtertap[1][0] :: _RADIUS :: sample radius", 0.3f, 0.0f, 20.0f, Game::dvar_flags::none);

		// SSAO FilterTap 2
		dvars::xo_ssao_contrast		= Game::Dvar_RegisterFloat("xo_ssao_contrast", "hlsl constant filtertap[2][0] :: _CONTRAST :: ao contrast", 0.65f, 0.0f, 20.0f, Game::dvar_flags::none);
		dvars::xo_ssao_attenuation	= Game::Dvar_RegisterFloat("xo_ssao_attenuation", "hlsl constant filtertap[2][1] :: _ATTENUATION :: ao attenuation", 30.0f, -200.0f, 200.0f, Game::dvar_flags::none);
		dvars::xo_ssao_angleBias	= Game::Dvar_RegisterFloat("xo_ssao_angleBias", "hlsl constant filtertap[2][2] :: _ANGLEBIAS :: in degrees", 16.0f, 0.0f, 90.0f, Game::dvar_flags::none);

		// Outliner FilterTap 0
		dvars::xo_outliner_scale			= Game::Dvar_RegisterFloat("xo_outliner_scale", "hlsl constant filtertap[0][0] :: edge sample radius / scale", 2.0f, 0.0f, 50.0f, Game::dvar_flags::none);
		dvars::xo_outliner_depthDiffScale	= Game::Dvar_RegisterFloat("xo_outliner_depthDiffScale", "hlsl constant filtertap[0][1] :: scale when edges with x depth difference get detected", 45.0f, 0.0f, 200.0f, Game::dvar_flags::none);
		dvars::xo_outliner_depthThreshold	= Game::Dvar_RegisterFloat("xo_outliner_depthThreshold", "hlsl constant filtertap[0][2] :: Edges are drawn between areas where the sampled depth is greater than depthThreshold", 6.0f, -10.0f, 60.0f, Game::dvar_flags::none);

		// Outliner FilterTap 2
		dvars::xo_outliner_toonEnable = Game::Dvar_RegisterBool("xo_outliner_toonEnable", "hlsl constant filtertap[2][0] :: toggle use of a simple toon shader", false, Game::dvar_flags::none);
		dvars::xo_outliner_toonShades = Game::Dvar_RegisterFloat("xo_outliner_toonShades", "hlsl constant filtertap[2][1] :: Max amount of shades / colors used by the toon shader", 6.0f, 0.0f, 64.0f, Game::dvar_flags::none);


		// -----
		// Hooks

		// Disable Change from FLOATZ to SHADOWCOOKIE, to prevent AO lag or freezing
		utils::hook::nop(0x658FB2, 2);

		// Rewrite RB_DrawDebugPostEffects (Entry for custom post-effects)
		utils::hook(0x64AD70, RB_DrawDebugPostEffects_stub, HOOK_JUMP).install()->quick();

		utils::hook(0x64B2E6, ssao_draw_stub, HOOK_JUMP).install()->quick();
	}
}
