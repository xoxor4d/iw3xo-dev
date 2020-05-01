#include "STDInclude.hpp"

namespace Components
{
	// global active modules struct
	activeModules_s active = activeModules_s();

	std::vector<Component*> Loader::Components;
	Utils::Memory::Allocator Loader::MemAllocator;

	void Loader::Initialize()
	{
		Loader::MemAllocator.clear();

		// global bools for more dynamic modules (choose which modules to load)
		active._UI = true;
		active._CG = true;
		active._Pmove = true;
		active._Debug = true;
		active._Game = true;
		active._Map = true;
		active._Common = true;
		active.Command = true;
		active.GScr_Methods = true;
		active.QuickPatch = true;
		active.Scheduler = true;
		
		active.D3D9Ex = true;
		active.MenuExport = true;
		active.PM_Movement = true;
		active.XO_Console = true;
		active.RB_DrawCollision = true;
		active.RB_ShaderOverlays = true;
		active.RadiantRemote = true;
		active.Window = true;
		active.Devgui = active.Window == active.D3D9Ex; // needs the window module and d3d9ex (enabled)
		
		// General Modules that need to be loaded
		REGISTER_MODULE(_UI);
		REGISTER_MODULE(_CG);
		REGISTER_MODULE(_Pmove);
		REGISTER_MODULE(_Debug);
		REGISTER_MODULE(_Game);
		REGISTER_MODULE(_Map);
		REGISTER_MODULE(_Common);
		REGISTER_MODULE(Command);
		REGISTER_MODULE(GScr_Methods);
		REGISTER_MODULE(QuickPatch);
		REGISTER_MODULE(Scheduler);

		// Addons
		REGISTER_MODULE(D3D9Ex);
		REGISTER_MODULE(MenuExport);
		REGISTER_MODULE(PM_Movement);
		REGISTER_MODULE(XO_Console);
		REGISTER_MODULE(RB_DrawCollision);
		REGISTER_MODULE(RB_ShaderOverlays);
		REGISTER_MODULE(RadiantRemote);
		REGISTER_MODULE(Window);
		REGISTER_MODULE(Devgui);
	}

	void Loader::Uninitialize()
	{
		std::reverse(Loader::Components.begin(), Loader::Components.end());
		for (auto component : Loader::Components)
		{
			delete component;
		}

		Loader::Components.clear();
		Loader::MemAllocator.clear();
	}

	void Loader::Register(Component* component)
	{
		if (component)
		{
			Game::Globals::loadedModules.append(Utils::VA("Component registered: %s\n", component->getName()));
			Loader::Components.push_back(component);
		}
	}

	bool Loader::Registered(const char *componentName)
	{
		std::reverse(Loader::Components.begin(), Loader::Components.end());
		for (auto component : Loader::Components)
		{
			if (!strcmp(componentName, component->getName()))
			{
				return true;
			}
		}

		return false;
	}

	Utils::Memory::Allocator* Loader::GetAlloctor()
	{
		return &Loader::MemAllocator;
	}
}
