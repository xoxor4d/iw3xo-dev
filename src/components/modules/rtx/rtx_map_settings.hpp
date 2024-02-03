#pragma once

namespace components
{
	class rtx_map_settings : public component
	{
	public:
		rtx_map_settings();
		~rtx_map_settings() = default;
		const char* get_name() override { return "rtx_map_settings"; }

		static rtx_map_settings* p_this;
		static rtx_map_settings* get();

		static inline float m_max_distance = 5000.0f;
		static inline game::GfxColor m_color = { 0x00FFFFFF };

		void set_settings_for_loaded_map(bool reload_settings = false);

	private:
		struct settings_s
		{
			std::string mapname;
			int skybox;
			float max_distance = 5000.0f;
			game::GfxColor m_color = { 0x00FFFFFF };
		};

		static inline std::vector<settings_s> m_settings;
		bool load_settings();
	};
}
