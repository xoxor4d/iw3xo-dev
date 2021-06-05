#include "STDInclude.hpp"

#define IMGUI_REGISTERMENU(menu, function)			\
    if (menu.menustate) {							\
		function;									\
		menu.was_open = true;						\
	}												\
	else if(menu.was_open) {						\
		if (!GET_GGUI.any_menus_open) {				\
			Gui::reset_mouse();						\
		}											\
		Gui::save_settings();						\
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
#define CMDEXEC(command) Game::Cmd_ExecuteSingleCommand(0, 0, command)

namespace Components
{
	struct my_markdown : public imgui_md
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

			if (!Game::Globals::mainmenu_fadeDone)
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
			ShellExecuteA(NULL, "open", m_href.c_str(), NULL, NULL, SW_SHOW);
		}


		bool get_image(image_info& nfo) const override
		{
			nfo.texture_id = 0;
			nfo.size = { 40,20 };
			nfo.uv0 = { 0,0 };
			nfo.uv1 = { 1,1 };
			nfo.col_tint = { 1,1,1,1 };
			nfo.col_border = { 0,0,0,0 };

			if (Game::Globals::loaded_MainMenu && m_href != ""s)
			{
				Game::Material* material = Game::Material_RegisterHandle(m_href.c_str(), 3);
				if (material)
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
	void Gui::markdown(const char* str, const char* str_end)
	{
		static my_markdown s_printer;
		s_printer.print(str, str_end);
	}

	void Gui::redraw_cursor()
	{
		ImGuiIO& io = ImGui::GetIO();

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

		_UI::ScrPlace_ApplyRect(&cur_x, &cur_w, &cur_y, &cur_h, HORIZONTAL_ALIGN_FULLSCREEN, VERTICAL_ALIGN_FULLSCREEN);

		if (Game::Globals::loaded_MainMenu)
		{
			Game::Material* material = Game::Material_RegisterHandle("ui_cursor", 3);
			if (material)
			{
				float cur_size	= 54.0f;
				float ofs_x		= 0.0f;
				float ofs_y		= cur_size;

				ImTextureID image = material->textureTable->u.image->texture.data;
				
				ImGui::GetWindowDrawList()->AddImageQuad(
					image,
					ImVec2(cur_x +	ofs_x,				cur_y + ofs_y),
					ImVec2(cur_x + cur_size + ofs_x,	cur_y + ofs_y),
					ImVec2(cur_x + cur_size + ofs_x,	cur_y - cur_size + ofs_y),
					ImVec2(cur_x + ofs_x,				cur_y - cur_size + ofs_y),
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
	void Gui::imgui_init()
	{
		IDirect3DDevice9* device;

		if (Components::active.D3D9Ex)
		{
			// get the device from D3D9Ex::_D3D9/Ex::CreateDevice
			device = Game::Globals::d3d9_device;
		}
		else
		{
			// get the device from the game
			device = *Game::dx9_device_ptr;
		}

		ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

		// H1 Bold Larger
		io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_bold_compressed_data, fonts::opensans_bold_compressed_size, 32.0f);
		// H2 Bold Large
		io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_bold_compressed_data, fonts::opensans_bold_compressed_size, 28.0f);
		// H1 Bold
		io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_bold_compressed_data, fonts::opensans_bold_compressed_size, 24.0f);

		// Regular Large
		io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 24.0f);
		// Regular
        io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 18.0f);

		ImGui_ImplWin32_Init(Window::GetWindow());
		ImGui_ImplDX9_Init(device);

		// Style
		ImGui::StyleColorsDevgui();
		GGUI_READY = true;
	}

	// *
	// main rendering loop (D3D9Ex::D3D9Device::EndScene())
	void Gui::render_loop()
	{
		auto& gui = GET_GGUI;

		if (!gui.dvars_initialized) {
			return;
		}

		if (!gui.imgui_initialized) {
			Gui::imgui_init();
		}

		Gui::begin_frame();
		Gui::any_open_menus();

		// ------------

		IMGUI_REGISTERMENU(gui.menus[Game::GUI_MENUS::DEMO], ImGui::ShowDemoWindow(&gui.menus[Game::GUI_MENUS::DEMO].menustate));
		IMGUI_REGISTERMENU(gui.menus[Game::GUI_MENUS::DEVGUI], Gui_Devgui::create_devgui(gui.menus[Game::GUI_MENUS::DEVGUI]));
		IMGUI_REGISTERMENU(gui.menus[Game::GUI_MENUS::CHANGELOG], _UI::create_changelog(gui.menus[Game::GUI_MENUS::CHANGELOG]));

		// ------------

		Gui::end_frame();
	}

	// *
	// shutdown imgui when game window resets (Window::CreateMainWindow())
	void Gui::reset()
	{
		if (GGUI_READY)
		{
			ImGui_ImplDX9_Shutdown();
			ImGui_ImplWin32_Shutdown();

			Gui::reset_mouse();
			memset(&Game::Globals::gui, 0, sizeof(Game::gui_t));
			GGUI_READY_DVARS = true;
		}
	}

	// *
	// toggle a imgui menu by command (or key (scheduler))
	void Gui::toggle(Game::gui_menus_t &menu, int keycatcher, bool onCommand = false)
	{
		if (!GGUI_READY) {
			return;
		}

		if (Game::playerKeys->keys[keycatcher].down) {
			menu.hk_is_clicked = false; menu.hk_is_down = true;
		}
		else if (!Game::playerKeys->keys[keycatcher].down && menu.hk_is_down) {
			menu.hk_is_clicked = true; menu.hk_is_down = false;
		}
		else {
			menu.hk_is_clicked = false; menu.hk_is_down = false;
		}

		// toggle menu by key or command
		if (menu.hk_is_clicked || onCommand)
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
			Gui::toggle_mouse(menu.menustate);
		}
	}

	// *
	// toggle the mouse
	void Gui::toggle_mouse(bool state)
	{
		static auto in_mouse = Game::Dvar_FindVar("in_mouse");

		if (in_mouse)
		{
			Game::Dvar_SetValue(in_mouse, state ? 0 : 1);

			Utils::function<void()>(0x575E90)(); // In_Shutdown
			*Game::mouse_enabled = in_mouse->current.enabled;
		}
	}

	// *
	// reset the mouse
	void Gui::reset_mouse()
	{
		static auto in_mouse = Game::Dvar_FindVar("in_mouse");

		if (in_mouse)
		{
			Game::Dvar_SetValue(in_mouse, true);
			*Game::mouse_enabled = in_mouse->current.enabled;
		}
	}

	// *
	// set menu layout (origin / size / anker)
	void Gui::set_menu_layout(Game::gui_menus_t& menu, const float x, const float y, const float width, const float height, const int horzAlign = VERTICAL_APPLY_NONE, const int vertAlign = HORIZONTAL_APPLY_NONE)
	{
		menu.position[0] = x;
		menu.position[1] = y;

		menu.size[0] = width;
		menu.size[1] = height;

		menu.horzAlign = horzAlign;
		menu.vertAlign = vertAlign;
	}

	// *
	void Gui::begin_frame()
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	// *
	void Gui::end_frame()
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	// *
	// unused
	void Gui::update_hWnd(void* hwnd)
	{
		if (GGUI_READY)
		{
			ImGui_SethWnd(hwnd);
		}
	}

	// *
	// called on init from Commands::ForceDvarsOnInit()
	void Gui::load_settings()
	{
		// pretty useless but might be needed later
		GGUI_READY_DVARS = true;
	}

	// *
	// save dvars to disk (only modified dvars are saved)
	void Gui::save_settings()
	{
		// trigger config writing
		Game::Cmd_ExecuteSingleCommand(0, 0, Utils::VA("set _imgui_saving %d", !Dvars::_imgui_saving->current.enabled));
	}

	// *
	// check if there are any open menus
	bool Gui::any_open_menus()
	{
		auto& gui = GET_GGUI;
		const auto cl_ingame = Game::Dvar_FindVar("cl_ingame");

		for (int m = 0; m < GGUI_MENU_COUNT; m++)
		{
			if (gui.menus[m].menustate)
			{
				// positive flag and ingame
				if (!gui.any_menus_open && cl_ingame && cl_ingame->current.enabled)
				{
					// activate a dummy menu to block game input
					CMDEXEC("menu_open_ingame pregame_loaderror_mp");
				}

				if (gui.menus[m].mouse_ignores_menustate)
				{
					gui.any_menus_open = false;
					return false;
				}
				else
				{
					gui.any_menus_open = true;
					return true;
				}
			}
		}

		// negative flag
		if (gui.any_menus_open)
		{
			// close the dummy menu
			if (cl_ingame && cl_ingame->current.enabled)
			{
				CMDEXEC("menu_closebyname pregame_loaderror_mp");
			}

			gui.any_menus_open = false;
		}

		
		return false;
	}

	// *
	// not using a macro so one can see the structure while debugging
	Game::gui_menus_t& Gui::GetMenu(Game::GUI_MENUS id)
	{
		return Game::Globals::gui.menus[id];
	}

	// *
	// 
	Gui::Gui()
	{
#if DEBUG
		// check hotkeys every frame
		Scheduler::on_frame([this]()
		{
			Gui::toggle(GET_GGUI.menus[Game::GUI_MENUS::DEMO], KEYCATCHER_HOME);
			Gui::toggle(GET_GGUI.menus[Game::GUI_MENUS::DEVGUI], KEYCATCHER_END);
			Gui::toggle(GET_GGUI.menus[Game::GUI_MENUS::CHANGELOG], KEYCATCHER_F2);

		}, Scheduler::thread::main);
#endif

		// *
		// Commands

		Command::Add("devgui_demo", "", "opens the imgui demo menu", [this](Command::Params)
		{
			//if (!Dvars::r_d3d9ex->current.enabled)
			//{
			//	Game::Com_PrintMessage(0, "Please enable <r_d3d9ex> first!", 0);
			//	return;
			//}

			Gui::toggle(GET_GGUI.menus[Game::GUI_MENUS::DEMO], 0, true);
		});

		Command::Add("devgui", "", "opens the devgui", [this](Command::Params)
		{
			//if (!Dvars::r_d3d9ex->current.enabled)
			//{
			//	Game::Com_PrintMessage(0, "Please enable <r_d3d9ex> first!", 0);
			//	return;
			//}

			Gui::toggle(GET_GGUI.menus[Game::GUI_MENUS::DEVGUI], 0, true);
		});

		// *
		// Dvars

		Dvars::_imgui_saving = Game::Dvar_RegisterBool(
			/* name		*/ "_imgui_saving",
			/* desc		*/ "saving flag",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::_imgui_window_alpha = Game::Dvar_RegisterFloat(
			/* name		*/ "_imgui_window_alpha",
			/* desc		*/ "imgui window alpha",
			/* default	*/ 0.8f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);
	}

	Gui::~Gui()
	{ }
}