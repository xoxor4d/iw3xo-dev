#include "std_include.hpp"

namespace components
{
	// on renderer initialization
	void print_loaded_modules()
	{
		ShowWindow(GetConsoleWindow(), SW_HIDE);


		game::Com_PrintMessage(0, utils::va("-------------- Loaded Modules -------------- \n%s\n", game::glob::loaded_modules.c_str()), 0);

		// add FS Path output print
		if (const auto& dedicated = game::Dvar_FindVar("dedicated"); 
						dedicated && dedicated->current.integer == 0)
		{
			game::FS_DisplayPath(1);
		}
	}

	__declspec(naked) void CL_PreInitRenderer_stub()
	{
		const static uint32_t CL_PreInitRenderer_func = 0x46CCB0;
		const static uint32_t retn_addr = 0x46FD05;
		__asm
		{
			pushad;
			call	print_loaded_modules;
			popad;

			call	CL_PreInitRenderer_func;
			jmp		retn_addr;
		}
	}

	// r_init
	__declspec(naked) void console_printfix_stub_01()
	{
		const static uint32_t retn_addr = 0x5F4EE6;
		const static char* print = "\n-------------- R_Init --------------\n";
		__asm
		{
			push	print;
			jmp		retn_addr;
		}
	}

	// working directory
	__declspec(naked) void console_printfix_stub_02()
	{
		const static uint32_t retn_addr = 0x5776A2;
		const static char* print = "Working directory: %s\n\n";
		__asm
		{
			push	print;
			jmp		retn_addr;
		}
	}

	// server initialization
	__declspec(naked) void console_printfix_stub_03()
	{
		const static uint32_t retn_addr = 0x52F3F3;
		const static char* print = "\n------- Server Initialization ---------\n";
		__asm
		{
			push	print;
			jmp		retn_addr;
		}
	}

	// helper function because cba to do that in asm
	void print_build_on_init() 
	{
		game::Com_PrintMessage(0, "\n-------- Game Initialization ----------\n", 0);
		game::Com_PrintMessage(0, utils::va("> Build: iw3xo %s :: %s\n", VERSION, __TIMESTAMP__), 0);
	}

	// game init + game name and version
	__declspec(naked) void console_printfix_stub_04()
	{
		const static uint32_t retn_addr = 0x4BF04F;
		__asm
		{
			pushad;
			call	print_build_on_init;
			popad;

			jmp		retn_addr;
		}
	}
	
	patches::patches()
	{
		// Force debug logging
		utils::hook::nop(0x4FCB9D, 8);

		// Enable console log
		utils::hook::nop(0x4FCBA3, 2);

		// Ignore hardware changes
		utils::hook::set<DWORD>(0x57676B, 0xC359C032);
		utils::hook::set<DWORD>(0x576691, 0xC359C032);

		// Remove improper quit popup
		utils::hook::set<BYTE>(0x5773F6, 0xEB);

		// Fix microphone shit
		utils::hook::set<BYTE>(0x57AB09, 0xEB);
		utils::hook::nop(0x4ED366, 5);

		// Disable AutoUpdate Check?
		utils::hook::nop(0x4D76DA, 5);

		// Disable developer check for Alt + Enter
		utils::hook::nop(0x57BEBD, 2);

		// Precaching beyond level load
		utils::hook::nop(0x4E2216, 2); // model 1
		utils::hook::set<BYTE>(0x4E2282, 0xEB); // model 2

		// (c) Snake :: Fix mouse lag by moving SetThreadExecutionState call out of event loop
		utils::hook::nop(0x57BB2C, 8);
		scheduler::on_frame([]()
		{
			SetThreadExecutionState(ES_DISPLAY_REQUIRED);
		});


		// *
		// Console prints

		// Print loaded modules to console
		utils::hook(0x46FD00, CL_PreInitRenderer_stub, HOOK_JUMP).install()->quick();
		
		// Remove "setstat: developer_script must be false"
		utils::hook::nop(0x46FCFB, 5);

		// Add newlines 
		utils::hook(0x5F4EE1, console_printfix_stub_01, HOOK_JUMP).install()->quick();
		utils::hook(0x57769D, console_printfix_stub_02, HOOK_JUMP).install()->quick();
		utils::hook(0x52F3EE, console_printfix_stub_03, HOOK_JUMP).install()->quick();
		utils::hook(0x4BF04A, console_printfix_stub_04, HOOK_JUMP).install()->quick(); 
		utils::hook::nop(0x4BF05B, 5); // gamename
		utils::hook::nop(0x4BF06C, 5); // gamedate

		// 7263D4
		//utils::hook::set<BYTE>(0x7263D4, 0x2);
		//utils::hook::set<BYTE>(0x7263D8, 0x2);

		// *
		// bouncepatch stuff

		//utils::hook::set<BYTE>(0x40E3FA, 0xEB); utils::hook::set<BYTE>(0x40E3FA + 1, 0x20); // jnp 0040E41C -> jmp 0040E41C = Bounce on everything - projVelocity
		//utils::hook::set<BYTE>(0x40E3E8, 0xD9); utils::hook::set<BYTE>(0x40E3E8 + 1, 0xEB); // fld1 to fldpi - projVelocity
		//utils::hook::set<FLOAT>(0x6D8D84, 0.005f); // Float 0.3 -> 0.005 = ignore angles - stepslidemove

		// elevator slide along all edges
		//utils::hook::set<BYTE>(0x4103E2, 0x01);


		// *
		// Fix fps on windows 10 (stuck at 500) :: sleep(1) to sleep(0)

		utils::hook::set<BYTE>(0x500014 + 1, 0x0);
		utils::hook::set<BYTE>(0x50007F + 1, 0x0); 

#if 0
		// *
		// patch shadowmap

		Game::Dvar_RegisterFloat(
			/* name		*/ "sm_sunShadowScale",
			/* desc		*/ "Overwritten non-cheat sm_sunShadowScale",
			/* default	*/ 1.0f,
			/* minVal	*/ 0.01f,
			/* maxVal	*/ 8.0f,
			/* flags	*/ Game::dvar_flags::none);

		Game::Dvar_RegisterFloat(
			/* name		*/ "sm_sunSampleSizeNear",
			/* desc		*/ "Overwritten non-cheat sm_sunSampleSizeNear",
			/* default	*/ 0.25f,
			/* minVal	*/ 0.0001f,
			/* maxVal	*/ 64.0f,
			/* flags	*/ Game::dvar_flags::none);

		// R_InitRenderTargets :: shadowmapSun rendertarget size to 2048
		//utils::hook::set<BYTE>(0x63082B + 2, 0x08);
#endif

		command::add("xasset_spworld", "[optional] <offset array by num>", "creates a node_dump (sp maps only)", [this](command::params parms)
		{
			int node_array_offset = 0;

			if (parms.length() >= 2)
			{
				node_array_offset = utils::try_stoi(parms[1], true);
			}

			game::GameWorldSp* sp = game::DB_FindXAssetHeader(game::ASSET_TYPE_GAMEWORLD_SP, game::cm->name).gameWorldSp;

			if (const auto& fs_basepath = game::Dvar_FindVar("fs_basepath");
							fs_basepath)
			{
				std::string base_path = fs_basepath->current.string;
							base_path += "\\iw3xo\\spworld\\";

				if (std::filesystem::create_directories(base_path))
				{
					game::Com_PrintMessage(0, "Created directory \"root/iw3xo/spworld\"\n", 0);
				}

				std::ofstream nodes;

				// steam to .map file
				nodes.open((base_path + "node_dump.txt").c_str());

				nodes << "// generated by iw3xo - https://github.com/xoxor4d/iw3xo-dev \n";
				nodes << "// spworld pathnode dump\n" << std::endl;;

				nodes << "// total nodes = " << sp->path.nodeCount << std::endl;
				nodes << "// node num, origin, num links, links to" << std::endl;

				
				for (auto n = 0u; n < sp->path.nodeCount; n++)
				{
					// check for additional link via target - targetname
					int link_to = -1;
					int link_from = -1;

					{
						// if current node has a target
						if (sp->path.nodes[n].constant.target)
						{
							// search all nodes for fitting targetname
							for (auto lx = 0u; lx < sp->path.nodeCount; lx++)
							{
								// check if node matches target
								if (sp->path.nodes[lx].constant.targetname == sp->path.nodes[n].constant.target)
								{
									bool already_connected = false;

									// check if node is not already linked because its nearby
									for (auto link = 0u; link < static_cast<std::uint32_t>(sp->path.nodes[n].constant.totalLinkCount); link++)
									{
										if (sp->path.nodes[n].constant.Links[link].nodeNum == lx)
										{
											already_connected = true;
											break;
										}
									}

									if (!already_connected)
									{
										link_to = node_array_offset + static_cast<int>(lx);
									}

									break;
								}
							}
						}

						// if current node has a targetname
						if (sp->path.nodes[n].constant.targetname)
						{
							// search all nodes for fitting target
							for (auto lx = 0u; lx < sp->path.nodeCount; lx++)
							{
								// check if node matches targetname
								if (sp->path.nodes[lx].constant.target == sp->path.nodes[n].constant.targetname)
								{
									bool already_connected = false;

									// check if node is not already linked because its nearby
									for (auto link = 0u; link < static_cast<std::uint32_t>(sp->path.nodes[n].constant.totalLinkCount); link++)
									{
										if (sp->path.nodes[n].constant.Links[link].nodeNum == lx)
										{
											already_connected = true;
											break;
										}
									}

									if (!already_connected)
									{
										link_from = node_array_offset + static_cast<int>(lx);
									}

									break;
								}
							}
						}
					}

					const auto static_num_links = static_cast<std::uint32_t>(sp->path.nodes[n].constant.totalLinkCount);

					// inc num links
					int pp = 0;
					if (link_from != -1) pp++;
					if (link_to != -1) pp++;

					const auto dynamic_num_links = static_num_links + pp;

					nodes << n + node_array_offset << ", "
						<< std::fixed
						<< std::setprecision(2)
						<< sp->path.nodes[n].constant.vOrigin[0] << " "
						<< sp->path.nodes[n].constant.vOrigin[1] << " "
						<< sp->path.nodes[n].constant.vOrigin[2] << ", " << dynamic_num_links;

					nodes << ",";

					for (auto link = 0u; link < static_num_links; link++)
					{
						nodes << " ";
						nodes << sp->path.nodes[n].constant.Links[link].nodeNum + node_array_offset;
					}

					// add target-targetname link

					if (link_to != -1)
					{
						nodes << " ";
						nodes << link_to;
					}

					if (link_from != -1)
					{
						nodes << " ";
						nodes << link_from;
					}

					nodes << std::endl;
				}
				
				game::Com_PrintMessage(0, utils::va("dumped spworld pathnodes to: %s", (base_path + "node_dump.txt").c_str()), 0);
				nodes.close();
			}
		});
	}
}
