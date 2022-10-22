#include "std_include.hpp"

#define GET_UIMATERIAL(ptr) (game::Material*) *(DWORD*)(ptr)

// spacing dummy
#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 

// seperator with spacing
#define SEPERATORV(spacing, enabled) SPACING(0.0f, spacing); \
	if(enabled) { ImGui::Separator(); SPACING(0.0f, spacing); }

// fade from black into main menu on first start 
#define MAINMENU_INIT_FADE_TIME 50.0f

namespace components
{
	int _ui::get_text_height(game::Font_s *font)
	{
		if (font)
		{
			return font->pixelHeight;
		}

		return 16;
	}

	float _ui::scrplace_apply_x(int horz_align, const float x, const float x_offs)
	{
		float result;
		switch (horz_align)
		{
		case HORIZONTAL_APPLY_LEFT:
			result = game::scrPlace->scaleVirtualToReal[0] * x + x_offs + game::scrPlace->realViewableMin[0];
			break;

		case HORIZONTAL_APPLY_CENTER:
			result = game::scrPlace->scaleVirtualToReal[0] * x + x_offs + game::scrPlace->realViewportSize[0] * 0.5f;
			break;

		case HORIZONTAL_APPLY_RIGHT:
			result = game::scrPlace->scaleVirtualToReal[0] * x + x_offs + game::scrPlace->realViewableMax[0];
			break;

		case HORIZONTAL_APPLY_FULLSCREEN:
			result = game::scrPlace->scaleVirtualToFull[0] * x + x_offs;
			break;

		case HORIZONTAL_APPLY_NONE:
			result = x;
			break;

		case HORIZONTAL_APPLY_TO640:
			result = game::scrPlace->scaleRealToVirtual[0] * x + x_offs;
			break;

		case HORIZONTAL_APPLY_CENTER_SAFEAREA:
			result = (game::scrPlace->realViewableMax[0] + game::scrPlace->realViewableMin[0]) * 0.5f + x_offs + game::scrPlace->scaleVirtualToReal[0] * x;
			break;

		case HORIZONTAL_APPLY_CONSOLE_SPECIAL:
			result = game::scrPlace->realViewableMax[0] - x_offs + game::scrPlace->scaleVirtualToReal[0] * x;
			break;

		default:
			result = x;
		}

		return result;
	}

	float _ui::scrplace_apply_y(int vert_align, const float y, const float y_offs)
	{
		float result;
		switch (vert_align)
		{
		case VERTICAL_APPLY_TOP:
			result = game::scrPlace->scaleVirtualToReal[1] * y + y_offs + game::scrPlace->realViewableMin[1];
			break;

		case VERTICAL_APPLY_CENTER:
			result = game::scrPlace->scaleVirtualToReal[1] * y + y_offs + game::scrPlace->realViewportSize[1] * 0.5f;
			break;

		case VERTICAL_APPLY_BOTTOM:
			result = game::scrPlace->scaleVirtualToReal[1] * y + y_offs + game::scrPlace->realViewableMax[1];
			break;

		case VERTICAL_APPLY_FULLSCREEN:
			result = game::scrPlace->scaleVirtualToFull[1] * y + y_offs;
			break;

		case VERTICAL_APPLY_NONE:
			result = y;
			break;

		case VERTICAL_APPLY_TO640:
			result = game::scrPlace->scaleRealToVirtual[1] * y + y_offs;
			break;

		case VERTICAL_APPLY_CENTER_SAFEAREA:
			result = (game::scrPlace->realViewableMax[1] + game::scrPlace->realViewableMin[1]) * 0.5f + y_offs + game::scrPlace->scaleVirtualToReal[1] * y;
			break;

		default:
			result = y;
		}

		return result;
	}

	void _ui::scrplace_apply_rect(float *offs_x, float *w, float *offs_y, float *h, int horz_anker, int vert_anker)
	{
		float _x, _w, _y, _h;

		switch (horz_anker)
		{
		case HORIZONTAL_ALIGN_SUBLEFT:
			_x = game::scrPlace->scaleVirtualToReal[0] * *offs_x + game::scrPlace->subScreenLeft;
			_w = game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_LEFT:
			_x = game::scrPlace->scaleVirtualToReal[0] * *offs_x + game::scrPlace->realViewableMin[0];
			_w = game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_CENTER:
			_x = game::scrPlace->scaleVirtualToReal[0] * *offs_x + game::scrPlace->realViewportSize[0] * 0.5f;
			_w = game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_RIGHT:
			_x = game::scrPlace->scaleVirtualToReal[0] * *offs_x + game::scrPlace->realViewableMax[0];
			_w = game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_FULLSCREEN:
			_x = game::scrPlace->scaleVirtualToFull[0] * *offs_x;
			_w = game::scrPlace->scaleVirtualToFull[0] * *w;
			break;

		case HORIZONTAL_ALIGN_NOSCALE:
			goto USE_VERT_ALIGN; // we might wan't vertical alignment

		case HORIZONTAL_ALIGN_TO640:
			_x = game::scrPlace->scaleRealToVirtual[0] * *offs_x;
			_w = game::scrPlace->scaleRealToVirtual[0] * *w;
			break;

		case HORIZONTAL_ALIGN_CENTER_SAFEAREA:
			_x = (game::scrPlace->realViewableMax[0] + game::scrPlace->realViewableMin[0]) * 0.5f + game::scrPlace->scaleVirtualToReal[0] * *offs_x;
			_w = game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		default:
			goto USE_VERT_ALIGN; // we might wan't vertical alignment

		}

		*offs_x = _x;
		*w = _w;

	USE_VERT_ALIGN:
		switch (vert_anker)
		{
		case VERTICAL_ALIGN_TOP:
			_y = game::scrPlace->scaleVirtualToReal[1] * *offs_y + game::scrPlace->realViewableMin[1];
			_h = game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_CENTER:
			_y = game::scrPlace->scaleVirtualToReal[1] * *offs_y + game::scrPlace->realViewportSize[1] * 0.5f;
			_h = game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_BOTTOM:
			_y = game::scrPlace->scaleVirtualToReal[1] * *offs_y + game::scrPlace->realViewableMax[1];
			_h = game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_FULLSCREEN:
			_y = game::scrPlace->scaleVirtualToFull[1] * *offs_y;
			_h = game::scrPlace->scaleVirtualToFull[1] * *h;
			break;

		case VERTICAL_ALIGN_NOSCALE:
			return;

		case VERTICAL_ALIGN_TO480:
			_y = game::scrPlace->scaleRealToVirtual[1] * *offs_y;
			_h = game::scrPlace->scaleRealToVirtual[1] * *h;
			break;

		case VERTICAL_ALIGN_CENTER_SAFEAREA:
			_y = game::scrPlace->scaleVirtualToReal[1] * *offs_y + (game::scrPlace->realViewableMax[1] + game::scrPlace->realViewableMin[1]) * 0.5f;
			_h = game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_SUBTOP:
			_y = game::scrPlace->scaleVirtualToReal[1] * *offs_y;
			_h = game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		default:
			return;
		}

		*offs_y = _y;
		*h = _h;
	}

	void _ui::scrplacefull_apply_rect(float *offs_x, float *w, float *offs_y, float *h, int horz_anker, int vert_anker)
	{
		float _x, _w, _y, _h;

		switch (horz_anker)
		{
		case HORIZONTAL_ALIGN_SUBLEFT:
			_x = game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x + game::scrPlaceFull->subScreenLeft;
			_w = game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_LEFT:
			_x = game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x + game::scrPlaceFull->realViewableMin[0];
			_w = game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_CENTER:
			_x = game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x + game::scrPlaceFull->realViewportSize[0] * 0.5f;
			_w = game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_RIGHT:
			_x = game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x + game::scrPlaceFull->realViewableMax[0];
			_w = game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_FULLSCREEN:
			_x = game::scrPlaceFull->scaleVirtualToFull[0] * *offs_x;
			_w = game::scrPlaceFull->scaleVirtualToFull[0] * *w;
			break;

		case HORIZONTAL_ALIGN_NOSCALE:
			goto USE_VERT_ALIGN; // we might wan't vertical alignment

		case HORIZONTAL_ALIGN_TO640:
			_x = game::scrPlaceFull->scaleRealToVirtual[0] * *offs_x;
			_w = game::scrPlaceFull->scaleRealToVirtual[0] * *w;
			break;

		case HORIZONTAL_ALIGN_CENTER_SAFEAREA:
			_x = (game::scrPlaceFull->realViewableMax[0] + game::scrPlaceFull->realViewableMin[0]) * 0.5f + game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x;
			_w = game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		default:
			goto USE_VERT_ALIGN; // we might wan't vertical alignment

		}

		*offs_x = _x;
		*w = _w;

	USE_VERT_ALIGN:
		switch (vert_anker)
		{
		case VERTICAL_ALIGN_TOP:
			_y = game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y + game::scrPlaceFull->realViewableMin[1];
			_h = game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_CENTER:
			_y = game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y + game::scrPlaceFull->realViewportSize[1] * 0.5f;
			_h = game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_BOTTOM:
			_y = game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y + game::scrPlaceFull->realViewableMax[1];
			_h = game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_FULLSCREEN:
			_y = game::scrPlaceFull->scaleVirtualToFull[1] * *offs_y;
			_h = game::scrPlaceFull->scaleVirtualToFull[1] * *h;
			break;

		case VERTICAL_ALIGN_NOSCALE:
			return;

		case VERTICAL_ALIGN_TO480:
			_y = game::scrPlaceFull->scaleRealToVirtual[1] * *offs_y;
			_h = game::scrPlaceFull->scaleRealToVirtual[1] * *h;
			break;

		case VERTICAL_ALIGN_CENTER_SAFEAREA:
			_y = game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y + (game::scrPlaceFull->realViewableMax[1] + game::scrPlaceFull->realViewableMin[1]) * 0.5f;
			_h = game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_SUBTOP:
			_y = game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y;
			_h = game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		default:
			return;
		}

		*offs_y = _y;
		*h = _h;
	}

	void _ui::redraw_cursor()
	{
		// get material handle
		const auto cur_material = game::Material_RegisterHandle("ui_cursor", 3);

		float cur_w = (32.0f * game::scrPlace->scaleVirtualToReal[0]) / game::scrPlace->scaleVirtualToFull[0];
		float cur_h = (32.0f * game::scrPlace->scaleVirtualToReal[1]) / game::scrPlace->scaleVirtualToFull[1];
		float cur_x = game::ui_context->cursor.x - 0.5f * cur_w;
		float cur_y = game::ui_context->cursor.y - 0.5f * cur_h;

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
		_ui::scrplace_apply_rect(&cur_x, &cur_w, &cur_y, &cur_h, VERTICAL_ALIGN_FULLSCREEN, VERTICAL_ALIGN_FULLSCREEN);
		game::R_AddCmdDrawStretchPic(cur_material, cur_x, cur_y, cur_w, cur_h, s0, t0, s1, t1, nullptr);
	}


	/* ---------------------------------------------------------- */
	/* ------------------- general main menu -------------------- */

	float mainmenu_fade_time = MAINMENU_INIT_FADE_TIME;
	float mainmenu_fade_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// d3d9ex::D3D9Device::Release() resets mainmenu vars on vid_restart
	void main_menu_fade_in()
	{
		if (!game::Sys_IsMainThread() || game::glob::mainmenu_fade_done || !game::ui_context)
		{
			return;
		}

		if (mainmenu_fade_time >= 0.0f)
		{
			mainmenu_fade_time -= 0.5f;
			mainmenu_fade_color[3] = (mainmenu_fade_time * (1.0f / MAINMENU_INIT_FADE_TIME));

			game::ConDraw_Box(mainmenu_fade_color, 0.0f, 0.0f, 
				static_cast<float>(game::ui_context->screenWidth),
				static_cast<float>(game::ui_context->screenHeight));

			return;
		}

		// reset fade vars
		mainmenu_fade_time = MAINMENU_INIT_FADE_TIME;
		mainmenu_fade_color[3] = 1.0f;

		game::glob::mainmenu_fade_done = true;
	}

	// *
	// called from <gui::render_loop>
	void _ui::create_changelog(game::gui_menus_t& menu)
	{
		// do not keep mouse cursor around if another ImGui was active
		menu.mouse_ignores_menustate = true;

		ImGuiIO& io = ImGui::GetIO();
		io.WantCaptureKeyboard = false;
		io.WantCaptureMouse = true;

		game::UiContext* ui = game::ui_context;
		int pushed_styles = 0, pushed_colors = 0;
		
		// ImGui is rendered on EndScene so UI_MainMenu_FadeIn wont work => fade manually
		if (!game::glob::mainmenu_fade_done)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f - mainmenu_fade_color[3]); pushed_styles++;
		}

		// set window size once
		if (!menu.one_time_init)
		{
			menu.got_layout_from_menu = false;

			if (ui)
			{
				for (auto m = 0; m < ui->openMenuCount && !menu.got_layout_from_menu; m++)
				{
					if (ui->menuStack[m] && !utils::q_stricmp(ui->menuStack[m]->window.name, "main_text"))
					{
						for (auto i = 0; i < ui->menuStack[m]->itemCount && !menu.got_layout_from_menu; i++)
						{
							if (ui->menuStack[m]->items[i] && ui->menuStack[m]->items[i]->window.name)
							{
								if (!utils::q_stricmp(ui->menuStack[m]->items[i]->window.name, "changelog_container"))
								{
									gui::set_menu_layout(
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
				_ui::scrplacefull_apply_rect(&menu.position[0], &menu.size[0], &menu.position[1], &menu.size[1], menu.horzAlign, menu.vertAlign);

				ImGui::SetNextWindowPos(ImVec2(menu.position[0], menu.position[1]), ImGuiCond_Always); //ImGuiCond_FirstUseEver);
				ImGui::SetNextWindowSize(ImVec2(menu.size[0], menu.size[1]), ImGuiCond_Always); //ImGuiCond_FirstUseEver);

				menu.one_time_init = true;
			}
		}
		else // disable changelog gui
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
			if (game::glob::changelog_html_body.length() == 0)
			{
				gui::markdown(markdownText.c_str(), markdownText.c_str() + markdownText.size());
			}
			// real changelog
			else
			{
				gui::markdown(game::glob::changelog_html_body.c_str(), game::glob::changelog_html_body.c_str() + game::glob::changelog_html_body.size());
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

		gui::redraw_cursor();
		ImGui::End();
	}

	void _ui::register_dvars()
	{
		dvars::ui_main_title = game::Dvar_RegisterString(
			/* name		*/ "ui_changelog_title",
			/* desc		*/ "menu helper",
			/* value	*/ utils::va(IW3XO_CHANGELOG_TITLE_FMT, VERSION, IW3XO_BUILDVERSION_DATE),
			/* flags	*/ game::dvar_flags::read_only);
	}

	// *
	// draw additional stuff to the main menu
	void main_menu()
	{
		if (!game::Sys_IsMainThread()) 
		{
			return;
		}

		const char* font;
		const float max = game::scrPlace->scaleVirtualToReal[1] * 0.3f;

		const auto ui_smallFont = game::Dvar_FindVar("ui_smallFont");
		const auto ui_extraBigFont = game::Dvar_FindVar("ui_extraBigFont");
		const auto ui_bigFont = game::Dvar_FindVar("ui_bigFont");

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

		const auto font_handle = game::R_RegisterFont(font, sizeof(font));
		if (!font_handle) 
		{
			return;
		}

		float offs_x = 10.0f; 
		float offs_y = -10.0f; 
		const float scale = 0.25f;

		float scale_x = scale * 48.0f / static_cast<float>(font_handle->pixelHeight);
		float scale_y = scale_x;

		// place container
		_ui::scrplace_apply_rect(&offs_x, &scale_x, &offs_y, &scale_y, HORIZONTAL_ALIGN_LEFT, VERTICAL_ALIGN_BOTTOM);

		const char* text_foreground = utils::va("IW3xo :: %s :: %s", VERSION, IW3XO_BUILDVERSION_DATE);
		const char* text_background = text_foreground;

		if (DEBUG)
		{
			text_foreground = utils::va("IW3xo :: %s :: %s :: %s", VERSION, IW3XO_BUILDVERSION_DATE, "^1DEBUG");
			text_background = utils::va("IW3xo :: %s :: %s :: %s", VERSION, IW3XO_BUILDVERSION_DATE, "DEBUG");
		}

		// Background String
		const float color_background[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

		game::R_AddCmdDrawTextASM(
			/* txt */ text_background,
			/* max */ 0x7FFFFFFF,
			/* fot */ font_handle,
			/*  x  */ offs_x + 3.0f,
			/*  y  */ offs_y + 3.0f,
			/* scX */ scale_x,
			/* scY */ scale_y,
			/* rot */ 0.0f,
			/* col */ color_background,
			/* sty */ 0);

		// Foreground String
		const float color_foreground[4] = { 1.0f, 0.8f, 0.7f, 1.0f };

		game::R_AddCmdDrawTextASM(
			/* txt */ text_foreground,
			/* max */ 0x7FFFFFFF,
			/* fot */ font_handle,
			/*  x  */ offs_x,
			/*  y  */ offs_y,
			/* scX */ scale_x,
			/* scY */ scale_y,
			/* rot */ 0.0f,
			/* col */ color_foreground,
			/* sty */ 0);

		// fade in the menu on first start
		main_menu_fade_in();


		// *
		// changelog gui
		
		// _client::on_disconnect() hides / shows the changelog when connecting/disconnecting
		// d3d9ex::D3D9Device::Release() resets mainmenu vars on vid_restart
		if (components::active.gui)
		{
			// do not draw the changelog when the Com_Error menu is open
			if (const auto& com_errorMessage = game::Dvar_FindVar("com_errorMessage"); 
							com_errorMessage && com_errorMessage->current.string[0])
			{
				return;
			}

			// open changelog gui at startup
			if (!game::glob::loaded_main_menu)
			{
				GET_GGUI.menus[game::GUI_MENUS::CHANGELOG].menustate = true;
			}

			if (const auto	ui = game::ui_context; 
							ui)
			{
				// draw/hide the gui using menu localvars
				for (auto var = 0; var < 255; var++)
				{
					if (ui->localVars.table[var].name && !utils::q_stricmp(ui->localVars.table[var].name, "ui_maincontent_update"))
					{
						if (ui->localVars.table[var].u.integer == 0)
						{
							GET_GGUI.menus[game::GUI_MENUS::CHANGELOG].menustate = true;
						}
						else // close
						{
							GET_GGUI.menus[game::GUI_MENUS::CHANGELOG].menustate = false;
						}

						break;
					}
				}
			}
		}

		game::glob::loaded_main_menu = true;
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
	float menu_game_time = 0.0f;

	void set_custom_codeconstants()
	{
		if (!game::ui_context || !game::gfxCmdBufSourceState)
		{
			return;
		}

		// check if we are in a menu to stop overwriting filtertap? needed?
		if (menu_game_time >= 10000.0f) {
			menu_game_time = 0.0f;
		}

		menu_game_time += 0.01f;

		// -- mouse position --
		glm::vec2 mouse_pos;

		// normalize mouse to ui resolution
		mouse_pos.x = (game::ui_context->cursor.x / 640.0f);
		mouse_pos.y = (game::ui_context->cursor.y / 480.0f);

		// screencenter will be 0.0 ( -1.0, 1.0 = bottom left )
		mouse_pos.x = mouse_pos.x * 2.0f - 1.0f;
		mouse_pos.y = mouse_pos.y * 2.0f - 1.0f;

		// filterTap 5
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_5][0] = dvars::ui_eyes_position->current.vector[0];
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_5][1] = dvars::ui_eyes_position->current.vector[1];
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_5][2] = dvars::ui_eyes_size->current.value;
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_5][3] = dvars::ui_eyes_alpha->current.value;

		// filterTap 6
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_6][0] = dvars::ui_button_highlight_radius->current.value;
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_6][1] = dvars::ui_button_highlight_brightness->current.value;
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_6][2] = dvars::ui_button_outline_radius->current.value;
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_6][3] = dvars::ui_button_outline_brightness->current.value;

		// filterTap 7
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_7][0] = mouse_pos.x;
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_7][1] = mouse_pos.y;
#if DEBUG		
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_7][2] = dvars::xo_menu_dbg->current.value;
#endif
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_7][3] = menu_game_time;

		// get current viewport width / height and update ui_dvars
		const auto viewport_width = floorf(_ui::scrplace_apply_x(HORIZONTAL_APPLY_RIGHT, 0.0f, 0.0f));
		const auto viewport_height = floorf(_ui::scrplace_apply_y(VERTICAL_APPLY_BOTTOM, 0.0f, 0.0f));

		// set rendersize
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_RENDER_TARGET_SIZE][0] = viewport_width;
		game::gfxCmdBufSourceState->input.consts[game::ShaderCodeConstants::CONST_SRC_CODE_RENDER_TARGET_SIZE][1] = viewport_height;

		if (dvars::ui_renderWidth && dvars::ui_renderHeight)
		{
			game::dvar_set_value_dirty(dvars::ui_renderWidth, static_cast<int>(viewport_width));
			game::dvar_set_value_dirty(dvars::ui_renderHeight, static_cast<int>(viewport_height));
		}
	}

	// hook "R_RenderAllLeftovers" in RB_Draw3DCommon to set custom codeconstants
	__declspec(naked) void RB_Draw3DCommon_stub()
	{
		const static uint32_t R_RenderAllLeftovers_func = 0x615570;
		const static uint32_t retn_addr = 0x6156F1;
		__asm
		{
			pushad;
			call	set_custom_codeconstants;
			popad;

			call	R_RenderAllLeftovers_func;
			jmp		retn_addr;
		}
	}


	/* ---------------------------------------------------------- */
	/* ------------------- ui fixes and addons ------------------ */
	
	game::Material* ui_white_material;
	
	void init_white_material()
	{
		ui_white_material = GET_UIMATERIAL(0xCAF06F0); // white material pointer
		if (!ui_white_material)
		{
			ui_white_material = game::Material_RegisterHandle("white", 3);
		}
	}
	
	// ITEM-SLIDER :: Change material "ui_scrollbar" to white and add a color (background)
	__declspec(naked) void Item_ListBox_Paint_SliderBackground_stub()
	{
		const static uint32_t retn_addr = 0x55330D; // jump onto the call to UI_DrawHandlePic

		const static float x_offs = 11.0f;
		const static float y_offs = -1.0f;
		const static float h_offs =  0.0f; // add additional height
		const static float meme2 =  0.0f; // add additional height
		const static float width =  6.0f;

		const static float  color[4] = LISTBOX_SLIDER_BACKGROUND_COLOR;
		const static float* color_ptr = color; // we need a ptr to our color

		// white material ptr location @ 0xCAF06F0 points-to 0xFA4EC8
		__asm
		{
			pushad;
			call	init_white_material;
			popad;

			fld     dword ptr[esp + 50h];		// stock op
			mov     eax, [edi];					// stock op
			fadd	y_offs;						// stock op -- itemheight + yOffs
			mov		ecx, [ebp + eax * 4 + 4];	// stock op
			mov		[ebp + eax * 4 + 8], ecx;	// stock op

			// material <-> color
			mov     edx, ui_white_material;	// ui white material handle
			mov     eax, [ebx + 18h];		// stock op
			fstp    dword ptr[esp + 50h];	// stock op
			fld     dword ptr[ebx + 10h];	// stock op
			mov     ecx, [ebx + 14h];		// stock op
			fadd	h_offs;					// stock op -- h - hOffs
			add     esp, 1Ch;				// stock op
			push    edx;					// stock op -- material
			push    eax;					// stock op -- vertAlign
			fstp    dword ptr[esp + 20h];	// stock op
			push    ecx;					// stock op -- horzAlign
			fld     dword ptr[esp + 24h];	// stock op
			sub     esp, 10h;				// stock op
			fadd	meme2;					// stock op -- new h + 1.0f
			mov     ecx, color_ptr;			// color -- was "xor ecx, ecx"

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
			fadd	x_offs;
			fstp    dword ptr[esp];			// stock op -- x

			jmp		retn_addr;
		}
	}

	// ITEM-SLIDER :: Change material "material_ui_scrollbar_thumb" to white and add a color (thumb)
	__declspec(naked) void Item_ListBox_Paint_SliderThumb_stub()
	{
		const static uint32_t retn_addr = 0x5533C7; // jump onto the call to UI_DrawHandlePic

		const static float boxWH =  6.0f;
		const static float x_offs = 11.0f;
		const static float h_offs = 40.0f;
		const static float y_offs = -(h_offs * 0.5f) + 3.0f;
		

		const static float  color[4] = LISTBOX_SLIDER_THUMB_COLOR;
		const static float* color_ptr = color; // we need a ptr to our color

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
			fadd	h_offs;
			fst     dword ptr[esp + 0Ch];		// stock op -- h (uses boxWH)
			mov     ecx, color_ptr;				// color -- was "xor ecx, ecx"
			fsub	h_offs;
			fstp    dword ptr[esp + 8];			// stock op -- w (uses boxWH)
			mov     edx, esi;					// stock op
			fld     dword ptr[esp + 34h];		// stock op
			fadd	y_offs;
			fstp    dword ptr[esp + 4];			// stock op -- y
			fld     dword ptr[esp + 30h];		// stock op
			fadd	x_offs;
			fstp    dword ptr[esp];				// stock op -- x

			jmp		retn_addr;
		}
	}

	// add custom uiScripts for menus :: returns true when a valid uiScript string was matched => skip the original function to avoid error prints and to make overwriting stock uiScripts a thing
	int scripts_addons(const char *arg)
	{
		if (!utils::q_stricmpn(arg, "StartServerCustom", 0x7FFFFFFF))
		{
			const char* spawn_server_str;
			const char* game_type_str;
			const char* map_name_str;

			const auto ui_dedicated	= game::Dvar_FindVar("ui_dedicated");
			const auto ui_netGameType = game::Dvar_FindVar("ui_netGameType");
			const auto ui_currentNetMap	= game::Dvar_FindVar("ui_currentNetMap");
			const auto dedicated	 = game::Dvar_FindVar("dedicated");
			const auto g_gametype = game::Dvar_FindVar("g_gametype");
			
			if (ui_dedicated && ui_dedicated->current.integer != 0) 
			{
				game::Com_PrintMessage(0, utils::va("^3Warning ^7:: setting \"dedicated\" to 0 because advanced server settings only supports listen servers!\n"), 0);
				game::dvar_set_value_dirty(dedicated, 0);
			}

			if (game::gameTypeEnum && *game::gameTypeEnum && ui_netGameType && ui_currentNetMap)
			{
				game_type_str = (const char*)game::gameTypeEnum[0x2 * ui_netGameType->current.integer];
				map_name_str = (const char*)game::mapNameEnum[0x28 * ui_currentNetMap->current.integer];
			}
			else
			{
				game::Com_PrintMessage(0, "^3Warning ^7:: gametype or map name was empty! Using Defaults!\n", 0);
				
				game_type_str = "dm";
				map_name_str = "mp_backlot";
			}

			// set gametype
			if (g_gametype)
			{
				game::Dvar_SetString(game_type_str, g_gametype);
			}
			
			// server spawn string
			if (dvars::ui_developer && dvars::ui_developer_script && dvars::ui_devmap)
			{
				spawn_server_str = utils::va("wait ; wait ; set developer %i; set developer_script %i; %s %s\n",
											dvars::ui_developer->current.integer, dvars::ui_developer_script->current.integer, dvars::ui_devmap->current.enabled ? "devmap" : "map", map_name_str);
			}
			else
			{
				spawn_server_str = utils::va("wait ; wait ; map %s", map_name_str);
			}
			
			game::Com_PrintMessage(0, utils::va("^2Spawning server ^7:: %s", spawn_server_str), 0);
			game::Cbuf_AddText(spawn_server_str, 0);

			// return true to skip the stock function that looks for valid uiScripts
			return 1;
		}

		// return false and continue with the stock function
		return 0;
	}

	// hook "Item_RunScript" to implement custom uiScripts callable from menu files
	__declspec(naked) void scripts_addons_stub()
	{
		const static char* overwritten_str = "StartServer";
		const static uint32_t stock_scripts_addr = 0x545BF2;  // next op after hook spot
		const static uint32_t if_addon_return_addr = 0x546E52; // jump to the valid return point if we had a valid match in addons
		__asm
		{
			lea     edx, [esp + 58h];		// 'out' arg was at 0x5C but we hooked at the push before it => 'out' arg at esp+5C - 4 ;)
	
			pushad;
			push	edx;
			call	scripts_addons;
			add		esp, 4;
			test	eax, eax;
			je		STOCK_FUNC;				// jump if UI_uiScriptsAddons was false
			popad;

			jmp		if_addon_return_addr;	// return to skip the stock function

			STOCK_FUNC:
			popad;
			push	overwritten_str;		// the original push we hooked at
			jmp		stock_scripts_addr;		// jump back and exec the original function
		}
	}

	// fix rect not being drawn properly when a border is applied
	_declspec(naked) void window_paint_border_side_fix() 
	{
		const static uint32_t retn_addr = 0x54B6FA;
		_asm 
		{
			cmp     dword ptr[ebx + 3Ch], 2;	// if border 2
			je		short border_2;

			cmp     dword ptr[ebx + 3Ch], 3;	// if border 3
			je		short border_3;

			//fix for full border
			fld     dword ptr[ebx + 48h];	// stock op
			fadd    dword ptr[esp + 10h];	// stock op
			fstp    dword ptr[esp + 10h];	// stock op
			fld     dword ptr[ebx + 48h];	// stock op
			fadd    dword ptr[esp + 14h];	// stock op
			fstp    dword ptr[esp + 14h];	// stock op
			fld     dword ptr[ebx + 48h];	// stock op
			fld     dword ptr[esp + 18h];	// stock op
			fsub    st, st(1);				// stock op
			fsub    st, st(1);
			fstp    dword ptr[esp + 18h];	// stock op
			fsubr   dword ptr[esp + 1Ch];	// subract border size (st0) from height, save in st0
			fsub	dword ptr[ebx + 48h];	// subract boder size from st0
			fstp    dword ptr[esp + 1Ch];	// stock op
			jmp		retn_addr;


			//fix for border 2
			border_2:

			fld     dword ptr[ebx + 48h];	// load bordersize
			fadd    dword ptr[esp + 14h];	// add y to bordersize
			fstp    dword ptr[esp + 14h];	// store y, pop st0

			fld     dword ptr[ebx + 48h];	// load bordersize
			fsubr	dword ptr[esp + 1Ch];
			fsub	dword ptr[ebx + 48h];
			fstp	dword ptr[esp + 1Ch];	// store width
			jmp		retn_addr;


			//fix for border 3
			border_3:
			
			fld     dword ptr[ebx + 48h];	// load bordersize
			fadd    dword ptr[esp + 10h];	// add x to bordersize
			fstp    dword ptr[esp + 10h];	// store x, pop st0

			fld     dword ptr[ebx + 48h];	// load bordersize
			fsubr   dword ptr[esp + 18h];
			fsub	dword ptr[ebx + 48h];
			fstp    dword ptr[esp + 18h];	// store height
			jmp		retn_addr;
		}
	}

	// do not drop the player when UI_LoadMenus_LoadObj fails to load a menu
	__declspec(naked) void load_raw_menulist_error_stub()
	{
		__asm
		{
			add     esp, 18h;	// hook is placed on call to FS_FOpenFileReadForThread, so fix the stack
			
			xor		eax, eax;	// return a nullptr
			pop		edi;		// epilog
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
		if (dvars::ui_ultrawide)
		{
			game::dvar_set_value_dirty(dvars::ui_ultrawide, state);
		}
	}

	void set_custom_aspect_ratio()
	{
		if (dvars::r_aspectRatio_custom)
		{
			*(float*)(0xCC9D0F8) = dvars::r_aspectRatio_custom->current.value;
		}
	}

	// hook R_AspectRatio to initially reset the ultrawide dvar (menu helper)
	__declspec(naked) void aspect_ratio_custom_reset_stub()
	{
		const static uint32_t retn_addr = 0x5F3534;
		__asm
		{
			pushad;
			push	0;
			call	set_ultrawide_dvar;
			add		esp, 4h;
			popad;

			mov     eax, [eax + 0Ch];	// overwritten op
			cmp     eax, 3;				// overwritten op

			jmp		retn_addr;			// jump back to break op
		}
	}

	// set custom aspect ratio by using the default switchcase in R_AspectRatio
	__declspec(naked) void aspect_ratio_custom_stub()
	{
		const static uint32_t retn_addr = 0x5F35E5;
		__asm
		{
			pushad;
			push	1;
			call	set_ultrawide_dvar;
			add		esp, 4h;

			Call	set_custom_aspect_ratio;
			popad;

			mov     ecx, 1;				// widescreen true
			jmp		retn_addr;			// jump back to break op
		}
	}

	DWORD WINAPI get_changelog(LPVOID)
	{
		// *
		// get changelog (raw markdown)

		std::string url = "https://raw.githubusercontent.com/wiki/xoxor4d/iw3xo-dev/Changelog.md";
		std::wstring header, html;
		utils::get_html(url, header, html);

		std::transform(html.begin(), html.end(), std::back_inserter(game::glob::changelog_html_body), [](wchar_t c)
		{
			return (char)c;
		});

		return TRUE;
	}

	_ui::_ui()
	{
		CreateThread(nullptr, 0, get_changelog, nullptr, 0, nullptr);

		// *
		// Main Menu Hooks

		// Main Menu Version (UI_VersionNumber Call in UI_Refresh)
		utils::hook(0x543535, main_menu_stub, HOOK_JUMP).install()->quick();

		// hook "R_RenderAllLeftovers" in RB_Draw3DCommon to set custom codeconstants
		utils::hook(0x6156EC, RB_Draw3DCommon_stub, HOOK_JUMP).install()->quick();

		// hook "Item_RunScript" to implement custom uiScripts callable from menu files
		utils::hook(0x545BED, scripts_addons_stub, HOOK_JUMP).install()->quick();


		// *
		// List Box Slider

		// Change material "ui_scrollbar" to white and add a color (background)
		utils::hook::nop(0x5532A0, 6);		utils::hook(0x5532A0, Item_ListBox_Paint_SliderBackground_stub, HOOK_JUMP).install()->quick();
		
		// disable drawing of upper arrow :: nop "UI_DrawHandlePic" call
		utils::hook::nop(0x55329B, 5);

		// disable drawing of lower arrow :: nop "UI_DrawHandlePic" call
		utils::hook::nop(0x55335A, 5);

		// Change material "material_ui_scrollbar_thumb" to white and add a color (thumb)
		utils::hook::nop(0x553394, 6);		utils::hook(0x553394, Item_ListBox_Paint_SliderThumb_stub, HOOK_JUMP).install()->quick();

		// Fix border drawing bug
		utils::hook::nop(0x54B6C9, 7);		utils::hook(0x54B6C9, window_paint_border_side_fix, HOOK_JUMP).install()->quick();

		// Do not drop the player when UI_LoadMenus_LoadObj fails to load a menu
		utils::hook(0x5587FF, load_raw_menulist_error_stub, HOOK_JUMP).install()->quick();


		// *
		// Commands

		command::add("mainmenu_reload", "", "reloads zone \"xcommon_iw3xo_menu\"", [](command::params)
		{
				// re-calculate the gui layout
				GET_GGUI.menus[game::GUI_MENUS::CHANGELOG].one_time_init = false;
				game::Cmd_ExecuteSingleCommand(0, 0, "loadzone xcommon_iw3xo_menu\n");
		});


		// loads a menulist (txt file) and adds menus within it to the uicontext->menu stack
		command::add("menu_loadlist_raw", "<menulist_name.txt>", "rawfile :: load a menulist (txt file) and add included menus to the uicontext->menu stack (<fs_usedevdir> must be enabled)", [](command::params params)
		{
			if (const auto fs_usedevdir = game::Dvar_FindVar("fs_usedevdir"); 
				!fs_usedevdir || fs_usedevdir && !fs_usedevdir->current.enabled)
			{
				game::Com_PrintMessage(0, "fs_usedevdir must be enabled to use this command! Make sure to reload your map after after enabling it!\n", 0);
				return;
			}

			if (params.length() < 2)
			{
				game::Com_PrintMessage(0, "Usage :: menu_loadlist <menulist_name.txt>\n", 0);
				return;
			}

			if (!game::ui_context)
			{
				game::Com_PrintMessage(0, "uiContext was null\n", 0);
				return;
			}

			

			if (const auto	list = static_cast<game::MenuList*>(game::UI_LoadMenus_LoadObj(std::string(params[1]).c_str(), 7)); 
							list)
			{
				// for each raw menufile in menulist
				for (auto raw_menu = 0; raw_menu < list->menuCount; raw_menu++)
				{
					bool replaced_with_raw = false;

					// for each loaded menufile in memory
					for (auto loaded_menus = 0; loaded_menus < game::ui_context->menuCount; loaded_menus++)
					{
						if (!utils::q_stricmp(list->menus[raw_menu]->window.name, game::ui_context->Menus[loaded_menus]->window.name))
						{
							game::ui_context->Menus[loaded_menus] = list->menus[raw_menu];

							game::Com_PrintMessage(0, utils::va("Menu <%s> is already loaded, overwriting\n", list->menus[raw_menu]->window.name), 0);
							replaced_with_raw = true;

							break;
						}
					}

					if (!replaced_with_raw)
					{
						game::ui_context->Menus[game::ui_context->menuCount] = list->menus[raw_menu];
						game::ui_context->menuCount++;

						game::Com_PrintMessage(0, utils::va("Added menu <%s> to the menu list.\n", list->menus[raw_menu]->window.name), 0);
					}
				}
			}
			else
			{
				game::Com_PrintMessage(0, "You either forgot to restart the map after enabling <fs_usedevdir> or tried to load a non-existing file!\n", 0);
			}
		});


		// open / re-open the specified menu from uicontext->menus
		command::add("menu_open", "<menu_name>", "open / re-open the specified menu from uicontext->menus", [](command::params params)
		{
			if (params.length() < 2) 
			{
				game::Com_PrintMessage(0, "Usage :: menu_open <menu_name>\n", 0);
				return;
			}

			if (!game::ui_context || !game::clientUI)
			{
				game::Com_PrintMessage(0, "uiContext | clientUI was null\n", 0);
				return;
			}

			const char * name = params[1];
			game::UiContext *ui = &game::ui_context[0];
			game::clientUI->displayHUDWithKeycatchUI = true;

			game::Menus_CloseByName(name, ui);
			game::Menus_OpenByName(name, ui);
		});


		command::add("menu_open_ingame", "<menu_name>", "wip", [](command::params params)
		{
			if (const auto& cl_ingame = game::Dvar_FindVar("cl_ingame"); cl_ingame)
			{
				if (cl_ingame->current.enabled && game::ui_cg_dc)
				{
					const char* name = params[1];
					game::UiContext* ui = &game::ui_cg_dc[0];

					game::Key_SetCatcher();
					//game::Menus_CloseAll(ui); // why was this here
					game::Menus_OpenByName(name, ui);
				}
				else
				{
					game::Com_PrintMessage(0, "^1Not in-game\n", 0);
				}
			}

			/*if (params.length() < 2)
			{
				game::Com_PrintMessage(0, "Usage :: menu_open_ingame <menu_name>\n", 0);
				return;
			}


			if (!game::ui_context)
			{
				game::Com_PrintMessage(0, "uiContext was null\n", 0);
				return;
			}

			const char* name = params[1];
			game::UiContext* ui = &game::ui_context[0];

			game::Key_SetCatcher();
			game::Menus_CloseAll(ui);
			game::Menus_OpenByName(name, ui);*/
		});


		// close the specified menu
		command::add("menu_closebyname", "<menu_name>", "close the specified menu", [](command::params params)
		{
			if (params.length() < 2)
			{
				game::Com_PrintMessage(0, "Usage :: menu_closebyname <menu_name>\n", 0);
				return;
			}

			if (!game::ui_context)
			{
				game::Com_PrintMessage(0, "uiContext was null\n", 0);
				return;
			}

			const char* name = params[1];
			game::Menus_CloseByName(name, game::ui_context);


			if (const auto& cl_ingame = game::Dvar_FindVar("cl_ingame"); cl_ingame)
			{
				if (cl_ingame->current.enabled && game::ui_cg_dc)
				{
					game::Menus_CloseByName(name, game::ui_cg_dc);
				}
			}
		});


		command::add("iw3xo_github", "", "opens https://github.com/xoxor4d/iw3xo-dev", [](command::params)
		{
			ShellExecute(0, 0, L"https://github.com/xoxor4d/iw3xo-dev/", 0, 0, SW_SHOW);
		});

		command::add("iw3xo_radiant_github", "", "opens https://github.com/xoxor4d/iw3xo-radiant", [](command::params)
		{
			ShellExecute(0, 0, L"https://github.com/xoxor4d/iw3xo-radiant/", 0, 0, SW_SHOW);
		});

		command::add("help", "", "opens https://xoxor4d.github.io/projects/iw3xo/#in-depth", [](command::params)
		{
			ShellExecute(0, 0, L"https://xoxor4d.github.io/projects/iw3xo/#in-depth", 0, 0, SW_SHOW);
		});


		// *
		// dvars

		dvars::ui_button_highlight_radius = game::Dvar_RegisterFloat(
			/* name		*/ "ui_button_highlight_radius",
			/* desc		*/ "highlight radius (scale)",
			/* default	*/ 0.85f,
			/* minVal	*/ 0.0001f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::ui_button_highlight_brightness = game::Dvar_RegisterFloat(
			/* name		*/ "ui_button_highlight_brightness",
			/* desc		*/ "highlight brightness (lerp)",
			/* default	*/ 0.5f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 2.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::ui_button_outline_radius = game::Dvar_RegisterFloat(
			/* name		*/ "ui_button_outline_radius",
			/* desc		*/ "outline radius (scale)",
			/* default	*/ 0.95f,
			/* minVal	*/ 0.0001f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::ui_button_outline_brightness = game::Dvar_RegisterFloat(
			/* name		*/ "ui_button_outline_brightness",
			/* desc		*/ "outline brightness (lerp)",
			/* default	*/ 0.7f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 2.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::ui_eyes_position = game::Dvar_RegisterVec2(
			/* name		*/ "ui_eyes_position",
			/* desc		*/ "position of main menu eyes",
			/* default	*/ 0.70f, 0.026f,
			/* minVal	*/ -2.0f,
			/* maxVal	*/ 2.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::ui_eyes_size = game::Dvar_RegisterFloat(
			/* name		*/ "ui_eyes_size",
			/* desc		*/ "size of main menu eyes",
			/* default	*/ 0.0041f,
			/* minVal	*/ 0.0001f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::ui_eyes_alpha = game::Dvar_RegisterFloat(
			/* name		*/ "ui_eyes_alpha",
			/* desc		*/ "alpha of main menu eyes",
			/* default	*/ 0.5f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

#if DEBUG
		dvars::xo_menu_dbg = game::Dvar_RegisterFloat(
			/* name		*/ "xo_menu_dbg",
			/* desc		*/ "dvar to debug menu shaders",
			/* default	*/ 1.0f,
			/* minVal	*/ -10000.0f,
			/* maxVal	*/ 10000.0f,
			/* flags	*/ game::dvar_flags::none);
#endif

		dvars::ui_subnavbar = game::Dvar_RegisterBool(
			/* name		*/ "ui_subnavbar",
			/* desc		*/ "menu helper",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::read_only);

		dvars::ui_subnavbar_item = game::Dvar_RegisterInt(
			/* name		*/ "ui_subnavbar_item",
			/* desc		*/ "menu helper",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 100,
			/* flags	*/ game::dvar_flags::read_only);

		dvars::ui_renderWidth = game::Dvar_RegisterInt(
			/* name		*/ "ui_renderWidth",
			/* desc		*/ "menu helper",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 7680,
			/* flags	*/ game::dvar_flags::read_only);

		dvars::ui_renderHeight = game::Dvar_RegisterInt(
			/* name		*/ "ui_renderHeight",
			/* desc		*/ "menu helper",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 4320,
			/* flags	*/ game::dvar_flags::read_only);

		dvars::ui_devmap = game::Dvar_RegisterBool(
			/* name		*/ "ui_devmap",
			/* desc		*/ "menu helper",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);

		dvars::ui_developer = game::Dvar_RegisterInt(
			/* name		*/ "ui_developer",
			/* desc		*/ "menu helper",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 2,
			/* flags	*/ game::dvar_flags::saved);

		dvars::ui_developer_script = game::Dvar_RegisterBool(
			/* name		*/ "ui_developer_script",
			/* desc		*/ "menu helper",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);


		// *
		// Display

		// hook R_AspectRatio to initially reset the ultrawide dvar (menu helper)
		utils::hook::nop(0x5F352E, 6);		utils::hook(0x5F352E, aspect_ratio_custom_reset_stub, HOOK_JUMP).install()->quick();

		// Set custom aspect ratio by using the default switchcase in R_AspectRatio
		utils::hook::nop(0x5F35FA, 6);		utils::hook(0x5F35FA, aspect_ratio_custom_stub, HOOK_JUMP).install()->quick();

		dvars::r_aspectRatio_custom = game::Dvar_RegisterFloat(
			/* name		*/ "r_aspectRatio_custom",
			/* desc		*/ "description",
			/* default	*/ 2.3333333f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ game::dvar_flags::saved);

		static std::vector <const char*> r_customAspectratio =
		{ 
			"auto", 
			"4:3", 
			"16:10", 
			"16:9", 
			"custom",
		};

		dvars::r_aspectRatio = game::Dvar_RegisterEnum(
			/* name		*/ "r_aspectRatio",
			/* desc		*/ "Screen aspect ratio. Use \"custom\" and \"r_aspectRatio_custom\" if your aspect ratio is not natively supported! (21/9 = 2.3333)",
			/* default	*/ 0,
			/* enumSize	*/ r_customAspectratio.size(),
			/* enumData */ r_customAspectratio.data(),
			/* flags	*/ game::dvar_flags::saved);

		dvars::ui_ultrawide = game::Dvar_RegisterBool(
			/* name		*/ "ui_ultrawide",
			/* desc		*/ "menu helper",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::read_only);
	}
}