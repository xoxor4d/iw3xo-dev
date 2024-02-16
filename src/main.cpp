#include "std_include.hpp"

namespace main
{
	static utils::hook entry_point_hook_;

	void initialize()
	{
		main::entry_point_hook_.uninstall();
		components::loader::initialize_();
	}

	void uninitialize()
	{
		components::loader::uninitialize_();
	}
}

__declspec(naked) void entry_point()
{
    __asm
    {
        // this has to be called, otherwise the hook is not uninstalled and we're deadlocking
		call	main::initialize;
		mov		eax, 0x67493C;
		jmp		eax;
    }
}

void load_addon_libaries()
{
	if (LoadLibraryA("iw3xo\\bin\\reshade.dll"))
	{
		game::glob::loaded_libaries.append("iw3xo\\bin\\reshade.dll\n");
	}

	// delay-load
	components::scheduler::once([]()
	{
		if (dvars::load_iw3mvm && dvars::load_iw3mvm->current.enabled)
		{
			if (LoadLibraryA("iw3xo\\bin\\iw3mvm.dll"))
			{
				game::glob::loaded_libaries.append("iw3xo\\bin\\iw3mvm.dll\n");
			}
		}
		
		if (!game::glob::loaded_libaries.empty())
		{
			game::Com_PrintMessage(0, utils::va("\n-------------- Loaded Libaries -------------- \n%s\n", game::glob::loaded_libaries.c_str()), 0);
		}
	});
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD  ul_reason_for_call, LPVOID /*lpReserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DWORD oldProtect;
		VirtualProtect(GetModuleHandle(nullptr), 0xD536000, PAGE_EXECUTE_READWRITE, &oldProtect);

		main::entry_point_hook_.initialize(0x67493C, entry_point)->install();
		FreeConsole();

		// load additional libaries from '\iw3xo\bin\'
		load_addon_libaries();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		main::uninitialize();
	}

	return TRUE;
}
