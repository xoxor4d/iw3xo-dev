#include "std_include.hpp"

namespace components
{
	// global active modules struct
	activeModules_s active = activeModules_s();

	std::vector<component*> loader::components_;
	utils::memory::allocator loader::component_allocator_;

	#define USE_MODULE(name, state)	active.##name = state

	void loader::initialize_()
	{
		loader::component_allocator_.clear();

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
		active.scheduler		= true;
		active.window			= true;
		
		active.cgaz				= true;
		active.compass			= true;
		active.console			= true;
		active.daynight_cycle	= true;
		active.draw_collision	= true;
		active.gui				= active.window == active.d3d9ex; // needs the window module and d3d9ex
		active.gui_devgui		= active.gui; // obv. needs imgui
		active.menu_export		= true;
		active.movement			= true;
		active.mvm				= true;
		active.ocean			= true;
		active.postfx_shaders	= true;
		active.radiant_livelink	= true;

		active.rtx					= true;
		active.rtx_fixed_function	= true;
		active.rtx_gui				= true;
		active.rtx_lights			= true;
		
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
		REGISTER_MODULE(scheduler);
		REGISTER_MODULE(window);

		// Addons
		REGISTER_MODULE(cgaz);
		REGISTER_MODULE(compass);
		REGISTER_MODULE(console);
		REGISTER_MODULE(daynight_cycle);
		REGISTER_MODULE(draw_collision);
		REGISTER_MODULE(gui);
		REGISTER_MODULE(gui_devgui);
		REGISTER_MODULE(menu_export);
		REGISTER_MODULE(movement);
		REGISTER_MODULE(mvm);
		REGISTER_MODULE(ocean);
		REGISTER_MODULE(postfx_shaders);
		REGISTER_MODULE(radiant_livelink);
		REGISTER_MODULE(rtx);
		REGISTER_MODULE(rtx_fixed_function);
		REGISTER_MODULE(rtx_gui);
		REGISTER_MODULE(rtx_lights);
	}

	void loader::uninitialize_()
	{
		std::ranges::reverse(loader::components_.begin(), loader::components_.end());
		for (const auto component : loader::components_)
		{
			delete component;
		}

		loader::components_.clear();
		loader::component_allocator_.clear();
	}

	void loader::register_(component* component)
	{
		if (component)
		{
			game::glob::loaded_modules.append(utils::va("Component registered: %s\n", component->get_name()));
			loader::components_.push_back(component);
		}
	}

	bool loader::is_registered(const char *componentName)
	{
		std::ranges::reverse(loader::components_.begin(), loader::components_.end());
		for (const auto component : loader::components_)
		{
			if (!strcmp(componentName, component->get_name()))
			{
				return true;
			}
		}

		return false;
	}

	utils::memory::allocator* loader::get_alloctor()
	{
		return &loader::component_allocator_;
	}
}
