#include "std_include.hpp"

#define CON_KEY_TOGGLE_CURSOR	KEYCATCHER_F1
#define CON_KEY_RESET_FLTCON	KEYCATCHER_F2
#define GET_CONSOLEFONT (game::Font_s*) *(DWORD*)(game::con_font_ptr)

const char* CON_HELP_PRINT =	"---------------- CONSOLE HELP ----------------------\"^2help^7\"--------------------------\n"
								"!     F1 :: Toggle cursor in-game\n"
								"!     F2 :: Reset floating console\n"
								"! Mouse1 :: Press and hold mouse1 on the input bar to move / resize the console\n"
								"! xo_con :: Prefix for additional console dvars\n"
								"------------------------------------------------------------------------------------\n"
								"\n";

// init console addon struct
game::Console_Addons console_addon_ = game::Console_Addons();

static const char* console_mapdir_ = "maps/mp";
static const char* console_mapext_ = "autocomplete"; // change d3dbsp to autocomplete

namespace components
{
	#pragma region CON-UTILS // ++++++

	int console::cmd_argc()
	{
		return game::cmd_args->argc[game::cmd_args->nesting];
	}

	bool is_dvar_command(char* cmd)
	{
		if (!_stricmp(cmd, "set"))
		{
			return true;
		}

		if (!_stricmp(cmd, "seta"))
		{
			return true;
		}

		if (!_stricmp(cmd, "sets"))
		{
			return true;
		}

		if (!_stricmp(cmd, "reset"))
		{
			return true;
		}

		if (_stricmp(cmd, "toggle"))
		{
			return _stricmp(cmd, "togglep") == false;
		}

		return true;
	}

	// box drawing helper function -- depth material
	void draw_rect(float* color, float x, float y, float w, float h, bool depth, bool ignore_fullscreen = false )
	{
		game::Material* material;

		if (depth && dvars::xo_con_useDepth->current.enabled) 
		{ 
			// depth input
			material = game::Material_RegisterHandle("floatz_display", 3); 
		}
		else 
		{
			// stock
			material = game::Material_RegisterHandle("white", 3); 
		}

		if (!game::con->outputVisible || ignore_fullscreen) 
		{
			// small console
			game::R_AddCmdDrawStretchPic(material, x, y, w, h, 0.0f, 0.0f, 0.0f, 0.0f, color);
		}
		else 
		{
			// fullscreen
			game::R_AddCmdDrawStretchPic(material, x, y, w, h, 0.0f, 0.0f, 0.0f, 0.0f, color);
		}
	}

	// register font and draw text
	void draw_input_text(float x, float y, float scale_x, float scale_y, const char* font, const float* color, const char* text)
	{
		void* font_handle = game::R_RegisterFont(font, sizeof(font));
		game::R_AddCmdDrawTextASM(text, 0x7FFFFFFF, font_handle, x, y, scale_x, scale_y, 0.0f, color, 0);
	}

	// draw text
	void draw_input_text(float x, float y, float scale_x, float scale_y, game::Font_s* font, const float* color, const char* text)
	{
		game::R_AddCmdDrawTextASM(text, 0x7FFFFFFF, font, x, y, scale_x, scale_y, 0.0f, color, 0);
	}

	#pragma endregion

	#pragma region CON-DRAW // ++++++

	// used for mid hooks where we changed conDrawInputGlob->y prior call to offset text, but we need to adjust the box we draw (within the hook)
	float  console_input_glob_y_fuckery_ = 15.0f; // 16
	float *console_glob_color_;

	// mid-func hook to alter hintbox drawing (upper box with dvar name and value)
	void draw_rect_detailed_match_upper(float* color, int lines)
	{
		// small console offsets --------------------------------------------------------
		// upper.x and upper.w get set in "DrawInput" :: Exact Match - Draw Details
		console_addon_.items.s_con.hint_box_upper.y = game::conDrawInputGlob->y - 8.0f - 15.0f + console_input_glob_y_fuckery_;
		console_addon_.items.s_con.hint_box_upper.h = static_cast<float>(lines) * game::conDrawInputGlob->fontHeight + 16.0f;

		// full console offsets --------------------------------------------------------
		// upper.x and upper.w get set in "DrawInput" :: Exact Match - Draw Details
		console_addon_.items.f_con.hint_box_upper.y = game::conDrawInputGlob->y - 10.0f - 11.0f + console_input_glob_y_fuckery_;
		console_addon_.items.f_con.hint_box_upper.h = static_cast<float>(lines) * game::conDrawInputGlob->fontHeight + 16.0f;//+ 4.0f;


		// get dimensions of upper + lower hint box (needed for clipping)
		// get upper box ::

		// small console total hintbox part 1/2 --------------------------------------------------------
		console_addon_.items.s_con.hint_box_total.x = console_addon_.items.s_con.hint_box_upper.x;
		console_addon_.items.s_con.hint_box_total.y = console_addon_.items.s_con.hint_box_upper.y;
		console_addon_.items.s_con.hint_box_total.w = console_addon_.items.s_con.hint_box_upper.w;
		console_addon_.items.s_con.hint_box_total.h = console_addon_.items.s_con.hint_box_upper.h;

		// full console total hintbox part 1/2 --------------------------------------------------------
		console_addon_.items.f_con.hint_box_total.x = console_addon_.items.f_con.hint_box_upper.x;
		console_addon_.items.f_con.hint_box_total.y = console_addon_.items.f_con.hint_box_upper.y;
		console_addon_.items.f_con.hint_box_total.w = console_addon_.items.f_con.hint_box_upper.w;
		console_addon_.items.f_con.hint_box_total.h = console_addon_.items.f_con.hint_box_upper.h;
			
		draw_rect(
		/*    c		*/ game::con->outputVisible ? console_glob_color_ : color,
		/*	  x		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.x : console_addon_.items.s_con.hint_box_upper.x,
		/*	  y		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.y : console_addon_.items.s_con.hint_box_upper.y,
		/*	  w		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.w : console_addon_.items.s_con.hint_box_upper.w,
		/*	  h		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.h : console_addon_.items.s_con.hint_box_upper.h,
		/*  depth	*/ false,
		/* ignoreFS	*/ true );


		// hintbox shadow --------------------------------------------------------
		float shadow_color[4] = { 0.0f, 0.0f, 0.0f, 0.125f };

		draw_rect(
			/*    c		*/ shadow_color,
			/*	  x		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.x : console_addon_.items.s_con.hint_box_upper.x,
			/*	  y		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.y : console_addon_.items.s_con.hint_box_upper.y,
			/*	  w		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.w : console_addon_.items.s_con.hint_box_upper.w,
			/*	  h		*/ 3.0f,
			/*  depth	*/ false,
			/* ignoreFS	*/ true);
	}

	// mid-func hook to alter hintbox drawing (lower box with dvar description) -- gets called after ConDrawInput_Box_DetailedMatch_UpperBox
	void draw_rect_detailed_match_lower(float* color, int lines)
	{
		// Game::conDrawInputGlob->y changes somewhere between ConDrawInput_Box_DetailedMatch_UpperBox & ConDrawInput_Box_DetailedMatch_LowerBox
		// so we only add Game::conDrawInputGlob->y to our lower hint box

		// small console offsets --------------------------------------------------------
		console_addon_.items.s_con.hint_box_lower.x = console_addon_.items.s_con.hint_box_upper.x;
		console_addon_.items.s_con.hint_box_lower.y = game::conDrawInputGlob->y - 8.0f - 15.0f + console_input_glob_y_fuckery_;
		console_addon_.items.s_con.hint_box_lower.h = lines * game::conDrawInputGlob->fontHeight + 16.0f;
		console_addon_.items.s_con.hint_box_lower.w = console_addon_.items.s_con.hint_box_upper.w;

		// full console offsets --------------------------------------------------------
		console_addon_.items.f_con.hint_box_lower.x = console_addon_.items.f_con.hint_box_upper.x;
		console_addon_.items.f_con.hint_box_lower.y = game::conDrawInputGlob->y - 10.0f - 11.0f + console_input_glob_y_fuckery_;
		console_addon_.items.f_con.hint_box_lower.h = lines * game::conDrawInputGlob->fontHeight + 16.0f;// + 4.0f;
		console_addon_.items.f_con.hint_box_lower.w = console_addon_.items.f_con.hint_box_upper.w;


		// get dimensions of upper + lower hint box (needed for clipping)
		// add lower box to upper box ::

		// small console total hintbox height part 2/2 --------------------------------------------------------
		console_addon_.items.s_con.hint_box_total.h += console_addon_.items.s_con.hint_box_lower.h;

		// full console total hintbox height part 2/2 --------------------------------------------------------
		console_addon_.items.f_con.hint_box_total.h += console_addon_.items.f_con.hint_box_lower.h;


		draw_rect(
		/*    c		*/ game::con->outputVisible ? console_glob_color_ : color,
		/*	  x		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_lower.x : console_addon_.items.s_con.hint_box_lower.x,//own_x,
		/*	  y		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_lower.y : console_addon_.items.s_con.hint_box_lower.y,//own_y,
		/*	  w		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_lower.w : console_addon_.items.s_con.hint_box_lower.w,//own_w,
		/*	  h		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_lower.h : console_addon_.items.s_con.hint_box_lower.h,//own_h,
		/*  depth	*/ false,
		/* ignoreFS	*/ true);
	}

	bool is_cursor_within_bounds(glm::vec2 bounds, const game::conAddons_CursorPos* cursor_pos)
	{
		if (cursor_pos->x < 0.0f || cursor_pos->x > bounds.x) 
		{
			return true;
		}

		if (cursor_pos->y < 0.0f || cursor_pos->y > bounds.y) 
		{
			return true;
		}

		return false;
	}

	bool is_cursor_within_item_box_bounds(const game::conItem_box* box, const game::conAddons_CursorPos* cursorPos)
	{
		// top left pt		= 0, 0	// top right pt		= 1, 0
		// bottom left pt	= 0, 1	// bottom right pt	= 1, 1

		// if cursor to right of left pt
		if (cursorPos->x > box->x) 
		{
			// if cursor to left of right pt
			if (cursorPos->x < (box->x + box->w)) 
			{
				// if cursor below top pt
				if (cursorPos->y > box->y) 
				{
					// if cursor above bottom pt
					if (cursorPos->y < (box->y + box->h)) 
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	void clip_console_against_borders()
	{
		// only clip the console when its open
		if (!(game::clientUI->keyCatchers & 1)) 
		{
			return;
		}

		// -------------------------------------------
		// clip the console against the window borders

		// clip at left border
		if (console_addon_.floating.anker.left <= 0.0f)
		{
			console_addon_.floating.anker.left = 0.0f;
			console_addon_.floating.clipped_at_border.left = true;

			// if we resize by pushing into the left border or with the resize button, limit the width to min width
			if (console_addon_.floating.anker.left + console_addon_.floating.anker.right < console_addon_.floating.min_dimensions.width) 
			{
				console_addon_.floating.anker.right = console_addon_.floating.min_dimensions.width;
			}
		}
		else 
		{
			// not clipped at border
			console_addon_.floating.clipped_at_border.left = false;
		}

		// min width check on resizing with resize button
		if (console_addon_.floating.anker.right - console_addon_.floating.anker.left < console_addon_.floating.min_dimensions.width) 
		{
			console_addon_.floating.anker.right = console_addon_.floating.min_dimensions.width + console_addon_.floating.anker.left;
		}

		// --------------------------------------------

		// clip at top border
		if (console_addon_.floating.anker.top <= 0.0f)
		{
			console_addon_.floating.anker.top = 0.0f;
			console_addon_.floating.clipped_at_border.top = true;
		}
		else 
		{
			// not clipped at border
			console_addon_.floating.clipped_at_border.top = false;
		}

		// --------------------------------------------

		// clip at right border
		if (console_addon_.floating.anker.right >= console_addon_.viewport_res.width)
		{
			console_addon_.floating.anker.right = console_addon_.viewport_res.width;
			console_addon_.floating.clipped_at_border.right = true;

			// if we resize by pushing into the right border or with the resize button, limit the width to min width
			if (console_addon_.floating.anker.right - console_addon_.floating.anker.left < console_addon_.floating.min_dimensions.width) 
			{
				console_addon_.floating.anker.left = console_addon_.floating.anker.right - console_addon_.floating.min_dimensions.width;
			}
		}
		else 
		{
			// not clipped at border
			console_addon_.floating.clipped_at_border.right = false;
		}

		// --------------------------------------------

		// clip at bottom border
		// we cannot clip the console at the bottom of the screen with con->screenMax[1] (bottom) 
		// because con->screenMax[1] is not used in any logic for the small console
		// so we clip con->screenMin[1] (top) instead

		// calculate the bottom position of the console and check if hits the bottom border of the screen
		if (console_addon_.floating.anker.top + console_addon_.items.s_con.output_box.h + console_addon_.items.s_con.input_box.h >= console_addon_.viewport_res.height)
		{
			// subtract from the bottom towards the top to get our min y
			console_addon_.floating.anker.top = console_addon_.viewport_res.height - console_addon_.items.s_con.output_box.h - console_addon_.items.s_con.input_box.h;
			console_addon_.floating.clipped_at_border.bottom = true;
		}
		else 
		{
			// not clipped at border
			console_addon_.floating.clipped_at_border.bottom = false;
		}
	}

	// fixes for consoleFont pixelHeight changes when loading / unloading mods / vid_restart?
	void check_resize_on_init()
	{
		// left HORIZONTAL_APPLY_LEFT
		game::con->screenMin[0] = floorf(_ui::scrplace_apply_x(HORIZONTAL_APPLY_LEFT, dvars::xo_con_padding->current.value, 0.0f));
		
		// top
		game::con->screenMin[1] = floorf(_ui::scrplace_apply_y(VERTICAL_APPLY_TOP, dvars::xo_con_padding->current.value, 0.0f));

		// right
		game::con->screenMax[0] = floorf(_ui::scrplace_apply_x(HORIZONTAL_APPLY_RIGHT, -dvars::xo_con_padding->current.value, 0.0f));

		// bottom
		game::con->screenMax[1] = floorf(_ui::scrplace_apply_y(VERTICAL_APPLY_BOTTOM, -dvars::xo_con_padding->current.value, 0.0f));

		// reinit floating console on vid_restart / cgame changes or w/e
		console_addon_.floating.initialized = false;

		// still needed as we do that in DrawInput? ... cba to test
		console_addon_.viewport_res.width = floorf(_ui::scrplace_apply_x(HORIZONTAL_APPLY_RIGHT, 0.0f, 0.0f));
		console_addon_.viewport_res.height = floorf(_ui::scrplace_apply_y(VERTICAL_APPLY_BOTTOM, 0.0f, 0.0f));

		// mouse unrelated stuff ---------------------------------------
		
		if (const auto  console_font = GET_CONSOLEFONT; 
						console_font)
		{
			if (console_font->pixelHeight != 16)
			{
				game::Com_PrintMessage(0, utils::va("Console :: consoleFont was %d", console_font->pixelHeight), 0);
			}

			game::con->fontHeight = static_cast<int>(( static_cast<float>(console_font->pixelHeight) * dvars::xo_con_fontSpacing->current.value) );
			game::con->visibleLineCount	= static_cast<int>( (game::con->screenMax[1] - game::con->screenMin[1] - static_cast<float>(2 * game::con->fontHeight)) / static_cast<float>(game::con->fontHeight) );
			game::con->visiblePixelWidth = static_cast<int>( game::con->screenMax[0] - game::con->screenMin[0] - -28.0f );
		}
		else
		{
			game::con->fontHeight = 0;
			game::con->visibleLineCount = 0;
			game::con->visiblePixelWidth = 0;
		}
	}

	// called from scheduler as we need 2 hooks at this place
	void console::check_resize()
	{
		// ------------------------------------------------------------
		// Cursor toggle key

		// settings
		const int CURSOR_TOGGLE_DELAY = 100;

		// only count till 300, start again when conAddon.cursorToggleTimeout was reset
		if (console_addon_.cursor_toggle_timeout < CURSOR_TOGGLE_DELAY) 
		{
			console_addon_.cursor_toggle_timeout++;
		}
		
		// if console is open
		if (game::clientUI->keyCatchers & 1) 
		{ 
			// if F1 and not showing cursor -> activate
			if (game::playerKeys->keys[CON_KEY_TOGGLE_CURSOR].down && !dvars::xo_con_cursorState->current.enabled && console_addon_.cursor_toggle_timeout >= CURSOR_TOGGLE_DELAY)
			{
				console_addon_.cursor_toggle_timeout = 0;
				game::dvar_set_value_dirty(dvars::xo_con_cursorState, true);
				game::UI_SetActiveMenu(0, 5);
			}

			// if F1 and showing cursor -> deactivate
			else if (game::playerKeys->keys[CON_KEY_TOGGLE_CURSOR].down && dvars::xo_con_cursorState->current.enabled && console_addon_.cursor_toggle_timeout >= CURSOR_TOGGLE_DELAY) 
			{
				goto DISABLE_CURSOR;
			}
		}
		else
		{
			// if console is not open but we still have the cursor menu open, close it
			if (dvars::xo_con_cursorState->current.enabled)
			{
				DISABLE_CURSOR:
				console_addon_.cursor_toggle_timeout = 0;
				game::dvar_set_value_dirty(dvars::xo_con_cursorState, false);

				game::UiContext *ui = &game::ui_context[0];
				game::Menus_CloseByName("pregame_loaderror_mp", ui);
			}
		}

		// ------------------------------------------------------------

		// one time init on start / vid_restart / cgame changes or w/e
		if (!console_addon_.floating.initialized)
		{
			console_addon_.viewport_res.width = floorf(_ui::scrplace_apply_x(HORIZONTAL_APPLY_RIGHT, 0.0f, 0.0f));
			console_addon_.viewport_res.height = floorf(_ui::scrplace_apply_y(VERTICAL_APPLY_BOTTOM, 0.0f, 0.0f));

			// min width / height of the floating console
			console_addon_.floating.min_dimensions.width = 840.0f;
			console_addon_.floating.min_dimensions.height = 470.0f;
		}

		// get scaled mouse cursor (640/480 <-> Game Resolution)
		console_addon_.cursor_pos.x = utils::float_to_range(0.0f, 640.0f, 0.0f, (float) game::ui_context->screenWidth,  game::ui_context->cursor.x);
		console_addon_.cursor_pos.y = utils::float_to_range(0.0f, 480.0f, 0.0f, (float) game::ui_context->screenHeight, game::ui_context->cursor.y);

		// enter if floating console state was saved to the config
		// otherwise skip the first frame to fully init the console
		// or skip if the fullscreen console is active
		if ((console_addon_.floating.initialized && !game::con->outputVisible) || (dvars::xo_con_fltCon->current.enabled && !game::con->outputVisible))
		{
			// if mouse down && console is actually open -- else use the last floating position
			if (game::playerKeys->keys[KEYCATCHER_MOUSE1].down && game::clientUI->keyCatchers & 1)
			{
				// ----------------
				// Console Resizing

				// check if the cursor is on the resize button -- we have to check resize first as the button is ontop of the grab bar
				if ((console_addon_.floating.is_resizing || is_cursor_within_item_box_bounds(&console_addon_.items.s_con.resize_button_trigger, &console_addon_.cursor_pos)) && !console_addon_.floating.is_moving)
				{
					// trigger on first click only 
					if (!console_addon_.floating.is_resizing)
					{
						// save the current cursor position for the next frame
						console_addon_.cursor_pos_saved.x = console_addon_.cursor_pos.x;
						console_addon_.cursor_pos_saved.y = console_addon_.cursor_pos.y;

						// the position where we hit the resize button
						console_addon_.cursor_pos_saved_on_click.x = console_addon_.cursor_pos.x;
						console_addon_.cursor_pos_saved_on_click.y = console_addon_.cursor_pos.y;
					}

					if (console_addon_.floating.is_resizing) {
						// can be used to skip something in the "first click frame"
					}

					// resizing console loop
					// ---------------------

					// update width / height
					console_addon_.floating.dimensions.width = console_addon_.floating.anker.right - console_addon_.floating.anker.left;
					console_addon_.floating.dimensions.height = console_addon_.floating.anker.bottom - console_addon_.floating.anker.top;

					// get difference between the cursor of the prior frame and the current pos and resize the console with that
					console_addon_.floating.anker.right -= console_addon_.cursor_pos_saved.x - console_addon_.cursor_pos.x;
					
					// change outputHeight downwards
					if (console_addon_.cursor_pos.y - console_addon_.cursor_pos_saved_on_click.y >= game::con->fontHeight * 0.85f)
					{
						console_addon_.cursor_pos_saved_on_click.y = console_addon_.cursor_pos.y;

						if (dvars::xo_con_outputHeight->current.integer + 1 <= dvars::xo_con_outputHeight->domain.integer.max) 
						{
							game::dvar_set_value_dirty(dvars::xo_con_outputHeight, dvars::xo_con_outputHeight->current.integer + 1);
						}
					}
					
					// change outputHeight upwards
					if (console_addon_.cursor_pos_saved_on_click.y - console_addon_.cursor_pos.y >= game::con->fontHeight * 0.85f)
					{
						console_addon_.cursor_pos_saved_on_click.y = console_addon_.cursor_pos.y;

						if (dvars::xo_con_outputHeight->current.integer - 1 >= dvars::xo_con_outputHeight->domain.integer.min) 
						{
							game::dvar_set_value_dirty(dvars::xo_con_outputHeight, dvars::xo_con_outputHeight->current.integer - 1);
						}
					}

					// update bottom anker
					console_addon_.floating.anker.bottom = console_addon_.floating.anker.top + console_addon_.items.s_con.output_box.h + console_addon_.items.s_con.input_box.h;

					// ---------------------------------------------------

					// save the current cursor position for the next frame
					console_addon_.cursor_pos_saved.x = console_addon_.cursor_pos.x;
					console_addon_.cursor_pos_saved.y = console_addon_.cursor_pos.y;

					console_addon_.floating.enabled		= true;
					console_addon_.floating.is_resizing	= true;
				}

				// -------------------
				// Console Translation

				// check if cursor is within the "grab" bar (input text box)
				// skip if the console is being moved
				else if((console_addon_.floating.is_moving || is_cursor_within_item_box_bounds(&console_addon_.items.s_con.input_box, &console_addon_.cursor_pos)) && !console_addon_.floating.is_resizing)
				{ 
					// trigger on first click only 
					if (!console_addon_.floating.is_moving)
					{
						// console is now floating
						console_addon_.floating.enabled = true;

						// get initial console position and dimensions if the floating console wasnt in use yet / was reset
						if (!console_addon_.floating.was_in_use || console_addon_.floating.was_reset)
						{
							console_addon_.floating.anker.left	= game::con->screenMin[0];
							console_addon_.floating.anker.top	= game::con->screenMin[1];
							console_addon_.floating.anker.right	= game::con->screenMax[0];
							console_addon_.floating.anker.bottom = game::con->screenMax[1];
						}
						
						// now the floating console will retain its last position even if the fullscreen console was active
						console_addon_.floating.was_in_use = true;
						console_addon_.floating.was_reset = false;

						// calculate width / height
						console_addon_.floating.dimensions.width = console_addon_.floating.anker.right - console_addon_.floating.anker.left;
						console_addon_.floating.dimensions.height = console_addon_.floating.anker.bottom - console_addon_.floating.anker.top;

						// save the current cursor position for the next frame
						console_addon_.cursor_pos_saved.x = console_addon_.cursor_pos.x;
						console_addon_.cursor_pos_saved.y = console_addon_.cursor_pos.y;
					}

					if (console_addon_.floating.is_moving) { 
						// can be used to skip something in the "first click frame"
					}

					// draggin console loop
					// ---------------------------------------------------

					// update width / height
					console_addon_.floating.dimensions.width  = console_addon_.floating.anker.right - console_addon_.floating.anker.left;
					console_addon_.floating.dimensions.height = console_addon_.floating.anker.bottom - console_addon_.floating.anker.top;

					// update floating console position every frame / resize it by clipping at borders
					// get difference between the cursor of the prior frame and the current pos and translate the console with that
					
					console_addon_.floating.anker.left	-= console_addon_.cursor_pos_saved.x - console_addon_.cursor_pos.x;
					console_addon_.floating.anker.top	-= console_addon_.cursor_pos_saved.y - console_addon_.cursor_pos.y;
					console_addon_.floating.anker.right	-= console_addon_.cursor_pos_saved.x - console_addon_.cursor_pos.x;

					// ---------------------------------------------------

					// save the current cursor position for the next frame
					console_addon_.cursor_pos_saved.x = console_addon_.cursor_pos.x;
					console_addon_.cursor_pos_saved.y = console_addon_.cursor_pos.y;

					// clip the console against screen bounds
					clip_console_against_borders();

					// ---------------------------------------------------

					// update bottom anker
					console_addon_.floating.anker.bottom = console_addon_.floating.anker.top + console_addon_.items.s_con.output_box.h + console_addon_.items.s_con.input_box.h;

					// console grabbed with cursor (setting this here allows us to skip something in the "first click frame" )
					console_addon_.floating.is_moving = true;
				}
			}

			// non moving floating console loop
			// ---------------------------------------------------

			// if mouse is not hold down anymore after the console was dragged / resized -- save current position to config
			else if (!game::playerKeys->keys[KEYCATCHER_MOUSE1].down && (console_addon_.floating.is_moving || console_addon_.floating.is_resizing))
			{
				console_addon_.floating.is_moving = false;
				console_addon_.floating.is_resizing = false;

				// set value doesnt really work here
				game::Dvar_SetFloat(dvars::xo_con_fltConLeft, console_addon_.floating.anker.left, 0);
				game::Dvar_SetFloat(dvars::xo_con_fltConTop, console_addon_.floating.anker.top, 0);
				game::Dvar_SetFloat(dvars::xo_con_fltConRight, console_addon_.floating.anker.right, 0);
				game::Dvar_SetFloat(dvars::xo_con_fltConBottom, console_addon_.floating.anker.bottom, 0);

				game::Cmd_ExecuteSingleCommand(0, 0, "xo_con_fltCon 1\n");
			}

			// clip the console against screen bounds when not moving the console or when we are resizing it
			if (!console_addon_.floating.is_moving || console_addon_.floating.is_resizing) 
			{
				clip_console_against_borders();
			}

			// if floating console was active and did not get reset, activate it to retain its last position
			if ((console_addon_.floating.was_in_use && !console_addon_.floating.was_reset) ) 
			{
				console_addon_.floating.enabled = true;
			}

			// if there is a saved state in the config file -- only on init
			if(dvars::xo_con_fltCon->current.enabled && !console_addon_.floating.initialized)
			{
				console_addon_.floating.anker.left	= dvars::xo_con_fltConLeft->current.value;
				console_addon_.floating.anker.top	= dvars::xo_con_fltConTop->current.value;
				console_addon_.floating.anker.right	= dvars::xo_con_fltConRight->current.value;
				console_addon_.floating.anker.bottom = dvars::xo_con_fltConBottom->current.value;

				// update width / height
				console_addon_.floating.dimensions.width = console_addon_.floating.anker.right - console_addon_.floating.anker.left;
				console_addon_.floating.dimensions.height = console_addon_.floating.anker.bottom - console_addon_.floating.anker.top;

				// update conAddon.fltCon.fltAnker.bottom
				//conAddon.fltCon.fltAnker.bottom = conAddon.fltCon.fltAnker.top + conAddon.conItems.s_con.outputBox.h + conAddon.conItems.s_con.inputBox.h;

				console_addon_.floating.enabled = true;
				console_addon_.floating.was_in_use = true;
				console_addon_.floating.was_reset = false;
			}
				
			// if the console is floating, update x/y and width/height here
			game::con->screenMin[0] = console_addon_.floating.anker.left;
			game::con->screenMin[1] = console_addon_.floating.anker.top;
			game::con->screenMax[0] = console_addon_.floating.anker.right;
			game::con->screenMax[1] = console_addon_.floating.anker.bottom;

			// fix for small hintbox (too many to display) getting stuck at the center after fullcon was used
			if (console_addon_.floating.enabled && !console_addon_.floating.is_moving) 
			{ 
				// update bottom anker
				console_addon_.floating.anker.bottom = console_addon_.floating.anker.top + console_addon_.items.s_con.output_box.h + console_addon_.items.s_con.input_box.h;
			}

			// draw a resize button with a trigger box bottom right
			// do not draw it here, but in some console drawing function as we are a frame behind
			// xo_con_DrawResizeBtn();
		}

		// reset floating console to aligned console with the END key while mouse 1 is not pressed
		if (game::playerKeys->keys[CON_KEY_RESET_FLTCON].down && !game::playerKeys->keys[KEYCATCHER_MOUSE1].down && console_addon_.floating.enabled || game::con->outputVisible)
		{
			// disable floating console when the fullscreen console is active
			console_addon_.floating.enabled = false;
			console_addon_.floating.is_moving = false;
			console_addon_.floating.is_resizing = false;

			game::Cmd_ExecuteSingleCommand(0, 0, "xo_con_fltCon 0\n");

			// only reset the floating console with the reset key and not when the fullscreen console is active
			if (!game::con->outputVisible) 
			{
				console_addon_.floating.was_reset = true;
			}
		}

		// do not set defaults when we load a saved floating console state on initialization
		// non floating console / fullscreen console position and dimensions
		if ((!console_addon_.floating.initialized && !dvars::xo_con_fltCon->current.enabled) || !console_addon_.floating.enabled || game::con->outputVisible)
		{
			// left HORIZONTAL_APPLY_LEFT
			game::con->screenMin[0] = floorf(_ui::scrplace_apply_x(HORIZONTAL_APPLY_LEFT, dvars::xo_con_padding->current.value, 0.0f));

			// top
			game::con->screenMin[1] = floorf(_ui::scrplace_apply_y(VERTICAL_APPLY_TOP, dvars::xo_con_padding->current.value, 0.0f));

			// right
			game::con->screenMax[0] = floorf(_ui::scrplace_apply_x(HORIZONTAL_APPLY_RIGHT, -dvars::xo_con_padding->current.value, 0.0f));

			// bottom
			game::con->screenMax[1] = floorf(_ui::scrplace_apply_y(VERTICAL_APPLY_BOTTOM, -dvars::xo_con_padding->current.value, 0.0f));

			// keep our struct updated even when the floating console isnt in use -- why did i do this?
			//conAddon.fltCon.fltAnker.left	= Game::con->screenMin[0];
			//conAddon.fltCon.fltAnker.top	= Game::con->screenMin[1];
			//conAddon.fltCon.fltAnker.right	= Game::con->screenMax[0];
			//conAddon.fltCon.fltAnker.bottom = Game::con->screenMax[1];
		}


		// mouse unrelated stuff ---------------------------------------
		
		if (const auto	consoleFont = GET_CONSOLEFONT; 
						consoleFont)
		{
			if (consoleFont->pixelHeight != 16)
			{
				game::Com_PrintMessage(0, utils::va("Console :: consoleFont was %d", consoleFont->pixelHeight), 0);
			}

			game::con->fontHeight = static_cast<int>( static_cast<float>(consoleFont->pixelHeight) * dvars::xo_con_fontSpacing->current.value );

			// adjust visibleLineCount for output text so we do not draw more lines then our rect can hold
			if (game::con->fontHeight) 
			{
				if (game::con->outputVisible) 
				{
					// full console
					game::con->visibleLineCount = static_cast<int>( (console_addon_.items.f_con.output_box.h - static_cast<float>(game::con->fontHeight * 2)) / static_cast<float>(game::con->fontHeight) );
				}
				else 
				{ 
					// small console
					game::con->visibleLineCount = static_cast<int>( (console_addon_.items.s_con.output_box.h - static_cast<float>(game::con->fontHeight)) / static_cast<float>(game::con->fontHeight) );
				}
			} 
			else if(DEBUG) 
			{
				game::Com_PrintMessage(0, utils::va("^1Con_CheckResize L#%d ^7:: con->fontHeight was NULL \n", __LINE__), 0);
			}

			game::con->visiblePixelWidth = static_cast<int>(game::con->screenMax[0] - game::con->screenMin[0] - -28.0f);
		}

		else
		{
			game::con->fontHeight = 0;
			game::con->visibleLineCount = 0;
			game::con->visiblePixelWidth = 0;
		}

		if (!console_addon_.floating.initialized && !console_addon_.floating.enabled)
		{
			// print help text on init
			game::Com_PrintMessage(0, "\n", 0);
			game::Com_PrintMessage(0, utils::va("%s", CON_HELP_PRINT), 0);
		}
		
		// floating console initialized
		console_addon_.floating.initialized = true;
	}

	// draw a resize button at the bottom right of the console input box
	void draw_resize_button(const float alpha)
	{
		glm::vec4 right, bottom;

		const float line_strength = 0.5f;
		float _alpha = alpha;

		if (_alpha + 0.2f <= 1.0f) 
		{
			_alpha += 0.2f;
		}

		float resize_button_color[4] = { 1.0f, 1.0f, 1.0f, _alpha };
		const float padding[2] = { 3.0f, 4.5f };

		right.x = console_addon_.items.s_con.input_box.x + console_addon_.items.s_con.input_box.w - padding[0] - line_strength;
		right.y = console_addon_.items.s_con.input_box.y + console_addon_.items.s_con.input_box.h * 0.5f; // draw from top to bottom -- half height of input bar
		right.z = line_strength; // line thickness
		right.w = console_addon_.items.s_con.input_box.h * 0.5f - padding[0];

		// right vertical bar
		game::ConDraw_Box(resize_button_color,
			/* x */	right.x,
			/* y */ right.y,
			/* w */ right.z,
			/* h */ right.w);

		bottom.x = console_addon_.items.s_con.input_box.x + console_addon_.items.s_con.input_box.w - (console_addon_.items.s_con.input_box.h * 0.5f); // draw from left to right so - its width
		bottom.y = console_addon_.items.s_con.input_box.y + console_addon_.items.s_con.input_box.h - padding[0] - 0.5f;
		bottom.z = console_addon_.items.s_con.input_box.h - 18.0f;
		bottom.w = 0.5f; // line thickness

		// bottom horizontal bar
		game::ConDraw_Box(resize_button_color,
			/* x */	bottom.x,
			/* y */ bottom.y,
			/* w */ bottom.z,
			/* h */ bottom.w);

		// ------------
		// small square

		resize_button_color[3] -= 0.25f;

		game::ConDraw_Box(resize_button_color,
			/* x */	right.x - line_strength - padding[1] - 3.0f,
			/* y */ bottom.y - padding[1] - 4.0f,
			/* w */ 4.0f,
			/* h */ 4.0f);

		console_addon_.items.s_con.resize_button_trigger.x = bottom.x - 2.0f;// +(bottom.z * 0.5f);
		console_addon_.items.s_con.resize_button_trigger.y = right.y - 2.0f;// +(bottom.w * 0.5f);
		console_addon_.items.s_con.resize_button_trigger.w = bottom.z + line_strength + 4.0f;
		console_addon_.items.s_con.resize_button_trigger.h = right.w + 5.0f;

		//float debugTriggerColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

		//// debug :: draw trigger
		//Game::ConDraw_Box(debugTriggerColor,
		//	/* x */	conAddon.conItems.s_con.resizeBtnTrigger.x,
		//	/* y */ conAddon.conItems.s_con.resizeBtnTrigger.y,
		//	/* w */ conAddon.conItems.s_con.resizeBtnTrigger.w,
		//	/* h */ conAddon.conItems.s_con.resizeBtnTrigger.h);
	}

	void draw_output_scrollbar(float x, float y, float width, float height, float padding = 0.0f)
	{
		const auto outputBarColor = game::Dvar_FindVar("con_outputBarColor")->current.vector;
		const auto outputSliderColor = game::Dvar_FindVar("con_outputSliderColor")->current.vector;

		const float background_width = 16.0f;
		const float background_pos_y = y + padding;
		const float background_pos_x = x + width - background_width;

		float background_height	= height;

		if (game::con->outputVisible) 
		{
			background_height -= padding;
		}

		const float nob_width = 16.0f;
		const float nob_min_height = 30.0f;
		const float nob_padding	= (background_width - nob_width) * 0.5f;

		// Background
		game::ConDraw_Box(outputBarColor,
			/* x */	background_pos_x,
			/* y */ background_pos_y,
			/* w */ background_width,
			/* h */ background_height);

		// if there is content out of bounds (scrolling enabled)
		if (game::con->consoleWindow.activeLineCount > game::con->visibleLineCount)
		{
			const int hidden_line_count = game::con->consoleWindow.activeLineCount - game::con->visibleLineCount;

			// current nob length
			const float portion	= (background_height * 0.01f) * static_cast<float>(hidden_line_count); // portion :: 1 percent of max nob height times invisible lines
			float nob_length = (background_height - portion);					// max nob height - portion;

			// if scaled nob length < min length
			if (nob_length < nob_min_height) 
			{
				nob_length = nob_min_height;
			}

			// current nob y position
			const float cursor_pos = static_cast<float>((game::con->displayLineOffset - game::con->visibleLineCount)) * (1.0f / static_cast<float>(hidden_line_count));
			float nob_y	= (y + height - nob_length - y) * utils::fmaxf3(cursor_pos, 0.0f, 1.0f) + y;

			// cap scrollbar nob top
			if (nob_y < background_pos_y) 
			{
				nob_y = background_pos_y;
			}

			game::ConDraw_Box(outputSliderColor,
				/* x */ background_pos_x + nob_padding,
				/* y */ nob_y,
				/* w */ nob_width - nob_padding,
				/* h */ nob_length);
		}
		else // nothing to scroll
		{
			game::ConDraw_Box(outputSliderColor,
				/* x */ background_pos_x + nob_padding,
				/* y */ background_pos_y,
				/* w */ nob_width - nob_padding,
				/* h */ background_height);
		}
	}

	void draw_output_version(float x, float y, float height)
	{
		std::string build = utils::va(IW3XO_BUILDSTRING);

		if (DEBUG) 
		{
			build += " :: ^1DEBUG";
		}

		game::SCR_DrawSmallStringExt(static_cast<int>(x), static_cast<int>(height - 16.0f + y), build.c_str());
	}

	void draw_output_text(float x, float y, float height = 0.0f)
	{
		// Game::con->visibleLineCount in Con_CheckResize defines the container-height (can be used as cutoff) 
		// rowIndex / rowIndex == top line (adjusting it cuts off the newest lines)
		// rowCount == lines at the bottom (^ oldest lines)

		bool first_offset_skipped = false;
		const auto font_height = static_cast<float>(game::con->fontHeight);

		int rowCount = game::con->visibleLineCount;
		int firstRow = game::con->displayLineOffset - game::con->visibleLineCount;

		const float lines_for_spacing = ((dvars::xo_con_fontSpacing->current.value + 0.2f) - 1.0f) * 10.0f;

		// no idea what i did here but it works
		y = y + height + font_height * 4.0f + font_height * 0.5f - lines_for_spacing * font_height * 0.5f;

		// adjust "cursor" 
		if (firstRow < 0)
		{
			y -= static_cast<float>(game::con->fontHeight * firstRow);

			rowCount = game::con->displayLineOffset;
			firstRow = 0;

			// first fullcon frame stuff here -- resets only when s_con was used
			if (game::con->outputVisible && !console_addon_.fcon_first_frame)
			{
				// if s_con was scrolled to the top, adjust the line offset once to put our fullcon text at the top
				game::con->displayLineOffset = game::con->visibleLineCount;
				console_addon_.fcon_first_frame = true;
			}
		}

		// get font handle
		game::Font_s* font_handle = game::R_RegisterFont(FONT_CONSOLE, sizeof(FONT_CONSOLE));

		for (auto row_index = 0; row_index < rowCount; ++row_index)
		{
			const auto line_index = (row_index + firstRow + game::con->consoleWindow.firstLineIndex) % game::con->consoleWindow.lineCount;
			
			// skip the first y offset as we add half a font height as padding
			if (first_offset_skipped) 
			{
				y += font_height;
			}
			else 
			{
				first_offset_skipped = true;
			}

			game::AddBaseDrawConsoleTextCmd(
				/*charCount	*/	game::con->consoleWindow.lines[line_index].textBufSize,
				/* colorFlt	*/	game::COLOR_WHITE,
				/* textPool	*/	game::con->consoleWindow.circularTextBuffer,
				/* poolSize	*/	game::con->consoleWindow.textBufSize,
				/* firstChar*/	game::con->consoleWindow.lines[line_index].textBufPos,
				/*   font	*/	font_handle,
				/*     x	*/	x,
				/*     y	*/	y,
				/*  xScale	*/	1.0f,
				/*  yScale	*/	1.0f,
				/*   style	*/	3); // 0 :: default -- 3 has slight shadows -- 6 even more :)
		}
	}

	void draw_output_window()
	{
		// output box
		console_addon_.items.f_con.output_box.x = console_addon_.items.f_con.input_box.x;
		console_addon_.items.f_con.output_box.y = console_addon_.items.f_con.input_box.y + console_addon_.items.f_con.input_box.h;
		console_addon_.items.f_con.output_box.w = console_addon_.items.f_con.input_box.w;
		console_addon_.items.f_con.output_box.h = game::con->screenMax[1] - console_addon_.items.f_con.output_box.y;

		const auto output_window_color = game::Dvar_FindVar("con_outputWindowColor")->current.vector;

		// output window
		draw_rect(
			/*	 color	*/	output_window_color,
			/*	   x	*/	console_addon_.items.f_con.output_box.x,
			/*	   y	*/	console_addon_.items.f_con.output_box.y,
			/*	   w	*/	console_addon_.items.f_con.output_box.w,
			/*	   h	*/	console_addon_.items.f_con.output_box.h,
			/*	depth	*/	false,
			/* ignoreFS	*/	true);

		// scrollbar
		draw_output_scrollbar(console_addon_.items.f_con.output_box.x, console_addon_.items.f_con.output_box.y, console_addon_.items.f_con.output_box.w, console_addon_.items.f_con.output_box.h, 0.0f);

		// build version long, bottom console
		draw_output_version(console_addon_.items.f_con.output_box.x + 6.0f, console_addon_.items.f_con.output_box.y - 4.0f, console_addon_.items.f_con.output_box.h);

		// console output below input text box
		draw_output_text(console_addon_.items.f_con.output_box.x + 6.0f, dvars::xo_con_padding->current.value * 2.0f, 0.0f);
	}

	void draw_detailed_cmd_match(game::cmd_function_s* cmd)
	{
		if (cmd)
		{
			if (game::Con_IsAutoCompleteMatch(cmd->name, game::conDrawInputGlob->inputText, game::conDrawInputGlob->inputTextLen)
				&& (!game::conDrawInputGlob->hasExactMatch || !cmd->name[game::conDrawInputGlob->inputTextLen]))
			{
				draw_rect_detailed_match_upper(game::Dvar_FindVar("con_inputHintBoxColor")->current.vector, (cmd->description ? 2 : 0) + 2);

				const auto console_font = GET_CONSOLEFONT;
				const float cmd_color[4] = { 0.8f, 0.8f, 1.0f, 1.0f };

				// draw cmd string
				draw_input_text(
					/* x	*/ game::conDrawInputGlob->x,
					/* y	*/ game::conDrawInputGlob->y + game::conDrawInputGlob->fontHeight,
					/* xScl	*/ 1.0f,
					/* yScl	*/ 1.0f,
					/* font	*/ console_font,
					/* colr	*/ cmd_color,
					/* text	*/ cmd->name);

				// draw type (cmd)
				draw_input_text(
					/* x	*/ game::conDrawInputGlob->x,
					/* y	*/ game::conDrawInputGlob->y + (game::conDrawInputGlob->fontHeight * 2),
					/* xScl	*/ 1.0f,
					/* yScl	*/ 1.0f,
					/* font	*/ console_font,
					/* colr	*/ game::con_matchtxtColor_domainDescription,
					/* text	*/ "  command");

				// draw arg usage
				if (cmd->args)
				{
					draw_input_text(
						/* x	*/ game::conDrawInputGlob->x + 16.0f + static_cast<float>(game::R_TextWidth(cmd->name, 256, console_font)),
						/* y	*/ game::conDrawInputGlob->y + game::conDrawInputGlob->fontHeight,
						/* xScl	*/ 1.0f,
						/* yScl	*/ 1.0f,
						/* font	*/ console_font,
						/* colr	*/ game::con_matchtxtColor_dvarDescription,
						/* text	*/ cmd->args);
				}

				// draw description
				if (cmd->description)
				{
					draw_input_text(
						/* x	*/ game::conDrawInputGlob->x,
						/* y	*/ game::conDrawInputGlob->y + (game::conDrawInputGlob->fontHeight * 4.0f),
						/* xScl	*/ 1.0f,
						/* yScl	*/ 1.0f,
						/* font	*/ console_font,
						/* colr	*/ game::con_matchtxtColor_dvarDescription,
						/* text	*/ cmd->description);
				}

				game::conDrawInputGlob->y += game::conDrawInputGlob->fontHeight;
				game::conDrawInputGlob->x = game::conDrawInputGlob->leftX;

				// skip autocomplete for cmds
			}
		}
	}
	
	// :: Main Console Input + Logic ( Key logic still outside in Console_Key() )
	void draw_input()
	{
		if (!(*game::Key_IsCatcherActive) || !game::Sys_IsMainThread()) 
		{
			return;
		}

		float	x, y, w, h;

		// hintbox txt color dvars
		memcpy(&game::con_matchtxtColor_currentDvar[0], &dvars::xo_con_hintBoxTxtColor_currentDvar->current.vector, sizeof(float[4]));
		memcpy(&game::con_matchtxtColor_currentValue[0], &dvars::xo_con_hintBoxTxtColor_currentValue->current.vector, sizeof(float[4]));
		memcpy(&game::con_matchtxtColor_defaultValue[0], &dvars::xo_con_hintBoxTxtColor_defaultValue->current.vector, sizeof(float[4]));
		memcpy(&game::con_matchtxtColor_dvarDescription[0], &dvars::xo_con_hintBoxTxtColor_dvarDescription->current.vector, sizeof(float[4]));
		memcpy(&game::con_matchtxtColor_domainDescription[0], &dvars::xo_con_hintBoxTxtColor_domainDescription->current.vector, sizeof(float[4]));

		// increase max drawing width for console input
		game::g_consoleField->drawWidth = 512;

		const auto con_inputBoxColor = game::Dvar_FindVar("con_inputBoxColor")->current.vector;

		// set con globals
		const auto console_font = GET_CONSOLEFONT;

		game::conDrawInputGlob->fontHeight = (float)(console_font->pixelHeight);
		game::conDrawInputGlob->x = game::con->screenMin[0];
		game::conDrawInputGlob->y = game::con->screenMin[1];
		game::conDrawInputGlob->leftX = game::conDrawInputGlob->x;
		
		// :: if small console
		if (!game::con->outputVisible)
		{
			// y = top of outputbox
			// h = height of outputbox

			x = game::conDrawInputGlob->x;
			y = game::conDrawInputGlob->y;
			w = game::con->screenMax[0] - game::con->screenMin[0] - (x - game::con->screenMin[0]);
			h = static_cast<float>((dvars::xo_con_outputHeight->current.integer + 4) * game::con->fontHeight); // add 4 because we get the first line @5

			// input box
			const float input_box_height = game::conDrawInputGlob->fontHeight + 15.0f;
			console_addon_.items.s_con.input_box.x = x;
			console_addon_.items.s_con.input_box.y = y + h - input_box_height;
			console_addon_.items.s_con.input_box.w = w;
			console_addon_.items.s_con.input_box.h = input_box_height;

			// output box
			console_addon_.items.s_con.output_box.x = x;
			console_addon_.items.s_con.output_box.y = y;
			console_addon_.items.s_con.output_box.w = w;
			console_addon_.items.s_con.output_box.h = h - console_addon_.items.s_con.input_box.h;

			// output box text
			console_addon_.items.s_con.output_text.x = game::conDrawInputGlob->x + 8.0f;
			console_addon_.items.s_con.output_text.y = y - h;
			console_addon_.items.s_con.output_text.h = h;

			// output box slider
			console_addon_.items.s_con.output_slider.x = x;
			console_addon_.items.s_con.output_slider.y = y;
			console_addon_.items.s_con.output_slider.w = w;
			console_addon_.items.s_con.output_slider.h = h - 32.0f;
			console_addon_.items.s_con.output_slider.padding = 0.0f;

			// ------------------------------ // output box
			draw_rect( 
			/* col */ game::Dvar_FindVar("con_outputWindowColor")->current.vector,
			/*  x  */ console_addon_.items.s_con.output_box.x,
			/*  y  */ console_addon_.items.s_con.output_box.y,
			/*  z  */ console_addon_.items.s_con.output_box.w,
			/*  h  */ console_addon_.items.s_con.output_box.h,
			/* dep */ true);

			// ------------------------------ // output box text
			draw_output_text(
			/*	  x		*/ console_addon_.items.s_con.output_text.x,
			/*	  y		*/ console_addon_.items.s_con.output_text.y,
			/* bxHeight	*/ console_addon_.items.s_con.output_box.h);

			// ------------------------------ // output box slider
			draw_output_scrollbar(
			/*  x  */ console_addon_.items.s_con.output_slider.x, 
			/*  y  */ console_addon_.items.s_con.output_slider.y, 
			/*  w  */ console_addon_.items.s_con.output_slider.w, 
			/*  h  */ console_addon_.items.s_con.output_slider.h,
			/* pad */ console_addon_.items.s_con.output_slider.padding);
			

			// offset input text:
			game::conDrawInputGlob->x += 5.0f;
			game::conDrawInputGlob->y += h - 24.0f;

			// ------------------------------ // input box
			draw_rect(
			/* col */ con_inputBoxColor,
			/*  x  */ console_addon_.items.s_con.input_box.x,
			/*  y  */ console_addon_.items.s_con.input_box.y,
			/*  z  */ console_addon_.items.s_con.input_box.w,
			/*  h  */ console_addon_.items.s_con.input_box.h,
			/* dep */ true);

			// draw a resize button with a trigger box bottom right
			// we have to call that after getting the newest inputBox values or else we lack a frame behind
			draw_resize_button(con_inputBoxColor[3]);

			// reset fullcon first frame toggle
			console_addon_.fcon_first_frame = false;
		}

		// :: if fullscreen console
		else
		{
			// y = top of outputbox
			// h = height of outputbox

			x = game::conDrawInputGlob->x;
			y = game::conDrawInputGlob->y;
			w = game::con->screenMax[0] - game::con->screenMin[0] - (x - game::con->screenMin[0]);
			h = game::conDrawInputGlob->fontHeight + 15.0f;

			// input box
			console_addon_.items.f_con.input_box.x = x;
			console_addon_.items.f_con.input_box.y = y;
			console_addon_.items.f_con.input_box.w = w;
			console_addon_.items.f_con.input_box.h = h;

			// ------------------------------ // input box
			draw_rect(
			/* col */ con_inputBoxColor,
			/*  x  */ console_addon_.items.f_con.input_box.x,
			/*  y  */ console_addon_.items.f_con.input_box.y,
			/*  w  */ console_addon_.items.f_con.input_box.w,
			/*  h  */ console_addon_.items.f_con.input_box.h,
			/* dep */ false);

			// output text box gets created in xo_con_DrawOutputWindow

			game::conDrawInputGlob->x += 5.0f;
			game::conDrawInputGlob->y += 5.0f;
		}

		// ------------------------------ // input text for small / full console
		draw_input_text(
		/*  x  */ game::conDrawInputGlob->x,
		/*  y  */ game::conDrawInputGlob->y + game::conDrawInputGlob->fontHeight + (game::con->outputVisible ? 2.0f : 0.0f),
		/* scX */ 0.75f,
		/* scY */ 0.75f,
		/* fon */ FONT_BIG,
		/* col */ game::COLOR_WHITE,
		/* txt */ utils::va("IW3 >"));

		// move cursor position
		game::conDrawInputGlob->x += 40.0f;

		// --------------------------------------------------------------------------- 
#pragma region CON-LOGIC

		game::conDrawInputGlob->leftX = game::conDrawInputGlob->x;
		const auto input_text_len_prev = game::conDrawInputGlob->inputTextLen;
		
		game::g_consoleField->widthInPixels = static_cast<int>(game::con->screenMax[0] - 6.0f - game::conDrawInputGlob->x);
		char* con_tokenized_input = game::Con_TokenizeInput();
		
		game::conDrawInputGlob->inputText = con_tokenized_input;
		auto input_text = strlen(con_tokenized_input);
		
		game::conDrawInputGlob->inputTextLen = static_cast<int>(input_text);
		game::conDrawInputGlob->autoCompleteChoice[0] = 0;

		int	 cmd_or_dvar = 0;

		auto var_con_original_command = con_tokenized_input;
		int  var_con_current_match_count = 0;
		int  var_con_is_dvar_command = 0;

		float output_slider_width = 16.0f;

		if (input_text_len_prev != static_cast<int>(input_text))
		{
			//Game::Con_CancelAutoComplete(); // Inlined ::
			if (game::conDrawInputGlob->matchIndex >= 0 && game::conDrawInputGlob->autoCompleteChoice[0]) // T5 way
			{
				game::conDrawInputGlob->matchIndex = -1;
				game::conDrawInputGlob->autoCompleteChoice[0] = 0;
			}

			input_text = game::conDrawInputGlob->inputTextLen;
			con_tokenized_input = (char*)game::conDrawInputGlob->inputText;
		}

		if (!input_text)
		{
			game::conDrawInputGlob->mayAutoComplete = false;

			// Input Cursor when input text is empty
			if (console_addon_.floating.initialized) 
			{
				game::Con_DrawInputPrompt(); 
			}

			game::Cmd_EndTokenizedString();
			goto REDRAW_CURSOR_RETURN;
		}

		var_con_original_command = con_tokenized_input;
		
		if (console::cmd_argc() > 1 && is_dvar_command(con_tokenized_input))
		{
			cmd_or_dvar = 1;
			game::conDrawInputGlob->inputText = game::Cmd_Argv(1); 
			game::conDrawInputGlob->inputTextLen = static_cast<int>(strlen(game::conDrawInputGlob->inputText)); 
			
			if (!game::conDrawInputGlob->inputTextLen)
			{
				game::conDrawInputGlob->mayAutoComplete = false;
				game::Con_DrawInputPrompt();
				game::Cmd_EndTokenizedString();

				goto REDRAW_CURSOR_RETURN;
			}
		}

		else 
		{
			cmd_or_dvar = 0;
		}

		game::conDrawInputGlob->hasExactMatch = false;
		game::conDrawInputGlob->matchCount = 0;

		if (game::Dvar_FindVar("con_matchPrefixOnly")->current.enabled == 0)
		{
			*game::extvar_con_ignoreMatchPrefixOnly = false;
			goto CON_MATCH_PREFIX_ONLY;
		}

	   *game::extvar_con_ignoreMatchPrefixOnly = true;
		game::Dvar_ForEachName(game::ConDrawInput_IncrMatchCounter);

		if (!cmd_or_dvar) 
		{
			game::Cmd_ForEachXO(game::ConDrawInput_IncrMatchCounter);
		}

		if (game::conDrawInputGlob->matchCount > dvars::xo_con_maxMatches->current.integer) // autocomplete avail.
		{
			game::conDrawInputGlob->hasExactMatch = false;
			game::conDrawInputGlob->matchCount = 0;
		   *game::extvar_con_ignoreMatchPrefixOnly = false;
			game::Dvar_ForEachName(game::ConDrawInput_IncrMatchCounter);
			
			game::Cmd_ForEachXO(game::ConDrawInput_IncrMatchCounter);

			if (!game::conDrawInputGlob->matchCount)
			{
				game::conDrawInputGlob->hasExactMatch = false;
				game::conDrawInputGlob->matchCount = 0;
			   *game::extvar_con_ignoreMatchPrefixOnly = true;


CON_MATCH_PREFIX_ONLY:
				game::Dvar_ForEachName(game::ConDrawInput_IncrMatchCounter);
				
				if (!cmd_or_dvar) 
				{
					game::Cmd_ForEachXO(game::ConDrawInput_IncrMatchCounter);
				}
			}
		}

		var_con_current_match_count = game::conDrawInputGlob->matchCount;

		if (!game::conDrawInputGlob->matchCount)
		{
			game::Con_DrawInputPrompt();
			game::Cmd_EndTokenizedString();

			goto REDRAW_CURSOR_RETURN;
		}

		var_con_is_dvar_command = game::conDrawInputGlob->matchIndex;

		if (game::conDrawInputGlob->matchIndex < game::conDrawInputGlob->matchCount && game::conDrawInputGlob->autoCompleteChoice[0])
		{
			if (game::conDrawInputGlob->matchIndex >= 0)
			{
				var_con_is_dvar_command = cmd_or_dvar;
				game::Con_DrawAutoCompleteChoice(var_con_is_dvar_command, var_con_original_command); // color AutoComplete String
				
				goto CON_SKIP_INPUT_PROMPT;
			}
		}

		else 
		{
			game::conDrawInputGlob->matchIndex = -1;
		}

		game::Con_DrawInputPrompt();

	CON_SKIP_INPUT_PROMPT:
		game::conDrawInputGlob->y = game::conDrawInputGlob->y + (2 * game::conDrawInputGlob->fontHeight + 15.0f);
		game::conDrawInputGlob->x = game::conDrawInputGlob->leftX;
#pragma endregion
		
		// --------------------------------------------------------------------------- 
		// :: HINTBOX

		// copy "con_inputHintBoxColor" to dynamically change its alpha without changing the dvar
		float loc_inputHintBoxColor[4]; 
		memcpy(&loc_inputHintBoxColor, game::Dvar_FindVar("con_inputHintBoxColor")->current.vector, sizeof(float[4]));

		// reduce alpha on fullscreen to make text more readable - will reset itself when small again
		if (loc_inputHintBoxColor[3] < 1.0f && game::con->outputVisible) 
		{
			loc_inputHintBoxColor[3] = 0.95f; // alpha
		}

		// --------------------------------------------------------------------------- 
		// :: Too many matches to show ( 1 line )

		output_slider_width = 16.0f;

		if(game::conDrawInputGlob->matchCount > dvars::xo_con_maxMatches->current.integer)
		{
			// create the text and get its width in pixels so we can center it
			const auto font_handle = game::R_RegisterFont(FONT_CONSOLE, sizeof(FONT_CONSOLE));
			const char* too_much_text = utils::va("%i matches (too many to show here, press TAB to print all, press SHIFT + TILDE to open full console)", var_con_current_match_count);
			
			const int txt_length = game::R_TextWidth(too_much_text, 0, font_handle);

			// small console --------------------------------------------------------

			// hint box
			console_addon_.items.s_con.hint_box_upper.x = x;
			console_addon_.items.s_con.hint_box_upper.y = game::conDrawInputGlob->y - game::conDrawInputGlob->fontHeight - 7.0f;
			console_addon_.items.s_con.hint_box_upper.w = w;
			console_addon_.items.s_con.hint_box_upper.h = game::conDrawInputGlob->fontHeight + 17.0f;

			// hint box text (centered)
			console_addon_.items.s_con.hint_box_upper_text.x = floorf((console_addon_.items.s_con.hint_box_upper.w * 0.5f) - ((float)txt_length * 0.5f) + console_addon_.items.s_con.input_box.x);
			//conAddon.conItems.s_con.hintBoxUpperText.x = x + 18.0f; // left aligned
			console_addon_.items.s_con.hint_box_upper_text.y = game::conDrawInputGlob->y;

			// full console --------------------------------------------------------

			// hint box
			console_addon_.items.f_con.hint_box_upper.x = x;
			console_addon_.items.f_con.hint_box_upper.y = game::conDrawInputGlob->y - game::conDrawInputGlob->fontHeight - 5.0f;
			console_addon_.items.f_con.hint_box_upper.w = w - output_slider_width;
			console_addon_.items.f_con.hint_box_upper.h = game::conDrawInputGlob->fontHeight + 17.0f;

			// hint box text (centered)
			console_addon_.items.f_con.hint_box_upper_text.x = (console_addon_.items.f_con.hint_box_upper.w * 0.5f) - ((float)txt_length * 0.5f) + console_addon_.items.f_con.input_box.x;
			//conAddon.conItems.fullCon.hintBoxUpperText.x = x + 10.0f + 34.0f; // left aligned
			console_addon_.items.f_con.hint_box_upper_text.y = game::conDrawInputGlob->y + 2.0f;

			// ---------------------------------------------------------------------

			// if small console is near the bottom or clipped at the bottom, put hintboxes above the output window
			if (!game::con->outputVisible && console_addon_.floating.enabled && (console_addon_.floating.clipped_at_border.bottom || console_addon_.floating.anker.bottom + console_addon_.items.s_con.hint_box_upper.h > console_addon_.viewport_res.height))
			{
				// calculate the new y
				console_addon_.items.s_con.hint_box_upper.y		= console_addon_.floating.anker.bottom - console_addon_.items.s_con.input_box.h - console_addon_.items.s_con.output_box.h - console_addon_.items.s_con.hint_box_upper.h;
				console_addon_.items.s_con.hint_box_upper_text.y	= console_addon_.items.s_con.hint_box_upper.y + ((console_addon_.items.s_con.hint_box_upper.h * 0.5f) + (game::conDrawInputGlob->fontHeight * 0.5f) + 0.5f); // offset by half a pixel (blurriness)
			}

			draw_rect(
			/* col */ loc_inputHintBoxColor,
			/*  x  */ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.x : console_addon_.items.s_con.hint_box_upper.x,
			/*  y  */ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.y : console_addon_.items.s_con.hint_box_upper.y,
			/*  w  */ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.w : console_addon_.items.s_con.hint_box_upper.w,
			/*  h  */ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.h : console_addon_.items.s_con.hint_box_upper.h,
			/* dep */ false,
			/* iFS */ true);

			// hintbox shadow --------------------------------------------------------
			float shadow_color[4] = { 0.0f, 0.0f, 0.0f, 0.125f };

			draw_rect(
				/*    c		*/ shadow_color,
				/*	  x		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.x : console_addon_.items.s_con.hint_box_upper.x,
				/*	  y		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.y : console_addon_.items.s_con.hint_box_upper.y,
				/*	  w		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.w + output_slider_width : console_addon_.items.s_con.hint_box_upper.w,
				/*	  h		*/ 3.0f,
				/*  depth	*/ false,
				/* ignoreFS	*/ true);

			draw_input_text(
			/*  x  */ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper_text.x : console_addon_.items.s_con.hint_box_upper_text.x,
			/*  y  */ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper_text.y : console_addon_.items.s_con.hint_box_upper_text.y,
			/* scX */ 1.0f,
			/* scY */ 1.0f,
			/* fon */ font_handle,
			/* col */ dvars::xo_con_hintBoxTxtColor_currentDvar->current.vector,
			/* txt */ too_much_text);
			
			game::Cmd_EndTokenizedString();
			goto REDRAW_CURSOR_RETURN;
		}

		// --------------------------------------------------------------------------- 
		// :: Exact Match - Draw Details

		if(game::conDrawInputGlob->matchCount == 1 || game::conDrawInputGlob->hasExactMatch && game::Con_AnySpaceAfterCommand())
		{
			console_glob_color_ = loc_inputHintBoxColor;

			// small console --------------------------------------------------------

			// hint box -- we calculate y & h within DetailedMatch_UpperBox && DetailedMatch_LowerBox
			console_addon_.items.s_con.hint_box_upper.x = x;
			console_addon_.items.s_con.hint_box_upper.w = w;

			// hint box text
			// we do not handle text for dvar matches

			// full console --------------------------------------------------------

			// hint box -- we calculate y & h within DetailedMatch_UpperBox && DetailedMatch_LowerBox
			console_addon_.items.f_con.hint_box_upper.x = x;
			console_addon_.items.f_con.hint_box_upper.w = w - output_slider_width;

			// hint box text
			// we do not handle text for dvar matches

			// ---------------------------------------------------------------------

			// if small console is near the bottom or clipped at the bottom, put hintboxes above the output window
			if (!game::con->outputVisible && (console_addon_.floating.clipped_at_border.bottom || console_addon_.floating.anker.bottom + console_addon_.items.s_con.hint_box_total.h > console_addon_.viewport_res.height))
			{
				// we calculate the box offsets in DetailedMatch_UpperBox && DetailedMatch_LowerBox
				// but we have to offset Game::conDrawInputGlob->y here. Unfortunately, we do not know the height, so we have to "think" a frame later 
				//Game::conDrawInputGlob->y = conAddon.fltCon.fltAnker.bottom - conAddon.conItems.s_con.inputBox.h - conAddon.conItems.s_con.outputBox.h - conAddon.conItems.s_con.hintBoxTotal.h + conDrawInputGlobY_fuckery + 8.0f;
			
				const float tmp_top_space = console_addon_.floating.anker.top;
				const float tmp_bottom_space = console_addon_.viewport_res.height - console_addon_.floating.anker.bottom;
			
				// compare top with bottom
				if (tmp_top_space > tmp_bottom_space)
				{
					// top now offers more space, so move the hintbox above the output box
					game::conDrawInputGlob->y = console_addon_.floating.anker.bottom - console_addon_.items.s_con.input_box.h - console_addon_.items.s_con.output_box.h - console_addon_.items.s_con.hint_box_total.h + console_input_glob_y_fuckery_ + 7.0f;

					// calculate the new y
					console_addon_.items.s_con.hint_box_upper.y = console_addon_.floating.anker.bottom - console_addon_.items.s_con.input_box.h - console_addon_.items.s_con.output_box.h - console_addon_.items.s_con.hint_box_upper.h;
				
					// lets offset text in the to the left if we display hintboxes over the console ;)
					game::conDrawInputGlob->x -= 36.0f;
					game::conDrawInputGlob->leftX = game::conDrawInputGlob->x;
				}
			}

			// we have to subtract from Game::conDrawInputGlob->y because we want to offset the match text in "ConDrawInput_Detailed..." (we add it back when drawing the box)
			// this is because we only hook the box drawing part of "ConDrawInput_Detailed..." and not the text drawing
			game::conDrawInputGlob->y -= console_input_glob_y_fuckery_;

			// mid-hook to change box drawing
			// -> ConDrawInput_Box_DetailedMatch_UpperBox && ConDrawInput_Box_DetailedMatch_LowerBox
			game::Dvar_ForEachName(game::ConDrawInput_DetailedDvarMatch); 

			if (!cmd_or_dvar) 
			{
				game::Cmd_ForEach_PassCmd(draw_detailed_cmd_match);
				//Game::Cmd_ForEachXO(Game::ConDrawInput_DetailedCmdMatch);
			}
		}

		// --------------------------------------------------------------------------- 
		// :: Multiple Matches :: Draw with Values

		else
		{
			// y = top of outputbox
			// h = height of outputbox

			// small console --------------------------------------------------------

			// hint box --  we calculate y & h within DetailedMatch_UpperBox && DetailedMatch_LowerBox
			console_addon_.items.s_con.hint_box_upper.x = x;
			console_addon_.items.s_con.hint_box_upper.y = y + h; // bad
			console_addon_.items.s_con.hint_box_upper.w = w;
			console_addon_.items.s_con.hint_box_upper.h = static_cast<float>(game::conDrawInputGlob->matchCount) * game::conDrawInputGlob->fontHeight + 20.0f;//var_con_currentMatchCount * Game::conDrawInputGlob->fontHeight + 20.0f; // bad

			// hint box text
			// we do not handle text for dvar matches

			// full console --------------------------------------------------------

			// hint box -- we calculate y & h within DetailedMatch_UpperBox && DetailedMatch_LowerBox
			console_addon_.items.f_con.hint_box_upper.x = x;
			console_addon_.items.f_con.hint_box_upper.y = y + h; // bad
			console_addon_.items.f_con.hint_box_upper.w = w - output_slider_width;
			console_addon_.items.f_con.hint_box_upper.h = static_cast<float>(game::conDrawInputGlob->matchCount) * game::conDrawInputGlob->fontHeight + 20.0f;//var_con_currentMatchCount * Game::conDrawInputGlob->fontHeight + 20.0f; // bad

			// hint box text
			// we do not handle text for dvar matches

			// ---------------------------------------------------------------------

			// check if hintbox goes out of bounds
			if (!game::con->outputVisible && (console_addon_.floating.clipped_at_border.bottom || console_addon_.floating.anker.bottom + console_addon_.items.s_con.hint_box_upper.h > console_addon_.viewport_res.height))
			{
				// check if we have more "space" above the console, if not, allow a part of the hintbox to move out of bounds till the top has more space then the bottom
				const float tmp_top_space = console_addon_.floating.anker.top;
				const float tmp_bottom_space = console_addon_.viewport_res.height - console_addon_.floating.anker.bottom;

				// compare top with bottom
				if (tmp_top_space > tmp_bottom_space) 
				{
					// top now offers more space, so move the hintbox above the output box
					game::conDrawInputGlob->y = console_addon_.floating.anker.bottom - console_addon_.items.s_con.input_box.h - console_addon_.items.s_con.output_box.h - console_addon_.items.s_con.hint_box_upper.h + console_input_glob_y_fuckery_ + 8.0f;
				
					// calculate the new y
					console_addon_.items.s_con.hint_box_upper.y = console_addon_.floating.anker.bottom - console_addon_.items.s_con.input_box.h - console_addon_.items.s_con.output_box.h - console_addon_.items.s_con.hint_box_upper.h;
				
					// lets offset text in the to the left if we display hintboxes over the console ;)
					game::conDrawInputGlob->x -= 36.0f;
					game::conDrawInputGlob->leftX = game::conDrawInputGlob->x;
				}
			}

			// we have to subtract from Game::conDrawInputGlob->y because we want to offset the match text in "ConDrawInput_Detailed..." (we add it back when drawing the box)
			// this is because we only hook the box drawing part of "ConDrawInput_Detailed..." and not the text drawing
			game::conDrawInputGlob->y -= console_input_glob_y_fuckery_;

			draw_rect(
				/* col */ loc_inputHintBoxColor,
				/*  x  */ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.x : console_addon_.items.s_con.hint_box_upper.x,//x,
				/*  y  */ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.y : console_addon_.items.s_con.hint_box_upper.y,//y + h,
				/*  w  */ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.w : console_addon_.items.s_con.hint_box_upper.w,//w,
				/*  h  */ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.h : console_addon_.items.s_con.hint_box_upper.h,//var_con_currentMatchCount * Game::conDrawInputGlob->fontHeight + 20.0f,
				/* dep */ false,
				/* iFS */ true );

			// hintbox shadow --------------------------------------------------------
			float shadow_color[4] = { 0.0f, 0.0f, 0.0f, 0.125f };

			draw_rect(
				/*    c		*/ shadow_color,
				/*	  x		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.x : console_addon_.items.s_con.hint_box_upper.x,
				/*	  y		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.y : console_addon_.items.s_con.hint_box_upper.y,
				/*	  w		*/ game::con->outputVisible ? console_addon_.items.f_con.hint_box_upper.w + output_slider_width : console_addon_.items.s_con.hint_box_upper.w,
				/*	  h		*/ 3.0f,
				/*  depth	*/ false,
				/* ignoreFS	*/ true);

			game::Dvar_ForEachName(game::ConDrawInput_DvarMatch);
			
			if (!cmd_or_dvar)
			{
				game::Cmd_ForEachXO(game::ConDrawInput_CmdMatch);
				game::Cmd_EndTokenizedString();

				goto REDRAW_CURSOR_RETURN;
			}
		}

		game::Cmd_EndTokenizedString();

		// *
		// -----------------
		REDRAW_CURSOR_RETURN:

		// Overdraw the cursor so its above the console so we obv. can not return early ;)
		if ((game::ui_context->openMenuCount || dvars::xo_con_cursorState->current.enabled) && dvars::xo_con_cursorOverdraw->current.enabled)
		{
			_ui::redraw_cursor();
		}
	}

	#pragma endregion

	#pragma region CON-ASM // ++++++

	// Replace ConDrawInput_Box in ConDrawInput_DetailedDvarMatch ( mid-hook ) -- upper box
	__declspec(naked) void detailed_dvar_match_stub_01()
	{
		const static uint32_t retn_addr = 0x460022;
		__asm
		{
			push	eax; // overwritten op
			push	esi;
			call	draw_rect_detailed_match_upper;
			pop		esi;
			add		esp, 4h;

			jmp		retn_addr;
		}
	}

	// Replace ConDrawInput_Box in ConDrawInput_DetailedDvarMatch ( mid-hook ) -- lower box
	__declspec(naked) void detailed_dvar_match_stub_02()
	{
		const static uint32_t retn_addr = 0x4601FB;
		__asm
		{
			push	eax; // overwritten op
			push	esi;
			call	draw_rect_detailed_match_lower;
			pop		esi;
			add		esp, 4h;

			jmp		retn_addr;
		}
	}

	// Replace ConDrawInput_Box in ConDrawInput_DetailedCmdMatch
	__declspec(naked) void detailed_cmd_match_stub()
	{
		const static uint32_t retn_addr = 0x4603BF;
		__asm
		{
			push	1;		// overwritten op
			push	esi;
			call	draw_rect_detailed_match_upper;
			pop		esi;
			add		esp, 4h;

			jmp		retn_addr;
		}
	}

	// helper func to offset matchbox values
	void offset_match_values()
	{
		game::conDrawInputGlob->x += 500.0f;
	}

	// offset values for multiple matches, as we increased the maximum amount of dvar chars displayed 
	__declspec(naked) void matchbox_offset_values_stub_01()
	{
		const static uint32_t retn_addr = 0x45FAFA;
		__asm
		{
			pushad;
			call	offset_match_values;
			popad;
			
			add     esp, 4;		// stock
			jmp		retn_addr;
		}
	}

	// offset values for detailed matches, as we increased the maximum amount of dvar chars displayed 
	__declspec(naked) void matchbox_offset_values_stub_02()
	{
		const static uint32_t retn_addr = 0x460054;
		__asm 
		{
			pushad;
			call	offset_match_values;
			popad;

			add     esp, 4;		// stock
			push    0x6BDF24;	// stock
			sub     esp, 10h;	// stock

			jmp		retn_addr;
		}
	}

	// ^
	__declspec(naked) void matchbox_offset_values_stub_03()
	{
		const static uint32_t retn_addr = 0x4600D4;
		__asm
		{
			pushad;
			call	offset_match_values;
			popad;

			add     esp, 4;		// stock
			push    0x6BDF34;	// stock
			sub     esp, 10h;	// stock

			jmp		retn_addr;
		}
	}

	// ^
	__declspec(naked) void matchbox_offset_values_stub_04()
	{
		const static uint32_t retn_addr = 0x460150;
		__asm
		{
			pushad;
			call	offset_match_values;
			popad;

			add     esp, 4;		// stock
			push    0x6BDF34;	// stock
			sub     esp, 10h;	// stock

			jmp		retn_addr;
		}
	}

	// fully disable cmd autocompletion box drawing
	__declspec(naked) void disable_autocomplete_box_stub()
	{
		const static uint32_t retn_addr = 0x460433;
		__asm
		{
			jmp		retn_addr;
		}
	}

	// change devmap autocomplete extension to "autocomplete" :: files are in main/maps/mp
	__declspec(naked) void con_devmap_autocompl_stub()
	{
		const static uint32_t retn_addr = 0x528F96;
		__asm
		{
			push	ecx;
			mov		ecx, console_mapdir_;
			mov		[eax + 8], ecx;

			mov		ecx, console_mapext_;
			mov		[eax + 0Ch], ecx;

			pop		ecx;
			jmp		retn_addr;
		}
	}

	// change map autocomplete extension to "autocomplete" :: files are in main/maps/mp
	__declspec(naked) void con_map_autocompl_stub()
	{
		const static uint32_t retn_addr = 0x528E5D;
		__asm
		{
			push	ecx;

			mov		ecx, console_mapext_;
			mov		ebx, ecx;

			mov     esi, edi;	// stock op inbetween

			mov		ecx, console_mapdir_;
			mov		[eax + 8], ecx;

			pop		ecx;
			jmp		retn_addr;
		}
	}

	// *
	// minicon drawing (Con_DrawMessageWindowOldToNew)
	void draw_message_window_old_to_new(DWORD* msgWnd, int s_xPos, int s_yPos, int s_charHeight, int s_horzAlign, int s_vertAlign, int s_mode, game::Font_s* s_font, const float* s_color, int s_textStyle, float s_msgwndScale, int s_textAlignMode)
	{
		// get font handle
		const auto font_str	= _cg::get_font_for_style(dvars::con_minicon_font->current.integer);
		const auto font_handle =game::R_RegisterFont(font_str, sizeof(font_str));

		game::Con_DrawMessageWindowOldToNew(
			msgWnd,
			0,
			static_cast<int>(dvars::con_minicon_position->current.vector[0]),
			static_cast<int>(dvars::con_minicon_position->current.vector[1]),
			dvars::con_minicon_fontHeight->current.integer,
			s_horzAlign,
			s_vertAlign,
			s_mode,
			font_handle,
			dvars::con_minicon_fontColor->current.vector,
			dvars::con_minicon_fontStyle->current.integer,
			s_msgwndScale,
			s_textAlignMode
		);
	}

	// wrapper (the game did not push any args besides esi)
	__declspec(naked) void con_minicon_stub()
	{
		const static uint32_t retn_addr = 0x4615CA;
		__asm
		{
			push	4;					// textAlignMode
			fstp	dword ptr[esp];		// msgwndScale
			push	3;					// textStyle
			lea     eax, [esp + 1Ch];	// color
			push    eax;				// color
			push    edi;				// font
			push    edx;				// mode
			push    1;					// vert
			push    1;					// horz
			push    12;					// charHeight
			push    4;					// yPos
			push    2;					// xPos
			push	esi;				// esi holds msgWnd
			
			call	draw_message_window_old_to_new;
			add     esp, 2Ch;

			jmp		retn_addr;				// after <add esp, 30h>
		}
	}
	#pragma endregion

	console::console()
	{
		// -----
		// DVARS

		dvars::con_minicon_position = game::Dvar_RegisterVec2(
			/* name		*/ "con_minicon_position",
			/* desc		*/ "minicon position (int)",
			/* x		*/ 115.0f,
			/* y		*/ 17.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 640.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::con_minicon_font = game::Dvar_RegisterInt(
			/* name		*/ "con_minicon_font",
			/* desc		*/ "minicon font",
			/* default	*/ 8,
			/* minVal	*/ 0,
			/* maxVal	*/ 8,
			/* flags	*/ game::dvar_flags::saved);

		dvars::con_minicon_fontHeight = game::Dvar_RegisterInt(
			/* name		*/ "con_minicon_fontHeight",
			/* desc		*/ "minicon char height",
			/* default	*/ 11,
			/* minVal	*/ 0,
			/* maxVal	*/ 64,
			/* flags	*/ game::dvar_flags::saved);

		dvars::con_minicon_fontColor = game::Dvar_RegisterVec4(
			/* name		*/ "con_minicon_fontColor",
			/* desc		*/ "minicon font color",
			/* x		*/ 1.0f,
			/* y		*/ 1.0f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::con_minicon_fontStyle = game::Dvar_RegisterInt(
			/* name		*/ "con_minicon_fontStyle",
			/* desc		*/ "3 = <renderFlags 4>; 6 = <renderFlags 12>; 128 = <renderFlags 1>",
			/* default	*/ 3,
			/* minVal	*/ 0,
			/* maxVal	*/ 128,
			/* flags	*/ game::dvar_flags::saved);
		
		dvars::xo_con_fltCon = game::Dvar_RegisterBool(
			/* name		*/ "xo_con_fltCon",
			/* desc		*/ "console :: floating console state",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_fltConLeft = game::Dvar_RegisterFloat(
			/* name		*/ "xo_con_fltConLeft",
			/* desc		*/ "console :: floating console left anker",
			/* default	*/ 0.0f,
			/* minVal	*/ -7680.0f,
			/* maxVal	*/ 7680.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_fltConTop = game::Dvar_RegisterFloat(
			/* name		*/ "xo_con_fltConTop",
			/* desc		*/ "console :: floating console top anker",
			/* default	*/ 0.0f,
			/* minVal	*/ -4320.0f,
			/* maxVal	*/ 4320.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_fltConRight = game::Dvar_RegisterFloat(
			/* name		*/ "xo_con_fltConRight",
			/* desc		*/ "console :: floating console right anker",
			/* default	*/ 0.0f,
			/* minVal	*/ -7680.0f,
			/* maxVal	*/ 7680.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_fltConBottom = game::Dvar_RegisterFloat(
			/* name		*/ "xo_con_fltConBottom",
			/* desc		*/ "console :: floating console bottom anker",
			/* default	*/ 0.0f,
			/* minVal	*/ -4320.0f,
			/* maxVal	*/ 4320.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_outputHeight = game::Dvar_RegisterInt(
			/* name		*/ "xo_con_outputHeight",
			/* desc		*/ "console :: height / lines of console output.",
			/* default	*/ 8,
			/* minVal	*/ 1,
			/* maxVal	*/ 30,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_maxMatches = game::Dvar_RegisterInt(
			/* name		*/ "xo_con_maxMatches",
			/* desc		*/ "console :: maximum amout of matches to show till \"too many to show here\" is drawn.",
			/* default	*/ 24,
			/* minVal	*/ 0,
			/* maxVal	*/ 50,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_useDepth = game::Dvar_RegisterBool(
			/* name		*/ "xo_con_useDepth",
			/* desc		*/ "console :: use scene depth as inputBox background.",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_padding = game::Dvar_RegisterFloat(
			/* name		*/ "xo_con_padding",
			/* desc		*/ "console :: distance screen border <-> console (all sides)",
			/* default	*/ 0.0f,
			/* minVal	*/ -2000.0f,
			/* maxVal	*/ 2000.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_fontSpacing = game::Dvar_RegisterFloat(
			/* name		*/ "xo_con_fontSpacing",
			/* desc		*/ "console output :: space between lines",
			/* default	*/ 0.8f,
			/* minVal	*/ 0.8f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_cursorOverdraw = game::Dvar_RegisterBool(
			/* name		*/ "xo_con_cursorOverdraw",
			/* desc		*/ "console :: redraw the menu cursor so its above the console.",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_cursorState = game::Dvar_RegisterBool(
			/* name		*/ "xo_con_cursorState",
			/* desc		*/ "console :: current state of the cursor.",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none);

		dvars::xo_con_hintBoxTxtColor_currentDvar = game::Dvar_RegisterVec4(
			/* name		*/ "xo_con_hintBoxTxtColor_currentDvar",
			/* desc		*/ "console :: color of dvar names in the hintbox",
			/* x		*/ 0.8f,
			/* y		*/ 0.8f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_hintBoxTxtColor_currentValue = game::Dvar_RegisterVec4(
			/* name		*/ "xo_con_hintBoxTxtColor_currentValue",
			/* desc		*/ "console :: color of dvar values in the hintbox",
			/* x		*/ 0.8f,
			/* y		*/ 0.8f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_hintBoxTxtColor_defaultValue = game::Dvar_RegisterVec4(
			/* name		*/ "xo_con_hintBoxTxtColor_defaultValue",
			/* desc		*/ "console :: color of default dvar value in the hintbox",
			/* x		*/ 0.8f,
			/* y		*/ 0.8f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_hintBoxTxtColor_dvarDescription = game::Dvar_RegisterVec4(
			/* name		*/ "xo_con_hintBoxTxtColor_dvarDescription",
			/* desc		*/ "console :: color of dvar description in the hintbox",
			/* x		*/ 0.8f,
			/* y		*/ 0.8f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		dvars::xo_con_hintBoxTxtColor_domainDescription = game::Dvar_RegisterVec4(
			/* name		*/ "xo_con_hintBoxTxtColor_domainDescription",
			/* desc		*/ "console :: color of dvar domain in the hintbox",
			/* x		*/ 0.8f,
			/* y		*/ 0.8f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);


		// -----
		// Hooks


		// Minicon drawing - wrapper to proxy
		utils::hook::nop(0x4615A8, 6);
		utils::hook(0x4615A8, con_minicon_stub, HOOK_JUMP).install()->quick(); // in Con_DrawMiniConsole

		// Con_CheckResize - main position
		utils::hook(0x45EB81, check_resize_on_init, HOOK_CALL).install()->quick(); // in Con_OneTimeInit
		utils::hook(0x46CC6B, check_resize_on_init, HOOK_CALL).install()->quick(); // in CL_InitRenderer
		

		// Con_DrawOuputWindow -- in "Con_DrawSolidConsole" before "Con_DrawInput"
		utils::hook(0x461D44, draw_output_window, HOOK_CALL).install()->quick(); // no flicker but latched value bug on vec4

		// Con_DrawInput ( 2x IfElse )
		utils::hook(0x461D34, draw_input, HOOK_CALL).install()->quick();
		utils::hook(0x461D49, draw_input, HOOK_CALL).install()->quick(); // our active func

		// Disable the need for Forward/Backslash for console cmds
		utils::hook::nop(0x46752F, 5);

		// Replace ConDrawInput_Box in ConDrawInput_DetailedDvarMatch ( mid-hook ) -- upper box
		utils::hook(0x46001C, detailed_dvar_match_stub_01, HOOK_CALL).install()->quick();
		utils::hook::nop(0x460021, 1);

		// Replace ConDrawInput_Box in ConDrawInput_DetailedDvarMatch ( mid-hook ) -- lower box
		utils::hook(0x4601F5, detailed_dvar_match_stub_02, HOOK_CALL).install()->quick();
		utils::hook::nop(0x4601FA, 1);

		// Replace ConDrawInput_Box in ConDrawInput_DetailedCmdMatch
		utils::hook(0x4603B8, detailed_cmd_match_stub, HOOK_CALL).install()->quick();
		utils::hook::nop(0x4603BD, 2);

		// Increase max chars to draw for dvar matches
		utils::hook::set<BYTE>(0x45FADA, 0x40); // Stock 0x18

		// Increase draw width for matched dvar - dvar string
		utils::hook::set<BYTE>(0x46002E, 0x40); // Stock 0x18

		// Increase distance to matched dvars as we increased the maximum amount of chars for found matches
		utils::hook::nop(0x45FAE5, 6); // nop the instruction first, then install our jmp
		utils::hook(0x45FAE5, matchbox_offset_values_stub_01, HOOK_JUMP).install()->quick();
		
		// same for detailed match (3 times for current, latched, default)
		utils::hook::nop(0x460037, 6); // nop the instruction first, then install our jmp
		utils::hook(0x460037, matchbox_offset_values_stub_02, HOOK_JUMP).install()->quick();

		utils::hook::nop(0x4600B7, 6); // nop the instruction first, then install our jmp
		utils::hook(0x4600B7, matchbox_offset_values_stub_03, HOOK_JUMP).install()->quick();

		utils::hook::nop(0x460133, 6); // nop the instruction first, then install our jmp
		utils::hook(0x460133, matchbox_offset_values_stub_04, HOOK_JUMP).install()->quick();
		
		// fully disable cmd autocomplete box with dir searching
		utils::hook::nop(0x4603FC, 6); utils::hook(0x4603FC, disable_autocomplete_box_stub, HOOK_JUMP).install()->quick();
		
		//// devmap autocomplete :: change extentsion to .autocomplete and place nullfiles in main/maps/mp/mp_mapname.autocomplete
		//utils::hook(0x528F8C, con_devmap_autocompl_stub, HOOK_JUMP).install()->quick();
		//utils::hook::nop(0x528F91, 2);
		//// map autocomplete :: change extentsion to .autocomplete and place nullfiles in main/maps/mp/mp_mapname.autocomplete
		//utils::hook(0x528E4F, con_map_autocompl_stub, HOOK_JUMP).install()->quick();

		command::add("condump", "", "dump console contents to root/iw3xo/condump", [&](auto)
		{
				
			if (const auto& base_path = game::Dvar_FindVar("fs_basepath"); 
							base_path)
			{
				const std::string file_path = base_path->current.string + "\\iw3xo\\condump\\"s;
				if(!std::filesystem::exists(file_path))
				{
					std::filesystem::create_directories(file_path);
				}

				std::ofstream condump;
				const std::string file_name = file_path + "condump_";

				for (auto i = 0; i < 1024; i++)
				{
					if(std::filesystem::exists(file_name + std::to_string(i) + ".txt"))
					{
						continue;
					}

					condump.open((file_name + std::to_string(i) + ".txt").c_str());

					std::string line;
					condump << game::con->consoleText;
					condump.close();

					break;
				}
			}
		});
	}
}
