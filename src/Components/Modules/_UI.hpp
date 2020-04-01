#pragma once

#define LISTBOX_SLIDER_THUMB_COLOR { 1.0f, 1.0f, 1.0f, 0.1f }
#define LISTBOX_SLIDER_BACKGROUND_COLOR { 1.0f, 1.0f, 1.0f, 0.02f }

#define HORIZONTAL_ALIGN_SUBLEFT		0	// left edge of a 4:3 screen (safe area not included)
#define HORIZONTAL_ALIGN_LEFT			1	// left viewable (safe area) edge
#define HORIZONTAL_ALIGN_CENTER			2	// center of the screen (reticle)
#define HORIZONTAL_ALIGN_RIGHT			3	// right viewable (safe area) edge
#define HORIZONTAL_ALIGN_FULLSCREEN		4	// disregards safe area
#define HORIZONTAL_ALIGN_NOSCALE		5	// uses exact parameters - neither adjusts for safe area nor scales for screen size
#define HORIZONTAL_ALIGN_TO640			6	// scales a real-screen resolution x down into the 0 - 640 range
#define HORIZONTAL_ALIGN_CENTER_SAFEAREA 7	// center of the safearea

#define VERTICAL_ALIGN_SUBTOP			0	// top edge of the 4:3 screen (safe area not included)
#define VERTICAL_ALIGN_TOP				1	// top viewable (safe area) edge
#define VERTICAL_ALIGN_CENTER			2	// center of the screen (reticle)
#define VERTICAL_ALIGN_BOTTOM			3	// bottom viewable (safe area) edge
#define VERTICAL_ALIGN_FULLSCREEN		4	// disregards safe area
#define VERTICAL_ALIGN_NOSCALE			5	// uses exact parameters - neither adjusts for safe area nor scales for screen size
#define VERTICAL_ALIGN_TO480			6	// scales a real-screen resolution y down into the 0 - 480 range
#define VERTICAL_ALIGN_CENTER_SAFEAREA	7	// center of the save area

#define HORIZONTAL_APPLY_LEFT				0
#define HORIZONTAL_APPLY_CENTER				1
#define HORIZONTAL_APPLY_RIGHT				2
#define HORIZONTAL_APPLY_FULLSCREEN			3
#define HORIZONTAL_APPLY_NONE				4
#define HORIZONTAL_APPLY_TO640				5
#define HORIZONTAL_APPLY_CENTER_SAFEAREA	6
#define HORIZONTAL_APPLY_CONSOLE_SPECIAL	7

#define VERTICAL_APPLY_TOP					0
#define VERTICAL_APPLY_CENTER				1
#define VERTICAL_APPLY_BOTTOM				2
#define VERTICAL_APPLY_FULLSCREEN			3
#define VERTICAL_APPLY_NONE					4
#define VERTICAL_APPLY_TO640				5
#define VERTICAL_APPLY_CENTER_SAFEAREA		6

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
	private:
	};
}
