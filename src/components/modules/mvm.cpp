#include "std_include.hpp"

namespace components
{
	bool iw3mvm_loaded	 = false;
	bool demo_paused	 = false;
	bool streams_prepare = false;

	int  streams_shotCount = 0;
	int  streams_currentView = 0;
	int  client_msec = 0;


	void screenshot(std::string prefix = "recording")
	{
		std::string mod_name, demo_name;

		const auto& base_path = game::Dvar_FindVar("fs_basepath");
		if (!base_path) 
		{
			return;
		}

		if (const auto& fs_game = game::Dvar_FindVar("fs_game"); 
						fs_game)
		{
			mod_name = fs_game->current.string;
			utils::replace(mod_name, "mods/"s, "");
		}

		if (game::clc.demoplaying) 
		{
			demo_name = game::clc.demoName;
		}

		const std::string file_path = base_path->current.string + "\\iw3xo\\screenshots\\"s + mod_name + "\\"s + demo_name;
		std::filesystem::create_directories(file_path);


		const char* screenshot_path = utils::va("%s\\%s_%07d.tga", file_path.c_str(), prefix.c_str(), streams_shotCount);

		for (; utils::file_exists(screenshot_path) ;)
		{
			streams_shotCount++;
			screenshot_path = utils::va("%s\\%s_%07d.tga", file_path.c_str(), prefix.c_str(), streams_shotCount);
		}


		IDirect3DSurface9* backbuffer;
		game::glob::d3d9_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE::D3DBACKBUFFER_TYPE_MONO, &backbuffer);

		D3DXSaveSurfaceToFileA(screenshot_path, D3DXIFF_TGA, backbuffer, NULL, NULL);
		backbuffer->Release();
	}


	void toggle_greenscreen(bool state)
	{
		const auto& r_clear = game::Dvar_FindVar("r_clear");
		const auto& r_clearcolor = game::Dvar_FindVar("r_clearcolor");
		const auto& r_clearcolor2 = game::Dvar_FindVar("r_clearcolor2");

		const auto& r_skippvs = game::Dvar_FindVar("r_skippvs");
		const auto& fx_enable = game::Dvar_FindVar("fx_enable");
		const auto& fx_marks = game::Dvar_FindVar("fx_marks");

		const auto& r_zfar = game::Dvar_FindVar("r_zfar");
		const auto& r_znear = game::Dvar_FindVar("r_znear");

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

			game::dvar_set_value_dirty(r_skippvs, true);
			game::dvar_set_value_dirty(fx_enable, false);
			game::dvar_set_value_dirty(fx_marks, false);

			game::dvar_set_value_dirty(r_zfar, 0.001f);
			game::dvar_set_value_dirty(r_znear, 10000.0f);
		}
		else
		{
			game::dvar_set_value_dirty(r_clear, false);
			game::dvar_set_value_dirty(r_skippvs, false);
			game::dvar_set_value_dirty(fx_enable, true);
			game::dvar_set_value_dirty(fx_marks, true);

			game::dvar_set_value_dirty(r_zfar, 0.0f);
			game::dvar_set_value_dirty(r_znear, 4.0f);
		}
	}


	void toggle_depthmap(bool state)
	{
		if (state)
		{
			game::dvar_set_value_dirty(dvars::xo_shaderoverlay, 4);
		}
		else
		{
			game::dvar_set_value_dirty(dvars::xo_shaderoverlay, 0);
		}
	}


	// called from d3d9ex::D3D9Device::BeginScene()
	void mvm::avidemo_streams()
	{
		if (game::clc.demoplaying)
		{
			// do not take screenshots if iw3mvm is loaded or we get duplicates
			if (!iw3mvm_loaded)
			{
				const auto& cl_avidemo = game::Dvar_FindVar("cl_avidemo");
				const auto& cl_forceavidemo = game::Dvar_FindVar("cl_forceavidemo");

				if (cl_avidemo && cl_forceavidemo && (cl_avidemo->current.integer || cl_forceavidemo->current.enabled))
				{
					screenshot("shot");
				}

				if (dvars::cl_pause_demo && !dvars::cl_avidemo_streams->current.integer)
				{
					demo_paused = dvars::cl_pause_demo->current.enabled;
				}
			}

			if (dvars::cl_avidemo_streams)
			{
				const auto& cg_draw2D = game::Dvar_FindVar("cg_draw2D");
				const auto& cg_drawGun = game::Dvar_FindVar("cg_drawGun");

				if (dvars::cl_avidemo_streams->current.integer)
				{
					// setup on first run, no screenshot
					// greenscreen-gun -> world-no-gun -> depth-no-gun -> hud

					if (streams_currentView == 0)
					{
						// reset cl_pause_demo dvar
						if (dvars::cl_pause_demo)
						{
							game::dvar_set_value_dirty(dvars::cl_pause_demo, false);
						}

						// pause
						if (!demo_paused) demo_paused = demo_paused ? 0 : 1;

						if (dvars::cl_avidemo_streams_greenscreen && dvars::cl_avidemo_streams_greenscreen->current.enabled)
						{
							toggle_greenscreen(1);
						}
	
						game::dvar_set_value_dirty(cg_draw2D, false);
						game::dvar_set_value_dirty(cg_drawGun, true);

						streams_prepare = true;
					}
					else
					{
						switch (streams_currentView)
						{
							// greenscreen-gun 
						case 2:
							if (dvars::cl_avidemo_streams_greenscreen && dvars::cl_avidemo_streams_greenscreen->current.enabled)
							{
								screenshot("greenscreen");
							}
	
							streams_prepare = false;
							break;


							// setup world-no-gun
						case 3:
							toggle_greenscreen(0);

							if (dvars::cl_avidemo_streams_viewmodel)
							{
								game::dvar_set_value_dirty(cg_drawGun, dvars::cl_avidemo_streams_viewmodel->current.enabled);
							}

							streams_prepare = true;
							break;

							// ^
						case 5:
							screenshot("normal");

							streams_prepare = false;
							break;


							// setup depth-no-gun
						case 6:
							if (dvars::cl_avidemo_streams_depth && dvars::cl_avidemo_streams_depth->current.enabled)
							{
								toggle_depthmap(1);
							}

							if (dvars::cl_avidemo_streams_viewmodel)
							{
								game::dvar_set_value_dirty(cg_drawGun, dvars::cl_avidemo_streams_viewmodel->current.enabled);
							}
							
							streams_prepare = true;
							break;

							// ^
						case 8:
							if (dvars::cl_avidemo_streams_depth && dvars::cl_avidemo_streams_depth->current.enabled)
							{
								screenshot("depth");
							}
	
							streams_prepare = false;
							break;


							// setup hud
						case 9:
							toggle_depthmap(0);

							if (dvars::cl_avidemo_streams_hud && dvars::cl_avidemo_streams_hud->current.enabled)
							{
								toggle_greenscreen(1);
								game::dvar_set_value_dirty(cg_draw2D, true);
								game::dvar_set_value_dirty(cg_drawGun, false);
							}
							
							streams_prepare = true;
							break;

							// ^
						case 11:
							if (dvars::cl_avidemo_streams_hud && dvars::cl_avidemo_streams_hud->current.enabled)
							{
								screenshot("hud");
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

						game::dvar_set_value_dirty(cg_draw2D, true);
						game::dvar_set_value_dirty(cg_drawGun, true);

						// unpause
						if (demo_paused) demo_paused = demo_paused ? 0 : 1;
					}
				}
				else // (dvars::cl_avidemo_streams->current.integer
				{
					if (dvars::cl_pause_demo && !dvars::cl_pause_demo->current.enabled)
					{
						if (demo_paused) demo_paused = demo_paused ? 0 : 1;
					}

					// reset after setting cl_avidemo_streams to 0
					if (streams_currentView != 0)
					{
						// reset
						toggle_greenscreen(0);
						toggle_depthmap(0);

						game::dvar_set_value_dirty(cg_draw2D, true);
						game::dvar_set_value_dirty(cg_drawGun, true);

						streams_currentView = 0;
						streams_shotCount++;
					}
				}
			}
		}
	}

	void CL_RunOncePerClientFrame_streams()
	{
		const auto& cl_avidemo = game::Dvar_FindVar("cl_avidemo");
		const auto& cl_forceavidemo = game::Dvar_FindVar("cl_forceavidemo");

		// check if iw3mvm is present
		if (*(BYTE*)(0x46C90F) == 0x90) 
		{
			iw3mvm_loaded = true;
		}
		else 
		{
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

				client_msec = static_cast<int>(1000.0 / cl_avidemo->current.integer * *game::com_timescaleValue);

				if (!client_msec)
				{
					client_msec = 1;
				}
			}
		}

		else if (dvars::cl_avidemo_streams->current.integer)
		{
			client_msec = static_cast<int>(1000.0 / dvars::cl_avidemo_streams->current.integer * *game::com_timescaleValue);

			if (!client_msec)
			{
				client_msec = 1;
			}
		}
	}

	__declspec(naked) void CL_RunOncePerClientFrame_stub()
	{
		const static uint32_t retn_addr_stock = 0x46C94B; // to "add cls.realtime, esi" (iw3mvm hook spot)
		const static uint32_t retn_addr_paused = 0x46C951; // to "mov eax, ecx" (not setting realtime)

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
			mov     ecx, game::com_frameTime;	// stock
			mov		ecx, dword ptr[ecx];		// val of com_frameTime


			pushad;
			push	eax;
			mov		eax, game::clc;
			mov		eax, dword ptr[eax + 40194h]; // clc.demoplaying
			cmp		eax, 1;
			pop		eax;

			// jmp if not demo playback
			jne		JMP_STOCK;

			// if demo is playing
			push	eax;
			mov		al, demo_paused;
			cmp		al, 1;
			pop		eax;d;

			// jmp if demo not paused (overjump next op and iw3mvm "hook")
			jne		JMP_STOCK;

			popad;
			jmp		retn_addr_paused;

		JMP_STOCK:
			popad;
			jmp		retn_addr_stock;
		}
	}


	void should_draw_console()
	{
		if (!dvars::cl_avidemo_streams->current.integer || !streams_prepare)
		{
			// Con_DrawSolidConsole
			utils::function<void()>(0x461CD0)();
		}
	}

	__declspec(naked) void Con_DrawSolidConsole_stub()
	{
		const static uint32_t retn_addr = 0x475065;
		__asm
		{
			pushad;
			call	should_draw_console;
			popad;

			jmp		retn_addr;
		}
	}

	mvm::mvm()
	{
#if 0
		dvars::load_iw3mvm = Game::Dvar_RegisterBool(
			/* name		*/ "load_iw3mvm",
			/* desc		*/ "load iw3mvm on startup",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::cl_avidemo_streams = Game::Dvar_RegisterIntWrapper(
			/* name		*/ "cl_avidemo_streams",
			/* desc		*/ "just like mvm_streams",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 1000,
			/* flags	*/ Game::dvar_flags::none);

		dvars::cl_avidemo_streams_greenscreen = Game::Dvar_RegisterBool(
			/* name		*/ "cl_avidemo_streams_greenscreen",
			/* desc		*/ "take greenscreen shots of the viewmodel",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::cl_avidemo_streams_viewmodel = Game::Dvar_RegisterBool(
			/* name		*/ "cl_avidemo_streams_viewmodel",
			/* desc		*/ "show viewmodel in normal shots",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::cl_avidemo_streams_depth = Game::Dvar_RegisterBool(
			/* name		*/ "cl_avidemo_streams_depth",
			/* desc		*/ "take depth shots of the world",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::cl_avidemo_streams_hud = Game::Dvar_RegisterBool(
			/* name		*/ "cl_avidemo_streams_hud",
			/* desc		*/ "take greenscreen shots of the hud",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		dvars::cl_pause_demo = Game::Dvar_RegisterBool(
			/* name		*/ "cl_pause_demo",
			/* desc		*/ "pause the demo",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);
#endif

		// mvm hooks at the next op
		//utils::hook(0x46C8EB, CL_RunOncePerClientFrame_stub, HOOK_JUMP).install()->quick();

		// disable console drawing when taking screenshots using avidemo_streams
		//utils::hook(0x475060, Con_DrawSolidConsole_stub, HOOK_JUMP).install()->quick();

		// fix out of memory error when trying to take screenshots with R_TakeScreenshot when using heavy mods
		utils::hook::set<BYTE>(0x60E624 + 1, 0x2); // CreateOffscreenPlainSurface D3DPOOL_SCRATCH -> D3DPOOL_SYSTEMMEM
	}
}