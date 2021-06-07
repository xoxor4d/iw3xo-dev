#include "STDInclude.hpp"

namespace Main
{
	static Utils::Hook EntryPointHook;

	void Initialize()
	{
		Main::EntryPointHook.uninstall();
		Components::Loader::Initialize();
	}

	void Uninitialize()
	{
		Components::Loader::Uninitialize();
	}
}

__declspec(naked) void EntryPoint()
{
    __asm
    {
        // This has to be called, otherwise the hook is not uninstalled and we're deadlocking
		call	Main::Initialize;

		mov		eax, 0x67493C;
		jmp		eax;
    }
}

void load_addon_libaries()
{
	if (LoadLibraryA("iw3xo\\bin\\reshade.dll"))
	{
		Game::Globals::loaded_libaries.append("iw3xo\\bin\\reshade.dll\n");
	}

	// delay-load
	Components::Scheduler::once([]()
	{
		if (Dvars::load_iw3mvm && Dvars::load_iw3mvm->current.enabled)
		{
			if (LoadLibraryA("iw3xo\\bin\\iw3mvm.dll"))
			{
				Game::Globals::loaded_libaries.append("iw3xo\\bin\\iw3mvm.dll\n");
			}
		}
		
		if (Game::Globals::loaded_libaries.size())
		{
			Game::Com_PrintMessage(0, Utils::VA("\n-------------- Loaded Libaries -------------- \n%s\n", Game::Globals::loaded_libaries.c_str()), 0);
		}
	});
}


BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD  ul_reason_for_call, LPVOID /*lpReserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// Ensure we're working with our desired binary
		if (Utils::Hook::Get<DWORD>(0x4C0FFF) != 0x013EB894)
		{
			return FALSE;
		}

		DWORD oldProtect;
		VirtualProtect(GetModuleHandle(nullptr), 0xD536000, PAGE_EXECUTE_READWRITE, &oldProtect);

		Main::EntryPointHook.initialize(0x67493C, EntryPoint)->install();
		
		FreeConsole();

		// load additional libaries from '\iw3xo\bin\'
		load_addon_libaries();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		Main::Uninitialize();
	}

	//FreeConsole();
	return TRUE;
}
