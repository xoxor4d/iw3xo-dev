#pragma once

namespace Components
{
	class _Debug : public Component
	{
	public:
		_Debug();
		~_Debug();

		const char* getName() override { return "_Debug"; };

		// -------------------
		// Lines

		static void AddDebugLineClient(float *start, float *end, float *color, int depthTest, int duration);
		static void AddDebugLineClient(const glm::vec3 start, const glm::vec3 end, const glm::vec4 color, int depthTest, int duration);
		static void AddDebugLineServer(const float* start, const float* end, const float* color, const int depthTest, const int duration);
		static void DebugLines_InitSync(Game::dbgLinesDelaySync_t *sync);
		static bool DebugLines_Sync(Game::dbgLinesDelaySync_t *sync);

		static int  RB_AddDebugLine(const float* start, const float* end, const float* color, bool depthTest, int vertCount, int vertLimit, Game::GfxPointVertex* verts);
		static int	RB_AddDebugLine(const glm::vec3& start, const glm::vec3& end, const float* color, bool depthTest, int lineWidth, int vertCount, int vertLimit, Game::GfxPointVertex* verts);
		static void RB_AddAndDrawDebugLines(const int numPoints, float(*points)[3], const float* colorFloat);
		static void RB_AddAndDrawDebugLines(const int numLines, const Game::dbgLines_t* lines, const float* colorFloat, bool depthTest, int lineWidth);
		static void RB_EndDebugLines(int vertCount, Game::GfxPointVertex* verts);
		static void RB_EndDebugLines(int vertCount, Game::GfxPointVertex* verts, int lineWidth);

		// -------------------
		// Strings

		static void AddDebugStringClient(const glm::vec3 xyz, const glm::vec4 color, float scale, const char *text, int duration);


		// -------------------
		// Polygons

		static void		RB_CheckTessOverflow(int vertexCount);
		static void		RB_SetPolyVertWithNormal(const float* xyz, const float* normal, Game::GfxColor color, int vertCount, int vertNum);
		static void		RB_SetPolyVert(const float *xyz, Game::GfxColor color, int vertCount, int vertNum = 0);
		static void		RB_DrawPoly(const int numPoints, float(*points)[3], const float* brushColor, bool brushLit, bool outlines, const float* outlineColor, bool depthCheck, bool twoSidesPoly);

	private:
	};
}
