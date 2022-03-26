#pragma once

#define LISTBOX_SLIDER_THUMB_COLOR { 1.0f, 1.0f, 1.0f, 0.1f }
#define LISTBOX_SLIDER_BACKGROUND_COLOR { 1.0f, 1.0f, 1.0f, 0.02f }

namespace components
{
	class _ui final : public component
	{
	public:
		_ui();
		const char* getName() override { return "_ui"; };

		static int		get_text_height(Game::Font_s *font);
		static float	scrplace_apply_x(int horz_align, const float x, const float x_offs);
		static float	scrplace_apply_y(int vert_align, const float y, const float y_offs);

		static void		scrplace_apply_rect(float *offs_x, float *w, float *offs_y, float *h, int horz_anker, int vert_anker);
		static void		scrplacefull_apply_rect(float *offs_x, float *w, float *offs_y, float *h, int horz_anker, int vert_anker);

		static void		register_dvars();
		static void		create_changelog(Game::gui_menus_t& menu);
		static void		redraw_cursor();
	};
}
