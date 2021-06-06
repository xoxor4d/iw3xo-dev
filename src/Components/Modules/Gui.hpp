#pragma once

namespace Components
{
	class Gui : public Component
	{
	public:
		Gui();
		~Gui();

		const char* getName() override { return "Gui"; };

		static void imgui_init();
		static void reset();
		static void update_hWnd(void* hwnd);
		static void begin_frame();
		static void end_frame(); 

		static void render_loop();
		static void toggle(Game::gui_menus_t& menu, int keycatcher, bool onCommand);

		static void toggle_mouse(bool state);
		static void reset_mouse();
		static void redraw_cursor();

		static void set_menu_layout(Game::gui_menus_t& menu, const float x, const float y, const float width, const float height, const int horzAlign, const int vertAlign);

		static void load_settings();
		static void save_settings();
		static bool any_open_menus();

		//static void Markdown(const std::string& markdown_);
		static void markdown(const char* str, const char* str_end);

		static Game::gui_menus_t&	GetMenu(Game::GUI_MENUS id);

		// *
		// set latched values + modified flag and return value pointer 
		template <typename T>
		static T DvarGetSet(Game::dvar_s* dvar, T swap_value = nullptr)
		{
			if (dvar)
			{
				auto vecSize = 0;

				switch (dvar->type)
				{
				case Game::dvar_type::boolean:
					dvar->latched.enabled = dvar->current.enabled;
					dvar->modified = true;
					return reinterpret_cast<T>(&dvar->current.enabled);

				case Game::dvar_type::value:
					dvar->latched.value = dvar->current.value;
					dvar->modified = true;
					return reinterpret_cast<T>(&dvar->current.value);

				case Game::dvar_type::enumeration:
				case Game::dvar_type::integer:
					dvar->latched.integer = dvar->current.integer;
					dvar->modified = true;
					return reinterpret_cast<T>(&dvar->current.integer);

				case Game::dvar_type::rgb:
				case Game::dvar_type::color:
					Game::Com_Error(1, "dvar type rgb | color not supported!");
					return nullptr;

				default:
					Game::Com_Error(1, "unhandled dvar_type!");
					return nullptr;
	

				case Game::dvar_type::vec2:
					vecSize = 2;
					break;
				case Game::dvar_type::vec3:
					vecSize = 3;
					break;
				case Game::dvar_type::vec4:
					vecSize = 4;
					break;
				}

				// vectors here
				Utils::vector::_VectorCopy(dvar->current.vector, dvar->latched.vector, vecSize);
				dvar->modified = true;
				return reinterpret_cast<T>(dvar->current.vector);
			}

			Game::Com_Error(1, "dvar was null!");
			return nullptr;
		}

       

	private:
	};
}
