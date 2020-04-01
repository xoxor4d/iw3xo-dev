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
        call Main::Initialize

        mov eax, 67493Ch
        jmp eax
    }
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
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		Main::Uninitialize();
	}

	FreeConsole();

	return TRUE;
}
