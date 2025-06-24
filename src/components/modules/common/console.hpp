#pragma once

namespace common
{
	static bool g_external_console_created = false;
    inline void console()
    {
        if (!g_external_console_created)
        {
			g_external_console_created = true;
            
            setvbuf(stdout, nullptr, _IONBF, 0);
            if (AllocConsole())
            {
                FILE* file = nullptr;
                freopen_s(&file, "CONIN$", "r", stdin);
                freopen_s(&file, "CONOUT$", "w", stdout);
                freopen_s(&file, "CONOUT$", "w", stderr);
                SetConsoleTitleA("iw3xo debug console");
            }
        }
    }
}
