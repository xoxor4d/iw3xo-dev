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
		if (Dvars::r_noborder && Dvars::r_noborder->current.enabled)
		{
			__asm
			{
				mov ebp, WS_VISIBLE | WS_POPUP
				retn
			}
		}
		else
		{
			__asm
			{
				mov ebp, WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX
				retn
			}
		}
	}

	int WINAPI Window::ShowCursorHook(BOOL show)
	{
		return ShowCursor(show);
	}

	HWND WINAPI Window::CreateMainWindow(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		Window::MainWindow = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		
		if (Components::active.Gui)
		{
			Gui::reset();
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

		if (Components::active.Gui && GGUI_READY)
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

		return Utils::Hook::Call<BOOL(__stdcall)(HWND, UINT, WPARAM, LPARAM)>(0x57BB20)(hWnd, Msg, wParam, lParam);
	}

	bool Window::is_noborder()
	{
		if (Dvars::r_noborder && Dvars::r_noborder->current.enabled)
			return true;

		return false;
	}

	__declspec(naked) void vid_xypos_stub()
	{
		const static uint32_t retnPt = 0x5F4C50;

		__asm
		{
			mov		[esi + 10h], eax	// overwritten op (wndParms->y)
			mov		dword ptr[esi], 0	// overwritten op

			pushad
			Call	Window::is_noborder
			test	al, al
			jnz		NO_BORDER

			popad
			jmp	retnPt


			NO_BORDER:
			popad				
			xor		eax, eax			// clear eax
			mov		[esi + 0Ch], eax	// set wndParms->x to 0 (4 byte)
			mov		[esi + 10h], eax	// set wndParms->y to 0 (4 byte)
			jmp		retnPt
		}
	}

	Window::Window()
	{
		Dvars::r_noborder = Game::Dvar_RegisterBool(
			/* name		*/ "r_noborder",
			/* desc		*/ "Do not use a border in windowed mode",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		// Main window border
		Utils::Hook(0x5F4963, Window::StyleHookStub, HOOK_CALL).install()->quick();

		// Main window creation
		Utils::Hook::Nop(0x5F49CA, 6);
		Utils::Hook(0x5F49CA, Window::CreateMainWindow, HOOK_CALL).install()->quick();

		// Don't let the game interact with the native cursor
		Utils::Hook::Set(0x69128C, Window::ShowCursorHook);

		// Use custom message handler
		Utils::Hook::Set(0x5774EE, Window::MessageHandler);

		// Do not use vid_xpos / vid_ypos when r_noborder is enabled
		Utils::Hook::Nop(0x5F4C47, 9);
		Utils::Hook(0x5F4C47, vid_xypos_stub, HOOK_JUMP).install()->quick();
	}

	Window::~Window()
	{ }
}
