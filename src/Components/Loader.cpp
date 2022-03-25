#include "STDInclude.hpp"

namespace Components
{
	// global active modules struct
	activeModules_s active = activeModules_s();

	std::vector<Component*> Loader::Components;
	utils::Memory::Allocator Loader::MemAllocator;

	#define USE_MODULE(name, state)	active.##name = state

	void Loader::Initialize()
	{
		Loader::MemAllocator.clear();

		// global bools for more dynamic modules (choose which modules to load)
		active._cg			= true;
		active._client		= true;
		active._common		= true;
		active._debug		= true;
		active._ggame		= true;
		active._map			= true;
		active._pmove		= true;
		active._renderer	= true;
		active._ui			= true;
		active.command		= true;
		active.gscr_methods = true;
		active.patches	= true;
		active.Scheduler	= true;
		
		active.d3d9ex			 = true;
		active.movement			 = true;
		active.XO_Console		 = true;
		active.RB_DrawCollision	 = true;
		active.RB_ShaderOverlays = true;
		active.radiant_livelink	 = true;
		active.Window			 = true;
		active.gui				 = active.Window == active.d3d9ex; // needs the window module and d3d9ex
		active.gui_devgui		 = active.gui; // obv. needs imgui
		active.menu_export		 = true;
		active.compass			 = true;
		active.cgaz				 = true;
		active.mvm				 = true;
		active.daynight_cycle	 = true;
		active.ocean			 = true;
		
		// General Modules that need to be loaded
		REGISTER_MODULE(_cg);
		REGISTER_MODULE(_client);
		REGISTER_MODULE(_common);
		REGISTER_MODULE(_debug);
		REGISTER_MODULE(_ggame);
		REGISTER_MODULE(_map);
		REGISTER_MODULE(_pmove);
		REGISTER_MODULE(_renderer);
		REGISTER_MODULE(_ui);
		REGISTER_MODULE(command);
		REGISTER_MODULE(gscr_methods);
		REGISTER_MODULE(patches);
		REGISTER_MODULE(Scheduler);

		// Addons
		REGISTER_MODULE(d3d9ex);
		REGISTER_MODULE(menu_export);
		REGISTER_MODULE(movement);
		REGISTER_MODULE(XO_Console);
		REGISTER_MODULE(RB_DrawCollision);
		REGISTER_MODULE(RB_ShaderOverlays);
		REGISTER_MODULE(radiant_livelink);
		REGISTER_MODULE(Window);
		REGISTER_MODULE(gui);
		REGISTER_MODULE(gui_devgui);
		REGISTER_MODULE(compass);
		REGISTER_MODULE(cgaz);
		REGISTER_MODULE(mvm);
		REGISTER_MODULE(daynight_cycle);
		REGISTER_MODULE(ocean);
	}

	void Loader::Uninitialize()
	{
		std::ranges::reverse(Loader::Components.begin(), Loader::Components.end());
		for (const auto component : Loader::Components)
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
			Game::Globals::loaded_modules.append(utils::va("Component registered: %s\n", component->getName()));
			Loader::Components.push_back(component);
		}
	}

	bool Loader::Registered(const char *componentName)
	{
		std::ranges::reverse(Loader::Components.begin(), Loader::Components.end());
		for (const auto component : Loader::Components)
		{
			if (!strcmp(componentName, component->getName()))
			{
				return true;
			}
		}

		return false;
	}

	utils::Memory::Allocator* Loader::GetAlloctor()
	{
		return &Loader::MemAllocator;
	}
}
