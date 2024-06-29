#include "std_include.hpp"

namespace components
{
	void rtx_api::initialize_rpc_client()
	{
		try
		{
			g_client = std::make_unique<rpc::client>("localhost", 8080);
			std::lock_guard lock(g_init_mutex);
			g_initialized.store(true);
		}
		catch (const std::exception& e)
		{
			std::cerr << "[RTX API] Failed to init the rpc client: " << e.what() << "\n";
		}
	}

	rtx_api::rtx_api()
	{
		p_this = this;
		g_client = nullptr;
		g_initialized = false;

		DEBUG_PRINT("[RTX API] Creating the rpc client .. \n")
		std::cout << "[RTX API] Creating the rpc client .." << "\n";
		g_init_thread = std::thread(&rtx_api::initialize_rpc_client, this);
		g_init_thread.detach();

		// #
		// #

		command::add("foo", "", "calls the x64 bit process func foo", [this]([[maybe_unused]] command::params parms)
		{
			if (const auto api = rtx_api::get(); api->is_ready())
			{
				try
				{
					auto result = api->get_client()->call("foo").get();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[RPC] call failed: " << e.what() << "\n";
				}
			}
			else
			{
				std::cerr << "[RPC] Client not initialized." << "\n";
			}
		});
	}

	rtx_api::~rtx_api()
	{
		if (is_ready())
		{
			std::lock_guard lock(g_init_mutex);

			try
			{
				g_client->wait_all_responses();
			}
			catch (const std::exception& e)
			{
				std::cerr << "[RTX API] Failed closing the rpc client: " << e.what() << "\n";
			}

			if (g_init_thread.joinable()) 
			{
				g_init_thread.join();
			}

			g_initialized.store(false);
		}
	}
}