#pragma once

namespace components
{
	class _debug final : public component
	{
	public:
		_debug();
		const char* get_name() override { return "_debug"; };

		// -------------------
		// Lines

		static void add_debug_line_client(float *start, float *end, float *color, int depthTest, int duration);
		static void add_debug_line_client(const glm::vec3 start, const glm::vec3 end, const glm::vec4 color, int depthTest, int duration);
		static void add_debug_line_server(const float* start, const float* end, const float* color, const int depthTest, const int duration);
		static void debug_lines_init_sync(Game::dbgLinesDelaySync_t *sync);
		static bool debug_lines_sync(Game::dbgLinesDelaySync_t *sync);

		static int  add_debug_line(const float* start, const float* end, const float* color, bool depth_test, int vert_count, int vert_limit, Game::GfxPointVertex* verts);
		static int	add_debug_line(const glm::vec3& start, const glm::vec3& end, const float* color, bool depth_test, int line_width, int vert_count, int vert_limit, Game::GfxPointVertex* verts);
		static void add_and_draw_debug_lines(const int num_points, float(*points)[3], const float* color_float);
		static void add_and_draw_debug_lines(const int num_lines, const Game::dbgLines_t* lines, const float* color_float, bool depth_test, int line_width);
		static void end_debug_lines(int vert_count, Game::GfxPointVertex* verts);
		static void end_debug_lines(int vert_count, Game::GfxPointVertex* verts, int line_width);

		// -------------------
		// Strings

		static void add_debug_string_client(const glm::vec3 xyz, const glm::vec4 color, float scale, const char *text, int duration);


		// -------------------
		// PolygonsW

		static void	check_tess_overflow(int vertex_count);
		static void	set_poly_vert_with_normal(const float* xyz, const float* normal, Game::GfxColor color, int vert_count, int vertNum);
		static void	set_poly_vert(const float *xyz, Game::GfxColor color, int vertCount, int vertNum = 0);
		static void	draw_poly(const int num_points, float(*points)[3], const float* brush_color, bool brush_lit, bool outlines, const float* outline_color, bool depth_check, bool two_sides_poly);
	};
}
