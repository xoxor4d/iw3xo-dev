#pragma once

namespace components
{
	class rtx_api final : public component
	{
	public:
		rtx_api();
		~rtx_api();
		const char* get_name() override { return "rtx_api"; };

		static inline rtx_api* p_this = nullptr;
		static rtx_api* get() { return p_this; }

		rpc::client* get_client() const { return g_client.get(); }
		bool is_ready() const { return g_initialized.load(); }

	private:
		void initialize_rpc_client();

		std::unique_ptr<rpc::client> g_client;
		std::atomic<bool> g_initialized;
		std::thread g_init_thread;
		static inline std::mutex g_init_mutex;
	};
}
