#pragma once

namespace Components
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
		bool patches;
		bool gscr_methods;
		bool Scheduler;

		bool d3d9ex;
		bool movement;
		bool XO_Console;
		bool RB_DrawCollision;
		bool RB_ShaderOverlays;
		bool radiant_livelink;
		bool Window;
		bool gui;
		bool gui_devgui;
		bool menu_export;
		bool compass;
		bool cgaz;
		bool mvm;
		bool daynight_cycle;
		bool ocean;
	};

	extern activeModules_s active;

#define REGISTER_MODULE(name)			\
	if(active.##name)					\
		Loader::Register(new name())

	class Component
	{
	public:
		Component() = default;
		virtual ~Component() = default;
		virtual const char* getName() { return "Unknown"; };
	};

	class Loader
	{
	public:
		static void Initialize();
		static void Uninitialize();
		static void Register(Component* component);//, bool& registered);
		static bool Registered(const char *componentName);

		static utils::Memory::Allocator* GetAlloctor();
		
	private:
		static std::vector<Component*> Components;
		static utils::Memory::Allocator MemAllocator;
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
#include "Modules/patches.hpp"
#include "Modules/gscr_methods.hpp"

// Addons
#include "Modules/d3d9ex.hpp"
#include "Modules/menu_export.hpp"
#include "Modules/movement.hpp"
#include "Modules/XO_Console.hpp"
#include "Modules/RB_DrawCollision.hpp"
#include "Modules/RB_ShaderOverlays.hpp"
#include "Modules/radiant_livelink.hpp"
#include "Modules/Window.hpp"
#include "Modules/gui.hpp"
#include "Modules/gui_devgui.hpp"
#include "Modules/compass.hpp"
#include "Modules/cgaz.hpp"
#include "Modules/mvm.hpp"
#include "Modules/daynight_cycle.hpp"
#include "Modules/ocean.hpp"

// General Modules with dynamic hooks depending on addon component loading
#include "Modules/Scheduler.hpp"
