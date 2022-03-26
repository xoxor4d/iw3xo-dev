#pragma once

namespace components
{
	// add new modules here
	struct activeModules_s
	{
		bool _cg;
		bool _client;
		bool _common;
		bool _debug;
		bool _ggame;
		bool _map;
		bool _pmove;
		bool _renderer;
		bool _ui;
		bool command;
		bool d3d9ex;
		bool gscr_methods;
		bool patches;
		bool Scheduler;
		bool Window;

		bool cgaz;
		bool compass;
		bool daynight_cycle;
		bool draw_collision;
		bool gui;
		bool gui_devgui;
		bool menu_export;
		bool movement;
		bool mvm;
		bool ocean;
		bool radiant_livelink;
		bool RB_ShaderOverlays;
		bool XO_Console;
	};

	extern activeModules_s active;

#define REGISTER_MODULE(name)			\
	if(active.##name)					\
		loader::register_(new name())

	class component
	{
	public:
		component() = default;
		virtual ~component() = default;
		virtual const char* getName() { return "Unknown"; };
	};

	class loader
	{
	public:
		static void initialize_();
		static void uninitialize_();
		static void register_(component* component);//, bool& registered);
		static bool is_registered(const char *componentName);

		static utils::Memory::Allocator* get_alloctor();
		
	private:
		static std::vector<component*> components_;
		static utils::Memory::Allocator mem_allocator_;
	};
}

// General modules that have to be included
#include "Modules/_cg.hpp"
#include "Modules/_client.hpp"
#include "Modules/_common.hpp"
#include "Modules/_debug.hpp"
#include "Modules/_ggame.hpp"
#include "Modules/_map.hpp"
#include "Modules/_pmove.hpp"
#include "Modules/_renderer.hpp"
#include "Modules/_ui.hpp"
#include "Modules/command.hpp"
#include "Modules/d3d9ex.hpp"
#include "Modules/gscr_methods.hpp"
#include "Modules/patches.hpp"

#include "Modules/cgaz.hpp"
#include "Modules/compass.hpp"
#include "Modules/daynight_cycle.hpp"
#include "Modules/draw_collision.hpp"
#include "Modules/gui.hpp"
#include "Modules/gui_devgui.hpp"
#include "Modules/menu_export.hpp"
#include "Modules/movement.hpp"
#include "Modules/mvm.hpp"
#include "Modules/ocean.hpp"
#include "Modules/radiant_livelink.hpp"
#include "Modules/RB_ShaderOverlays.hpp"
#include "Modules/Window.hpp"
#include "Modules/XO_Console.hpp"

// General Modules with dynamic hooks depending on addon component loading
#include "Modules/Scheduler.hpp"
