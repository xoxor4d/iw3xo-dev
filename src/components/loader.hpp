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
		bool flags;
		bool gscr_methods;
		bool patches;
		bool scheduler;
		bool window;

		bool cgaz;
		bool compass;
		bool console;
		bool daynight_cycle;
		bool draw_collision;
		bool gui;
		bool gui_devgui;
		bool menu_export;
		bool movement;
		bool mvm;
		bool ocean;
		bool postfx_shaders;
		bool radiant_livelink;
		bool rtx;
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
		virtual const char* get_name() { return "unknown"; };
	};

	class loader
	{
	public:
		static void initialize_();
		static void uninitialize_();
		static void register_(component* component);//, bool& registered);
		static bool is_registered(const char *componentName);

		static utils::memory::allocator* get_alloctor();
		
	private:
		static std::vector<component*> components_;
		static utils::memory::allocator component_allocator_;
	};
}

// General modules that have to be included
#include "modules/_cg.hpp"
#include "modules/_client.hpp"
#include "modules/_common.hpp"
#include "modules/_debug.hpp"
#include "modules/_ggame.hpp"
#include "modules/_map.hpp"
#include "modules/_pmove.hpp"
#include "modules/_renderer.hpp"
#include "modules/_ui.hpp"
#include "modules/command.hpp"
#include "modules/flags.hpp"
#include "modules/d3d9ex.hpp"
#include "modules/gscr_methods.hpp"
#include "modules/patches.hpp"

#include "modules/cgaz.hpp"
#include "modules/compass.hpp"
#include "modules/console.hpp"
#include "modules/daynight_cycle.hpp"
#include "modules/draw_collision.hpp"
#include "modules/gui.hpp"
#include "modules/gui_devgui.hpp"
#include "modules/menu_export.hpp"
#include "modules/movement.hpp"
#include "modules/mvm.hpp"
#include "modules/ocean.hpp"
#include "modules/postfx_shaders.hpp"
#include "modules/radiant_livelink.hpp"
#include "modules/rtx.hpp"
#include "modules/window.hpp"

// General Modules with dynamic hooks depending on addon component loading
#include "modules/scheduler.hpp"
