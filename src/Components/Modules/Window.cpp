// https://github.com/IW4x/iw4x-client/blob/develop/src/Components/Modules/Window.cpp

#include "STDInclude.hpp"
IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Components
{
	HWND Window::MainWindow = nullptr;

	int Window::Width()
	{
		return Window::Width(Window::MainWindow);
	}

	int Window::Height()
	{
		return Window::Height(Window::MainWindow);
	}

	int Window::Width(HWND window)
	{
		RECT rect;
		Window::Dimension(window, &rect);
		return (rect.right - rect.left);
	}

	int Window::Height(HWND window)
	{
		RECT rect;
		Window::Dimension(window, &rect);
		return (rect.bottom - rect.top);
	}

	void Window::Dimension(RECT* rect)
	{
		Window::Dimension(Window::MainWindow, rect);
	}

	void Window::Dimension(HWND window, RECT* rect)
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

	HWND Window::GetWindow()
	{
		return Window::MainWindow;
	}

	__declspec(naked) void Window::StyleHookStub()
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
			// else
			mov		ebp, WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
			jmp		retn_pt;
		}
	}

	int WINAPI Window::ShowCursorHook(BOOL show)
	{
		return ShowCursor(show);
	}

	HWND WINAPI Window::CreateMainWindow(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		Window::MainWindow = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		
		if (Components::active.gui)
		{
			gui::reset();
		}

		return Window::MainWindow;
	}

	void Window::ApplyCursor()
	{
		//SetCursor(LoadCursor(nullptr, Game::Globals::loaded_MainMenu ? IDC_ARROW : IDC_APPSTARTING));
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
	}

	BOOL WINAPI Window::MessageHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		auto menu_open = false;

		if (Components::active.gui && GGUI_READY)
		{
			// handle input and mouse cursor for open menus
			for (auto menu = 0; menu < GGUI_MENU_COUNT; menu++)
			{
				if (Game::Globals::gui.menus[menu].menustate)
				{
					if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
					{
						ImGui::GetIO().MouseDrawCursor = 1;
						return true;
					}

					menu_open = true;
				}
			}

			ImGui::GetIO().MouseDrawCursor = 0;
		}

		// draw cursor (if no imgui menu opened) when cod4 inactive and hovering over it
		if(!menu_open)
		{
			if (Msg == WM_SETCURSOR)
			{
				Window::ApplyCursor();
				return TRUE;
			}
		}

		// MainWndProc
		return utils::hook::Call<BOOL(__stdcall)(HWND, UINT, WPARAM, LPARAM)>(0x57BB20)(hWnd, Msg, wParam, lParam);
	}

	bool Window::is_noborder()
	{
		if (dvars::r_noborder && dvars::r_noborder->current.enabled)
		{
			return true;
		}

		return false;
	}

	__declspec(naked) void vid_xypos_stub()
	{
		const static uint32_t retnPt = 0x5F4C50;
		__asm
		{
			mov		[esi + 10h], eax;	// overwritten op (wndParms->y)
			mov		dword ptr[esi], 0;	// overwritten op

			pushad;
			call	Window::is_noborder;
			test	al, al;
			jnz		NO_BORDER;

			popad;
			jmp		retnPt;


		NO_BORDER:
			popad;
			xor		eax, eax;			// clear eax
			mov		[esi + 0Ch], eax;	// set wndParms->x to 0 (4 byte)
			mov		[esi + 10h], eax;	// set wndParms->y to 0 (4 byte)
			jmp		retnPt;
		}
	}

	Window::Window()
	{
		dvars::r_noborder = Game::Dvar_RegisterBool(
			/* name		*/ "r_noborder",
			/* desc		*/ "Do not use a border in windowed mode",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		// Main window border
		utils::hook(0x5F4963, Window::StyleHookStub, HOOK_JUMP).install()->quick(); // was HOOK_CALL :>

		// Main window creation
		utils::hook::nop(0x5F49CA, 6);		utils::hook(0x5F49CA, Window::CreateMainWindow, HOOK_CALL).install()->quick();

		// Don't let the game interact with the native cursor
		utils::hook::set(0x69128C, Window::ShowCursorHook);

		// Use custom message handler
		utils::hook::set(0x5774EE, Window::MessageHandler);

		// Do not use vid_xpos / vid_ypos when r_noborder is enabled
		utils::hook::nop(0x5F4C47, 9);		utils::hook(0x5F4C47, vid_xypos_stub, HOOK_JUMP).install()->quick();
	}

	Window::~Window()
	{ }
}
