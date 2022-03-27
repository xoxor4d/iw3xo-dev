#include "std_include.hpp"

namespace components
{
	// -------------------------------------------------------------------------
	// Lines

	// *
	// adds a debugline no matter what thread calls it
	void _debug::add_debug_line_client(float *start, float *end, float *color, int depthTest, int duration)
	{
		// renderer needs to be running and debug lines allocated
		if (game::cls->rendererStarted && game::CreateDebugLinesIfNeeded())
		{
			game::clientDebugLineInfo_t *info = game::clientDebugLineInfo_client;
			if (info->num + 1 <= info->max)
			{
				game::trDebugLine_t* line = &info->lines[info->num];
				line->start[0] = start[0];
				line->start[1] = start[1];
				line->start[2] = start[2];
				line->end[0] = end[0];
				line->end[1] = end[1];
				line->end[2] = end[2];
				line->color[0] = *color;
				line->color[1] = color[1];
				line->color[2] = color[2];
				line->color[3] = color[3];
				line->depthTest = depthTest;
				info->durations[info->num++] = duration;
			}
		}
	}

	// *
	// adds a debugline no matter what thread calls it (using glm vectors)
	void _debug::add_debug_line_client(const glm::vec3 start, const glm::vec3 end, const glm::vec4 color, int depthTest, int duration)
	{
		// renderer needs to be running and debug lines allocated
		if (game::cls->rendererStarted && game::CreateDebugLinesIfNeeded())
		{
			game::clientDebugLineInfo_t *info = game::clientDebugLineInfo_client;
			if (info->num + 1 <= info->max)
			{
				game::trDebugLine_t* line = &info->lines[info->num];
				line->start[0] = start.x;
				line->start[1] = start.y;
				line->start[2] = start.z;
				line->end[0] = end.x;
				line->end[1] = end.y;
				line->end[2] = end.z;
				line->color[0] = color.r;
				line->color[1] = color.g;
				line->color[2] = color.b;
				line->color[3] = color.a;
				line->depthTest = depthTest;
				info->durations[info->num++] = duration;
			}
		}
	}

	// *
	// adds a debugline no matter what thread calls it
	void _debug::add_debug_line_server(const float* start, const float* end, const float* color, const int depthTest, const int duration)
	{
		// renderer needs to be running and debug lines allocated
		if (game::cls->rendererStarted && game::CreateDebugLinesIfNeeded())
		{
			game::clientDebugLineInfo_t* info = game::clientDebugLineInfo_server;
			if (info->num + 1 <= info->max)
			{
				game::trDebugLine_t* line = &info->lines[info->num];
				line->start[0] = start[0];
				line->start[1] = start[1];
				line->start[2] = start[2];
				line->end[0] = end[0];
				line->end[1] = end[1];
				line->end[2] = end[2];
				line->color[0] = color[0];
				line->color[1] = color[1];
				line->color[2] = color[2];
				line->color[3] = color[3];
				line->depthTest = depthTest;
				info->durations[info->num++] = duration;
			}
		}
	}

	// *
	// init and create 1 timer for all debug lines
	void _debug::debug_lines_init_sync(game::debug_lines_delay_sync_s *sync)
	{
		if (!sync->initialized)
		{
			sync->delay_in_frames = 10;
			sync->initialized = true;
		}

		if (sync->delay_counter < sync->delay_in_frames)
		{
			sync->sync = false;
			sync->delay_counter++;
			return;
		}

		sync->delay_counter = 0;
		// sync is true for 1 frame 
		sync->sync = true;
	}

	// *
	// true if in sync
	bool _debug::debug_lines_sync(game::debug_lines_delay_sync_s *sync)
	{
		if (sync->sync)
		{
			return true;
		}
			
		return false;
	}

	// *
	// directly draw 3D lines
	game::GfxPointVertex debug_line_verts[2725];

	bool rb_adding_debug_lines = false;
	bool rb_last_depth_test = false;

	// add debugLines
	int _debug::add_debug_line(const float* start, const float* end, const float* color, bool depth_test, int vert_count, int vert_limit, game::GfxPointVertex* verts)
	{
		if (vert_count + 2 > vert_limit || rb_adding_debug_lines && rb_last_depth_test != depth_test)
		{
			game::RB_DrawLines3D(vert_count / 2, dvars::r_drawCollision_lineWidth->current.integer, verts, rb_last_depth_test);
			vert_count = 0;
		}

		rb_last_depth_test = depth_test;
		rb_adding_debug_lines = true;

		game::R_ConvertColorToBytes(color, verts[vert_count].color);

		*(DWORD*)verts[vert_count + 1].color = *(DWORD*)verts[vert_count].color;

		verts[vert_count].xyz[0] = start[0];
		verts[vert_count].xyz[1] = start[1];
		verts[vert_count].xyz[2] = start[2];

		verts[vert_count + 1].xyz[0] = end[0];
		verts[vert_count + 1].xyz[1] = end[1];
		verts[vert_count + 1].xyz[2] = end[2];

		return vert_count + 2;
	}

	// add debugLines
	int _debug::add_debug_line(const glm::vec3& start, const glm::vec3& end, const float* color, bool depth_test, int line_width, int vert_count, int vert_limit, game::GfxPointVertex* verts)
	{
		if (vert_count + 2 > vert_limit || rb_adding_debug_lines && rb_last_depth_test != depth_test)
		{
			game::RB_DrawLines3D(vert_count / 2, line_width, verts, rb_last_depth_test);
			vert_count = 0;
		}

		rb_last_depth_test = depth_test;
		rb_adding_debug_lines = true;

		game::R_ConvertColorToBytes(color, verts[vert_count].color);
		*(DWORD*)verts[vert_count + 1].color = *(DWORD*)verts[vert_count].color;

		glm::set_float3(verts[vert_count].xyz, start);
		glm::set_float3(verts[vert_count + 1].xyz, end);

		return vert_count + 2;
	}

	// draw all created debugLines 
	void _debug::end_debug_lines(int vert_count, game::GfxPointVertex* verts)
	{
		if (vert_count >= 2)
		{
			game::RB_DrawLines3D(vert_count, dvars::r_drawCollision_lineWidth->current.integer, verts, rb_last_depth_test);
			rb_adding_debug_lines = false;
		}
	}

	// add and draw debuglines
	void _debug::add_and_draw_debug_lines(const int num_points, float(*points)[3], const float* color_float)
	{
		if (num_points < 2)
		{
			return;
		}

		int vert_count = 0;
		int vert_index_prev = num_points - 1;

		for (auto vert_index = 0; vert_index < num_points; ++vert_index)
		{
			vert_count = _debug::add_debug_line(&(*points)[3 * vert_index_prev],
				&(*points)[3 * vert_index],
				color_float,
				dvars::r_drawCollision_polyDepth->current.enabled,
				vert_count,
				2725,
				debug_line_verts);

			vert_index_prev = vert_index;
		}

		// draw all added debuglines
		_debug::end_debug_lines(vert_count / 2, debug_line_verts);
	}

	void _debug::end_debug_lines(int vert_count, game::GfxPointVertex* verts, int line_width)
	{
		if (vert_count >= 2)
		{
			game::RB_DrawLines3D(vert_count, line_width, verts, rb_last_depth_test);
			rb_adding_debug_lines = false;
		}
	}

	// add and draw debuglines (only call from renderer thread)
	void _debug::add_and_draw_debug_lines(const int num_lines, const game::debug_line_s* lines, const float* color_float, bool depth_test, int line_width)
	{
		int vert_count = 0;

		for (auto line = 0; line < num_lines; ++line)
		{
			vert_count = _debug::add_debug_line(
				/* start	 */ lines[line].ptFrom,
				/* end		 */ lines[line].ptTo,
				/* color	 */ color_float,
				/* depth	 */ depth_test,
				/* linewidth */ line_width,
				/* vertcount */ vert_count,
				/* maxverts	 */ 2725,
				/* buffer	 */ debug_line_verts);
		}

		// draw all added debuglines
		_debug::end_debug_lines(vert_count / 2, debug_line_verts, line_width);
	}

	// -------------------------------------------------------------------------
	// Strings

	// *
	// add debugStrings
	void _debug::add_debug_string_client(const glm::vec3 xyz, const glm::vec4 color, float scale, const char *text, int duration)
	{
		// renderer needs to be running and debug strings allocated
		if (game::cls->rendererStarted && game::CreateDebugStringsIfNeeded())
		{
			if (const auto	info = &game::cls->debug.clStrings; 
							info->num + 1 <= info->max)
			{
				game::trDebugString_t* string = &info->strings[info->num];
				string->xyz[0] = xyz.x;
				string->xyz[1] = xyz.y;
				string->xyz[2] = xyz.z;
				string->color[0] = color.r;
				string->color[1] = color.g;
				string->color[2] = color.b;
				string->color[3] = color.a;
				string->scale = scale;
				info->durations[info->num++] = duration;

				strncpy(string->text, text, 0x5Fu);
				string->text[95] = 0;
			}
		}
	}


	// -------------------------------------------------------------------------
	// Polygons

	// *
	// overflow check for render-surf
	void _debug::check_tess_overflow(int vertex_count)
	{
		if (vertex_count + game::tess->vertexCount > 5450 || game::tess->indexCount + 6 > 0x100000)
		{
			game::RB_EndTessSurface();
			game::RB_BeginSurface(game::gfxCmdBufState->origTechType, game::gfxCmdBufState->origMaterial);
		}
	}

	// *
	// set vertices for current render-surface
	void _debug::set_poly_vert_with_normal(const float* xyz, const float *normal, game::GfxColor color, int vert_count, int vertNum)
	{
		game::tess->verts[vert_count].xyzw[0] = xyz[0];
		game::tess->verts[vert_count].xyzw[1] = xyz[1];
		game::tess->verts[vert_count].xyzw[2] = xyz[2];
		game::tess->verts[vert_count].xyzw[3] = 1.0f; // 0.0 to make it a sky
		game::tess->verts[vert_count].color.packed = color.packed;

		switch (vertNum)
		{
		case 0:
			game::tess->verts[vert_count].texCoord[0] = 0.0f;
			game::tess->verts[vert_count].texCoord[1] = 0.0f;
			break;

		case 1:
			game::tess->verts[vert_count].texCoord[0] = 0.0f;
			game::tess->verts[vert_count].texCoord[1] = 1.0f;
			break;
		case 2:
			game::tess->verts[vert_count].texCoord[0] = 1.0f;
			game::tess->verts[vert_count].texCoord[1] = 1.0f;
			break;
		case 3:
			game::tess->verts[vert_count].texCoord[0] = 1.0f;
			game::tess->verts[vert_count].texCoord[1] = 0.0f;
			break;

		default:
			game::tess->verts[vert_count].texCoord[0] = 0.0f;
			game::tess->verts[vert_count].texCoord[1] = 0.0f;
			break;
		}

		game::tess->verts[vert_count].normal = game::Vec3PackUnitVec(normal);
	}

	// *
	// set vertices for current render-surface
	void _debug::set_poly_vert(const float *xyz, game::GfxColor color, int vertCount, int vertNum)
	{
		game::tess->verts[vertCount].xyzw[0] = xyz[0];
		game::tess->verts[vertCount].xyzw[1] = xyz[1];
		game::tess->verts[vertCount].xyzw[2] = xyz[2];
		game::tess->verts[vertCount].xyzw[3] = 1.0f; // 0.0 to make it a sky
		game::tess->verts[vertCount].color.packed = color.packed;

		switch (vertNum)
		{
		case 0:
			game::tess->verts[vertCount].texCoord[0] = 0.0f;
			game::tess->verts[vertCount].texCoord[1] = 0.0f;
			break;

		case 1:
			game::tess->verts[vertCount].texCoord[0] = 0.0f;
			game::tess->verts[vertCount].texCoord[1] = 1.0f;
			break;
		case 2:
			game::tess->verts[vertCount].texCoord[0] = 1.0f;
			game::tess->verts[vertCount].texCoord[1] = 1.0f;
			break;
		case 3:
			game::tess->verts[vertCount].texCoord[0] = 1.0f;
			game::tess->verts[vertCount].texCoord[1] = 0.0f;
			break;

		default:
			game::tess->verts[vertCount].texCoord[0] = 0.0f;
			game::tess->verts[vertCount].texCoord[1] = 0.0f;
			break;
		}

		game::tess->verts[vertCount].normal.packed = 1073643391;
	}

	// *
	// draws a debug polygon :: needs atleast 3 valid points
	void _debug::draw_poly(const int num_points, float(*points)[3], const float *brush_color, bool brush_lit, bool outlines, const float *outline_color, bool depth_check, bool two_sides_poly)
	{
		if (num_points < 3)
		{
			return;
		}

		int vert_index;

		game::GfxColor color = {};
		game::R_ConvertColorToBytes(brush_color, (char *)&color);

		// check render-surface overflow
		if (game::gfxCmdBufState->origMaterial != game::rgp->lineMaterial || game::gfxCmdBufState->origTechType != game::MaterialTechniqueType::TECHNIQUE_UNLIT)
		{
			// draw / skip left over polys
			if (game::tess->indexCount)
			{
				game::RB_EndTessSurface();
			}

			if (brush_lit)
			{
				// use a custom material for polygons
				game::Material* unlit_material = game::Material_RegisterHandle("iw3xo_showcollision_fakelight", 3);
			
				if (!unlit_material)
				{
					game::Com_Error(0, utils::va("^1_debug::draw_poly L#%d ^7:: unlit_material was null\n", __LINE__));
				}

				// dirty shader constants for our fakelight shader
				game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_3][0] = game::gfx_world->sunParse.ambientScale * game::gfx_world->sunParse.ambientColor[0];
				game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_3][1] = game::gfx_world->sunParse.ambientScale * game::gfx_world->sunParse.ambientColor[1];
				game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_3][2] = game::gfx_world->sunParse.ambientScale * game::gfx_world->sunParse.ambientColor[2];

				// start poly
				game::RB_BeginSurface_CustomMaterial(game::MaterialTechniqueType::TECHNIQUE_UNLIT, unlit_material);
			}
			else
			{
				// patch default line material so that it uses Blend and PolyOffset
				const auto unlit_material = game::rgp->lineMaterial;//reinterpret_cast<game::Material*>(*(DWORD32*)(game::builtIn_material_unlit_depth));

				// fill poly on both sides
				if (two_sides_poly)
				{
					// blendFunc Blend + cullFace "None"
					unlit_material->stateBitsTable->loadBits[0] = 422072677;
				}
				else // 1 sided poly
				{
					// blendFunc Blend + cullFace "Back"
					unlit_material->stateBitsTable->loadBits[0] = 422089061;
				}

				// give poly a slight offset to stop z-fighting :: polyOffset StaticDecal
				unlit_material->stateBitsTable->loadBits[1] = 44;

				// start poly
				game::RB_BeginSurface(game::MaterialTechniqueType::TECHNIQUE_UNLIT, depth_check ? game::rgp->lineMaterial : game::rgp->lineMaterialNoDepth);
			}
		}

		// render all added polys if we would overflow the surface by adding new ones
		_debug::check_tess_overflow(num_points);

		if (brush_lit)
		{
			game::vec3_t pt1, pt2, normal;

			utils::vector::subtract3(points[1], points[0], pt1);
			utils::vector::subtract3(points[2], points[0], pt2);
			utils::vector::cross3(pt1, pt2, normal);

			for (vert_index = 0; vert_index < num_points; ++vert_index)
			{
				_debug::set_poly_vert_with_normal(&(*points)[3 * vert_index], normal, color, game::tess->vertexCount + vert_index, vert_index);
			}
		}
		else
		{
			for (vert_index = 0; vert_index < num_points; ++vert_index)
			{
				_debug::set_poly_vert(&(*points)[3 * vert_index], color, game::tess->vertexCount + vert_index, vert_index);
			}
		}

		for (vert_index = 0; vert_index < num_points - 2; ++vert_index)
		{
			game::tess->indices[game::tess->indexCount + 0] = (unsigned short int)(0);
			game::tess->indices[game::tess->indexCount + 1] = (unsigned short int)(vert_index + 2);
			game::tess->indices[game::tess->indexCount + 2] = (unsigned short int)(vert_index + 1);
			game::tess->indexCount += 3;
		}

		game::tess->vertexCount += num_points;

		// draw all added polys
		game::RB_EndTessSurface();


		// ------------------------------
		// Draw Polys as wireframe on top

		if (outlines)
		{
			// check render-surface overflow
			if (game::gfxCmdBufState->origMaterial != game::rgp->lineMaterial || game::gfxCmdBufState->origTechType != game::MaterialTechniqueType::TECHNIQUE_WIREFRAME_SOLID)
			{
				// draw / skip left over polys
				if (game::tess->indexCount)
				{
					game::RB_EndTessSurface();
				}

				// use a custom material for outlines
				const auto unlit_material = game::Material_RegisterHandle("iw3xo_showcollision_wire", 3);

				if (!unlit_material)
				{
					game::Com_Error(0, utils::va("^1_debug::draw_poly L#%d ^7:: unlit_material was null\n", __LINE__));
					return;
				}

				game::RB_BeginSurface_CustomMaterial(game::MaterialTechniqueType::TECHNIQUE_UNLIT, unlit_material);
			}

			// Check if we would overflow our Surface and if we would, render all added polys
			_debug::check_tess_overflow(num_points);

			if (outline_color == nullptr)
			{
				float tempColor[3] = { 1.0f, 0.0f, 0.0f };
				game::R_ConvertColorToBytes(tempColor, (char*)&color);
			}
			else
			{
				game::R_ConvertColorToBytes(outline_color, (char*)&color);
			}
			
			// set our surface verts (tess->verts)
			for (vert_index = 0; vert_index < num_points; ++vert_index)
			{
				_debug::set_poly_vert(&(*points)[3 * vert_index], color, game::tess->vertexCount + vert_index);
			}

			// counter-clockwise polys?
			for (vert_index = 0; vert_index < num_points - 2; ++vert_index)
			{
				game::tess->indices[game::tess->indexCount + 0] = (unsigned short int)(0);
				game::tess->indices[game::tess->indexCount + 1] = (unsigned short int)(vert_index + 2);
				game::tess->indices[game::tess->indexCount + 2] = (unsigned short int)(vert_index + 1);

				game::tess->indexCount += 3;
			}

			game::tess->vertexCount += num_points;

			// Draw all added polys
			game::RB_EndTessSurface();
		}
	}


	// *
	// Main

	// pre RB_DrawDebug :: additional debug functions here
	void additional_debug(game::GfxViewParms *view_parms)
	{
		if (components::active.draw_collision)
		{
			// Draw Debug Collision
			draw_collision::debug_collision_frame(view_parms);
		}
		
		if (components::active.radiant_livelink)
		{
			// Radiant Live-Link
			radiant_livelink::draw_debug_brush();
		}

		if (components::active._pmove)
		{
			_pmove::draw_debug();
		}
	}

	// additional_debug :: hook RB_DrawDebug call to implement additional debug functions
	__declspec(naked) void RB_EndSceneRendering_stub()
	{
		const static uint32_t RB_DrawDebug_func = 0x658860;
		__asm
		{
			pushad;
			push	[0xD540220];
			call	additional_debug;
			add		esp, 4h;
			popad;

			call	RB_DrawDebug_func;
			retn;
		}
	}

	_debug::_debug()
	{ 
		// disable the need for developer 1 to display collisions / debug lines / strings w/e (nop check if dvar is set in RB_EndSceneRendering)
		utils::hook::nop(0x6496D8, 3);

		// hook RB_DrawDebug call in RB_EndSceneRendering to implement additional debug functions
		utils::hook(0x6496EC, RB_EndSceneRendering_stub, HOOK_CALL).install()->quick();
	}
}