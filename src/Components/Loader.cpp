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

		// global bools for more dynamic modules (no module sorting needed)
		// modules to be loaded ::
		active._UI = true;
		active._CG = true;
		active._Pmove = true;
		active._Debug = true;
		active._Game = true;
		active._Map = true;
		active.Command = true;
		active.GScr_Methods = true;
		active.QuickPatch = true;
		active.PM_Movement = true;
		active.XO_Console = true;
		active.RB_DrawCollision = true;
		active.RB_ShaderOverlays = true;
		active.RadiantRemote = true;
		active.Scheduler = true;

		// General Modules that need to be loaded
		REGISTER_MODULE(_CG);
		REGISTER_MODULE(_UI);
		REGISTER_MODULE(_Pmove);
		REGISTER_MODULE(_Debug);
		REGISTER_MODULE(_Game);
		REGISTER_MODULE(_Map);
		// Addons
		REGISTER_MODULE(Command);
		REGISTER_MODULE(GScr_Methods);
		REGISTER_MODULE(QuickPatch);
		REGISTER_MODULE(PM_Movement);
		REGISTER_MODULE(XO_Console);
		REGISTER_MODULE(RB_DrawCollision);
		REGISTER_MODULE(RB_ShaderOverlays);
		REGISTER_MODULE(RadiantRemote);
		REGISTER_MODULE(Scheduler);
		
		// General Modules that need to be loaded
		/*Loader::Register(new _CG(),	active._CG);
		Loader::Register(new _UI(),	active._UI);
		Loader::Register(new _Pmove(), active._Pmove);
		Loader::Register(new _Debug(), active._Debug);
		Loader::Register(new _Game(), active._Game);
		Loader::Register(new _Map(), active._Map);
		Loader::Register(new Command(),	active.Command);
		Loader::Register(new GScr_Methods(), active.GScr_Methods);
		Loader::Register(new QuickPatch(), active.QuickPatch);*/

		// ---------
		
		// Addons - undefine to disable modules
		/*Loader::Register(new PM_Movement(), active.PM_Movement);
		Loader::Register(new XO_Console(), active.XO_Console);
		Loader::Register(new RB_DrawCollision(), active.RB_DrawCollision);
		Loader::Register(new RB_ShaderOverlays(), active.RB_ShaderOverlays);
		Loader::Register(new RadiantRemote(), active.RadiantRemote);*/

		// ---------

		// General Modules with dynamic hooks depending on addon component loading needs to be loaded last
		//Loader::Register(new Scheduler(), active.Scheduler); // checks if XO_Console is registered
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
		//fflush(stdout);
		//fflush(stderr);
	}

	void Loader::Register(Component* component)//bool& registered)
	{
		if (component)
		{
			Game::Globals::loadedModules.append(Utils::VA("Component registered: %s\n", component->getName()));
			Loader::Components.push_back(component);
			//registered = true;
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
