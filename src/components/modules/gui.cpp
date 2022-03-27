#include "std_include.hpp"

#define IMGUI_REGISTERMENU(menu, function)			\
    if (menu.menustate) {							\
		function;									\
		menu.was_open = true;						\
	}												\
	else if(menu.was_open) {						\
		if (!GET_GGUI.any_menus_open) {				\
			gui::reset_mouse();						\
		}											\
		gui::save_settings();						\
		menu.was_open = false;						\
	}

// -------------------------------------------------------------------

// show tooltip after x seconds
#define TTDELAY 1.0f 

// tooltip with delay
#define TT(tooltip) if (ImGui::IsItemHoveredDelay(TTDELAY)) { ImGui::SetTooltip(tooltip); }

// spacing dummy
#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 

// seperator with spacing
#define SEPERATORV(spacing) SPACING(0.0f, spacing); ImGui::Separator(); SPACING(0.0f, spacing); 

// execute a single command
#define CMDEXEC(command) game::Cmd_ExecuteSingleCommand(0, 0, command)

namespace components
{
	struct my_markdown : imgui_md
	{
		// fade-in fix
		void line(ImColor c, bool under) override
		{
			ImVec2 mi = ImGui::GetItemRectMin();
			ImVec2 ma = ImGui::GetItemRectMax();

			if (!under) 
			{
				ma.y -= ImGui::GetFontSize() / 2;
			}

			mi.y = ma.y;

			if (!game::glob::mainmenu_fade_done)
			{
				const ImGuiStyle& s = ImGui::GetStyle();

				if (s.Alpha < c.Value.w)
				{
					c.Value.w = s.Alpha;
				}
			}
			else
			{
				m_mainmenu_fade_done = true;
			}

			ImGui::GetWindowDrawList()->AddLine(mi, ma, c, 1.0f);
		}


		ImFont* get_font() const override
		{
			ImGuiIO& io = ImGui::GetIO();

			if (m_is_table_header) 
			{
				return io.Fonts->Fonts[FONTS::BOLD];
			}

			switch (m_hlevel)
			{
				case 0:
					return m_is_strong ? io.Fonts->Fonts[FONTS::BOLD] : io.Fonts->Fonts[FONTS::REGULAR];
				case 1:
					return io.Fonts->Fonts[FONTS::BOLD_LARGER];
				case 2:
					return io.Fonts->Fonts[FONTS::BOLD_LARGE];
				default:
					return io.Fonts->Fonts[FONTS::BOLD];
			}
		};


		void open_url() const override
		{
			ShellExecuteA(0, "open", m_href.c_str(), 0, 0, SW_SHOW);
		}


		bool get_image(image_info& nfo) const override
		{
			nfo.texture_id = nullptr;
			nfo.size = { 40,20 };
			nfo.uv0 = { 0,0 };
			nfo.uv1 = { 1,1 };
			nfo.col_tint = { 1,1,1,1 };
			nfo.col_border = { 0,0,0,0 };

			if (game::glob::loaded_main_menu && m_href != ""s)
			{
				if (const auto	material = game::Material_RegisterHandle(m_href.c_str(), 3); 
								material)
				{
					nfo.texture_id = material->textureTable->u.image->texture.data;
					nfo.size = ImVec2(material->textureTable->u.image->width, material->textureTable->u.image->height);
				}
			}

			return true;
		}


		void html_div(const std::string& dclass, bool e) override
		{
			if (dclass == "red") 
			{
				if (e) 
				{
					m_table_border = false;
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
				}
				else 
				{
					ImGui::PopStyleColor();
					m_table_border = true;
				}
			}
		}
	};

	//call this function to render your markdown
	void gui::markdown(const char* str, const char* str_end)
	{
		static my_markdown s_printer;
		s_printer.print(str, str_end);
	}

	void gui::redraw_cursor()
	{
		float cur_w = (32.0f * game::scrPlace->scaleVirtualToReal[0]) / game::scrPlace->scaleVirtualToFull[0];
		float cur_h = (32.0f * game::scrPlace->scaleVirtualToReal[1]) / game::scrPlace->scaleVirtualToFull[1];
		float cur_x = game::ui_context->cursor.x - 0.5f * cur_w;
		float cur_y = game::ui_context->cursor.y - 0.5f * cur_h;

		if (cur_w < 0.0f)
		{
			cur_w = -cur_w;
		}

		if (cur_h < 0.0f)
		{
			cur_h = -cur_h;
		}

		_ui::scrplace_apply_rect(&cur_x, &cur_w, &cur_y, &cur_h, HORIZONTAL_ALIGN_FULLSCREEN, VERTICAL_ALIGN_FULLSCREEN);

		if (game::glob::loaded_main_menu)
		{
			if (const auto	material = game::Material_RegisterHandle("ui_cursor", 3); 
							material)
			{
				const float cur_size = 54.0f;
				const float offs_x = 0.0f;
				const float offs_y = cur_size;

				const ImTextureID image = material->textureTable->u.image->texture.data;
				
				ImGui::GetWindowDrawList()->AddImageQuad(
					image,
					ImVec2(cur_x + offs_x,			cur_y + offs_y),
					ImVec2(cur_x + cur_size + offs_x,	cur_y + offs_y),
					ImVec2(cur_x + cur_size + offs_x,	cur_y - cur_size + offs_y),
					ImVec2(cur_x + offs_x,			cur_y - cur_size + offs_y),
					ImVec2(0.0f, 1.0f),
					ImVec2(1.0f, 1.0f),
					ImVec2(1.0f, 0.0f),
					ImVec2(0.0f, 0.0f)
				);
			}
		}
	}


	// *
	// initialize imgui
	void gui::imgui_init()
	{
		IDirect3DDevice9* device = *game::dx9_device_ptr;

		if (components::active.d3d9ex)
		{
			// get the device from D3D9Ex::_D3D9/Ex::CreateDevice
			device = game::glob::d3d9_device;
		}

		ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

		// *
		// init fonts (see Fonts.cpp)

		// H1 Bold Larger
		io.Fonts->AddFontFromMemoryCompressedTTF(imgui_fonts::opensans_bold_compressed_data, imgui_fonts::opensans_bold_compressed_size, 32.0f);
		// H2 Bold Large
		io.Fonts->AddFontFromMemoryCompressedTTF(imgui_fonts::opensans_bold_compressed_data, imgui_fonts::opensans_bold_compressed_size, 28.0f);
		// H1 Bold
		io.Fonts->AddFontFromMemoryCompressedTTF(imgui_fonts::opensans_bold_compressed_data, imgui_fonts::opensans_bold_compressed_size, 24.0f);

		// Regular Large
		io.Fonts->AddFontFromMemoryCompressedTTF(imgui_fonts::opensans_regular_compressed_data, imgui_fonts::opensans_regular_compressed_size, 24.0f);
		// Regular
        io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(imgui_fonts::opensans_regular_compressed_data, imgui_fonts::opensans_regular_compressed_size, 18.0f);

		ImGui_ImplWin32_Init(window::GetWindow());
		ImGui_ImplDX9_Init(device);

		// Style
		ImGui::StyleColorsDevgui();
		GGUI_READY = true;
	}

	// *
	// main rendering loop (D3D9Ex::D3D9Device::EndScene())
	void gui::render_loop()
	{
		auto& ggui = GET_GGUI;

		if (!ggui.dvars_initialized) 
		{
			return;
		}

		if (!ggui.imgui_initialized) 
		{
			gui::imgui_init();
		}

		gui::begin_frame();
		gui::any_open_menus();

		// ------------

		IMGUI_REGISTERMENU(ggui.menus[game::GUI_MENUS::DEMO], ImGui::ShowDemoWindow(&ggui.menus[game::GUI_MENUS::DEMO].menustate));
		IMGUI_REGISTERMENU(ggui.menus[game::GUI_MENUS::DEVGUI], gui_devgui::create_devgui(ggui.menus[game::GUI_MENUS::DEVGUI]));
		IMGUI_REGISTERMENU(ggui.menus[game::GUI_MENUS::CHANGELOG], _ui::create_changelog(ggui.menus[game::GUI_MENUS::CHANGELOG]));

		// ------------

		gui::end_frame();
	}

	// *
	// shutdown imgui when game window resets (window::create_main_window())
	void gui::reset()
	{
		if (GGUI_READY)
		{
			ImGui_ImplDX9_Shutdown();
			ImGui_ImplWin32_Shutdown();

			gui::reset_mouse();
			memset(&game::glob::gui, 0, sizeof(game::gui_t));
			GGUI_READY_DVARS = true;
		}
	}

	// *
	// toggle a imgui menu by command (or key (scheduler))
	void gui::toggle(game::gui_menus_t &menu, int keycatcher, bool on_command = false)
	{
		if (!GGUI_READY) 
		{
			return;
		}

		if (game::playerKeys->keys[keycatcher].down) 
		{
			menu.hk_is_clicked = false;
			menu.hk_is_down = true;
		}
		else if (!game::playerKeys->keys[keycatcher].down && menu.hk_is_down) 
		{
			menu.hk_is_clicked = true;
			menu.hk_is_down = false;
		}
		else 
		{
			menu.hk_is_clicked = false;
			menu.hk_is_down = false;
		}

		// toggle menu by key or command
		if (menu.hk_is_clicked || on_command)
		{
			ImGuiIO& io = ImGui::GetIO();
			std::fill_n(io.KeysDown, 512, 0); // fix keys getting stuck on close / vid_restart

			menu.menustate = !menu.menustate;

			// on close
			if (!menu.menustate)
			{
				// check if there is still some open menu
				if (GGUI_ANY_MENUS_OPEN)
				{
					// do nothing with the mouse
					return;
				}
			}

			// toggle the mouse
			gui::toggle_mouse(menu.menustate);
		}
	}

	// *
	// toggle the mouse
	void gui::toggle_mouse(bool state)
	{
		if (const auto& in_mouse = game::Dvar_FindVar("in_mouse"); 
						in_mouse)
		{
			game::dvar_set_value_dirty(in_mouse, state ? 0 : 1);

			utils::function<void()>(0x575E90)(); // In_Shutdown
			game::s_wmv->mouseInitialized = in_mouse->current.enabled;
		}
	}

	// *
	// reset the mouse
	void gui::reset_mouse()
	{
		if (const auto& in_mouse = game::Dvar_FindVar("in_mouse");
						in_mouse)
		{
			game::dvar_set_value_dirty(in_mouse, true);
			game::s_wmv->mouseInitialized = in_mouse->current.enabled;
		}
	}

	// *
	// set menu layout (origin / size / anker)
	void gui::set_menu_layout(game::gui_menus_t& menu, const float x, const float y, const float width, const float height, const int horz_align = VERTICAL_APPLY_NONE, const int vert_align = HORIZONTAL_APPLY_NONE)
	{
		menu.position[0] = x;
		menu.position[1] = y;

		menu.size[0] = width;
		menu.size[1] = height;

		menu.horzAlign = horz_align;
		menu.vertAlign = vert_align;
	}

	// *
	void gui::begin_frame()
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	// *
	void gui::end_frame()
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	// *
	// called on init from commands::force_dvars_on_init()
	void gui::load_settings()
	{
		// pretty useless but might be needed later
		GGUI_READY_DVARS = true;
	}

	// *
	// save dvars to disk (only modified dvars are saved)
	void gui::save_settings()
	{
		// trigger config writing
		game::Cmd_ExecuteSingleCommand(0, 0, utils::va("set _imgui_saving %d", !dvars::_imgui_saving->current.enabled));
	}

	// *
	// check if there are any open menus
	bool gui::any_open_menus()
	{
		auto& ggui = GET_GGUI;
		const auto& cl_ingame = game::Dvar_FindVar("cl_ingame");

		for (int m = 0; m < GGUI_MENU_COUNT; m++)
		{
			if (ggui.menus[m].menustate)
			{
				// positive flag and ingame
				if (!ggui.any_menus_open && cl_ingame && cl_ingame->current.enabled)
				{
					// activate a dummy menu to block game input
					CMDEXEC("menu_open_ingame pregame_loaderror_mp");
				}

				if (ggui.menus[m].mouse_ignores_menustate)
				{
					ggui.any_menus_open = false;
					return false;
				}

				ggui.any_menus_open = true;
				return true;
			}
		}

		// negative flag
		if (ggui.any_menus_open)
		{
			// close the dummy menu
			if (cl_ingame && cl_ingame->current.enabled)
			{
				CMDEXEC("menu_closebyname pregame_loaderror_mp");
			}

			ggui.any_menus_open = false;
		}

		
		return false;
	}

	// *
	// not using a macro so one can see the structure while debugging
	game::gui_menus_t& gui::get_menu(game::GUI_MENUS id)
	{
		return game::glob::gui.menus[id];
	}

	// *
	// 
	gui::gui()
	{
#if DEBUG
		// check hotkeys every frame
		scheduler::on_frame([this]()
		{
			gui::toggle(GET_GGUI.menus[game::GUI_MENUS::DEMO], KEYCATCHER_HOME);
			gui::toggle(GET_GGUI.menus[game::GUI_MENUS::DEVGUI], KEYCATCHER_END);
			gui::toggle(GET_GGUI.menus[game::GUI_MENUS::CHANGELOG], KEYCATCHER_INS);

		}, scheduler::thread::main);
#endif

		// *
		// Commands

		command::add("devgui_demo", "", "opens the imgui demo menu", [this](command::params)
		{
			gui::toggle(GET_GGUI.menus[game::GUI_MENUS::DEMO], 0, true);
		});

		command::add("devgui", "", "opens the devgui", [this](command::params)
		{
			gui::toggle(GET_GGUI.menus[game::GUI_MENUS::DEVGUI], 0, true);
		});

		// *
		// dvars

		dvars::_imgui_saving = game::Dvar_RegisterBool(
			/* name		*/ "_imgui_saving",
			/* desc		*/ "saving flag",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);

		dvars::_imgui_window_alpha = game::Dvar_RegisterFloat(
			/* name		*/ "_imgui_window_alpha",
			/* desc		*/ "imgui window alpha",
			/* default	*/ 0.8f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);
	}
}