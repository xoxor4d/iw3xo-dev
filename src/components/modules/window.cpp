// https://github.com/IW4x/iw4x-client/blob/develop/src/Components/Modules/Window.cpp

#include "std_include.hpp"
IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace components
{
	HWND window::main_window_ = nullptr;

	int window::width()
	{
		return window::width(window::main_window_);
	}

	int window::height()
	{
		return window::height(window::main_window_);
	}

	int window::width(HWND window)
	{
		RECT rect;
		window::dimension(window, &rect);
		return (rect.right - rect.left);
	}

	int window::height(HWND window)
	{
		RECT rect;
		window::dimension(window, &rect);
		return (rect.bottom - rect.top);
	}

	void window::dimension(RECT* rect)
	{
		window::dimension(window::main_window_, rect);
	}

	void window::dimension(HWND window, RECT* rect)
	{
		if (rect)
		{
			ZeroMemory(rect, sizeof(RECT));

			if (window && IsWindow(window))
			{
				GetWindowRect(window, rect);
			}
		}
	}

	HWND window::GetWindow()
	{
		return window::main_window_;
	}

	__declspec(naked) void window::style_hook_stub()
	{
		const static uint32_t retn_pt = 0x5F4968;
		__asm
		{
			push	eax;
			mov		eax, dvars::r_noborder;
			cmp		byte ptr[eax + 12], 1;
			pop		eax;

			// jump if noborder is false
			jne		STOCK;

			// if (dvars::r_noborder->current.enabled)
			mov		ebp, WS_VISIBLE | WS_POPUP;
			jmp		retn_pt;

		STOCK:
			mov		ebp, WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
			jmp		retn_pt;
		}
	}

	int WINAPI window::show_cursor_hk(BOOL show)
	{
		return ShowCursor(show);
	}

	HWND WINAPI window::create_main_window(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		window::main_window_ = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		
		if (components::active.gui)
		{
			gui::reset();
		}

		return window::main_window_;
	}

	void window::apply_cursor()
	{
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
	}

	BOOL WINAPI window::MessageHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		auto menu_open = false;

		if (components::active.gui && GGUI_READY)
		{
			// handle input and mouse cursor for open menus
			for (auto menu = 0; menu < GGUI_MENU_COUNT; menu++)
			{
				if (game::glob::gui.menus[menu].menustate)
				{
					if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
					{
						ImGui::GetIO().MouseDrawCursor = true;
						return true;
					}

					menu_open = true;
				}
			}

			ImGui::GetIO().MouseDrawCursor = false;
		}

		// draw cursor (if no imgui menu opened) when cod4 inactive and hovering over it
		if(!menu_open)
		{
			if (Msg == WM_SETCURSOR)
			{
				window::apply_cursor();
				return TRUE;
			}
		}

		// MainWndProc
		return utils::hook::call<BOOL(__stdcall)(HWND, UINT, WPARAM, LPARAM)>(0x57BB20)(hWnd, Msg, wParam, lParam);
	}

	bool window::is_noborder()
	{
		if (dvars::r_noborder && dvars::r_noborder->current.enabled)
		{
			return true;
		}

		return false;
	}

	__declspec(naked) void vid_xypos_stub()
	{
		const static uint32_t retn_addr = 0x5F4C50;
		__asm
		{
			mov		[esi + 10h], eax;	// overwritten op (wndParms->y)
			mov		dword ptr[esi], 0;	// overwritten op

			pushad;
			call	window::is_noborder;
			test	al, al;
			jnz		NO_BORDER;

			popad;
			jmp		retn_addr;


		NO_BORDER:
			popad;
			xor		eax, eax;			// clear eax
			mov		[esi + 0Ch], eax;	// set wndParms->x to 0 (4 byte)
			mov		[esi + 10h], eax;	// set wndParms->y to 0 (4 byte)
			jmp		retn_addr;
		}
	}

	window::window()
	{
		dvars::r_noborder = game::Dvar_RegisterBool(
			/* name		*/ "r_noborder",
			/* desc		*/ "Do not use a border in windowed mode",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);


		// Main window border
		utils::hook(0x5F4963, window::style_hook_stub, HOOK_JUMP).install()->quick();

		// Main window creation
		utils::hook::nop(0x5F49CA, 6);
			 utils::hook(0x5F49CA, window::create_main_window, HOOK_CALL).install()->quick();

		// Don't let the game interact with the native cursor
		utils::hook::set(0x69128C, window::show_cursor_hk);

		// Use custom message handler
		utils::hook::set(0x5774EE, window::MessageHandler);

		// Do not use vid_xpos / vid_ypos when r_noborder is enabled
		utils::hook::nop(0x5F4C47, 9);
			 utils::hook(0x5F4C47, vid_xypos_stub, HOOK_JUMP).install()->quick();
	}
}
