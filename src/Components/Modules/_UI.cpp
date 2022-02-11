#include "STDInclude.hpp"

#define GET_UIMATERIAL(ptr) (Game::Material*) *(DWORD*)(ptr)

// spacing dummy
#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 

// seperator with spacing
#define SEPERATORV(spacing, enabled) SPACING(0.0f, spacing); \
	if(enabled) { ImGui::Separator(); SPACING(0.0f, spacing); }

// fade from black into main menu on first start 
#define mainmenu_init_fadeTime 50.0f

namespace Components
{
	int _UI::GetTextHeight(Game::Font_s *font)
	{
		if (font)
		{
			return font->pixelHeight;
		}
		else
		{
			return 16;
		}
	}

	float _UI::ScrPlace_ApplyX(int horzAlign, const float x, const float xOffs)
	{
		float result;
		switch (horzAlign)
		{
		case HORIZONTAL_APPLY_LEFT:
			result = Game::scrPlace->scaleVirtualToReal[0] * x + xOffs + Game::scrPlace->realViewableMin[0];
			break;

		case HORIZONTAL_APPLY_CENTER:
			result = Game::scrPlace->scaleVirtualToReal[0] * x + xOffs + Game::scrPlace->realViewportSize[0] * 0.5f;
			break;

		case HORIZONTAL_APPLY_RIGHT:
			result = Game::scrPlace->scaleVirtualToReal[0] * x + xOffs + Game::scrPlace->realViewableMax[0];
			break;

		case HORIZONTAL_APPLY_FULLSCREEN:
			result = Game::scrPlace->scaleVirtualToFull[0] * x + xOffs;
			break;

		case HORIZONTAL_APPLY_NONE:
			result = x;
			break;

		case HORIZONTAL_APPLY_TO640:
			result = Game::scrPlace->scaleRealToVirtual[0] * x + xOffs;
			break;

		case HORIZONTAL_APPLY_CENTER_SAFEAREA:
			result = (Game::scrPlace->realViewableMax[0] + Game::scrPlace->realViewableMin[0]) * 0.5f + xOffs + Game::scrPlace->scaleVirtualToReal[0] * x;
			break;

		case HORIZONTAL_APPLY_CONSOLE_SPECIAL:
			result = Game::scrPlace->realViewableMax[0] - xOffs + Game::scrPlace->scaleVirtualToReal[0] * x;
			break;

		default:
			result = x;
		}

		return result;
	}

	float _UI::ScrPlace_ApplyY(int vertAlign, const float y, const float yOffs)
	{
		float result;
		switch (vertAlign)
		{
		case VERTICAL_APPLY_TOP:
			result = Game::scrPlace->scaleVirtualToReal[1] * y + yOffs + Game::scrPlace->realViewableMin[1];
			break;

		case VERTICAL_APPLY_CENTER:
			result = Game::scrPlace->scaleVirtualToReal[1] * y + yOffs + Game::scrPlace->realViewportSize[1] * 0.5f;
			break;

		case VERTICAL_APPLY_BOTTOM:
			result = Game::scrPlace->scaleVirtualToReal[1] * y + yOffs + Game::scrPlace->realViewableMax[1];
			break;

		case VERTICAL_APPLY_FULLSCREEN:
			result = Game::scrPlace->scaleVirtualToFull[1] * y + yOffs;
			break;

		case VERTICAL_APPLY_NONE:
			result = y;
			break;

		case VERTICAL_APPLY_TO640:
			result = Game::scrPlace->scaleRealToVirtual[1] * y + yOffs;
			break;

		case VERTICAL_APPLY_CENTER_SAFEAREA:
			result = (Game::scrPlace->realViewableMax[1] + Game::scrPlace->realViewableMin[1]) * 0.5f + yOffs + Game::scrPlace->scaleVirtualToReal[1] * y;
			break;

		default:
			result = y;
		}

		return result;
	}

	void _UI::ScrPlace_ApplyRect(float *offs_x, float *w, float *offs_y, float *h, int horzAnker, int vertAnker)
	{
		float _x, _w, _y, _h;

		switch (horzAnker)
		{
		case HORIZONTAL_ALIGN_SUBLEFT:
			_x = Game::scrPlace->scaleVirtualToReal[0] * *offs_x + Game::scrPlace->subScreenLeft;
			_w = Game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_LEFT:
			_x = Game::scrPlace->scaleVirtualToReal[0] * *offs_x + Game::scrPlace->realViewableMin[0];
			_w = Game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_CENTER:
			_x = Game::scrPlace->scaleVirtualToReal[0] * *offs_x + Game::scrPlace->realViewportSize[0] * 0.5f;
			_w = Game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_RIGHT:
			_x = Game::scrPlace->scaleVirtualToReal[0] * *offs_x + Game::scrPlace->realViewableMax[0];
			_w = Game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_FULLSCREEN:
			_x = Game::scrPlace->scaleVirtualToFull[0] * *offs_x;
			_w = Game::scrPlace->scaleVirtualToFull[0] * *w;
			break;

		case HORIZONTAL_ALIGN_NOSCALE:
			goto USE_VERT_ALIGN; // we might wan't vertical alignment

		case HORIZONTAL_ALIGN_TO640:
			_x = Game::scrPlace->scaleRealToVirtual[0] * *offs_x;
			_w = Game::scrPlace->scaleRealToVirtual[0] * *w;
			break;

		case HORIZONTAL_ALIGN_CENTER_SAFEAREA:
			_x = (Game::scrPlace->realViewableMax[0] + Game::scrPlace->realViewableMin[0]) * 0.5f + Game::scrPlace->scaleVirtualToReal[0] * *offs_x;
			_w = Game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		default:
			goto USE_VERT_ALIGN; // we might wan't vertical alignment

		}

		*offs_x = _x;
		*w = _w;

	USE_VERT_ALIGN:
		switch (vertAnker)
		{
		case VERTICAL_ALIGN_TOP:
			_y = Game::scrPlace->scaleVirtualToReal[1] * *offs_y + Game::scrPlace->realViewableMin[1];
			_h = Game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_CENTER:
			_y = Game::scrPlace->scaleVirtualToReal[1] * *offs_y + Game::scrPlace->realViewportSize[1] * 0.5f;
			_h = Game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_BOTTOM:
			_y = Game::scrPlace->scaleVirtualToReal[1] * *offs_y + Game::scrPlace->realViewableMax[1];
			_h = Game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_FULLSCREEN:
			_y = Game::scrPlace->scaleVirtualToFull[1] * *offs_y;
			_h = Game::scrPlace->scaleVirtualToFull[1] * *h;
			break;

		case VERTICAL_ALIGN_NOSCALE:
			return; // don't do a thing

		case VERTICAL_ALIGN_TO480:
			_y = Game::scrPlace->scaleRealToVirtual[1] * *offs_y;
			_h = Game::scrPlace->scaleRealToVirtual[1] * *h;
			break;

		case VERTICAL_ALIGN_CENTER_SAFEAREA:
			_y = Game::scrPlace->scaleVirtualToReal[1] * *offs_y + (Game::scrPlace->realViewableMax[1] + Game::scrPlace->realViewableMin[1]) * 0.5f;
			_h = Game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_SUBTOP:
			_y = Game::scrPlace->scaleVirtualToReal[1] * *offs_y;
			_h = Game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		default:
			return; // don't do a thing
		}

		*offs_y = _y;
		*h = _h;
	}

	void _UI::ScrPlaceFull_ApplyRect(float *offs_x, float *w, float *offs_y, float *h, int horzAnker, int vertAnker)
	{
		float _x, _w, _y, _h;

		switch (horzAnker)
		{
		case HORIZONTAL_ALIGN_SUBLEFT:
			_x = Game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x + Game::scrPlaceFull->subScreenLeft;
			_w = Game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_LEFT:
			_x = Game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x + Game::scrPlaceFull->realViewableMin[0];
			_w = Game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_CENTER:
			_x = Game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x + Game::scrPlaceFull->realViewportSize[0] * 0.5f;
			_w = Game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_RIGHT:
			_x = Game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x + Game::scrPlaceFull->realViewableMax[0];
			_w = Game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_FULLSCREEN:
			_x = Game::scrPlaceFull->scaleVirtualToFull[0] * *offs_x;
			_w = Game::scrPlaceFull->scaleVirtualToFull[0] * *w;
			break;

		case HORIZONTAL_ALIGN_NOSCALE:
			goto USE_VERT_ALIGN; // we might wan't vertical alignment

		case HORIZONTAL_ALIGN_TO640:
			_x = Game::scrPlaceFull->scaleRealToVirtual[0] * *offs_x;
			_w = Game::scrPlaceFull->scaleRealToVirtual[0] * *w;
			break;

		case HORIZONTAL_ALIGN_CENTER_SAFEAREA:
			_x = (Game::scrPlaceFull->realViewableMax[0] + Game::scrPlaceFull->realViewableMin[0]) * 0.5f + Game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x;
			_w = Game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		default:
			goto USE_VERT_ALIGN; // we might wan't vertical alignment

		}

		*offs_x = _x;
		*w = _w;

	USE_VERT_ALIGN:
		switch (vertAnker)
		{
		case VERTICAL_ALIGN_TOP:
			_y = Game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y + Game::scrPlaceFull->realViewableMin[1];
			_h = Game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_CENTER:
			_y = Game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y + Game::scrPlaceFull->realViewportSize[1] * 0.5f;
			_h = Game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_BOTTOM:
			_y = Game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y + Game::scrPlaceFull->realViewableMax[1];
			_h = Game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_FULLSCREEN:
			_y = Game::scrPlaceFull->scaleVirtualToFull[1] * *offs_y;
			_h = Game::scrPlaceFull->scaleVirtualToFull[1] * *h;
			break;

		case VERTICAL_ALIGN_NOSCALE:
			return; // don't do a thing

		case VERTICAL_ALIGN_TO480:
			_y = Game::scrPlaceFull->scaleRealToVirtual[1] * *offs_y;
			_h = Game::scrPlaceFull->scaleRealToVirtual[1] * *h;
			break;

		case VERTICAL_ALIGN_CENTER_SAFEAREA:
			_y = Game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y + (Game::scrPlaceFull->realViewableMax[1] + Game::scrPlaceFull->realViewableMin[1]) * 0.5f;
			_h = Game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_SUBTOP:
			_y = Game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y;
			_h = Game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		default:
			return; // don't do a thing
		}

		*offs_y = _y;
		*h = _h;
	}

	void _UI::redraw_cursor()
	{
		// get material handle
		void* cur_material = Game::Material_RegisterHandle("ui_cursor", 3);

		float cur_w = (32.0f * Game::scrPlace->scaleVirtualToReal[0]) / Game::scrPlace->scaleVirtualToFull[0];
		float cur_h = (32.0f * Game::scrPlace->scaleVirtualToReal[1]) / Game::scrPlace->scaleVirtualToFull[1];
		float cur_x = Game::_uiContext->cursor.x - 0.5f * cur_w;
		float cur_y = Game::_uiContext->cursor.y - 0.5f * cur_h;

		// set up texcoords
		float s0, s1;
		float t0, t1;

		if (cur_w >= 0.0f)
		{
			s0 = 0.0f;
			s1 = 1.0f;
		}
		else
		{
			cur_w = -cur_w;
			s0 = 1.0f;
			s1 = 0.0f;
		}

		if (cur_h >= 0.0f)
		{
			t0 = 0.0f;
			t1 = 1.0f;
		}
		else
		{
			cur_h = -cur_h;
			t0 = 1.0f;
			t1 = 0.0f;
		}

		// scale 640x480 rect to viewport resolution and draw the cursor
		_UI::ScrPlace_ApplyRect(&cur_x, &cur_w, &cur_y, &cur_h, VERTICAL_ALIGN_FULLSCREEN, VERTICAL_ALIGN_FULLSCREEN);
		Game::R_AddCmdDrawStretchPic(cur_material, cur_x, cur_y, cur_w, cur_h, s0, t0, s1, t1, 0);
	}


	/* ---------------------------------------------------------- */
	/* ------------------- general main menu -------------------- */

	float	mainmenu_fadeTime = mainmenu_init_fadeTime;
	float	mainmenu_fadeColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// D3D9Ex::D3D9Device::Release() resets mainmenu vars on vid_restart
	void main_menu_fade_in()
	{
		if (!Game::Sys_IsMainThread() || Game::Globals::mainmenu_fadeDone || !Game::_uiContext)
		{
			return;
		}

		if (mainmenu_fadeTime >= 0.0f)
		{
			mainmenu_fadeTime -= 0.5f;
			mainmenu_fadeColor[3] = (mainmenu_fadeTime * (1.0f / mainmenu_init_fadeTime));

			Game::ConDraw_Box(mainmenu_fadeColor, 0.0f, 0.0f, (float)Game::_uiContext->screenWidth, (float)Game::_uiContext->screenHeight);
			return;
		}

		// reset fade vars
		mainmenu_fadeTime = mainmenu_init_fadeTime;
		mainmenu_fadeColor[3] = 1.0f;

		Game::Globals::mainmenu_fadeDone = true;
	}

	// *
	// called from <Gui::render_loop>
	void _UI::create_changelog(Game::gui_menus_t& menu)
	{
		// do not keep mouse cursor around if another ImGui was active
		menu.mouse_ignores_menustate = true;

		//ImGuiContext* g = ImGui::GetCurrentContext();
		ImGuiStyle& style = ImGui::GetStyle();
		ImGuiIO& io = ImGui::GetIO();

		io.WantCaptureKeyboard = false;
		io.WantCaptureMouse = true;

		Game::UiContext* ui = Game::_uiContext;
		int pushed_styles = 0, pushed_colors = 0;
		
		// ImGui is rendered on EndScene so UI_MainMenu_FadeIn wont work => fade manually
		if (!Game::Globals::mainmenu_fadeDone)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f - mainmenu_fadeColor[3]);		pushed_styles++;
		}

		// set window size once
		if (!menu.one_time_init)
		{
			menu.got_layout_from_menu = false;

			if (ui)
			{
				for (auto m = 0; m < ui->openMenuCount && !menu.got_layout_from_menu; m++)
				{
					if (ui->menuStack[m] && !Utils::Q_stricmp(ui->menuStack[m]->window.name, "main_text"))
					{
						for (auto i = 0; i < ui->menuStack[m]->itemCount && !menu.got_layout_from_menu; i++)
						{
							if (ui->menuStack[m]->items[i] && ui->menuStack[m]->items[i]->window.name)
							{
								if (!Utils::Q_stricmp(ui->menuStack[m]->items[i]->window.name, "changelog_container"))
								{
									Gui::set_menu_layout(
										menu,
										ui->menuStack[m]->items[i]->window.rect.x,
										ui->menuStack[m]->items[i]->window.rect.y,
										ui->menuStack[m]->items[i]->window.rect.w,
										ui->menuStack[m]->items[i]->window.rect.h,
										ui->menuStack[m]->items[i]->window.rect.horzAlign,
										ui->menuStack[m]->items[i]->window.rect.vertAlign);

									menu.got_layout_from_menu = true;
								}
							}
						}
					}
				}
			}
		}

		// only draw the changelog if we found the "changelog_container" itemdef (the user might not have loaded )
		if (menu.got_layout_from_menu)
		{
			if (!menu.one_time_init)
			{
				_UI::ScrPlaceFull_ApplyRect(&menu.position[0], &menu.size[0], &menu.position[1], &menu.size[1], menu.horzAlign, menu.vertAlign);
				ImGui::SetNextWindowPos(ImVec2(menu.position[0], menu.position[1]), ImGuiCond_Always); //ImGuiCond_FirstUseEver);
				ImGui::SetNextWindowSize(ImVec2(menu.size[0], menu.size[1]), ImGuiCond_Always); //ImGuiCond_FirstUseEver);

				menu.one_time_init = true;
			}
		}
		// disable changelog gui
		else
		{
			if (pushed_styles)
			{
				ImGui::PopStyleVar(pushed_styles);	pushed_styles = 0;
			}

			menu.menustate = false;
			return;
		}

		// changelog styles
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);				pushed_styles++;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);				pushed_styles++;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));	pushed_styles++;

		// scollbar style
		ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(1, 1, 1, 0.025f));

		// early out if the window is collapsed, as an optimization.
		if (!ImGui::Begin("Changelog", &menu.menustate, 
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | 
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus)) // < always keep changelog behind the cursor gui
		{
			// pop scrollbar style
			ImGui::PopStyleColor(1);

			// pop all styles
			ImGui::PopStyleVar(pushed_styles);	pushed_styles = 0;

			ImGui::End();
			return;
		}

		// pop scrollbar style
		ImGui::PopStyleColor(1);

		// default changelog if WinHttp fails
		// https://raw.githubusercontent.com/wiki/xoxor4d/iw3xo-dev/Changelog.md
		// https://github.com/mekhontsev/imgui_md
		
		const std::string markdownText = 
R"(
## Failed to load changelog
**Visit: [https://github.com/xoxor4d/iw3xo-dev/wiki/Changelog](https://github.com/xoxor4d/iw3xo-dev/wiki/Changelog)**
)";

		// pop styles but keep the first one (alpha)
		ImGui::PopStyleVar(pushed_styles - 1);	pushed_styles = 1;
		

		// *
		// markdown

		// tooltip styles
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);				pushed_styles++;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);				pushed_styles++;
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);					pushed_styles++;


		// markdown link color and underline :: IM_COL32()
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor(200, 229, 120, 203)));	pushed_colors++;
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(182, 209, 107, 100)));		pushed_colors++;

		// draw markdown
		ImGui::Indent(16.0f); SPACING(0.0f, 4.0f);

			// default changelog if WinHttp failed
			if (Game::Globals::changelog_html_body.length() == 0)
			{
				Gui::markdown(markdownText.c_str(), markdownText.c_str() + markdownText.size());
			}
			// real changelog
			else
			{
				Gui::markdown(Game::Globals::changelog_html_body.c_str(), Game::Globals::changelog_html_body.c_str() + Game::Globals::changelog_html_body.size());
			}

		ImGui::Indent(-16.0f); SPACING(0.0f, 4.0f);

		// pop all colors/styles
		ImGui::PopStyleColor(pushed_colors);	pushed_colors = 0;
		ImGui::PopStyleVar(pushed_styles);		pushed_styles = 0;

		ImGui::End();


		// *
		// re-draw the mouse cursor

		// fullscreen gui
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);

		io.WantCaptureKeyboard = false;
		io.WantCaptureMouse = false;

		if (!ImGui::Begin("", &menu.menustate, 
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoBackground))
		{
			ImGui::End();
			return;
		}

		Gui::redraw_cursor();
		
		ImGui::End();
	}

	void _UI::main_menu_register_dvars()
	{
		Dvars::ui_main_title = Game::Dvar_RegisterString(
			/* name		*/ "ui_changelog_title",
			/* desc		*/ "menu helper",
			/* value	*/ Utils::VA(IW3XO_CHANGELOG_TITLE_FMT, IW3X_BUILDNUMBER, IW3XO_BUILDVERSION_DATE),
			/* flags	*/ Game::dvar_flags::read_only);
	}

	// *
	// draw additional stuff to the main menu
	void main_menu()
	{
		if (!Game::Sys_IsMainThread()) 
		{
			return;
		}

		float max = Game::scrPlace->scaleVirtualToReal[1] * 0.3f;
		const char*	font;

		auto ui_smallFont		= Game::Dvar_FindVar("ui_smallFont");
		auto ui_extraBigFont	= Game::Dvar_FindVar("ui_extraBigFont");
		auto ui_bigFont			= Game::Dvar_FindVar("ui_bigFont");

		if (ui_smallFont && ui_smallFont->current.value < max)
		{
			if (ui_extraBigFont && ui_extraBigFont->current.value > max)
			{
				font = FONT_BIG;

				if (ui_bigFont && ui_bigFont->current.value > max)
				{
					font = FONT_NORMAL;
				}
			}
			else
			{
				font = FONT_EXTRA_BIG;
			}
		}
		else
		{
			font = FONT_SMALL;
		}

		// get font handle
		auto font_handle = Game::R_RegisterFont(font, sizeof(font));
		if (!font_handle) 
		{
			return;
		}

		float offs_x = 10.0f; 
		float offs_y = -10.0f; 
		float scale = 0.25f;

		float scale_x = scale * 48.0f / font_handle->pixelHeight;
		float scale_y = scale_x;

		// place container
		_UI::ScrPlace_ApplyRect(&offs_x, &scale_x, &offs_y, &scale_y, HORIZONTAL_ALIGN_LEFT, VERTICAL_ALIGN_BOTTOM);

		const char* textForeground = Utils::VA("IW3xo :: %.lf :: %s", IW3X_BUILDNUMBER, IW3XO_BUILDVERSION_DATE);
		const char* textBackground = textForeground;

		if (DEBUG)
		{
			textForeground = Utils::VA("IW3xo :: %.lf :: %s :: %s", IW3X_BUILDNUMBER, IW3XO_BUILDVERSION_DATE, "^1DEBUG");
			textBackground = Utils::VA("IW3xo :: %.lf :: %s :: %s", IW3X_BUILDNUMBER, IW3XO_BUILDVERSION_DATE, "DEBUG");
		}

		// Background String
		float colorBackground[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

		Game::R_AddCmdDrawTextASM(
			/* txt */ textBackground,
			/* max */ 0x7FFFFFFF,
			/* fot */ font_handle,
			/*  x  */ offs_x + 3.0f,
			/*  y  */ offs_y + 3.0f,
			/* scX */ scale_x,
			/* scY */ scale_y,
			/* rot */ 0.0f,
			/* col */ colorBackground,
			/* sty */ 0);

		// Foreground String
		float colorForeground[4] = { 1.0f, 0.8f, 0.7f, 1.0f };

		Game::R_AddCmdDrawTextASM(
			/* txt */ textForeground,
			/* max */ 0x7FFFFFFF,
			/* fot */ font_handle,
			/*  x  */ offs_x,
			/*  y  */ offs_y,
			/* scX */ scale_x,
			/* scY */ scale_y,
			/* rot */ 0.0f,
			/* col */ colorForeground,
			/* sty */ 0);

		// fade in the menu on first start
		main_menu_fade_in();


		// *
		// changelog gui
		
		// _Client::OnDisconnect() hides / shows the changelog when connecting/disconnecting
		// D3D9Ex::D3D9Device::Release() resets mainmenu vars on vid_restart
		if (Components::active.Gui)
		{
			// do not draw the changelog when the Com_Error menu is open
			const auto& com_errorMessage = Game::Dvar_FindVar("com_errorMessage");
			if (com_errorMessage && com_errorMessage->current.string[0])
			{
				return;
			}

			// open changelog gui at startup
			if (!Game::Globals::loaded_MainMenu)
			{
				GET_GGUI.menus[Game::GUI_MENUS::CHANGELOG].menustate = true;
			}

			Game::UiContext* ui = Game::_uiContext;

			if (ui)
			{
				// draw/hide the gui using menu localvars
				for (auto var = 0; var < 255; var++)
				{
					if (ui->localVars.table[var].name && !Utils::Q_stricmp(ui->localVars.table[var].name, "ui_maincontent_update"))
					{
						if (ui->localVars.table[var].u.integer == 0)
						{
							GET_GGUI.menus[Game::GUI_MENUS::CHANGELOG].menustate = true;
						}
						else // close
						{
							GET_GGUI.menus[Game::GUI_MENUS::CHANGELOG].menustate = false;
						}

						break;
					}
				}
			}
		}

		Game::Globals::loaded_MainMenu = true;
	}

	// Main Menu Version (UI_VersionNumber Call in UI_Refresh)
	__declspec(naked) void main_menu_stub()
	{
		__asm
		{
			call	main_menu;
			push	0x54353A;
			retn;
		}
	}

	
	/* ---------------------------------------------------------- */
	/* --------------- main menu shader constants --------------- */

	// gameTime constant is only updated in-game, lets fix that
	float menu_gameTime = 0.0f;

	void set_custom_codeconstants()
	{
		if (!Game::_uiContext || !Game::gfxCmdBufSourceState)
		{
			return;
		}

		// check if we are in a menu to stop overwriting filtertap? needed?
		if (menu_gameTime >= 10000.0f) {
			menu_gameTime = 0.0f;
		}

		menu_gameTime += 0.01f;

		// -- mouse position --
		glm::vec2 mousePos;

		// normalize mouse to ui resolution
		mousePos.x = (Game::_uiContext->cursor.x / 640.0f);
		mousePos.y = (Game::_uiContext->cursor.y / 480.0f);

		// screencenter will be 0.0 ( -1.0, 1.0 = bottom left )
		mousePos.x = mousePos.x * 2.0f - 1.0f;
		mousePos.y = mousePos.y * 2.0f - 1.0f;

		// filterTap 5
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_5][0] = Dvars::ui_eyes_position->current.vector[0];
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_5][1] = Dvars::ui_eyes_position->current.vector[1];
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_5][2] = Dvars::ui_eyes_size->current.value;
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_5][3] = Dvars::ui_eyes_alpha->current.value;

		// filterTap 6
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_6][0] = Dvars::ui_button_highlight_radius->current.value;
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_6][1] = Dvars::ui_button_highlight_brightness->current.value;
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_6][2] = Dvars::ui_button_outline_radius->current.value;
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_6][3] = Dvars::ui_button_outline_brightness->current.value;

		// filterTap 7
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_7][0] = mousePos.x;
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_7][1] = mousePos.y;
#if DEBUG		
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_7][2] = Dvars::xo_menu_dbg->current.value;
#endif
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_7][3] = menu_gameTime;

		// get current viewport width / height and update ui_dvars
		int vpWidth = (int)floorf(_UI::ScrPlace_ApplyX(HORIZONTAL_APPLY_RIGHT, 0.0f, 0.0f));
		int vpHeight = (int)floorf(_UI::ScrPlace_ApplyY(VERTICAL_APPLY_BOTTOM, 0.0f, 0.0f));

		// set rendersize (needed?)
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_RENDER_TARGET_SIZE][0] = static_cast<float>(vpWidth);
		Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_RENDER_TARGET_SIZE][1] = static_cast<float>(vpHeight);

		if (Dvars::ui_renderWidth && Dvars::ui_renderHeight)
		{
			Game::Dvar_SetValue(Dvars::ui_renderWidth, vpWidth);
			Game::Dvar_SetValue(Dvars::ui_renderHeight, vpHeight);
		}
	}

	// hook "R_RenderAllLeftovers" in RB_Draw3DCommon to set custom codeconstants
	__declspec(naked) void RB_Draw3DCommon_stub()
	{
		const static uint32_t R_RenderAllLeftovers_Func = 0x615570;
		__asm
		{
			pushad;
			call	set_custom_codeconstants;
			popad;

			// args are on the stack
			call	R_RenderAllLeftovers_Func;

			push	0x6156F1;
			retn;
		}
	}


	/* ---------------------------------------------------------- */
	/* ------------------- ui fixes and addons ------------------ */
	
	Game::Material* ui_white_material;
	
	void init_white_material()
	{
		ui_white_material = GET_UIMATERIAL(0xCAF06F0); // white material pointer
		if (!ui_white_material)
		{
			ui_white_material = Game::Material_RegisterHandle("white", 3);
		}
	}
	
	// ITEM-SLIDER :: Change material "ui_scrollbar" to white and add a color (background)
	__declspec(naked) void Item_ListBox_Paint_SliderBackground_stub()
	{
		const static uint32_t returnPt = 0x55330D; // jump onto the call to UI_DrawHandlePic

		const static float xOffs = 11.0f;
		const static float yOffs = -1.0f;
		const static float hOffs =  0.0f; // add additional height
		const static float meme2 =  0.0f; // add additional height
		const static float width =  6.0f;

		const static float  color[4] = LISTBOX_SLIDER_BACKGROUND_COLOR;
		const static float* colorPtr = color; // we need a ptr to our color
		// white material ptr location @ 0xCAF06F0 points-to 0xFA4EC8
		__asm
		{
			pushad;
			call	init_white_material;
			popad;

			fld     dword ptr[esp + 50h];		// stock op
			mov     eax, [edi];					// stock op
			fadd	yOffs;						// stock op -- itemheight + yOffs
			mov		ecx, [ebp + eax * 4 + 4];	// stock op
			mov		[ebp + eax * 4 + 8], ecx;	// stock op

			// material <-> color
			mov     edx, ui_white_material;	// ui white material handle
			mov     eax, [ebx + 18h];		// stock op
			fstp    dword ptr[esp + 50h];	// stock op
			fld     dword ptr[ebx + 10h];	// stock op
			mov     ecx, [ebx + 14h];		// stock op
			fadd	hOffs;					// stock op -- h - hOffs
			add     esp, 1Ch;				// stock op
			push    edx;					// stock op -- material
			push    eax;					// stock op -- vertAlign
			fstp    dword ptr[esp + 20h];	// stock op
			push    ecx;					// stock op -- horzAlign
			fld     dword ptr[esp + 24h];	// stock op
			sub     esp, 10h;				// stock op
			fadd	meme2;					// stock op -- new h + 1.0f
			mov     ecx, colorPtr;			// color -- was "xor ecx, ecx"

			// continue till we are at x
			mov     edx, esi;				// stock op -- placement
			fstp    dword ptr[esp + 44h];	// stock op
			fld     dword ptr[esp + 44h];	// stock op
			fstp    dword ptr[esp + 0Ch];	// stock op -- h
			fld		width;					// stock op
			fstp    dword ptr[esp + 8];		// stock op -- w
			fld     dword ptr[esp + 50h];	// stock op
			fstp    dword ptr[esp + 4];		// stock op -- y
			fld     dword ptr[esp + 30h];	// stock op
			fadd	xOffs;
			fstp    dword ptr[esp];			// stock op -- x

			jmp		returnPt;
		}
	}

	// ITEM-SLIDER :: Change material "material_ui_scrollbar_thumb" to white and add a color (thumb)
	__declspec(naked) void Item_ListBox_Paint_SliderThumb_stub()
	{
		const static uint32_t returnPt = 0x5533C7; // jump onto the call to UI_DrawHandlePic

		const static float boxWH =  6.0f;
		const static float xOffs = 11.0f;
		const static float hOffs = 40.0f;
		const static float yOffs = -(hOffs * 0.5f) + 3.0f;
		

		const static float  color[4] = LISTBOX_SLIDER_THUMB_COLOR;
		const static float* colorPtr = color; // we need a ptr to our color
		// white material ptr location @ 0xCAF06F0 points-to 0xFA4EC8
		__asm
		{
			pushad;
			call	init_white_material;
			popad;

			// material <-> color
			mov     edx, ui_white_material;		// ui white material handle
			fld		boxWH;						// stock op
			mov     eax, [ebx + 18h];			// stock op
			mov     ecx, [ebx + 14h];			// stock op
			push    edx;						// stock op -- material
			push    eax;						// stock op -- vertAlign
			push    ecx;						// stock op -- horzAlign
			sub     esp, 10h;					// stock op
			fadd	hOffs;
			fst     dword ptr[esp + 0Ch];		// stock op -- h (uses boxWH)
			mov     ecx, colorPtr;				// color -- was "xor ecx, ecx"
			fsub	hOffs;
			fstp    dword ptr[esp + 8];			// stock op -- w (uses boxWH)
			mov     edx, esi;					// stock op
			fld     dword ptr[esp + 34h];		// stock op
			fadd	yOffs;
			fstp    dword ptr[esp + 4];			// stock op -- y
			fld     dword ptr[esp + 30h];		// stock op
			fadd	xOffs;
			fstp    dword ptr[esp];				// stock op -- x

			jmp		returnPt;
		}
	}

	// add custom uiScripts for menus :: returns true when a valid uiScript string was matched => skip the original function to avoid error prints and to make overwriting stock uiScripts a thing
	int scripts_addons(const char *arg)
	{
		if (!Utils::Q_stricmpn(arg, "StartServerCustom", 0x7FFFFFFF))
		{
			const char* spawnServerStr;
			const char* gameTypeStr;
			const char* mapNameStr;

			auto ui_dedicated			= Game::Dvar_FindVar("ui_dedicated");
			auto ui_netGameType			= Game::Dvar_FindVar("ui_netGameType");
			auto ui_currentNetMap		= Game::Dvar_FindVar("ui_currentNetMap");
			auto dedicated				= Game::Dvar_FindVar("dedicated");
			auto g_gametype				= Game::Dvar_FindVar("g_gametype");
			
			if (ui_dedicated && ui_dedicated->current.integer != 0) 
			{
				Game::Com_PrintMessage(0, Utils::VA("^3Warning ^7:: setting \"dedicated\" to 0 because advanced server settings only supports listen servers!\n"), 0);
				Game::Dvar_SetValue(dedicated, 0);
			}

			if (Game::gameTypeEnum && *Game::gameTypeEnum && ui_netGameType && ui_currentNetMap)
			{
				gameTypeStr = (const char*)Game::gameTypeEnum[0x2 * ui_netGameType->current.integer];
				mapNameStr	= (const char*)Game::mapNameEnum[0x28 * ui_currentNetMap->current.integer];
			}
			else
			{
				Game::Com_PrintMessage(0, "^3Warning ^7:: gametype or map name was empty! Using Defaults!\n", 0);
				
				gameTypeStr = "dm";
				mapNameStr	= "mp_backlot";
			}

			// set gametype
			if (g_gametype)
			{
				Game::Dvar_SetString(gameTypeStr, g_gametype);
			}
			
			// server spawn string
			if (Dvars::ui_developer && Dvars::ui_developer_script && Dvars::ui_devmap)
			{
				spawnServerStr = Utils::VA("wait ; wait ; set developer %i; set developer_script %i; %s %s\n",
											Dvars::ui_developer->current.integer, Dvars::ui_developer_script->current.integer, Dvars::ui_devmap->current.enabled ? "devmap" : "map", mapNameStr);
			}
			else
			{
				spawnServerStr = Utils::VA("wait ; wait ; map %s", mapNameStr);
			}
			
			Game::Com_PrintMessage(0, Utils::VA("^2Spawning server ^7:: %s", spawnServerStr), 0);
			Game::Cbuf_AddText(spawnServerStr, 0);

			// return true to skip the stock function that looks for valid uiScripts
			return 1;
		}

		// return false and continue with the stock function
		return 0;
	}

	// hook "Item_RunScript" to implement custom uiScripts callable from menu files
	__declspec(naked) void scripts_addons_stub()
	{
		const static char* overwrittenStr = "StartServer";
		const static uint32_t stockScripts = 0x545BF2;  // next op after hook spot
		const static uint32_t ifAddonReturn = 0x546E52; // jump to the valid return point if we had a valid match in addons
		__asm
		{
			lea     edx, [esp + 58h];	// 'out' arg was at 0x5C but we hooked at the push before it => 'out' arg at esp+5C - 4 ;)
	
			pushad;
			push	edx;
			call	scripts_addons;
			add		esp, 4;
			test	eax, eax;
			je		STOCK_FUNC;			// jump if UI_uiScriptsAddons was false
			popad;

			jmp		ifAddonReturn;		// return to skip the stock function

			STOCK_FUNC:
			popad;
			push	overwrittenStr;		// the original push we hooked at
			jmp		stockScripts;		// jump back and exec the original function
		}
	}

	// fix rect not being drawn properly when a border is applied
	_declspec(naked) void Window_Paint_Border_Side_Fix() 
	{
		const static uint32_t returnPT = 0x54B6FA;
		_asm 
		{
			cmp     dword ptr[ebx + 3Ch], 2;	//if border 2
			je		short border_2;

			cmp     dword ptr[ebx + 3Ch], 3;	//if border 3
			je		short border_3;

			//fix for full border
			fld     dword ptr[ebx + 48h];	//stock op
			fadd    dword ptr[esp + 10h];	//stock op
			fstp    dword ptr[esp + 10h];	//stock op
			fld     dword ptr[ebx + 48h];	//stock op
			fadd    dword ptr[esp + 14h];	//stock op
			fstp    dword ptr[esp + 14h];	//stock op
			fld     dword ptr[ebx + 48h];	//stock op
			fld     dword ptr[esp + 18h];	//stock op
			fsub    st, st(1);				//stock op
			fsub    st, st(1);
			fstp    dword ptr[esp + 18h];	//stock op
			fsubr   dword ptr[esp + 1Ch];	//subract border size (st0) from height, save in st0
			fsub	dword ptr[ebx + 48h];	//subract boder size from st0
			fstp    dword ptr[esp + 1Ch];	//stock op
			jmp		returnPT;


			//fix for border 2
			border_2:

			fld     dword ptr[ebx + 48h];	//load bordersize
			fadd    dword ptr[esp + 14h];	//add y to bordersize
			fstp    dword ptr[esp + 14h];	//store y, pop st0

			fld     dword ptr[ebx + 48h];	//load bordersize
			fsubr	dword ptr[esp + 1Ch];
			fsub	dword ptr[ebx + 48h];
			fstp	dword ptr[esp + 1Ch];	//store width
			jmp		returnPT;


			//fix for border 3
			border_3:
			
			fld     dword ptr[ebx + 48h];	//load bordersize
			fadd    dword ptr[esp + 10h];	//add x to bordersize
			fstp    dword ptr[esp + 10h];	//store x, pop st0

			fld     dword ptr[ebx + 48h];	//load bordersize
			fsubr   dword ptr[esp + 18h];
			fsub	dword ptr[ebx + 48h];
			fstp    dword ptr[esp + 18h];	//store height
			jmp		returnPT;
		}
	}

	// do not drop the player when UI_LoadMenus_LoadObj fails to load a menu
	__declspec(naked) void load_raw_menulist_error_stub()
	{
		__asm
		{
			add     esp, 18h;	// hook is placed on call to FS_FOpenFileReadForThread, so fix the stack
			
			xor		eax, eax;	// return a nullptr
			pop		edi;			// epilog
			pop		esi;
			pop		ebp;
			pop		ebx;
			pop		ecx;
			retn;
		}
	}


	/* ---------------------------------------------------------- */
	/* ---------------------- aspect ratio ---------------------- */

	void set_ultrawide_dvar(bool state)
	{
		if (Dvars::ui_ultrawide)
		{
			Game::Dvar_SetValue(Dvars::ui_ultrawide, state);
		}
	}

	void set_custom_aspect_ratio()
	{
		if (Dvars::r_aspectRatio_custom)
		{
			*(float*)(0xCC9D0F8) = Dvars::r_aspectRatio_custom->current.value;
		}
	}

	// hook R_AspectRatio to initially reset the ultrawide dvar (menu helper)
	__declspec(naked) void aspect_ratio_custom_reset_stub()
	{
		const static uint32_t returnPt = 0x5F3534;
		__asm
		{
			pushad;
			push	0;
			call	set_ultrawide_dvar;
			add		esp, 4h;
			popad;

			mov     eax, [eax + 0Ch];	// overwritten op
			cmp     eax, 3;				// overwritten op

			jmp		returnPt;			// jump back to break op
		}
	}

	// set custom aspect ratio by using the default switchcase in R_AspectRatio
	__declspec(naked) void aspect_ratio_custom_stub()
	{
		const static float ultraWideAspect = 2.3333333f;
		const static uint32_t returnPt = 0x5F35E5;
		__asm
		{
			pushad;
			push	1;
			call	set_ultrawide_dvar;
			add		esp, 4h;

			Call	set_custom_aspect_ratio;
			popad;

			mov     ecx, 1;				// widescreen true
			jmp		returnPt;			// jump back to break op
		}
	}

	_UI::_UI()
	{
		// *
		// get changelog (raw markdown)

		std::string url = "https://raw.githubusercontent.com/wiki/xoxor4d/iw3xo-dev/Changelog.md";
		std::wstring header, html;
		Utils::get_html(url, header, html);

		std::transform(html.begin(), html.end(), std::back_inserter(Game::Globals::changelog_html_body), [](wchar_t c)
		{
			return (char)c;
		});


		// *
		// Main Menu Hooks

		// Main Menu Version (UI_VersionNumber Call in UI_Refresh)
		Utils::Hook(0x543535, main_menu_stub, HOOK_JUMP).install()->quick();

		// hook "R_RenderAllLeftovers" in RB_Draw3DCommon to set custom codeconstants
		Utils::Hook(0x6156EC, RB_Draw3DCommon_stub, HOOK_JUMP).install()->quick();

		// hook "Item_RunScript" to implement custom uiScripts callable from menu files
		Utils::Hook(0x545BED, scripts_addons_stub, HOOK_JUMP).install()->quick();


		// *
		// List Box Slider

		// Change material "ui_scrollbar" to white and add a color (background)
		Utils::Hook::Nop(0x5532A0, 6);		Utils::Hook(0x5532A0, Item_ListBox_Paint_SliderBackground_stub, HOOK_JUMP).install()->quick();
		
		// disable drawing of upper arrow :: nop "UI_DrawHandlePic" call
		Utils::Hook::Nop(0x55329B, 5);

		// disable drawing of lower arrow :: nop "UI_DrawHandlePic" call
		Utils::Hook::Nop(0x55335A, 5);

		// Change material "material_ui_scrollbar_thumb" to white and add a color (thumb)
		Utils::Hook::Nop(0x553394, 6);		Utils::Hook(0x553394, Item_ListBox_Paint_SliderThumb_stub, HOOK_JUMP).install()->quick();

		// Fix border drawing bug
		Utils::Hook::Nop(0x54B6C9, 7);		Utils::Hook(0x54B6C9, Window_Paint_Border_Side_Fix, HOOK_JUMP).install()->quick();

		// Do not drop the player when UI_LoadMenus_LoadObj fails to load a menu
		Utils::Hook(0x5587FF, load_raw_menulist_error_stub, HOOK_JUMP).install()->quick();


		// *
		// Commands

		Command::Add("mainmenu_reload", "", "reloads zone \"xcommon_iw3xo_menu\"", [](Command::Params)
		{
				// re-calculate the gui layout
				GET_GGUI.menus[Game::GUI_MENUS::CHANGELOG].one_time_init = false;

				Game::Cmd_ExecuteSingleCommand(0, 0, "loadzone xcommon_iw3xo_menu\n");
		});

		// loads a menulist (txt file) and adds menus within it to the uicontext->menu stack
		Command::Add("menu_loadlist_raw", "<menulist_name.txt>", "rawfile :: load a menulist (txt file) and add included menus to the uicontext->menu stack (<fs_usedevdir> must be enabled)", [](Command::Params params)
		{
			auto fs_usedevdir = Game::Dvar_FindVar("fs_usedevdir");

			if (!fs_usedevdir || fs_usedevdir && !fs_usedevdir->current.enabled)
			{
				Game::Com_PrintMessage(0, "fs_usedevdir must be enabled to use this command! Make sure to reload your map after after enabling it!\n", 0);
				return;
			}

			if (params.Length() < 2)
			{
				Game::Com_PrintMessage(0, "Usage :: menu_loadlist <menulist_name.txt>\n", 0);
				return;
			}

			if (!Game::_uiContext)
			{
				Game::Com_PrintMessage(0, "uiContext was null\n", 0);
				return;
			}

			Game::MenuList* list = (Game::MenuList*)Game::UI_LoadMenus_LoadObj(std::string(params[1]).c_str(), 7);

			if (list)
			{
				// for each raw menufile in menulist
				for (auto rawMenu = 0; rawMenu < list->menuCount; rawMenu++)
				{
					bool replacedWithRaw = false;

					// for each loaded menufile in memory
					for (auto loadedMenus = 0; loadedMenus < Game::_uiContext->menuCount; loadedMenus++)
					{
						if (!Utils::Q_stricmp(list->menus[rawMenu]->window.name, Game::_uiContext->Menus[loadedMenus]->window.name))
						{
							Game::_uiContext->Menus[loadedMenus] = list->menus[rawMenu];

							Game::Com_PrintMessage(0, Utils::VA("Menu <%s> is already loaded, overwriting\n", list->menus[rawMenu]->window.name), 0);
							replacedWithRaw = true;

							break;
						}
					}

					if (!replacedWithRaw)
					{
						Game::_uiContext->Menus[Game::_uiContext->menuCount] = list->menus[rawMenu];
						Game::_uiContext->menuCount++;

						Game::Com_PrintMessage(0, Utils::VA("Added menu <%s> to the menu list.\n", list->menus[rawMenu]->window.name), 0);
					}
				}
			}
			else
			{
				Game::Com_PrintMessage(0, "You either forgot to restart the map after enabling <fs_usedevdir> or tried to load a non-existing file!\n", 0);
			}
		});

		// open / re-open the specified menu from uicontext->menus
		Command::Add("menu_open", "<menu_name>", "open / re-open the specified menu from uicontext->menus", [](Command::Params params)
		{
			if (params.Length() < 2) 
			{
				Game::Com_PrintMessage(0, "Usage :: menu_open <menu_name>\n", 0);
				return;
			}

			if (!Game::_uiContext || !Game::clientUI)
			{
				Game::Com_PrintMessage(0, "uiContext | clientUI was null\n", 0);
				return;
			}

			const char * name = params[1];
			Game::UiContext *ui = &Game::_uiContext[0];
			Game::clientUI->displayHUDWithKeycatchUI = true;

			Game::Menus_CloseByName(name, ui);
			Game::Menus_OpenByName(name, ui);
		});

		Command::Add("menu_open_ingame", "<menu_name>", "not even sure what i tried here", [](Command::Params params)
		{
			if (params.Length() < 2)
			{
				Game::Com_PrintMessage(0, "Usage :: menu_open_ingame <menu_name>\n", 0);
				return;
			}

			if (!Game::_uiContext)
			{
				Game::Com_PrintMessage(0, "uiContext was null\n", 0);
				return;
			}

			const char* name = params[1];
			Game::UiContext* ui = &Game::_uiContext[0];

			Game::Key_SetCatcher();
			Game::Menus_CloseAll(ui);
			Game::Menus_OpenByName(name, ui);
		});

		// close the specified menu
		Command::Add("menu_closebyname", "<menu_name>", "close the specified menu", [](Command::Params params)
		{
			if (params.Length() < 2)
			{
				Game::Com_PrintMessage(0, "Usage :: menu_closebyname <menu_name>\n", 0);
				return;
			}

			if (!Game::_uiContext)
			{
				Game::Com_PrintMessage(0, "uiContext was null\n", 0);
				return;
			}

			const char* name = params[1];
			Game::UiContext* ui = &Game::_uiContext[0];

			Game::Menus_CloseByName(name, ui);
		});

		Command::Add("iw3xo_github", "", "opens https://github.com/xoxor4d/iw3xo-dev", [](Command::Params)
		{
			ShellExecute(0, 0, L"https://github.com/xoxor4d/iw3xo-dev/", 0, 0, SW_SHOW);
		});

		Command::Add("iw3xo_radiant_github", "", "opens https://github.com/xoxor4d/iw3xo-radiant", [](Command::Params)
		{
			ShellExecute(0, 0, L"https://github.com/xoxor4d/iw3xo-radiant/", 0, 0, SW_SHOW);
		});

		Command::Add("help", "", "opens https://xoxor4d.github.io/projects/iw3xo/#in-depth", [](Command::Params)
		{
			ShellExecute(0, 0, L"https://xoxor4d.github.io/projects/iw3xo/#in-depth", 0, 0, SW_SHOW);
		});


		// *
		// Dvars

		Dvars::ui_button_highlight_radius = Game::Dvar_RegisterFloat(
			/* name		*/ "ui_button_highlight_radius",
			/* desc		*/ "highlight radius (scale)",
			/* default	*/ 0.85f,
			/* minVal	*/ 0.0001f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_button_highlight_brightness = Game::Dvar_RegisterFloat(
			/* name		*/ "ui_button_highlight_brightness",
			/* desc		*/ "highlight brightness (lerp)",
			/* default	*/ 0.5f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 2.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_button_outline_radius = Game::Dvar_RegisterFloat(
			/* name		*/ "ui_button_outline_radius",
			/* desc		*/ "outline radius (scale)",
			/* default	*/ 0.95f,
			/* minVal	*/ 0.0001f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_button_outline_brightness = Game::Dvar_RegisterFloat(
			/* name		*/ "ui_button_outline_brightness",
			/* desc		*/ "outline brightness (lerp)",
			/* default	*/ 0.7f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 2.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_eyes_position = Game::Dvar_RegisterVec2(
			/* name		*/ "ui_eyes_position",
			/* desc		*/ "position of main menu eyes",
			/* default	*/ 0.70f, 0.026f,
			/* minVal	*/ -2.0f,
			/* maxVal	*/ 2.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_eyes_size = Game::Dvar_RegisterFloat(
			/* name		*/ "ui_eyes_size",
			/* desc		*/ "size of main menu eyes",
			/* default	*/ 0.0041f,
			/* minVal	*/ 0.0001f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_eyes_alpha = Game::Dvar_RegisterFloat(
			/* name		*/ "ui_eyes_alpha",
			/* desc		*/ "alpha of main menu eyes",
			/* default	*/ 0.5f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

#if DEBUG
		Dvars::xo_menu_dbg = Game::Dvar_RegisterFloat(
			/* name		*/ "xo_menu_dbg",
			/* desc		*/ "dvar to debug menu shaders",
			/* default	*/ 1.0f,
			/* minVal	*/ -10000.0f,
			/* maxVal	*/ 10000.0f,
			/* flags	*/ Game::dvar_flags::none);
#endif

		Dvars::ui_subnavbar = Game::Dvar_RegisterBool(
			/* name		*/ "ui_subnavbar",
			/* desc		*/ "menu helper",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::read_only);

		Dvars::ui_subnavbar_item = Game::Dvar_RegisterInt(
			/* name		*/ "ui_subnavbar_item",
			/* desc		*/ "menu helper",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 100,
			/* flags	*/ Game::dvar_flags::read_only);

		Dvars::ui_renderWidth = Game::Dvar_RegisterInt(
			/* name		*/ "ui_renderWidth",
			/* desc		*/ "menu helper",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 7680,
			/* flags	*/ Game::dvar_flags::read_only);

		Dvars::ui_renderHeight = Game::Dvar_RegisterInt(
			/* name		*/ "ui_renderHeight",
			/* desc		*/ "menu helper",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 4320,
			/* flags	*/ Game::dvar_flags::read_only);

		Dvars::ui_devmap = Game::Dvar_RegisterBool(
			/* name		*/ "ui_devmap",
			/* desc		*/ "menu helper",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_developer = Game::Dvar_RegisterInt(
			/* name		*/ "ui_developer",
			/* desc		*/ "menu helper",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 2,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_developer_script = Game::Dvar_RegisterBool(
			/* name		*/ "ui_developer_script",
			/* desc		*/ "menu helper",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);


		// *
		// Display

		// Hook R_AspectRatio to initially reset the ultrawide dvar (menu helper)
		Utils::Hook::Nop(0x5F352E, 6);		Utils::Hook(0x5F352E, aspect_ratio_custom_reset_stub, HOOK_JUMP).install()->quick();

		// Set custom aspect ratio by using the default switchcase in R_AspectRatio
		Utils::Hook::Nop(0x5F35FA, 6);		Utils::Hook(0x5F35FA, aspect_ratio_custom_stub, HOOK_JUMP).install()->quick();

		Dvars::r_aspectRatio_custom = Game::Dvar_RegisterFloat(
			/* name		*/ "r_aspectRatio_custom",
			/* desc		*/ "description",
			/* default	*/ 2.3333333f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ Game::dvar_flags::saved);

		static std::vector <const char*> r_customAspectratio =
		{ 
			"auto", 
			"4:3", 
			"16:10", 
			"16:9", 
			//"21:9", 
			"custom",
		};

		Dvars::r_aspectRatio = Game::Dvar_RegisterEnum(
			/* name		*/ "r_aspectRatio",
			/* desc		*/ "Screen aspect ratio. Use \"custom\" and \"r_aspectRatio_custom\" if your aspect ratio is not natively supported! (21/9 = 2.3333)",
			/* default	*/ 0,
			/* enumSize	*/ r_customAspectratio.size(),
			/* enumData */ r_customAspectratio.data(),
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_ultrawide = Game::Dvar_RegisterBool(
			/* name		*/ "ui_ultrawide",
			/* desc		*/ "menu helper",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::read_only);
	}

	_UI::~_UI()
	{ }
}