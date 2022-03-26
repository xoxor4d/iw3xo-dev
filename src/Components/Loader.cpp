#include "STDInclude.hpp"

namespace components
{
	// global active modules struct
	activeModules_s active = activeModules_s();

	std::vector<component*> loader::components_;
	utils::Memory::Allocator loader::mem_allocator_;

	#define USE_MODULE(name, state)	active.##name = state

	void loader::initialize_()
	{
		loader::mem_allocator_.clear();

		// global bools for more dynamic modules (choose which modules to load)
		active._cg				= true;
		active._client			= true;
		active._common			= true;
		active._debug			= true;
		active._ggame			= true;
		active._map				= true;
		active._pmove			= true;
		active._renderer		= true;
		active._ui				= true;
		active.command			= true;
		active.d3d9ex			= true;
		active.gscr_methods		= true;
		active.patches			= true;
		active.Scheduler		= true;
		active.Window			= true;
		
		active.cgaz				 = true;
		active.compass			 = true;
		active.daynight_cycle	 = true;
		active.draw_collision	 = true;
		active.gui				 = active.Window == active.d3d9ex; // needs the window module and d3d9ex
		active.gui_devgui		 = active.gui; // obv. needs imgui
		active.menu_export		 = true;
		active.movement			 = true;
		active.mvm				 = true;
		active.ocean			 = true;
		active.radiant_livelink	 = true;
		active.RB_ShaderOverlays = true;
		active.XO_Console		 = true;
		
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
		REGISTER_MODULE(d3d9ex);
		REGISTER_MODULE(gscr_methods);
		REGISTER_MODULE(patches);
		REGISTER_MODULE(Scheduler);
		REGISTER_MODULE(Window);

		// Addons
		REGISTER_MODULE(cgaz);
		REGISTER_MODULE(compass);
		REGISTER_MODULE(daynight_cycle);
		REGISTER_MODULE(draw_collision);
		REGISTER_MODULE(gui);
		REGISTER_MODULE(gui_devgui);
		REGISTER_MODULE(menu_export);
		REGISTER_MODULE(movement);
		REGISTER_MODULE(mvm);
		REGISTER_MODULE(ocean);
		REGISTER_MODULE(radiant_livelink);
		REGISTER_MODULE(RB_ShaderOverlays);
		REGISTER_MODULE(XO_Console);
	}

	void loader::uninitialize_()
	{
		std::ranges::reverse(loader::components_.begin(), loader::components_.end());
		for (const auto component : loader::components_)
		{
			delete component;
		}

		loader::components_.clear();
		loader::mem_allocator_.clear();
	}

	void loader::register_(component* component)
	{
		if (component)
		{
			Game::Globals::loaded_modules.append(utils::va("Component registered: %s\n", component->getName()));
			loader::components_.push_back(component);
		}
	}

	bool loader::is_registered(const char *componentName)
	{
		std::ranges::reverse(loader::components_.begin(), loader::components_.end());
		for (const auto component : loader::components_)
		{
			if (!strcmp(componentName, component->getName()))
			{
				return true;
			}
		}

		return false;
	}

	utils::Memory::Allocator* loader::get_alloctor()
	{
		return &loader::mem_allocator_;
	}
}
