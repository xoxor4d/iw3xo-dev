// https://github.com/IW4x/iw4x-client/blob/develop/src/Components/Modules/Window.cpp

#include "STDInclude.hpp"

namespace Components
{
	HWND Window::MainWindow = nullptr;

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
		return Window::MainWindow;
	}

	void Window::ApplyCursor()
	{
		SetCursor(LoadCursor(nullptr, Game::Globals::loaded_MainMenu ? IDC_APPSTARTING : IDC_ARROW));
	}

	BOOL WINAPI Window::MessageHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		if (Msg == WM_SETCURSOR)
		{
			Window::ApplyCursor();
			return TRUE;
		}

		return Utils::Hook::Call<BOOL(__stdcall)(HWND, UINT, WPARAM, LPARAM)>(0x57BB20)(hWnd, Msg, wParam, lParam);
	}

#pragma warning(disable:4740)
	__declspec(naked) void vid_xypos_stub()
	{
		const static uint32_t retnPt = 0x5F4C50;

		__asm mov	[esi + 10h], eax	// overwritten op (wndParms->y)
		__asm mov	dword ptr[esi], 0	// overwritten op

		__asm pushad
		if (Dvars::r_noborder && Dvars::r_noborder->current.enabled) 
		{
			__asm
			{
				popad
				mov		[esi + 0Ch], 0	// wndParms->x
				mov		[esi + 10h], 0	// wndParms->y

				jmp		retnPt
			}
		}
		__asm popad

		__asm jmp	retnPt
	}
#pragma warning(default:4740)

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
