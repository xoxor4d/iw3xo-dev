#include "std_include.hpp"

namespace components
{
	// RB_GaussianFilterImage
	void gaussian_filter_image(float radius, game::GfxRenderTargetId src_render_target, game::GfxRenderTargetId dest_render_target, [[maybe_unused]] float dest_width = 0.0, [[maybe_unused]] float dest_height = 0.0)
	{
		float radius_y = game::v_aspectRatioWindow * radius / 480.0f;
		float radius_x = game::v_postProcessAspect * radius_y;

		auto src_width = static_cast<int>(game::gfxRenderTargets[src_render_target].width);
		auto src_height = static_cast<int>(game::gfxRenderTargets[src_render_target].height);

		auto dst_width = static_cast<int>( static_cast<float>(game::gfxRenderTargets[src_render_target].width) * 0.25f);
		auto dst_height = static_cast<int>( static_cast<float>(game::gfxRenderTargets[src_render_target].height) * 0.25f);

		game::GfxImageFilter filter = {};
		filter.sourceImage = game::gfxRenderTargets[src_render_target].image;
		filter.finalTarget = dest_render_target;

		// generates 1 pass if src is 2x bigger then dest
		filter.passCount = game::RB_GenerateGaussianFilterChain( radius_x, radius_y, src_width, src_height, dst_width, dst_height, filter.passes );

		if (filter.passCount) 
		{
			game::RB_FilterImage(&filter);
		}
	}

	void draw_to_rendertarget(game::GfxRenderTargetId render_target, const char *material_name, const float x, const float y, const float width, const float height)
	{
		game::R_Set2D();
		game::R_SetRenderTarget(render_target);

		if (const auto	material = game::Material_RegisterHandle(material_name, 3); 
						material)
		{
			game::RB_DrawStretchPic(material, x, y, width, height, 0.0, 0.0, 1.0, 1.0);
		}

		game::RB_EndTessSurface();
	}

	void draw_custom_postfx_shaders(const char *shader)
	{
		if (!game::gfxCmdBufSourceState)
		{
			return;
		}

		if (game::tess->indexCount)
		{
			game::RB_EndTessSurface();
		}

		const float fullscreen_x = game::scrPlace->realViewableMax[0];
		const float fullscreen_y = game::scrPlace->realViewableMax[1];
		
		// POSTFX_OUTLINER
		if (dvars::xo_shaderoverlay->current.integer == game::XO_SHADEROVERLAY::Z_SHADER_POSTFX_OUTLINER)
		{
			// Depth prepass
			draw_to_rendertarget(game::GfxRenderTargetId::R_RENDERTARGET_FLOAT_Z, "floatz_display", 0.0f, 0.0f, fullscreen_x, fullscreen_y);

			// FilterTap 0
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][0] = dvars::xo_outliner_scale->current.value;
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][1] = dvars::xo_outliner_depthDiffScale->current.value;
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][2] = dvars::xo_outliner_depthThreshold->current.value;

			// FilterTap 2
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][0] = dvars::xo_outliner_toonEnable->current.enabled ? 1.0f : 0.0f;
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][1] = dvars::xo_outliner_toonShades->current.value;

			// Draw Outliner
			draw_to_rendertarget(game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_outliner", 0.0f, 0.0f, fullscreen_x, fullscreen_y);
		}

		else 
		{
			if(dvars::xo_shaderoverlay->current.integer != game::XO_SHADEROVERLAY::Z_SHADER_SSAO)
			{
				// If using other overlay shaders
				draw_to_rendertarget(game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, shader, 0.0f, 0.0f, fullscreen_x, fullscreen_y);
			}
		}
	}

	// rewrite of RB_DrawDebugPostEffects
	void draw_debug_postfx(const game::GfxViewInfo* view_info)
	{
		const auto& r_showFbColorDebug = game::Dvar_FindVar("r_showFbColorDebug");
		const auto& r_showFloatZDebug = game::Dvar_FindVar("r_showFloatZDebug");
		const auto& sc_showDebug	= game::Dvar_FindVar("sc_showDebug");
		
		if (r_showFbColorDebug && r_showFbColorDebug->current.integer == 1)
		{
			game::RB_ShowFbColorDebug_Screen();
		}

		else if (r_showFbColorDebug && r_showFbColorDebug->current.integer == 2)
		{
			game::RB_ShowFbColorDebug_Feedback();
		}

		else if (r_showFloatZDebug && r_showFloatZDebug->current.enabled)
		{
			game::RB_ShowFloatZDebug();
		}

		else if (sc_showDebug && sc_showDebug->current.enabled)
		{
			game::RB_ShowShadowsDebug();
		}

		else if (dvars::xo_shaderoverlay && dvars::xo_ssao_debugnormal && (dvars::xo_shaderoverlay->current.integer != 0 || dvars::xo_ssao_debugnormal->current.enabled))
		{
			const auto& r_zFeather = game::Dvar_FindVar("r_zFeather");
			const auto& r_distortion	= game::Dvar_FindVar("r_distortion");
			const auto& r_glow_allowed = game::Dvar_FindVar("r_glow_allowed");
			
			// force depthbuffer
			if (r_zFeather && !r_zFeather->current.enabled)
			{
				game::Cmd_ExecuteSingleCommand(0, 0, "r_zFeather 1\n");
			}

			// enable distortion (its rendertarget is needed)
			if (r_distortion && !r_distortion->current.enabled)
			{
				game::Cmd_ExecuteSingleCommand(0, 0, "r_distortion 1\n");
			}

			// disable glow as it uses the backbuffer exclusively 
			if (r_glow_allowed && r_glow_allowed->current.enabled)
			{
				game::Cmd_ExecuteSingleCommand(0, 0, "r_glow_allowed 0\n");
			}

			// use a custom material
			if (dvars::xo_shaderoverlay_custom && dvars::xo_shaderoverlay->current.integer == 5)
			{
				draw_custom_postfx_shaders(dvars::xo_shaderoverlay_custom->current.string);
			}

			// use pre-defined materials
			else
			{
				draw_custom_postfx_shaders(game::Dvar_EnumToString(dvars::xo_shaderoverlay));
			}
		}

		// *
		// overwrite resolvedPostSun to include filmTweaks

		game::R_Set2D();
		game::R_SetRenderTarget(game::GfxRenderTargetId::R_RENDERTARGET_RESOLVED_POST_SUN);

		const auto rgp = reinterpret_cast<game::r_global_permanent_t*>(0xCC98280);

		if (const auto	material = rgp->postFxColorMaterial;
						material)
		{
			game::RB_DrawStretchPic(material, 0.0f, 0.0f, game::scrPlace->realViewableMax[0], game::scrPlace->realViewableMax[1], 0.0, 0.0, 1.0, 1.0);
		}

		game::RB_EndTessSurface();
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
		if (dvars::xo_shaderoverlay->current.integer == game::XO_SHADEROVERLAY::Z_SHADER_SSAO)
		{
			game::R_Set2D();

			const float fullscreen_x = game::scrPlace->realViewableMax[0];
			const float fullscreen_y = game::scrPlace->realViewableMax[1];

			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][0] = dvars::xo_ssao_noisescale->current.value;
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][1] = dvars::xo_ssao_quality->current.value;
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][2] = fullscreen_x; // DONT USE FilterTap[0][2] for dvars -> Used as TextureSize.x
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][3] = fullscreen_y; // DONT USE FilterTap[0][3] for dvars -> Used as TextureSize.y

			// FilterTap 1
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_1][0] = dvars::xo_ssao_radius->current.value;
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_1][2] = game::Dvar_FindVar("cg_fov")->current.value / game::Dvar_FindVar("cg_fovScale")->current.value;

			// FilterTap 2
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][0] = dvars::xo_ssao_contrast->current.value;
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][1] = dvars::xo_ssao_attenuation->current.value;
			game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][2] = dvars::xo_ssao_angleBias->current.value;

			// DEBUG :: fullscreen normals
			if (dvars::xo_ssao_debugnormal->current.enabled)
			{
				draw_to_rendertarget(game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_ssao_normal", 0.0f, 0.0f, fullscreen_x, fullscreen_y);
				return;
			}

			game::R_SetRenderTarget(game::GfxRenderTargetId::R_RENDERTARGET_RESOLVED_SCENE);

			auto material = game::Material_RegisterHandle("z_shader_ssao", 3);
			game::RB_DrawFullScreenColoredQuad(material, 0.0f, 0.0f, 1.0f, 1.0f, -1);

			game::GfxRenderTarget& resolved_scene = *reinterpret_cast<game::GfxRenderTarget*>(0xD573F00);

			// needs r_depthPrepass or the zBuffer might freeze
			resolved_scene.image->texture.map->GetSurfaceLevel(0, &resolved_scene.surface.color);
			game::gfxCmdBufState->prim.device->StretchRect(resolved_scene.surface.color, 0, resolved_scene.surface.color, 0, D3DTEXF_LINEAR);


			game::gfxCmdBufSourceState->input.codeImages[11] = resolved_scene.image;

			material = game::Material_RegisterHandle("z_shader_ssao_blur", 3);
			game::RB_DrawFullScreenColoredQuad(material, 0.0f, 0.0f, 1.0f, 1.0f, -1);

			game::R_SetRenderTarget(game::GfxRenderTargetId::R_RENDERTARGET_SCENE);

			material = game::Material_RegisterHandle("z_shader_ssao_apply", 3);
			game::RB_DrawFullScreenColoredQuad(material, 0.0f, 0.0f, 1.0f, 1.0f, -1);
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
		dvars::xo_shaderoverlay_custom = game::Dvar_RegisterString(
			/* name		*/ "xo_shaderoverlay_custom",
			/* desc		*/ "<postfx material name>",
			/* default	*/ "default",
			/* flags	*/ game::dvar_flags::none);
	}

	postfx_shaders::postfx_shaders()
	{
		// -----
		// dvars

		 dvars::r_dumpShaders = game::Dvar_RegisterBool(
			/* name		*/ "r_dumpShaders",
			/* desc		*/ "dump loaded shaders at runtime",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none);

		static std::vector <const char*> xo_shaderoverlay_enum =
		{
			"NONE",
			"Z_SHADER_SSAO",
			"Z_SHADER_POSTFX_CELLSHADING",
			"Z_SHADER_POSTFX_OUTLINER",
			"FLOATZ_DISPLAY",
			"CUSTOM",
		};

		dvars::xo_shaderoverlay = game::Dvar_RegisterEnum(
			/* name		*/ "xo_shaderoverlay",
			/* desc		*/ "fullscreen shaderoverlays. <CUSTOM> uses the material defined with \"xo_shaderoverlay_custom\"",
			/* default	*/ 0,
			/* enumSize	*/ xo_shaderoverlay_enum.size(),
			/* enumData */ xo_shaderoverlay_enum.data(),
			/* flags	*/ game::dvar_flags::none);

		dvars::xo_ssao_debugnormal = game::Dvar_RegisterBool(
			/* name		*/ "xo_ssao_debugnormal",
			/* desc		*/ "draw reconstructed normals from depth",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none);

		// SSAO FilterTap 0
		dvars::xo_ssao_noisescale	= game::Dvar_RegisterFloat("xo_ssao_noisescale", "hlsl constant filtertap[0][0] :: _NOISESCALE :: scale of noisemap", 0.0f, 0.0f, 100.0f, game::dvar_flags::none);
		dvars::xo_ssao_quality		= game::Dvar_RegisterFloat("xo_ssao_quality", "hlsl constant filtertap[0][1] :: _QUALITY :: 0 = Low, 1 = High", 1.0f, 0.0f, 1.0f, game::dvar_flags::none);

		// SSAO FilterTap 1
		dvars::xo_ssao_radius		= game::Dvar_RegisterFloat("xo_ssao_radius", "hlsl constant filtertap[1][0] :: _RADIUS :: sample radius", 0.3f, 0.0f, 20.0f, game::dvar_flags::none);

		// SSAO FilterTap 2
		dvars::xo_ssao_contrast		= game::Dvar_RegisterFloat("xo_ssao_contrast", "hlsl constant filtertap[2][0] :: _CONTRAST :: ao contrast", 0.65f, 0.0f, 20.0f, game::dvar_flags::none);
		dvars::xo_ssao_attenuation	= game::Dvar_RegisterFloat("xo_ssao_attenuation", "hlsl constant filtertap[2][1] :: _ATTENUATION :: ao attenuation", 30.0f, -200.0f, 200.0f, game::dvar_flags::none);
		dvars::xo_ssao_angleBias	= game::Dvar_RegisterFloat("xo_ssao_angleBias", "hlsl constant filtertap[2][2] :: _ANGLEBIAS :: in degrees", 16.0f, 0.0f, 90.0f, game::dvar_flags::none);

		// Outliner FilterTap 0
		dvars::xo_outliner_scale			= game::Dvar_RegisterFloat("xo_outliner_scale", "hlsl constant filtertap[0][0] :: edge sample radius / scale", 2.0f, 0.0f, 50.0f, game::dvar_flags::none);
		dvars::xo_outliner_depthDiffScale	= game::Dvar_RegisterFloat("xo_outliner_depthDiffScale", "hlsl constant filtertap[0][1] :: scale when edges with x depth difference get detected", 45.0f, 0.0f, 200.0f, game::dvar_flags::none);
		dvars::xo_outliner_depthThreshold	= game::Dvar_RegisterFloat("xo_outliner_depthThreshold", "hlsl constant filtertap[0][2] :: Edges are drawn between areas where the sampled depth is greater than depthThreshold", 6.0f, -10.0f, 60.0f, game::dvar_flags::none);

		// Outliner FilterTap 2
		dvars::xo_outliner_toonEnable = game::Dvar_RegisterBool("xo_outliner_toonEnable", "hlsl constant filtertap[2][0] :: toggle use of a simple toon shader", false, game::dvar_flags::none);
		dvars::xo_outliner_toonShades = game::Dvar_RegisterFloat("xo_outliner_toonShades", "hlsl constant filtertap[2][1] :: Max amount of shades / colors used by the toon shader", 6.0f, 0.0f, 64.0f, game::dvar_flags::none);


		// -----
		// Hooks

		// Disable Change from FLOATZ to SHADOWCOOKIE, to prevent AO lag or freezing
		utils::hook::nop(0x658FB2, 2);

		// Rewrite RB_DrawDebugPostEffects (Entry for custom post-effects)
		utils::hook(0x64AD70, RB_DrawDebugPostEffects_stub, HOOK_JUMP).install()->quick();

		utils::hook(0x64B2E6, ssao_draw_stub, HOOK_JUMP).install()->quick();
	}
}
