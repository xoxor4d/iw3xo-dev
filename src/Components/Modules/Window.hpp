#pragma once

namespace Components
{
	class Window : public Component
	{
	public:
		Window();
		~Window();

		const char* getName() override { return "Window"; };
		
		static int Width();
		static int Height();
		static int Width(HWND window);
		static int Height(HWND window);
		static void Dimension(RECT* rect);
		static void Dimension(HWND window, RECT* rect);
		static HWND GetWindow();
		static bool is_noborder();

	private:
		static BOOL CursorVisible;
		static HWND MainWindow;

		static void ApplyCursor();

		static BOOL WINAPI MessageHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
		static int WINAPI ShowCursorHook(BOOL show);

		static void StyleHookStub();
		static HWND WINAPI CreateMainWindow(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	};
}
