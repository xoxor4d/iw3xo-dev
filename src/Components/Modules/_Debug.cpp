#include "STDInclude.hpp"

namespace Components
{
	// -------------------------------------------------------------------------
	// Lines

	// *
	// Adds a debugline no matter what thread calls it
	void _Debug::AddDebugLineClient(float *start, float *end, float *color, int depthTest, int duration)
	{
		// renderer needs to be running and debug lines allocated
		if (Game::cls->rendererStarted && Game::CreateDebugLinesIfNeeded())
		{
			Game::clientDebugLineInfo_t *info = Game::clientDebugLineInfo_client;
			Game::trDebugLine_t *line;

			if (info->num + 1 <= info->max)
			{
				line = &info->lines[info->num];
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
	// Adds a debugline no matter what thread calls it (using glm vectors)
	void _Debug::AddDebugLineClient(const glm::vec3 start, const glm::vec3 end, const glm::vec4 color, int depthTest, int duration)
	{
		// renderer needs to be running and debug lines allocated
		if (Game::cls->rendererStarted && Game::CreateDebugLinesIfNeeded())
		{
			Game::clientDebugLineInfo_t *info = Game::clientDebugLineInfo_client;
			Game::trDebugLine_t *line;

			if (info->num + 1 <= info->max)
			{
				line = &info->lines[info->num];
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
	// Adds a debugline no matter what thread calls it
	void _Debug::AddDebugLineServer(const float* start, const float* end, const float* color, const int depthTest, const int duration)
	{
		// renderer needs to be running and debug lines allocated
		if (Game::cls->rendererStarted && Game::CreateDebugLinesIfNeeded())
		{
			Game::clientDebugLineInfo_t* info = Game::clientDebugLineInfo_server;
			Game::trDebugLine_t* line;

			if (info->num + 1 <= info->max)
			{
				line = &info->lines[info->num];
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
	void _Debug::DebugLines_InitSync(Game::dbgLinesDelaySync_t *sync)
	{
		if (!sync->initialized)
		{
			sync->delayInFrames = 10;
			sync->initialized = true;
		}

		if (sync->delayCounter < sync->delayInFrames)
		{
			sync->sync = false;
			sync->delayCounter++;
			return;
		}

		sync->delayCounter = 0;
		// sync is true for 1 frame 
		sync->sync = true;
	}

	// *
	// true if in sync
	bool _Debug::DebugLines_Sync(Game::dbgLinesDelaySync_t *sync)
	{
		if (sync->sync)
		{
			return true;
		}
			
		return false;
	}

	// *
	// directly draw 3D lines
	Game::GfxPointVertex debugLineVerts[2725];

	bool rb_addingDebugLines = false;
	bool rb_lastDepthTest = false;

	// add debugLines
	int _Debug::RB_AddDebugLine(const float* start, const float* end, const float* color, bool depthTest, int vertCount, int vertLimit, Game::GfxPointVertex* verts)
	{
		if (vertCount + 2 > vertLimit || rb_addingDebugLines && rb_lastDepthTest != depthTest)
		{
			Game::RB_DrawLines3D(vertCount / 2, Dvars::r_drawCollision_lineWidth->current.integer, verts, rb_lastDepthTest);
			vertCount = 0;
		}

		rb_lastDepthTest = depthTest;
		rb_addingDebugLines = true;

		Game::R_ConvertColorToBytes(color, verts[vertCount].color);

		*(DWORD*)verts[vertCount + 1].color = *(DWORD*)verts[vertCount].color;

		verts[vertCount].xyz[0] = start[0];
		verts[vertCount].xyz[1] = start[1];
		verts[vertCount].xyz[2] = start[2];

		verts[vertCount + 1].xyz[0] = end[0];
		verts[vertCount + 1].xyz[1] = end[1];
		verts[vertCount + 1].xyz[2] = end[2];

		return vertCount + 2;
	}

	// add debugLines
	int _Debug::RB_AddDebugLine(const glm::vec3& start, const glm::vec3& end, const float* color, bool depthTest, int lineWidth, int vertCount, int vertLimit, Game::GfxPointVertex* verts)
	{
		if (vertCount + 2 > vertLimit || rb_addingDebugLines && rb_lastDepthTest != depthTest)
		{
			Game::RB_DrawLines3D(vertCount / 2, lineWidth, verts, rb_lastDepthTest);
			vertCount = 0;
		}

		rb_lastDepthTest = depthTest;
		rb_addingDebugLines = true;

		Game::R_ConvertColorToBytes(color, verts[vertCount].color);
		*(DWORD*)verts[vertCount + 1].color = *(DWORD*)verts[vertCount].color;

		glm::setFloat3(verts[vertCount].xyz, start);
		glm::setFloat3(verts[vertCount + 1].xyz, end);

		return vertCount + 2;
	}

	// draw all created debugLines 
	void _Debug::RB_EndDebugLines(int vertCount, Game::GfxPointVertex* verts)
	{
		if (vertCount >= 2)
		{
			Game::RB_DrawLines3D(vertCount, Dvars::r_drawCollision_lineWidth->current.integer, verts, rb_lastDepthTest);
			rb_addingDebugLines = false;
		}
	}

	// add and draw debuglines
	void _Debug::RB_AddAndDrawDebugLines(const int numPoints, float(*points)[3], const float* colorFloat)
	{
		if (numPoints < 2)
		{
			return;
		}

		int vertCount = 0;
		int vertIndexPrev = numPoints - 1;

		for (auto vertIndex = 0; vertIndex < numPoints; ++vertIndex)
		{
			vertCount = _Debug::RB_AddDebugLine(&(*points)[3 * vertIndexPrev],
				&(*points)[3 * vertIndex],
				colorFloat,
				Dvars::r_drawCollision_polyDepth->current.enabled,
				vertCount,
				2725,
				debugLineVerts);

			vertIndexPrev = vertIndex;
		}

		// Draw all added debuglines
		_Debug::RB_EndDebugLines(vertCount / 2, debugLineVerts);
	}

	void _Debug::RB_EndDebugLines(int vertCount, Game::GfxPointVertex* verts, int lineWidth)
	{
		if (vertCount >= 2)
		{
			Game::RB_DrawLines3D(vertCount, lineWidth, verts, rb_lastDepthTest);
			rb_addingDebugLines = false;
		}
	}

	// add and draw debuglines (only call from renderer thread)
	void _Debug::RB_AddAndDrawDebugLines(const int numLines, const Game::dbgLines_t* lines, const float* colorFloat, bool depthTest, int lineWidth)
	{
		int vertCount = 0;

		for (auto line = 0; line < numLines; ++line)
		{
			vertCount = _Debug::RB_AddDebugLine(
				/* start	 */ lines[line].ptFrom,
				/* end		 */ lines[line].ptTo,
				/* color	 */ colorFloat,
				/* depth	 */ depthTest,
				/* linewidth */ lineWidth,
				/* vertcount */ vertCount,
				/* maxverts	 */ 2725,
				/* buffer	 */ debugLineVerts);
		}

		// Draw all added debuglines
		_Debug::RB_EndDebugLines(vertCount / 2, debugLineVerts, lineWidth);
	}

	// -------------------------------------------------------------------------
	// Strings

	// *
	// Add debugStrings
	void _Debug::AddDebugStringClient(const glm::vec3 xyz, const glm::vec4 color, float scale, const char *text, int duration)
	{
		// renderer needs to be running and debug strings allocated
		if (Game::ifRendererStarted && Game::CreateDebugStringsIfNeeded())
		{
			//Game::clientDebugStringInfo_t *info = Game::clsDebugSV_Strings; 
			Game::clientDebugStringInfo_t* info = Game::clsDebugCL_Strings;
			Game::trDebugString_t *string;

			if (info->num + 1 <= info->max)
			{
				string = &info->strings[info->num];
				string->xyz[0] = xyz.x;
				string->xyz[1] = xyz.y;
				string->xyz[2] = xyz.z;
				string->color[0] = color.r;
				string->color[1] = color.g;
				string->color[2] = color.b;
				string->color[3] = color.a;
				string->scale = scale;
				strncpy(string->text, text, 0x5Fu);
				string->text[95] = 0;
				info->durations[info->num++] = duration;
			}
		}
	}


	// -------------------------------------------------------------------------
	// Polygons

	// *
	// overflow check for render-surf
	void _Debug::RB_CheckTessOverflow(int vertexCount)
	{
		if (vertexCount + *Game::vertexCount > 5450 || (Game::tess->indexCount + 6) > 0x100000)
		{
			Game::RB_EndTessSurface();
			Game::RB_BeginSurface(*Game::OverflowTessTech, *Game::OverflowTessSurf);
		}
	}

	// *
	// set vertices for current render-surface
	void _Debug::RB_SetPolyVertWithNormal(const float* xyz, const float *normal, Game::GfxColor color, int vertCount, int vertNum)
	{
		Game::tess->verts[vertCount].xyzw[0] = xyz[0];
		Game::tess->verts[vertCount].xyzw[1] = xyz[1];
		Game::tess->verts[vertCount].xyzw[2] = xyz[2];
		Game::tess->verts[vertCount].xyzw[3] = 1.0f; // 0.0 to make it a sky

		Game::tess->verts[vertCount].color.packed = color.packed;
		//Game::tess->verts[vertCount].texCoord[0] = 0.0f;
		//Game::tess->verts[vertCount].texCoord[1] = 0.0f;

		switch (vertNum)
		{
		case 0:
			Game::tess->verts[vertCount].texCoord[0] = 0.0f;
			Game::tess->verts[vertCount].texCoord[1] = 0.0f;
			break;

		case 1:
			Game::tess->verts[vertCount].texCoord[0] = 0.0f;
			Game::tess->verts[vertCount].texCoord[1] = 1.0f;
			break;
		case 2:
			Game::tess->verts[vertCount].texCoord[0] = 1.0f;
			Game::tess->verts[vertCount].texCoord[1] = 1.0f;
			break;
		case 3:
			Game::tess->verts[vertCount].texCoord[0] = 1.0f;
			Game::tess->verts[vertCount].texCoord[1] = 0.0f;
			break;

		default:
			Game::tess->verts[vertCount].texCoord[0] = 0.0f;
			Game::tess->verts[vertCount].texCoord[1] = 0.0f;
			break;
		}

		auto packedNormal = Game::Vec3PackUnitVec(normal);
		Game::tess->verts[vertCount].normal = packedNormal;

		//Game::tess->verts[vertCount].normal.packed = 1073643391;
	}

	// *
	// set vertices for current render-surface
	void _Debug::RB_SetPolyVert(const float *xyz, Game::GfxColor color, int vertCount, int vertNum)
	{
		Game::tess->verts[vertCount].xyzw[0] = xyz[0];
		Game::tess->verts[vertCount].xyzw[1] = xyz[1];
		Game::tess->verts[vertCount].xyzw[2] = xyz[2];
		Game::tess->verts[vertCount].xyzw[3] = 1.0f; // 0.0 to make it a sky

		Game::tess->verts[vertCount].color.packed = color.packed;
		//Game::tess->verts[vertCount].texCoord[0] = 0.0f;
		//Game::tess->verts[vertCount].texCoord[1] = 0.0f;

		switch (vertNum)
		{
		case 0:
			Game::tess->verts[vertCount].texCoord[0] = 0.0f;
			Game::tess->verts[vertCount].texCoord[1] = 0.0f;
			break;

		case 1:
			Game::tess->verts[vertCount].texCoord[0] = 0.0f;
			Game::tess->verts[vertCount].texCoord[1] = 1.0f;
			break;
		case 2:
			Game::tess->verts[vertCount].texCoord[0] = 1.0f;
			Game::tess->verts[vertCount].texCoord[1] = 1.0f;
			break;
		case 3:
			Game::tess->verts[vertCount].texCoord[0] = 1.0f;
			Game::tess->verts[vertCount].texCoord[1] = 0.0f;
			break;

		default:
			Game::tess->verts[vertCount].texCoord[0] = 0.0f;
			Game::tess->verts[vertCount].texCoord[1] = 0.0f;
			break;
		}

		Game::tess->verts[vertCount].normal.packed = 1073643391;
	}

	// *
	// draws a debug polygon :: needs atleast 3 valid points
	void _Debug::RB_DrawPoly(const int numPoints, float(*points)[3], const float *brushColor, bool brushLit, bool outlines, const float *outlineColor, bool depthCheck, bool twoSidesPoly)
	{
		int vertIndex;
		Game::GfxColor color;

		if (numPoints < 3)
		{
			return;
		}

		// ----------
		// Draw Polys

		Game::R_ConvertColorToBytes(brushColor, (char *)&color);

		// check render-surface overflow
		if (Game::OverflowTessSurf != Game::builtIn_material_unlit_depth || *Game::OverflowTessTech != Game::MaterialTechniqueType::TECHNIQUE_UNLIT)
		{
			// draw / skip left over polys
			if (Game::tess->indexCount)
			{
				Game::RB_EndTessSurface();
			}

			if (brushLit)
			{
				// use a custom material for polygons
				Game::Material* unlit_material = reinterpret_cast<Game::Material*>(Game::Material_RegisterHandle("iw3xo_showcollision_fakelight", 3));
			
				if (!unlit_material)
				{
					Game::Com_Error(0, Utils::VA("^1_Debug::RB_DrawPoly L#%d ^7:: unlit_material was null\n", __LINE__));
				}

				// dirty shader constants for our fakelight shader
				Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_3][0] = Game::_gfxWorld->sunParse.ambientScale * Game::_gfxWorld->sunParse.ambientColor[0];
				Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_3][1] = Game::_gfxWorld->sunParse.ambientScale * Game::_gfxWorld->sunParse.ambientColor[1];
				Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_3][2] = Game::_gfxWorld->sunParse.ambientScale * Game::_gfxWorld->sunParse.ambientColor[2];

				// start poly
				Game::RB_BeginSurface_CustomMaterial(Game::MaterialTechniqueType::TECHNIQUE_UNLIT, unlit_material);
			}
			else
			{
				// patch default line material so that it uses Blend and PolyOffset
				Game::Material* unlit_material = reinterpret_cast<Game::Material*>(*(DWORD32*)(Game::builtIn_material_unlit_depth));

				// fill poly on both sides
				if (twoSidesPoly)
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
				Game::RB_BeginSurface(Game::MaterialTechniqueType::TECHNIQUE_UNLIT, depthCheck ? *Game::builtIn_material_unlit_depth : *Game::builtIn_material_unlit);
			}
		}

		// Check if we would overflow our Surface and if we would, render all added polys
		_Debug::RB_CheckTessOverflow(numPoints);

		if (brushLit)
		{
			// Calculate face normals
			glm::vec3 u, v, glNormal;

			// Cross
			u = glm::toVec3(points[1]) - glm::toVec3(points[0]);
			v = glm::toVec3(points[2]) - glm::toVec3(points[0]);

			glNormal.x = (u.y * v.z) - (u.z * v.y);
			glNormal.y = (u.z * v.x) - (u.x * v.z);
			glNormal.z = (u.x * v.y) - (u.y * v.x);

			float normal[3];
			glm::setFloat3(normal, glNormal);

			for (vertIndex = 0; vertIndex < numPoints; ++vertIndex)
			{
				_Debug::RB_SetPolyVertWithNormal(&(*points)[3 * vertIndex], normal, color, *Game::vertexCount + vertIndex, vertIndex);
			}
		}
		else
		{
			for (vertIndex = 0; vertIndex < numPoints; ++vertIndex)
			{
				_Debug::RB_SetPolyVert(&(*points)[3 * vertIndex], color, *Game::vertexCount + vertIndex, vertIndex);
			}
		}

		for (vertIndex = 0; vertIndex < numPoints - 2; ++vertIndex)
		{
			Game::tess->indices[Game::tess->indexCount + 0] = (unsigned short int)(0);
			Game::tess->indices[Game::tess->indexCount + 1] = (unsigned short int)(vertIndex + 2);
			Game::tess->indices[Game::tess->indexCount + 2] = (unsigned short int)(vertIndex + 1);
			Game::tess->indexCount += 3;
		}

		Game::tess->vertexCount += numPoints;

		// Draw all added polys
		Game::RB_EndTessSurface();


		// ------------------------------
		// Draw Polys as wireframe on top

		if (outlines)
		{
			// check render-surface overflow
			if (Game::OverflowTessSurf != Game::builtIn_material_unlit_depth || *Game::OverflowTessTech != Game::MaterialTechniqueType::TECHNIQUE_WIREFRAME_SOLID)
			{
				// draw / skip left over polys
				if (Game::tess->indexCount)
				{
					Game::RB_EndTessSurface();
				}

				// use a custom material for outlines
				Game::Material* unlit_material = reinterpret_cast<Game::Material*>(Game::Material_RegisterHandle("iw3xo_showcollision_wire", 3));

				if (!unlit_material)
				{
					Game::Com_Error(0, Utils::VA("^1_Debug::RB_DrawPoly L#%d ^7:: unlit_material was null\n", __LINE__));
					return;
				}

				Game::RB_BeginSurface_CustomMaterial(Game::MaterialTechniqueType::TECHNIQUE_UNLIT, unlit_material);
			}

			// Check if we would overflow our Surface and if we would, render all added polys
			_Debug::RB_CheckTessOverflow(numPoints);

			if (outlineColor == nullptr)
			{
				float tempColor[3] = { 1.0f, 0.0f, 0.0f };
				Game::R_ConvertColorToBytes(tempColor, (char*)&color);
			}
			else
			{
				Game::R_ConvertColorToBytes(outlineColor, (char*)&color);
			}
			
			// set our surface verts (tess->verts)
			for (vertIndex = 0; vertIndex < numPoints; ++vertIndex)
			{
				_Debug::RB_SetPolyVert(&(*points)[3 * vertIndex], color, *Game::vertexCount + vertIndex);
			}

			// counter-clockwise polys?
			for (vertIndex = 0; vertIndex < numPoints - 2; ++vertIndex)
			{
				Game::tess->indices[Game::tess->indexCount + 0] = (unsigned short int)(0);
				Game::tess->indices[Game::tess->indexCount + 1] = (unsigned short int)(vertIndex + 2);
				Game::tess->indices[Game::tess->indexCount + 2] = (unsigned short int)(vertIndex + 1);

				Game::tess->indexCount += 3;
			}

			Game::tess->vertexCount += numPoints;

			// Draw all added polys
			Game::RB_EndTessSurface();
		}
	}


	// *
	// Main

	// pre RB_DrawDebug :: additional debug functions here
	void RB_AdditionalDebug(Game::GfxViewParms *viewParms)
	{
		if (Components::active.RB_DrawCollision)
		{
			// Draw Debug Collision
			RB_DrawCollision::RB_ShowCollision(viewParms);
		}
		
		if (Components::active.RadiantRemote)
		{
			// Radiant Live-Link
			RadiantRemote::RadiantDebugBrush();
		}

		if (Components::active._Pmove)
		{
			_Pmove::PM_DrawDebug();
		}
	}

	// RB_AdditionalDebug :: Hook RB_DrawDebug call to implement additional debug functions
	__declspec(naked) void RB_EndSceneRendering_stub()
	{
		const static uint32_t RB_DrawDebug = 0x658860;
		__asm
		{
			pushad;
			push	[0xD540220];
			call	RB_AdditionalDebug;
			add		esp, 4h;
			popad;

			call	RB_DrawDebug;
			retn;
		}
	}

	_Debug::_Debug()
	{ 
		// Disable the need for developer 1 to display collisions / debug lines / strings w/e (nop check if dvar is set in RB_EndSceneRendering)
		Utils::Hook::Nop(0x6496D8, 3);

		// Hook RB_DrawDebug call in RB_EndSceneRendering to implement additional debug functions
		Utils::Hook(0x6496EC, RB_EndSceneRendering_stub, HOOK_CALL).install()->quick();
	}

	_Debug::~_Debug()
	{ }
}