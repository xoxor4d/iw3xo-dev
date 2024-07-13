#pragma once

namespace components
{
	class rtx_map_settings : public component
	{
	public:
		rtx_map_settings();
		~rtx_map_settings() = default;
		const char* get_name() override { return "rtx_map_settings"; }

		static inline rtx_map_settings* p_this = nullptr;
		static rtx_map_settings* get() { return p_this; }

		enum PARSE_MODE : std::uint32_t
		{
			SETTINGS,
			CULL,
			MARKER,
			API_VARS
		};

		struct cell_settings_s
		{
			//int cell_index = -1;
			std::vector<int> forced_cell_indices;
			bool active = false;
		};

		struct marker_settings_s
		{
			float origin[3];
			game::FxEffect* handle;
			bool active = false;
		};

		struct api_config_var
		{
			std::string variable;
			std::string value;
		};

		struct map_settings_s
		{
			std::string		mapname;
			float			fog_distance = 5000.0f;
			game::GfxColor	fog_color = { 0x00FFFFFF };
			game::vec3_t	sun_direction = { 75.0f, -15.0f, -35.0f };
			game::vec3_t	sun_color = { 255.0f, 255.0f, 255.0f };
			float			sun_intensity = 1.0f;
			int				sky_index = rtx_gui::SKY::SUNSET;
			std::vector<cell_settings_s> cell_settings;
			bool			cell_overrides_exist = false;
			std::vector<marker_settings_s> map_markers;
			std::vector<std::string> api_var_configs;
		};

		static inline const map_settings_s* settings() { return &m_loaded_map_settings; }
		void set_settings_for_loaded_map(bool reload_settings = false);

	private:
		static inline map_settings_s m_loaded_map_settings = {};
		static inline std::vector<map_settings_s> m_settings;
		static inline std::vector<std::string> m_args;

		map_settings_s* get_or_create_settings();
		void parse_culling();
		void parse_markers();
		void open_and_set_var_config(const std::string& config, bool warn_missing_conf = true);
		void parse_api_var_configs();
		void parse_settings();
		bool load_settings();
	};
}
