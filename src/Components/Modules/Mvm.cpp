#include "STDInclude.hpp"

namespace Components
{
	bool iw3mvm_loaded	 = false;
	bool demo_paused	 = false;
	bool streams_prepare = false;

	int  streams_shotCount = 0;
	int  streams_currentView = 0;
	int  client_msec = 0;


	void Screenshot(std::string prefix = "recording")
	{
		std::string mod_name, demo_name = "";

		const char* screenshot_path;

		const auto& base_path = Game::Dvar_FindVar("fs_basepath");
		const auto& fs_game = Game::Dvar_FindVar("fs_game");

		if (!base_path) 
		{
			return;
		}

		if (fs_game) 
		{
			mod_name = fs_game->current.string;
			Utils::Replace(mod_name, "mods/"s, "");
		}

		if (Game::clc.demoplaying) 
		{
			demo_name = Game::clc.demoName;
		}

		std::string filePath = base_path->current.string + "\\iw3xo\\screenshots\\"s + mod_name + "\\"s + demo_name;
		std::filesystem::create_directories(filePath);


		screenshot_path = Utils::VA("%s\\%s_%07d.tga", filePath.c_str(), prefix.c_str(), streams_shotCount);

		for (; Utils::FileExists(screenshot_path) ;)
		{
			streams_shotCount++;
			screenshot_path = Utils::VA("%s\\%s_%07d.tga", filePath.c_str(), prefix.c_str(), streams_shotCount);
		}


		IDirect3DSurface9* backbuffer;
		Game::Globals::d3d9_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE::D3DBACKBUFFER_TYPE_MONO, &backbuffer);

		D3DXSaveSurfaceToFileA(screenshot_path, D3DXIFF_TGA, backbuffer, NULL, NULL);
		backbuffer->Release();
	}


	void toggle_greenscreen(bool state)
	{
		auto r_clear = Game::Dvar_FindVar("r_clear");
		auto r_clearcolor = Game::Dvar_FindVar("r_clearcolor");
		auto r_clearcolor2 = Game::Dvar_FindVar("r_clearcolor2");

		auto r_skippvs = Game::Dvar_FindVar("r_skippvs");
		auto fx_enable = Game::Dvar_FindVar("fx_enable");
		auto fx_marks = Game::Dvar_FindVar("fx_marks");

		auto r_zfar = Game::Dvar_FindVar("r_zfar");
		auto r_znear = Game::Dvar_FindVar("r_znear");

		if (state)
		{
			// turn on greenscreen
			r_clear->current.integer = 3;

			if (r_clearcolor)
			{
				r_clearcolor->current.color[0] = unsigned char(0);
				r_clearcolor->current.color[1] = unsigned char(255);
				r_clearcolor->current.color[2] = unsigned char(0);
				r_clearcolor->current.color[3] = unsigned char(255);
			}

			if (r_clearcolor2)
			{
				r_clearcolor2->current.color[0] = unsigned char(0);
				r_clearcolor2->current.color[1] = unsigned char(255);
				r_clearcolor2->current.color[2] = unsigned char(0);
				r_clearcolor2->current.color[3] = unsigned char(255);
			}

			Game::Dvar_SetValue(r_skippvs, true);
			Game::Dvar_SetValue(fx_enable, false);
			Game::Dvar_SetValue(fx_marks, false);

			Game::Dvar_SetValue(r_zfar, 0.001f);
			Game::Dvar_SetValue(r_znear, 10000.0f);
		}
		else
		{
			Game::Dvar_SetValue(r_clear, false);
			Game::Dvar_SetValue(r_skippvs, false);
			Game::Dvar_SetValue(fx_enable, true);
			Game::Dvar_SetValue(fx_marks, true);

			Game::Dvar_SetValue(r_zfar, 0.0f);
			Game::Dvar_SetValue(r_znear, 4.0f);
		}
	}


	void toggle_depthmap(bool state)
	{
		if (state)
		{
			Game::Dvar_SetValue(Dvars::xo_shaderoverlay, 4);
		}
		else
		{
			Game::Dvar_SetValue(Dvars::xo_shaderoverlay, 0);
		}
	}


	// called from D3D9Ex::D3D9Device::BeginScene()
	void Mvm::avidemo_streams()
	{
		if (Game::clc.demoplaying)
		{
			// do not take screenshots if iw3mvm is loaded or we get duplicates
			if (!iw3mvm_loaded)
			{
				const auto& cl_avidemo = Game::Dvar_FindVar("cl_avidemo");
				const auto& cl_forceavidemo = Game::Dvar_FindVar("cl_forceavidemo");

				if (cl_avidemo && cl_forceavidemo && (cl_avidemo->current.integer || cl_forceavidemo->current.enabled))
				{
					Screenshot("shot");
				}

				if (Dvars::cl_pause_demo && !Dvars::cl_avidemo_streams->current.integer)
				{
					demo_paused = Dvars::cl_pause_demo->current.enabled;
				}
			}

			if (Dvars::cl_avidemo_streams)
			{
				auto cg_draw2D = Game::Dvar_FindVar("cg_draw2D");
				auto cg_drawGun = Game::Dvar_FindVar("cg_drawGun");

				if (Dvars::cl_avidemo_streams->current.integer)
				{
					// setup on first run, no screenshot
					// greenscreen-gun -> world-no-gun -> depth-no-gun -> hud

					if (streams_currentView == 0)
					{
						// reset cl_pause_demo dvar
						if (Dvars::cl_pause_demo)
						{
							Game::Dvar_SetValue(Dvars::cl_pause_demo, false);
						}

						// pause
						if (!demo_paused) demo_paused = demo_paused ? 0 : 1;

						if (Dvars::cl_avidemo_streams_greenscreen && Dvars::cl_avidemo_streams_greenscreen->current.enabled)
						{
							toggle_greenscreen(1);
						}
	
						Game::Dvar_SetValue(cg_draw2D, false);
						Game::Dvar_SetValue(cg_drawGun, true);

						streams_prepare = true;
					}
					else
					{
						switch (streams_currentView)
						{
							// greenscreen-gun 
						case 2:
							if (Dvars::cl_avidemo_streams_greenscreen && Dvars::cl_avidemo_streams_greenscreen->current.enabled)
							{
								Screenshot("greenscreen");
							}
	
							streams_prepare = false;
							break;


							// setup world-no-gun
						case 3:
							toggle_greenscreen(0);

							if (Dvars::cl_avidemo_streams_viewmodel)
							{
								Game::Dvar_SetValue(cg_drawGun, Dvars::cl_avidemo_streams_viewmodel->current.enabled);
							}

							streams_prepare = true;
							break;

							// ^
						case 5:
							Screenshot("normal");

							streams_prepare = false;
							break;


							// setup depth-no-gun
						case 6:
							if (Dvars::cl_avidemo_streams_depth && Dvars::cl_avidemo_streams_depth->current.enabled)
							{
								toggle_depthmap(1);
							}

							if (Dvars::cl_avidemo_streams_viewmodel)
							{
								Game::Dvar_SetValue(cg_drawGun, Dvars::cl_avidemo_streams_viewmodel->current.enabled);
							}
							
							streams_prepare = true;
							break;

							// ^
						case 8:
							if (Dvars::cl_avidemo_streams_depth && Dvars::cl_avidemo_streams_depth->current.enabled)
							{
								Screenshot("depth");
							}
	
							streams_prepare = false;
							break;


							// setup hud
						case 9:
							toggle_depthmap(0);

							if (Dvars::cl_avidemo_streams_hud && Dvars::cl_avidemo_streams_hud->current.enabled)
							{
								toggle_greenscreen(1);
								Game::Dvar_SetValue(cg_draw2D, true);
								Game::Dvar_SetValue(cg_drawGun, false);
							}
							
							streams_prepare = true;
							break;

							// ^
						case 11:
							if (Dvars::cl_avidemo_streams_hud && Dvars::cl_avidemo_streams_hud->current.enabled)
							{
								Screenshot("hud");
							}

							streams_prepare = false;
							break;

						default:
							break;
						}
					}

					if (streams_currentView < 11)
					{
						streams_currentView++;
					}
					else
					{
						streams_currentView = 0;
						streams_shotCount++;

						// reset
						toggle_greenscreen(0);
						toggle_depthmap(0);

						Game::Dvar_SetValue(cg_draw2D, true);
						Game::Dvar_SetValue(cg_drawGun, true);

						// unpause
						if (demo_paused) demo_paused = demo_paused ? 0 : 1;
					}
				}
				else // (Dvars::cl_avidemo_streams->current.integer
				{
					if (Dvars::cl_pause_demo && !Dvars::cl_pause_demo->current.enabled)
					{
						if (demo_paused) demo_paused = demo_paused ? 0 : 1;
					}

					// reset after setting cl_avidemo_streams to 0
					if (streams_currentView != 0)
					{
						// reset
						toggle_greenscreen(0);
						toggle_depthmap(0);

						Game::Dvar_SetValue(cg_draw2D, true);
						Game::Dvar_SetValue(cg_drawGun, true);

						streams_currentView = 0;
						streams_shotCount++;
					}
				}
			}
		}
	}

	void CL_RunOncePerClientFrame_streams()
	{
		const auto& cl_avidemo = Game::Dvar_FindVar("cl_avidemo");
		const auto& cl_forceavidemo = Game::Dvar_FindVar("cl_forceavidemo");

		// check if iw3mvm is present
		if (*(BYTE*)(0x46C90F) == 0x90) {
			iw3mvm_loaded = true;
		}
		else {
			iw3mvm_loaded = false;
		}

		if (cl_avidemo && cl_forceavidemo && cl_avidemo->current.integer)
		{
			if (client_msec)
			{
				// iw3mvm already does this
				/*if (!iw3mvm_loaded)
				{
					if (Game::clientUI->connectionState == 9 || cl_forceavidemo->current.enabled)
					{
						Game::Cmd_ExecuteSingleCommand(0, 0, "screenshot silent\n");
					}
				}*/

				client_msec = static_cast<int>(1000.0 / cl_avidemo->current.integer * *Game::com_timescaleValue);

				if (!client_msec)
				{
					client_msec = 1;
				}
			}
		}

		else if (Dvars::cl_avidemo_streams->current.integer)
		{
			client_msec = static_cast<int>(1000.0 / Dvars::cl_avidemo_streams->current.integer * *Game::com_timescaleValue);

			if (!client_msec)
			{
				client_msec = 1;
			}
		}
	}

	__declspec(naked) void CL_RunOncePerClientFrame_stub()
	{
		const static uint32_t retnPt_stock = 0x46C94B; // to "add cls.realtime, esi" (iw3mvm hook spot)
		const static uint32_t retnPt_paused = 0x46C951; // to "mov eax, ecx" (not setting realtime)

		__asm
		{
			// hooked on "mov eax, cl_avidemo"
			// esi = msec

			// move msec into glob var
			mov		client_msec, esi;

			pushad;
			call	CL_RunOncePerClientFrame_streams;
			popad;

			// move stock/modified msec back into esi
			mov		esi, client_msec;

			// operation before mvm hook
			mov     ecx, Game::com_frameTime; // stock
			mov		ecx, dword ptr[ecx]; // val of com_frameTime


			pushad;
			push	eax;
			mov		eax, Game::clc;
			mov		eax, dword ptr[eax + 40194h]; // clc.demoplaying
			cmp		eax, 1;
			pop		eax;

			// jmp if not demo playback
			jne		jmp_stock;

			// if demo is playing
			push	eax;
			mov		al, demo_paused;
			cmp		al, 1;
			pop		eax;d;

			// jmp if demo not paused (overjump next op and iw3mvm "hook")
			jne		jmp_stock;

			popad;
			jmp		retnPt_paused;

		jmp_stock:
			popad;
			jmp		retnPt_stock;
		}
	}


	void should_draw_console()
	{
		if (!Dvars::cl_avidemo_streams->current.integer || !streams_prepare)
		{
			// Con_DrawSolidConsole
			Utils::function<void()>(0x461CD0)();
		}
	}

	__declspec(naked) void Con_DrawSolidConsole_stub()
	{
		const static uint32_t retnPt = 0x475065;
		__asm
		{
			pushad;
			call	should_draw_console;
			popad;

			jmp		retnPt;
		}
	}

	Mvm::Mvm()
	{
#if 0
		Dvars::load_iw3mvm = Game::Dvar_RegisterBool(
			/* name		*/ "load_iw3mvm",
			/* desc		*/ "load iw3mvm on startup",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::cl_avidemo_streams = Game::Dvar_RegisterIntWrapper(
			/* name		*/ "cl_avidemo_streams",
			/* desc		*/ "just like mvm_streams",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 1000,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::cl_avidemo_streams_greenscreen = Game::Dvar_RegisterBool(
			/* name		*/ "cl_avidemo_streams_greenscreen",
			/* desc		*/ "take greenscreen shots of the viewmodel",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::cl_avidemo_streams_viewmodel = Game::Dvar_RegisterBool(
			/* name		*/ "cl_avidemo_streams_viewmodel",
			/* desc		*/ "show viewmodel in normal shots",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::cl_avidemo_streams_depth = Game::Dvar_RegisterBool(
			/* name		*/ "cl_avidemo_streams_depth",
			/* desc		*/ "take depth shots of the world",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::cl_avidemo_streams_hud = Game::Dvar_RegisterBool(
			/* name		*/ "cl_avidemo_streams_hud",
			/* desc		*/ "take greenscreen shots of the hud",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::cl_pause_demo = Game::Dvar_RegisterBool(
			/* name		*/ "cl_pause_demo",
			/* desc		*/ "pause the demo",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);
#endif

		// mvm hooks at the next op
		//Utils::Hook(0x46C8EB, CL_RunOncePerClientFrame_stub, HOOK_JUMP).install()->quick();

		// disable console drawing when taking screenshots using avidemo_streams
		//Utils::Hook(0x475060, Con_DrawSolidConsole_stub, HOOK_JUMP).install()->quick();

		// fix out of memory error when trying to take screenshots with R_TakeScreenshot when using heavy mods
		Utils::Hook::Set<BYTE>(0x60E624 + 1, 0x2); // CreateOffscreenPlainSurface D3DPOOL_SCRATCH -> D3DPOOL_SYSTEMMEM
	}

	Mvm::~Mvm()
	{ }
}