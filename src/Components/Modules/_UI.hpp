#pragma once

#define LISTBOX_SLIDER_THUMB_COLOR { 1.0f, 1.0f, 1.0f, 0.1f }
#define LISTBOX_SLIDER_BACKGROUND_COLOR { 1.0f, 1.0f, 1.0f, 0.02f }

namespace Components
{
	class _UI : public Component
	{
	public:
		_UI();
		~_UI();
		const char* getName() override { return "_UI"; };

		static int		GetTextHeight(Game::Font_s *font);
		static float	ScrPlace_ApplyX(int horzAlign, const float x, const float xOffs);
		static float	ScrPlace_ApplyY(int vertAlign, const float y, const float yOffs);

		static void		ScrPlace_ApplyRect(float *offs_x, float *w, float *offs_y, float *h, int horzAnker, int vertAnker);
		static void		ScrPlaceFull_ApplyRect(float *offs_x, float *w, float *offs_y, float *h, int horzAnker, int vertAnker);

		static void		main_menu_register_dvars();
		static void		create_changelog(Game::gui_menus_t& menu);
		static void		redraw_cursor();

	private:
	};
}
