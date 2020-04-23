#include "STDInclude.hpp"

#define CON_KEY_TOGGLE_CURSOR	KEYCATCHER_F1
#define CON_KEY_RESET_FLTCON	KEYCATCHER_F2
#define GET_CONSOLEFONT (Game::Font_s*) *(DWORD*)(Game::con_font_ptr)

const char* CON_HELP_PRINT =	"---------------- CONSOLE HELP ----------------------\"^2help^7\"--------------------------\n"
								"!     F1 :: Toggle cursor in-game\n"
								"!     F2 :: Reset floating console\n"
								"! Mouse1 :: Press and hold mouse1 on the input bar to move / resize the console\n"
								"! xo_con :: Prefix for additional console dvars\n"
								"------------------------------------------------------------------------------------\n"
								"\n";

// init console addon struct
Game::Console_Addons conAddon = Game::Console_Addons();

static char* con_mapdir = "maps/mp";
static char* con_mapext = "autocomplete"; // change d3dbsp to autocomplete

namespace Components
{
	#pragma region CON-UTILS // ++++++

	int XO_Console::Cmd_Argc()
	{
		return *&Game::argc_1410B84[4 * *Game::argc_1410B40];
	}

	bool Con_IsDvarCommand(char* cmd)
	{
		if (!_stricmp(cmd, "set"))
			return true;

		if (!_stricmp(cmd, "seta"))
			return true;

		if (!_stricmp(cmd, "sets"))
			return true;

		if (!_stricmp(cmd, "reset"))
			return true;

		if (_stricmp(cmd, "toggle"))
			return _stricmp(cmd, "togglep") == false;

		return true;
	}

	// box drawing helper function -- depth material
	void ConDraw_Box(float *color, float x, float y, float w, float h, bool depth, bool ignoreFullscreen = false )
	{
		void *material;

		if (depth && Dvars::xo_con_useDepth->current.enabled) 
		{ 
			// depth input
			material = Game::Material_RegisterHandle("floatz_display", 3); 
		}
		else 
		{
			// stock
			material = Game::Material_RegisterHandle("white", 3); 
		}

		if (!Game::con->outputVisible || ignoreFullscreen) 
		{
			// small console
			Game::R_AddCmdDrawStretchPic(material, x, y, w, h, 0.0f, 0.0f, 0.0f, 0.0f, color);
		}
		else 
		{
			// fullscreen
			Game::R_AddCmdDrawStretchPic(material, x, y, w, h, 0.0f, 0.0f, 0.0f, 0.0f, color);
		}
	}

	// register font and draw text
	void ConDrawInput_Text(float x, float y, float scaleX, float scaleY, char* font, const float *color, const char* text)
	{
		void* fontHandle = Game::R_RegisterFont(font, sizeof(font));
		Game::R_AddCmdDrawTextASM(text, 0x7FFFFFFF, fontHandle, x, y, scaleX, scaleY, 0.0f, color, 0);
	}

	// draw text
	void ConDrawInput_Text(float x, float y, float scaleX, float scaleY, Game::Font_s* font, const float *color, const char* text)
	{
		Game::R_AddCmdDrawTextASM(text, 0x7FFFFFFF, font, x, y, scaleX, scaleY, 0.0f, color, 0);
	}

	#pragma endregion

	#pragma region CON-DRAW // ++++++

	// used for mid hooks where we changed conDrawInputGlob->y prior call to offset text, but we need to adjust the box we draw (within the hook)
	float conDrawInputGlobY_fuckery = 15.0f; // 16
	float *glob_con_col;

	// mid-func hook to alter hintbox drawing (upper box with dvar name and value)
	void ConDrawInput_Box_DetailedMatch_UpperBox(float *color, int lines)
	{
		// small console offsets --------------------------------------------------------
		// upper.x and upper.w get set in "DrawInput" :: Exact Match - Draw Details
		conAddon.conItems.smallCon.hintBoxUpper.y = Game::conDrawInputGlob->y - 8.0f - 15.0f + conDrawInputGlobY_fuckery;
		conAddon.conItems.smallCon.hintBoxUpper.h = lines * Game::conDrawInputGlob->fontHeight + 16.0f;

		// full console offsets --------------------------------------------------------
		// upper.x and upper.w get set in "DrawInput" :: Exact Match - Draw Details
		conAddon.conItems.fullCon.hintBoxUpper.y = Game::conDrawInputGlob->y - 10.0f - 11.0f + conDrawInputGlobY_fuckery;
		conAddon.conItems.fullCon.hintBoxUpper.h = lines * Game::conDrawInputGlob->fontHeight + 16.0f;//+ 4.0f;


		// get dimensions of upper + lower hint box (needed for clipping)
		// get upper box ::

		// small console total hintbox part 1/2 --------------------------------------------------------
		conAddon.conItems.smallCon.hintBoxTotal.x = conAddon.conItems.smallCon.hintBoxUpper.x;
		conAddon.conItems.smallCon.hintBoxTotal.y = conAddon.conItems.smallCon.hintBoxUpper.y;
		conAddon.conItems.smallCon.hintBoxTotal.w = conAddon.conItems.smallCon.hintBoxUpper.w;
		conAddon.conItems.smallCon.hintBoxTotal.h = conAddon.conItems.smallCon.hintBoxUpper.h;

		// full console total hintbox part 1/2 --------------------------------------------------------
		conAddon.conItems.fullCon.hintBoxTotal.x = conAddon.conItems.fullCon.hintBoxUpper.x;
		conAddon.conItems.fullCon.hintBoxTotal.y = conAddon.conItems.fullCon.hintBoxUpper.y;
		conAddon.conItems.fullCon.hintBoxTotal.w = conAddon.conItems.fullCon.hintBoxUpper.w;
		conAddon.conItems.fullCon.hintBoxTotal.h = conAddon.conItems.fullCon.hintBoxUpper.h;
			
		ConDraw_Box(
		/*    c		*/ Game::con->outputVisible ? glob_con_col : color,
		/*	  x		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.x : conAddon.conItems.smallCon.hintBoxUpper.x,
		/*	  y		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.y : conAddon.conItems.smallCon.hintBoxUpper.y,
		/*	  w		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.w : conAddon.conItems.smallCon.hintBoxUpper.w,
		/*	  h		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.h : conAddon.conItems.smallCon.hintBoxUpper.h,
		/*  depth	*/ false,
		/* ignoreFS	*/ true );


		// hintbox shadow --------------------------------------------------------
		float shadow_color[4] = { 0.0f, 0.0f, 0.0f, 0.125f };

		ConDraw_Box(
			/*    c		*/ shadow_color,
			/*	  x		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.x : conAddon.conItems.smallCon.hintBoxUpper.x,
			/*	  y		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.y : conAddon.conItems.smallCon.hintBoxUpper.y,
			/*	  w		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.w : conAddon.conItems.smallCon.hintBoxUpper.w,
			/*	  h		*/ 3.0f,
			/*  depth	*/ false,
			/* ignoreFS	*/ true);
	}

	// mid-func hook to alter hintbox drawing (lower box with dvar description) -- gets called after ConDrawInput_Box_DetailedMatch_UpperBox
	void ConDrawInput_Box_DetailedMatch_LowerBox(float *color, int lines)
	{
		// Game::conDrawInputGlob->y changes somewhere between ConDrawInput_Box_DetailedMatch_UpperBox & ConDrawInput_Box_DetailedMatch_LowerBox
		// so we only add Game::conDrawInputGlob->y to our lower hint box

		// small console offsets --------------------------------------------------------
		conAddon.conItems.smallCon.hintBoxLower.x = conAddon.conItems.smallCon.hintBoxUpper.x;
		conAddon.conItems.smallCon.hintBoxLower.y = Game::conDrawInputGlob->y - 8.0f - 15.0f + conDrawInputGlobY_fuckery;
		conAddon.conItems.smallCon.hintBoxLower.h = lines * Game::conDrawInputGlob->fontHeight + 16.0f;
		conAddon.conItems.smallCon.hintBoxLower.w = conAddon.conItems.smallCon.hintBoxUpper.w;

		// full console offsets --------------------------------------------------------
		conAddon.conItems.fullCon.hintBoxLower.x = conAddon.conItems.fullCon.hintBoxUpper.x;
		conAddon.conItems.fullCon.hintBoxLower.y = Game::conDrawInputGlob->y - 10.0f - 11.0f + conDrawInputGlobY_fuckery;
		conAddon.conItems.fullCon.hintBoxLower.h = lines * Game::conDrawInputGlob->fontHeight + 16.0f;// + 4.0f;
		conAddon.conItems.fullCon.hintBoxLower.w = conAddon.conItems.fullCon.hintBoxUpper.w;


		// get dimensions of upper + lower hint box (needed for clipping)
		// add lower box to upper box ::

		// small console total hintbox height part 2/2 --------------------------------------------------------
		conAddon.conItems.smallCon.hintBoxTotal.h += conAddon.conItems.smallCon.hintBoxLower.h;

		// full console total hintbox height part 2/2 --------------------------------------------------------
		conAddon.conItems.fullCon.hintBoxTotal.h += conAddon.conItems.fullCon.hintBoxLower.h;


		ConDraw_Box(
		/*    c		*/ Game::con->outputVisible ? glob_con_col : color,
		/*	  x		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxLower.x : conAddon.conItems.smallCon.hintBoxLower.x,//own_x,
		/*	  y		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxLower.y : conAddon.conItems.smallCon.hintBoxLower.y,//own_y,
		/*	  w		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxLower.w : conAddon.conItems.smallCon.hintBoxLower.w,//own_w,
		/*	  h		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxLower.h : conAddon.conItems.smallCon.hintBoxLower.h,//own_h,
		/*  depth	*/ false,
		/* ignoreFS	*/ true);
	}

	bool fltCon_cursorOutOfBounds(glm::vec2 bounds, const Game::conAddons_CursorPos *cursorPos)
	{
		if (cursorPos->x < 0.0f || cursorPos->x > bounds.x) 
		{
			return true;
		}

		if (cursorPos->y < 0.0f || cursorPos->y > bounds.y) 
		{
			return true;
		}

		return false;
	}

	bool fltCon_isCursorWithinItemBoxBounds(const Game::conItem_box *box, const Game::conAddons_CursorPos *cursorPos)
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

	void fltCon_clipAgainstBorders()
	{
		// only clip the console when its open
		if (!(Game::clientUI->keyCatchers & 1)) 
		{
			return;
		}

		// -------------------------------------------
		// clip the console against the window borders

		// clip at left border
		if (conAddon.fltCon.fltAnker.left <= 0.0f)
		{
			conAddon.fltCon.fltAnker.left = 0.0f;
			conAddon.fltCon.fltClippedAtBorder.left = true;

			// if we resize by pushing into the left border or with the resize button, limit the width to min width
			if (conAddon.fltCon.fltAnker.left + conAddon.fltCon.fltAnker.right < conAddon.fltCon.fltMinDimensions.width) 
			{
				conAddon.fltCon.fltAnker.right = conAddon.fltCon.fltMinDimensions.width;
			}
		}
		else 
		{
			// not clipped at border
			conAddon.fltCon.fltClippedAtBorder.left = false;
		}

		// min width check on resizing with resize button
		if (conAddon.fltCon.fltAnker.right - conAddon.fltCon.fltAnker.left < conAddon.fltCon.fltMinDimensions.width) 
		{
			conAddon.fltCon.fltAnker.right = conAddon.fltCon.fltMinDimensions.width + conAddon.fltCon.fltAnker.left;
		}

		// --------------------------------------------

		// clip at top border
		if (conAddon.fltCon.fltAnker.top <= 0.0f)
		{
			conAddon.fltCon.fltAnker.top = 0.0f;
			conAddon.fltCon.fltClippedAtBorder.top = true;
		}
		else 
		{
			// not clipped at border
			conAddon.fltCon.fltClippedAtBorder.top = false;
		}

		// --------------------------------------------

		// clip at right border
		if (conAddon.fltCon.fltAnker.right >= conAddon.viewportRes.width)
		{
			conAddon.fltCon.fltAnker.right = conAddon.viewportRes.width;
			conAddon.fltCon.fltClippedAtBorder.right = true;

			// if we resize by pushing into the right border or with the resize button, limit the width to min width
			if (conAddon.fltCon.fltAnker.right - conAddon.fltCon.fltAnker.left < conAddon.fltCon.fltMinDimensions.width) 
			{
				conAddon.fltCon.fltAnker.left = conAddon.fltCon.fltAnker.right - conAddon.fltCon.fltMinDimensions.width;
			}
		}
		else 
		{
			// not clipped at border
			conAddon.fltCon.fltClippedAtBorder.right = false;
		}

		// --------------------------------------------

		// clip at bottom border
		// we cannot clip the console at the bottom of the screen with con->screenMax[1] (bottom) 
		// because con->screenMax[1] is not used in any logic for the small console
		// so we clip con->screenMin[1] (top) instead

		// calculate the bottom position of the console and check if hits the bottom border of the screen
		if (conAddon.fltCon.fltAnker.top + conAddon.conItems.smallCon.outputBox.h + conAddon.conItems.smallCon.inputBox.h >= conAddon.viewportRes.height)
		{
			// subtract from the bottom towards the top to get our min y
			conAddon.fltCon.fltAnker.top = conAddon.viewportRes.height - conAddon.conItems.smallCon.outputBox.h - conAddon.conItems.smallCon.inputBox.h;
			conAddon.fltCon.fltClippedAtBorder.bottom = true;
		}
		else 
		{
			// not clipped at border
			conAddon.fltCon.fltClippedAtBorder.bottom = false;
		}
	}

	// fixes for consoleFont pixelHeight changes when loading / unloading mods / vid_restart?
	void xo_con_CheckResize_On_Init()
	{
		// left HORIZONTAL_APPLY_LEFT
		Game::con->screenMin[0] = floorf(_UI::ScrPlace_ApplyX(HORIZONTAL_APPLY_LEFT, Dvars::xo_con_padding->current.value, 0.0f));
		
		// top
		Game::con->screenMin[1] = floorf(_UI::ScrPlace_ApplyY(VERTICAL_APPLY_TOP, Dvars::xo_con_padding->current.value, 0.0f));

		// right
		Game::con->screenMax[0] = floorf(_UI::ScrPlace_ApplyX(HORIZONTAL_APPLY_RIGHT, -Dvars::xo_con_padding->current.value, 0.0f));

		// bottom
		Game::con->screenMax[1] = floorf(_UI::ScrPlace_ApplyY(VERTICAL_APPLY_BOTTOM, -Dvars::xo_con_padding->current.value, 0.0f));

		// reinit floating console on vid_restart / cgame changes or w/e
		conAddon.fltCon.flt_initialized = false;

		// still needed as we do that in DrawInput? ... cba to test
		conAddon.viewportRes.width = floorf(_UI::ScrPlace_ApplyX(HORIZONTAL_APPLY_RIGHT, 0.0f, 0.0f));
		conAddon.viewportRes.height = floorf(_UI::ScrPlace_ApplyY(VERTICAL_APPLY_BOTTOM, 0.0f, 0.0f));

		// mouse unrelated stuff ---------------------------------------
		auto consoleFont = GET_CONSOLEFONT;
		if (consoleFont)
		{
			if (consoleFont->pixelHeight != 16)
			{
				Game::Com_PrintMessage(0, Utils::VA("Console :: consoleFont was %d", consoleFont->pixelHeight), 0);
			}

			Game::con->fontHeight			= (std::int32_t)(consoleFont->pixelHeight * Dvars::xo_con_fontSpacing->current.value);
			Game::con->visibleLineCount		= (std::int32_t)(Game::con->screenMax[1] - Game::con->screenMin[1] - (2 * Game::con->fontHeight)) / Game::con->fontHeight; //  - xo_con_fontPaddingBottom->current.value) / Game::con->fontHeight;
			Game::con->visiblePixelWidth	= (std::int32_t)(Game::con->screenMax[0] - Game::con->screenMin[0] - -28.0f);
		}

		else
		{
			Game::con->fontHeight = 0;
			Game::con->visibleLineCount = 0;
			Game::con->visiblePixelWidth = 0;
		}
	}

	// called from scheduler as we need 2 hooks at this place
	void XO_Console::xo_con_CheckResize()
	{
		// ------------------------------------------------------------
		// Cursor toggle key

		// settings
		int cursorToggleDelay = 100;

		// only count till 300, start again when conAddon.cursorToggleTimeout was reset
		if (conAddon.cursorToggleTimeout < cursorToggleDelay) 
		{
			conAddon.cursorToggleTimeout++;
		}
		
		// if console is open
		if (Game::clientUI->keyCatchers & 1) 
		{ 
			// if F1 and not showing cursor -> activate
			if (Game::playerKeys->keys[CON_KEY_TOGGLE_CURSOR].down && !Dvars::xo_con_cursorState->current.enabled && conAddon.cursorToggleTimeout >= cursorToggleDelay)
			{
				conAddon.cursorToggleTimeout = 0;
				Game::Dvar_SetValue(Dvars::xo_con_cursorState, true);
				Game::UI_SetActiveMenu(0, 5);
			}

			// if F1 and showing cursor -> deactivate
			else if (Game::playerKeys->keys[CON_KEY_TOGGLE_CURSOR].down && Dvars::xo_con_cursorState->current.enabled && conAddon.cursorToggleTimeout >= cursorToggleDelay) 
			{
				goto DISABLE_CURSOR;
			}
		}
		else
		{
			// if console is not open but we still have the cursor menu open, close it
			if (Dvars::xo_con_cursorState->current.enabled)
			{
				DISABLE_CURSOR:
				conAddon.cursorToggleTimeout = 0;
				Game::Dvar_SetValue(Dvars::xo_con_cursorState, false);

				Game::UiContext *ui = &Game::_uiContext[0];
				Game::Menus_CloseByName("pregame_loaderror_mp", ui);
			}
		}

		// ------------------------------------------------------------

		// one time init on start / vid_restart / cgame changes or w/e
		if (!conAddon.fltCon.flt_initialized)
		{
			conAddon.viewportRes.width = floorf(_UI::ScrPlace_ApplyX(HORIZONTAL_APPLY_RIGHT, 0.0f, 0.0f));
			conAddon.viewportRes.height = floorf(_UI::ScrPlace_ApplyY(VERTICAL_APPLY_BOTTOM, 0.0f, 0.0f));

			// min width / height of the floating console
			conAddon.fltCon.fltMinDimensions.width = 840.0f;
			conAddon.fltCon.fltMinDimensions.height = 470.0f;
		}

		// get scaled mouse cursor (640/480 <-> Game Resolution)
		conAddon.cursorPos.x = Utils::floatToRange(0.0f, 640.0f, 0.0f, (float) Game::_uiContext->screenWidth,  Game::_uiContext->cursor.x);
		conAddon.cursorPos.y = Utils::floatToRange(0.0f, 480.0f, 0.0f, (float) Game::_uiContext->screenHeight, Game::_uiContext->cursor.y);

		// enter if floating console state was saved to the config
		// otherwise skip the first frame to fully init the console
		// or skip if the fullscreen console is active
		if ((conAddon.fltCon.flt_initialized && !Game::con->outputVisible) || (Dvars::xo_con_fltCon->current.enabled && !Game::con->outputVisible))
		{
			// if mouse down && console is actually open -- else use the last floating position
			if (Game::playerKeys->keys[KEYCATCHER_MOUSE1].down && Game::clientUI->keyCatchers & 1)
			{
				// ----------------
				// Console Resizing

				// check if the cursor is on the resize button -- we have to check resize first as the button is ontop of the grab bar
				if ((conAddon.fltCon.flt_isResizing || fltCon_isCursorWithinItemBoxBounds(&conAddon.conItems.smallCon.resizeBtnTrigger, &conAddon.cursorPos)) && !conAddon.fltCon.flt_isMoving)
				{
					// trigger on first click only 
					if (!conAddon.fltCon.flt_isResizing)
					{
						// save the current cursor position for the next frame
						conAddon.cursorPosSaved.x = conAddon.cursorPos.x;
						conAddon.cursorPosSaved.y = conAddon.cursorPos.y;

						// the position where we hit the resize button
						conAddon.cursorPosSavedOnClick.x = conAddon.cursorPos.x;
						conAddon.cursorPosSavedOnClick.y = conAddon.cursorPos.y;
					}

					if (conAddon.fltCon.flt_isResizing) {
						// can be used to skip something in the "first click frame"
					}

					// resizing console loop
					// ---------------------

					// update width / height
					conAddon.fltCon.fltDimensions.width = conAddon.fltCon.fltAnker.right - conAddon.fltCon.fltAnker.left;
					conAddon.fltCon.fltDimensions.height = conAddon.fltCon.fltAnker.bottom - conAddon.fltCon.fltAnker.top;

					// get difference between the cursor of the prior frame and the current pos and resize the console with that
					conAddon.fltCon.fltAnker.right -= conAddon.cursorPosSaved.x - conAddon.cursorPos.x;
					
					// change outputHeight downwards
					if (conAddon.cursorPos.y - conAddon.cursorPosSavedOnClick.y >= Game::con->fontHeight * 0.85f)
					{
						conAddon.cursorPosSavedOnClick.y = conAddon.cursorPos.y;

						if (Dvars::xo_con_outputHeight->current.integer + 1 <= Dvars::xo_con_outputHeight->domain.integer.max) 
						{
							Game::Dvar_SetValue(Dvars::xo_con_outputHeight, Dvars::xo_con_outputHeight->current.integer + 1);
						}
					}
					
					// change outputHeight upwards
					if (conAddon.cursorPosSavedOnClick.y - conAddon.cursorPos.y >= Game::con->fontHeight * 0.85f)
					{
						conAddon.cursorPosSavedOnClick.y = conAddon.cursorPos.y;

						if (Dvars::xo_con_outputHeight->current.integer - 1 >= Dvars::xo_con_outputHeight->domain.integer.min) 
						{
							Game::Dvar_SetValue(Dvars::xo_con_outputHeight, Dvars::xo_con_outputHeight->current.integer - 1);
						}
					}

					// update bottom anker
					conAddon.fltCon.fltAnker.bottom = conAddon.fltCon.fltAnker.top + conAddon.conItems.smallCon.outputBox.h + conAddon.conItems.smallCon.inputBox.h;

					// ---------------------------------------------------

					// save the current cursor position for the next frame
					conAddon.cursorPosSaved.x = conAddon.cursorPos.x;
					conAddon.cursorPosSaved.y = conAddon.cursorPos.y;

					conAddon.fltCon.flt_enabled		= true;
					conAddon.fltCon.flt_isResizing	= true;
				}

				// -------------------
				// Console Translation

				// check if cursor is within the "grab" bar (input text box)
				// skip if the console is being moved
				else if((conAddon.fltCon.flt_isMoving || fltCon_isCursorWithinItemBoxBounds(&conAddon.conItems.smallCon.inputBox, &conAddon.cursorPos)) && !conAddon.fltCon.flt_isResizing)
				{ 
					// trigger on first click only 
					if (!conAddon.fltCon.flt_isMoving)
					{
						// console is now floating
						conAddon.fltCon.flt_enabled = true;

						// get initial console position and dimensions if the floating console wasnt in use yet / was reset
						if (!conAddon.fltCon.flt_wasUsed || conAddon.fltCon.flt_wasReset)
						{
							conAddon.fltCon.fltAnker.left	= Game::con->screenMin[0];
							conAddon.fltCon.fltAnker.top	= Game::con->screenMin[1];
							conAddon.fltCon.fltAnker.right	= Game::con->screenMax[0];
							conAddon.fltCon.fltAnker.bottom = Game::con->screenMax[1];
						}
						
						// now the floating console will retain its last position even if the fullscreen console was active
						conAddon.fltCon.flt_wasUsed = true;
						conAddon.fltCon.flt_wasReset = false;

						// calculate width / height
						conAddon.fltCon.fltDimensions.width = conAddon.fltCon.fltAnker.right - conAddon.fltCon.fltAnker.left;
						conAddon.fltCon.fltDimensions.height = conAddon.fltCon.fltAnker.bottom - conAddon.fltCon.fltAnker.top;

						// save the current cursor position for the next frame
						conAddon.cursorPosSaved.x = conAddon.cursorPos.x;
						conAddon.cursorPosSaved.y = conAddon.cursorPos.y;
					}

					if (conAddon.fltCon.flt_isMoving) { 
						// can be used to skip something in the "first click frame"
					}

					// draggin console loop
					// ---------------------------------------------------

					// update width / height
					conAddon.fltCon.fltDimensions.width  = conAddon.fltCon.fltAnker.right - conAddon.fltCon.fltAnker.left;
					conAddon.fltCon.fltDimensions.height = conAddon.fltCon.fltAnker.bottom - conAddon.fltCon.fltAnker.top;

					// update floating console position every frame / resize it by clipping at borders
					// get difference between the cursor of the prior frame and the current pos and translate the console with that
					
					conAddon.fltCon.fltAnker.left	-= conAddon.cursorPosSaved.x - conAddon.cursorPos.x;
					conAddon.fltCon.fltAnker.top	-= conAddon.cursorPosSaved.y - conAddon.cursorPos.y;
					conAddon.fltCon.fltAnker.right	-= conAddon.cursorPosSaved.x - conAddon.cursorPos.x;

					// ---------------------------------------------------

					// save the current cursor position for the next frame
					conAddon.cursorPosSaved.x = conAddon.cursorPos.x;
					conAddon.cursorPosSaved.y = conAddon.cursorPos.y;

					// clip the console against screen bounds
					fltCon_clipAgainstBorders();

					// ---------------------------------------------------

					// update bottom anker
					conAddon.fltCon.fltAnker.bottom = conAddon.fltCon.fltAnker.top + conAddon.conItems.smallCon.outputBox.h + conAddon.conItems.smallCon.inputBox.h;

					// console grabbed with cursor (setting this here allows us to skip something in the "first click frame" )
					conAddon.fltCon.flt_isMoving = true;
				}
			}

			// non moving floating console loop
			// ---------------------------------------------------

			// if mouse is not hold down anymore after the console was dragged / resized -- save current position to config
			else if (!Game::playerKeys->keys[KEYCATCHER_MOUSE1].down && (conAddon.fltCon.flt_isMoving || conAddon.fltCon.flt_isResizing))
			{
				conAddon.fltCon.flt_isMoving = false;
				conAddon.fltCon.flt_isResizing = false;

				// set value doesnt really work here
				Game::Dvar_SetFloat(Dvars::xo_con_fltConLeft, conAddon.fltCon.fltAnker.left, 0);
				Game::Dvar_SetFloat(Dvars::xo_con_fltConTop, conAddon.fltCon.fltAnker.top, 0);
				Game::Dvar_SetFloat(Dvars::xo_con_fltConRight, conAddon.fltCon.fltAnker.right, 0);
				Game::Dvar_SetFloat(Dvars::xo_con_fltConBottom, conAddon.fltCon.fltAnker.bottom, 0);

				Game::Cmd_ExecuteSingleCommand(0, 0, "xo_con_fltCon 1\n");
			}

			// clip the console against screen bounds when not moving the console or when we are resizing it
			if (!conAddon.fltCon.flt_isMoving || conAddon.fltCon.flt_isResizing) 
			{
				fltCon_clipAgainstBorders();
			}

			// if floating console was active and did not get reset, activate it to retain its last position
			if ((conAddon.fltCon.flt_wasUsed && !conAddon.fltCon.flt_wasReset) ) 
			{
				conAddon.fltCon.flt_enabled = true;
			}

			// if there is a saved state in the config file -- only on init
			if(Dvars::xo_con_fltCon->current.enabled && !conAddon.fltCon.flt_initialized)
			{
				conAddon.fltCon.fltAnker.left	= Dvars::xo_con_fltConLeft->current.value;
				conAddon.fltCon.fltAnker.top	= Dvars::xo_con_fltConTop->current.value;
				conAddon.fltCon.fltAnker.right	= Dvars::xo_con_fltConRight->current.value;
				conAddon.fltCon.fltAnker.bottom = Dvars::xo_con_fltConBottom->current.value;

				// update width / height
				conAddon.fltCon.fltDimensions.width = conAddon.fltCon.fltAnker.right - conAddon.fltCon.fltAnker.left;
				conAddon.fltCon.fltDimensions.height = conAddon.fltCon.fltAnker.bottom - conAddon.fltCon.fltAnker.top;

				// update conAddon.fltCon.fltAnker.bottom
				//conAddon.fltCon.fltAnker.bottom = conAddon.fltCon.fltAnker.top + conAddon.conItems.smallCon.outputBox.h + conAddon.conItems.smallCon.inputBox.h;

				conAddon.fltCon.flt_enabled = true;
				conAddon.fltCon.flt_wasUsed = true;
				conAddon.fltCon.flt_wasReset = false;
			}
				
			// if the console is floating, update x/y and width/height here
			Game::con->screenMin[0] = conAddon.fltCon.fltAnker.left;
			Game::con->screenMin[1] = conAddon.fltCon.fltAnker.top;
			Game::con->screenMax[0] = conAddon.fltCon.fltAnker.right;
			Game::con->screenMax[1] = conAddon.fltCon.fltAnker.bottom;

			// fix for small hintbox (too many to display) getting stuck at the center after fullcon was used
			if (conAddon.fltCon.flt_enabled && !conAddon.fltCon.flt_isMoving) 
			{ 
				// update bottom anker
				conAddon.fltCon.fltAnker.bottom = conAddon.fltCon.fltAnker.top + conAddon.conItems.smallCon.outputBox.h + conAddon.conItems.smallCon.inputBox.h;
			}

			// draw a resize button with a trigger box bottom right
			// do not draw it here, but in some console drawing function as we are a frame behind
			// xo_con_DrawResizeBtn();
		}

		// reset floating console to aligned console with the END key while mouse 1 is not pressed
		if (Game::playerKeys->keys[CON_KEY_RESET_FLTCON].down &! Game::playerKeys->keys[KEYCATCHER_MOUSE1].down && conAddon.fltCon.flt_enabled || Game::con->outputVisible)
		{
			// disable floating console when the fullscreen console is active
			conAddon.fltCon.flt_enabled = false;
			conAddon.fltCon.flt_isMoving = false;
			conAddon.fltCon.flt_isResizing = false;

			Game::Cmd_ExecuteSingleCommand(0, 0, "xo_con_fltCon 0\n");

			// only reset the floating console with the reset key and not when the fullscreen console is active
			if (!Game::con->outputVisible) 
			{
				conAddon.fltCon.flt_wasReset = true;
			}
		}

		// do not set defaults when we load a saved floating console state on initialization
		// non floating console / fullscreen console position and dimensions
		if ((!conAddon.fltCon.flt_initialized && !Dvars::xo_con_fltCon->current.enabled) || !conAddon.fltCon.flt_enabled || Game::con->outputVisible)
		{
			// left HORIZONTAL_APPLY_LEFT
			Game::con->screenMin[0] = floorf(_UI::ScrPlace_ApplyX(HORIZONTAL_APPLY_LEFT, Dvars::xo_con_padding->current.value, 0.0f));

			// top
			Game::con->screenMin[1] = floorf(_UI::ScrPlace_ApplyY(VERTICAL_APPLY_TOP, Dvars::xo_con_padding->current.value, 0.0f));

			// right
			Game::con->screenMax[0] = floorf(_UI::ScrPlace_ApplyX(HORIZONTAL_APPLY_RIGHT, -Dvars::xo_con_padding->current.value, 0.0f));

			// bottom
			Game::con->screenMax[1] = floorf(_UI::ScrPlace_ApplyY(VERTICAL_APPLY_BOTTOM, -Dvars::xo_con_padding->current.value, 0.0f));

			// keep our struct updated even when the floating console isnt in use -- why did i do this?
			//conAddon.fltCon.fltAnker.left	= Game::con->screenMin[0];
			//conAddon.fltCon.fltAnker.top	= Game::con->screenMin[1];
			//conAddon.fltCon.fltAnker.right	= Game::con->screenMax[0];
			//conAddon.fltCon.fltAnker.bottom = Game::con->screenMax[1];
		}


		// mouse unrelated stuff ---------------------------------------
		auto consoleFont = GET_CONSOLEFONT;
		if (consoleFont)
		{
			if (consoleFont->pixelHeight != 16)
			{
				Game::Com_PrintMessage(0, Utils::VA("Console :: consoleFont was %d", consoleFont->pixelHeight), 0);
			}

			Game::con->fontHeight = (std::int32_t)(consoleFont->pixelHeight * Dvars::xo_con_fontSpacing->current.value);

			// adjust visibleLineCount for output text so we do not draw more lines then our rect can hold
			if (Game::con->fontHeight) 
			{
				if (Game::con->outputVisible) 
				{
					// full console
					Game::con->visibleLineCount = (int)((conAddon.conItems.fullCon.outputBox.h - (Game::con->fontHeight * 2)) / Game::con->fontHeight);
				}
				else 
				{ 
					// small console
					Game::con->visibleLineCount = (int)((conAddon.conItems.smallCon.outputBox.h - Game::con->fontHeight) / Game::con->fontHeight);
				}
			} 
			else if(DEBUG) 
			{
				Game::Com_PrintMessage(0, Utils::VA("^1Con_CheckResize L#%d ^7:: con->fontHeight was NULL \n", __LINE__), 0);
			}

			Game::con->visiblePixelWidth = (std::int32_t)(Game::con->screenMax[0] - Game::con->screenMin[0] - -28.0f);
		}

		else
		{
			Game::con->fontHeight			= 0;
			Game::con->visibleLineCount		= 0;
			Game::con->visiblePixelWidth	= 0;
		}

		if (!conAddon.fltCon.flt_initialized && !conAddon.fltCon.flt_enabled)
		{
			// print help text on init
			Game::Com_PrintMessage(0, "\n", 0);
			Game::Com_PrintMessage(0, Utils::VA("%s", CON_HELP_PRINT), 0);
		}
		
		// floating console initialized
		conAddon.fltCon.flt_initialized = true;
	}

	// draw a resize button at the bottom right of the console input box
	void xo_con_DrawResizeBtn(const float alpha)
	{
		glm::vec4 right, bottom;

		float buttonHeight = conAddon.conItems.smallCon.inputBox.h;
		float buttonWidth = buttonHeight;

		float lineStrength = 0.5f;
		float _alpha = alpha;

		if (_alpha + 0.2f <= 1.0f) 
		{
			_alpha += 0.2f;
		}

		float resizeBtnColor[4] = { 1.0f, 1.0f, 1.0f, _alpha };
		float padding[2] = { 3.0f, 4.5f };

		right.x = conAddon.conItems.smallCon.inputBox.x + conAddon.conItems.smallCon.inputBox.w - padding[0] - lineStrength;
		right.y = conAddon.conItems.smallCon.inputBox.y + conAddon.conItems.smallCon.inputBox.h * 0.5f; // draw from top to bottom -- half height of input bar
		right.z = lineStrength; // line thickness
		right.w = conAddon.conItems.smallCon.inputBox.h * 0.5f - padding[0];

		// right vertical bar
		Game::ConDraw_Box(resizeBtnColor,
			/* x */	right.x,
			/* y */ right.y,
			/* w */ right.z,
			/* h */ right.w);

		bottom.x = conAddon.conItems.smallCon.inputBox.x + conAddon.conItems.smallCon.inputBox.w - (conAddon.conItems.smallCon.inputBox.h * 0.5f); // draw from left to right so - its width
		bottom.y = conAddon.conItems.smallCon.inputBox.y + conAddon.conItems.smallCon.inputBox.h - padding[0] - 0.5f;
		bottom.z = conAddon.conItems.smallCon.inputBox.h - 18.0f;
		bottom.w = 0.5f; // line thickness

		// bottom horizontal bar
		Game::ConDraw_Box(resizeBtnColor,
			/* x */	bottom.x,
			/* y */ bottom.y,
			/* w */ bottom.z,
			/* h */ bottom.w);

		// ------------
		// small square

		resizeBtnColor[3] -= 0.25f;

		Game::ConDraw_Box(resizeBtnColor,
			/* x */	right.x - lineStrength - padding[1] - 3.0f,
			/* y */ bottom.y - padding[1] - 4.0f,
			/* w */ 4.0f,
			/* h */ 4.0f);

		conAddon.conItems.smallCon.resizeBtnTrigger.x = bottom.x - 2.0f;// +(bottom.z * 0.5f);
		conAddon.conItems.smallCon.resizeBtnTrigger.y = right.y - 2.0f;// +(bottom.w * 0.5f);
		conAddon.conItems.smallCon.resizeBtnTrigger.w = bottom.z + lineStrength + 4.0f;
		conAddon.conItems.smallCon.resizeBtnTrigger.h = right.w + 5.0f;

		//float debugTriggerColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

		//// debug :: draw trigger
		//Game::ConDraw_Box(debugTriggerColor,
		//	/* x */	conAddon.conItems.smallCon.resizeBtnTrigger.x,
		//	/* y */ conAddon.conItems.smallCon.resizeBtnTrigger.y,
		//	/* w */ conAddon.conItems.smallCon.resizeBtnTrigger.w,
		//	/* h */ conAddon.conItems.smallCon.resizeBtnTrigger.h);
	}

	void xo_con_DrawOutputScrollBar(float x, float y, float width, float height, float padding = 0.0f)
	{
		auto outputBarColor		= (float *)&Game::Dvar_FindVar("con_outputBarColor")->current.integer;
		auto outputSliderColor	= (float *)&Game::Dvar_FindVar("con_outputSliderColor")->current.integer;

		float scrollbar_BackgroundPosY		= y + padding;
		float scrollbar_BackgroundHeight	= height;
		float scrollbar_BackgroundWidth		= 16.0f;
		float scrollbar_BackgroundPosX		= x + width - scrollbar_BackgroundWidth;

		if (Game::con->outputVisible) 
		{
			scrollbar_BackgroundHeight -= padding;
		}

		float scrollbar_NobWidth		= 16.0f;
		float scrollbar_NobMinHeight	= 30.0f;
		float scrollbar_NobPadding		= (scrollbar_BackgroundWidth - scrollbar_NobWidth) * 0.5f;

		// Background
		Game::ConDraw_Box(outputBarColor,
			/* x */	scrollbar_BackgroundPosX,
			/* y */ scrollbar_BackgroundPosY,
			/* w */ scrollbar_BackgroundWidth,
			/* h */ scrollbar_BackgroundHeight);

		// if there is content out of bounds (scrolling enabled)
		if (Game::con->consoleWindow.activeLineCount > Game::con->visibleLineCount)
		{
			float cursorPos, portion, scrollbar_NobY, scrollbar_NobLength;

			float hiddenLineCount = (float)(Game::con->consoleWindow.activeLineCount - Game::con->visibleLineCount);

			// current nob length
			portion				= (scrollbar_BackgroundHeight * 0.01f) * hiddenLineCount;	// portion :: 1 percent of max nob height times invisible lines
			scrollbar_NobLength = (scrollbar_BackgroundHeight - portion);					// max nob height - portion;

			// if scaled nob length < min length
			if (scrollbar_NobLength < scrollbar_NobMinHeight) {
				scrollbar_NobLength = scrollbar_NobMinHeight;
			}

			// current nob y position
			cursorPos		= (float)(Game::con->displayLineOffset - Game::con->visibleLineCount) * (1.0f / hiddenLineCount);
			scrollbar_NobY	= (y + height - scrollbar_NobLength - y) * Utils::fmaxOf3(cursorPos, 0.0f, 1.0f) + y;

			// cap scrollbar nob top
			if (scrollbar_NobY < scrollbar_BackgroundPosY) {
				scrollbar_NobY = scrollbar_BackgroundPosY;
			}

			Game::ConDraw_Box(outputSliderColor,
				/* x */ scrollbar_BackgroundPosX + scrollbar_NobPadding,
				/* y */ scrollbar_NobY,
				/* w */ scrollbar_NobWidth - scrollbar_NobPadding,
				/* h */ scrollbar_NobLength);
		}

		else // we have nothing to scroll
		{
			Game::ConDraw_Box(outputSliderColor,
				/* x */ scrollbar_BackgroundPosX + scrollbar_NobPadding,
				/* y */ scrollbar_BackgroundPosY,
				/* w */ scrollbar_NobWidth - scrollbar_NobPadding,
				/* h */ scrollbar_BackgroundHeight);
		}
	}

	void xo_con_DrawOutputVersion(float x, float y, float height)
	{
		auto build = "";

		if (DEBUG) 
		{
			build = Utils::VA(IW3XO_BUILDSTRING " :: ^1DEBUG");
		}
		else 
		{
			build = Utils::VA(IW3XO_BUILDSTRING);
		}

		Game::SCR_DrawSmallStringExt((int)(x), (int)(height - 16.0f + y), build);
	}

	void xo_con_DrawOutputText(float x, float y, float height = 0.0f)
	{
		// Game::con->visibleLineCount in Con_CheckResize defines the container-height (can be used as cutoff) 
		// rowIndex / rowIndex == top line (adjusting it cuts off the newest lines)
		// rowCount == lines at the bottom (^ oldest lines)

		int rowCount, firstRow, lineIndex, rowIndex;

		int maxLinesInRect		= Game::con->consoleWindow.lineCount; // 1024
		bool firstOffsetSkipped = false;

		rowCount = Game::con->visibleLineCount;
		firstRow = Game::con->displayLineOffset - Game::con->visibleLineCount;

		float linesForSpacing = ((Dvars::xo_con_fontSpacing->current.value + 0.2f) - 1.0f) * 10.0f;

		// no idea what i did here but it works
		y = y + height + (Game::con->fontHeight * 4) + (Game::con->fontHeight * 0.5f) - (linesForSpacing * Game::con->fontHeight * 0.5f);

		// adjust "cursor" 
		if (firstRow < 0)
		{
			y -= (float)(Game::con->fontHeight * firstRow);

			rowCount = Game::con->displayLineOffset;
			firstRow = 0;

			// first fullcon frame stuff here -- resets only when smallcon was used
			if (Game::con->outputVisible && !conAddon.fullConFirstFrame)
			{
				// if smallcon was scrolled to the top, adjust the line offset once to put our fullcon text at the top
				Game::con->displayLineOffset = Game::con->visibleLineCount;
				conAddon.fullConFirstFrame = true;
			}
		}

		// get font handle
		Game::Font_s *_font = reinterpret_cast<Game::Font_s*>(Game::R_RegisterFont(FONT_CONSOLE, sizeof(FONT_CONSOLE)));

		for (rowIndex = 0; rowIndex < rowCount; ++rowIndex)
		{
			lineIndex = (rowIndex + firstRow + Game::con->consoleWindow.firstLineIndex) % Game::con->consoleWindow.lineCount;
			
			// skip the first y offset as we add half a font height as padding
			if (firstOffsetSkipped) 
			{
				y += (float)Game::con->fontHeight;
			}
			else 
			{
				firstOffsetSkipped = true;
			}

			Game::AddBaseDrawConsoleTextCmd(
				/*charCount	*/	Game::con->consoleWindow.lines[lineIndex].textBufSize,
				/* colorFlt	*/	Game::COLOR_WHITE,
				/* textPool	*/	Game::con->consoleWindow.circularTextBuffer,
				/* poolSize	*/	Game::con->consoleWindow.textBufSize,
				/* firstChar*/	Game::con->consoleWindow.lines[lineIndex].textBufPos,
				/*   font	*/	_font,
				/*     x	*/	x,
				/*     y	*/	y,
				/*  xScale	*/	1.0f,
				/*  yScale	*/	1.0f,
				/*   style	*/	3); // 0 :: default -- 3 has slight shadows -- 6 even more :)
		}
	}

	void xo_con_DrawOutputWindow()
	{
		// output box
		conAddon.conItems.fullCon.outputBox.x = conAddon.conItems.fullCon.inputBox.x;
		conAddon.conItems.fullCon.outputBox.y = conAddon.conItems.fullCon.inputBox.y + conAddon.conItems.fullCon.inputBox.h;
		conAddon.conItems.fullCon.outputBox.w = conAddon.conItems.fullCon.inputBox.w;
		conAddon.conItems.fullCon.outputBox.h = Game::con->screenMax[1] - conAddon.conItems.fullCon.outputBox.y;

		auto outputWindowColor = (float *)&Game::Dvar_FindVar("con_outputWindowColor")->current.integer;

		// output window
		ConDraw_Box(
			/*	 color	*/	outputWindowColor,
			/*	   x	*/	conAddon.conItems.fullCon.outputBox.x,
			/*	   y	*/	conAddon.conItems.fullCon.outputBox.y,
			/*	   w	*/	conAddon.conItems.fullCon.outputBox.w,
			/*	   h	*/	conAddon.conItems.fullCon.outputBox.h,
			/*	depth	*/	false,
			/* ignoreFS	*/	true);

		// scrollbar
		xo_con_DrawOutputScrollBar(conAddon.conItems.fullCon.outputBox.x, conAddon.conItems.fullCon.outputBox.y, conAddon.conItems.fullCon.outputBox.w, conAddon.conItems.fullCon.outputBox.h, 0.0f);

		// build version long, bottom console
		xo_con_DrawOutputVersion(conAddon.conItems.fullCon.outputBox.x + 6.0f, conAddon.conItems.fullCon.outputBox.y - 4.0f, conAddon.conItems.fullCon.outputBox.h);

		// console output below input text box
		xo_con_DrawOutputText(conAddon.conItems.fullCon.outputBox.x + 6.0f, Dvars::xo_con_padding->current.value * 2.0f, 0.0f);
	}
	
	// :: Main Console Input + Logic ( Key logic still outside in Console_Key() )
	void xo_con_DrawInput()
	{
		if (!(*Game::Key_IsCatcherActive) || !Game::Sys_IsMainThread()) 
		{
			return;
		}

		float	x, y, w, h;

		// hintbox txt color dvars
		memcpy(&Game::con_matchtxtColor_currentDvar[0], &Dvars::xo_con_hintBoxTxtColor_currentDvar->current.vector, sizeof(float[4]));
		memcpy(&Game::con_matchtxtColor_currentValue[0], &Dvars::xo_con_hintBoxTxtColor_currentValue->current.vector, sizeof(float[4]));
		memcpy(&Game::con_matchtxtColor_defaultValue[0], &Dvars::xo_con_hintBoxTxtColor_defaultValue->current.vector, sizeof(float[4]));
		memcpy(&Game::con_matchtxtColor_dvarDescription[0], &Dvars::xo_con_hintBoxTxtColor_dvarDescription->current.vector, sizeof(float[4]));
		memcpy(&Game::con_matchtxtColor_domainDescription[0], &Dvars::xo_con_hintBoxTxtColor_domainDescription->current.vector, sizeof(float[4]));

		// increase max drawing width for console input
		Game::g_consoleField->drawWidth = 512;

		// copy "con_inputBoxColor" to dynamically change its alpha without changing the dvar
		//float loc_inputBoxColor[4];
		//memcpy(&loc_inputBoxColor, Game::Dvar_FindVar("con_inputBoxColor")->current.vector, sizeof(loc_inputBoxColor));
		auto loc_inputBoxColor = Game::Dvar_FindVar("con_inputBoxColor")->current.vector;

		// set con globals
		auto consoleFont = GET_CONSOLEFONT;
		Game::conDrawInputGlob->fontHeight = (float)(consoleFont->pixelHeight);
		Game::conDrawInputGlob->x = Game::con->screenMin[0];
		Game::conDrawInputGlob->y = Game::con->screenMin[1];
		Game::conDrawInputGlob->leftX = Game::conDrawInputGlob->x;
		
		// :: if small console
		if (!Game::con->outputVisible)
		{
			// y = top of outputbox
			// h = height of outputbox

			x = Game::conDrawInputGlob->x;
			y = Game::conDrawInputGlob->y;
			w = Game::con->screenMax[0] - Game::con->screenMin[0] - (x - Game::con->screenMin[0]);
			h = (float)(Dvars::xo_con_outputHeight->current.integer + 4) * Game::con->fontHeight; // we have to add 4 because @ 5 we get the first line ..

			// input box
			float inputBoxHeight = Game::conDrawInputGlob->fontHeight + 15.0f;
			conAddon.conItems.smallCon.inputBox.x = x;
			conAddon.conItems.smallCon.inputBox.y = y + h - inputBoxHeight;
			conAddon.conItems.smallCon.inputBox.w = w;
			conAddon.conItems.smallCon.inputBox.h = inputBoxHeight;

			// output box
			conAddon.conItems.smallCon.outputBox.x = x;
			conAddon.conItems.smallCon.outputBox.y = y;
			conAddon.conItems.smallCon.outputBox.w = w;
			conAddon.conItems.smallCon.outputBox.h = h - conAddon.conItems.smallCon.inputBox.h;

			// output box text
			conAddon.conItems.smallCon.outputText.x = Game::conDrawInputGlob->x + 8.0f;
			conAddon.conItems.smallCon.outputText.y = y - h;
			conAddon.conItems.smallCon.outputText.h = h;

			// output box slider
			conAddon.conItems.smallCon.outputSlider.x = x;
			conAddon.conItems.smallCon.outputSlider.y = y;
			conAddon.conItems.smallCon.outputSlider.w = w;
			conAddon.conItems.smallCon.outputSlider.h = h - 32.0f;
			conAddon.conItems.smallCon.outputSlider.padding = 0.0f;

			// ------------------------------ // output box
			ConDraw_Box( 
			/* col */ Game::Dvar_FindVar("con_outputWindowColor")->current.vector,
			/*  x  */ conAddon.conItems.smallCon.outputBox.x,
			/*  y  */ conAddon.conItems.smallCon.outputBox.y,
			/*  z  */ conAddon.conItems.smallCon.outputBox.w,
			/*  h  */ conAddon.conItems.smallCon.outputBox.h,
			/* dep */ true);

			// ------------------------------ // output box text
			xo_con_DrawOutputText(
			/*	  x		*/ conAddon.conItems.smallCon.outputText.x,
			/*	  y		*/ conAddon.conItems.smallCon.outputText.y,
			/* bxHeight	*/ conAddon.conItems.smallCon.outputBox.h);

			// ------------------------------ // output box slider
			xo_con_DrawOutputScrollBar(
			/*  x  */ conAddon.conItems.smallCon.outputSlider.x, 
			/*  y  */ conAddon.conItems.smallCon.outputSlider.y, 
			/*  w  */ conAddon.conItems.smallCon.outputSlider.w, 
			/*  h  */ conAddon.conItems.smallCon.outputSlider.h,
			/* pad */ conAddon.conItems.smallCon.outputSlider.padding);
			

			// offset input text:
			Game::conDrawInputGlob->x += 5.0f;
			Game::conDrawInputGlob->y += h - 24.0f;

			// ------------------------------ // input box
			ConDraw_Box(
			/* col */ loc_inputBoxColor,
			/*  x  */ conAddon.conItems.smallCon.inputBox.x,
			/*  y  */ conAddon.conItems.smallCon.inputBox.y,
			/*  z  */ conAddon.conItems.smallCon.inputBox.w,
			/*  h  */ conAddon.conItems.smallCon.inputBox.h,
			/* dep */ true);

			// draw a resize button with a trigger box bottom right
			// we have to call that after getting the newest inputBox values or else we lack a frame behind
			xo_con_DrawResizeBtn(loc_inputBoxColor[3]);

			// reset fullcon first frame toggle
			conAddon.fullConFirstFrame = false;
		}

		// :: IF Fullscreen Console
		else
		{
			// y = top of outputbox
			// h = height of outputbox

			x = Game::conDrawInputGlob->x;
			y = Game::conDrawInputGlob->y;
			w = Game::con->screenMax[0] - Game::con->screenMin[0] - (x - Game::con->screenMin[0]);
			h = Game::conDrawInputGlob->fontHeight + 15.0f;

			// input box
			conAddon.conItems.fullCon.inputBox.x = x;
			conAddon.conItems.fullCon.inputBox.y = y;
			conAddon.conItems.fullCon.inputBox.w = w;
			conAddon.conItems.fullCon.inputBox.h = h;

			// ------------------------------ // input box
			ConDraw_Box(
			/* col */ loc_inputBoxColor,
			/*  x  */ conAddon.conItems.fullCon.inputBox.x,
			/*  y  */ conAddon.conItems.fullCon.inputBox.y,
			/*  w  */ conAddon.conItems.fullCon.inputBox.w,
			/*  h  */ conAddon.conItems.fullCon.inputBox.h,
			/* dep */ false);

			// output text box gets created in xo_con_DrawOutputWindow

			Game::conDrawInputGlob->x += 5.0f;
			Game::conDrawInputGlob->y += 5.0f;
		}

		// ------------------------------ // input text for small / full console
		ConDrawInput_Text(
		/*  x  */ Game::conDrawInputGlob->x,
		/*  y  */ Game::conDrawInputGlob->y + Game::conDrawInputGlob->fontHeight + (Game::con->outputVisible ? 2.0f : 0.0f),
		/* scX */ 0.75f,
		/* scY */ 0.75f,
		/* fon */ (char*)FONT_BIG,
		/* col */ Game::COLOR_WHITE,
		/* txt */ Utils::VA("IW3 >"));

		// move cursor position
		Game::conDrawInputGlob->x += 40.0f;

		// --------------------------------------------------------------------------- 
#pragma region CON-LOGIC

		Game::conDrawInputGlob->leftX = Game::conDrawInputGlob->x;
		std::int32_t inputTextLenPrev = Game::conDrawInputGlob->inputTextLen;
		
		Game::g_consoleField->widthInPixels = (int)(Game::con->screenMax[0] - 6.0f - Game::conDrawInputGlob->x);
		char* con_tokenizedInput = Game::Con_TokenizeInput();
		
		Game::conDrawInputGlob->inputText = con_tokenizedInput;
		std::int32_t currentInputText = (std::int32_t)strlen(con_tokenizedInput);
		
		Game::conDrawInputGlob->inputTextLen = currentInputText;
		Game::conDrawInputGlob->autoCompleteChoice[0] = 0;


		if (inputTextLenPrev != currentInputText)
		{
			//Game::Con_CancelAutoComplete(); // Inlined ::
			if (Game::conDrawInputGlob->matchIndex >= 0 && Game::conDrawInputGlob->autoCompleteChoice[0]) // T5 way
			{
				Game::conDrawInputGlob->matchIndex = -1;
				Game::conDrawInputGlob->autoCompleteChoice[0] = 0;
			}

			currentInputText = Game::conDrawInputGlob->inputTextLen;
			con_tokenizedInput = (char*)Game::conDrawInputGlob->inputText;
		}

		if (!currentInputText)
		{
			Game::conDrawInputGlob->mayAutoComplete = 0;

			// Input Cursor when input text is empty
			if (conAddon.fltCon.flt_initialized) 
			{
				Game::Con_DrawInputPrompt(); 
			}

			Game::Cmd_EndTokenizedString();
			goto REDRAW_CURSOR_RETURN;
		}

		auto	var_con_originalCommand = con_tokenizedInput;
		int		CmdOrDvar;

		if (XO_Console::Cmd_Argc() > 1 && Con_IsDvarCommand(con_tokenizedInput))
		{
			CmdOrDvar = 1;
			Game::conDrawInputGlob->inputText = Game::Cmd_Argv(1); 
			Game::conDrawInputGlob->inputTextLen = strlen(Game::conDrawInputGlob->inputText); 
			
			if (!Game::conDrawInputGlob->inputTextLen)
			{
				Game::conDrawInputGlob->mayAutoComplete = 0;
				Game::Con_DrawInputPrompt();
				Game::Cmd_EndTokenizedString();

				goto REDRAW_CURSOR_RETURN;
			}
		}

		else 
		{
			CmdOrDvar = 0;
		}

		Game::conDrawInputGlob->hasExactMatch = 0;
		Game::conDrawInputGlob->matchCount = 0;

		if (Game::Dvar_FindVar("con_matchPrefixOnly")->current.enabled == 0)
		{
			*Game::extvar_con_ignoreMatchPrefixOnly = false;
			goto CON_MATCH_PREFIX_ONLY;
		}

	   *Game::extvar_con_ignoreMatchPrefixOnly = true;
		Game::Dvar_ForEachName(Game::ConDrawInput_IncrMatchCounter);

		if (!CmdOrDvar) 
		{
			Game::Cmd_ForEachXO(Game::ConDrawInput_IncrMatchCounter);
		}

		if (Game::conDrawInputGlob->matchCount > Dvars::xo_con_maxMatches->current.integer) // autocomplete avail.
		{
			Game::conDrawInputGlob->hasExactMatch = 0;
			Game::conDrawInputGlob->matchCount = 0;
		   *Game::extvar_con_ignoreMatchPrefixOnly = 0;
			Game::Dvar_ForEachName(Game::ConDrawInput_IncrMatchCounter);
			
			Game::Cmd_ForEachXO(Game::ConDrawInput_IncrMatchCounter);

			if (!Game::conDrawInputGlob->matchCount)
			{
				Game::conDrawInputGlob->hasExactMatch = 0;
				Game::conDrawInputGlob->matchCount = 0;
			   *Game::extvar_con_ignoreMatchPrefixOnly = 1;


CON_MATCH_PREFIX_ONLY:
				Game::Dvar_ForEachName(Game::ConDrawInput_IncrMatchCounter);
				
				if (!CmdOrDvar) 
				{
					Game::Cmd_ForEachXO(Game::ConDrawInput_IncrMatchCounter);
				}
			}
		}

		int var_con_currentMatchCount = Game::conDrawInputGlob->matchCount;
		if (!Game::conDrawInputGlob->matchCount)
		{
			Game::Con_DrawInputPrompt();
			Game::Cmd_EndTokenizedString();

			goto REDRAW_CURSOR_RETURN;
		}

		int var_con_isDvarCommand = Game::conDrawInputGlob->matchIndex;
		if (Game::conDrawInputGlob->matchIndex < Game::conDrawInputGlob->matchCount && Game::conDrawInputGlob->autoCompleteChoice[0])
		{
			if (Game::conDrawInputGlob->matchIndex >= 0)
			{
				var_con_isDvarCommand = CmdOrDvar;
				Game::Con_DrawAutoCompleteChoice(var_con_isDvarCommand, var_con_originalCommand); // color AutoComplete String
				
				goto CON_SKIP_INPUT_PROMPT;
			}
		}

		else 
		{
			Game::conDrawInputGlob->matchIndex = -1;
		}

		Game::Con_DrawInputPrompt();

	
	CON_SKIP_INPUT_PROMPT:
		Game::conDrawInputGlob->y = Game::conDrawInputGlob->y + (2 * Game::conDrawInputGlob->fontHeight + 15.0f);
		Game::conDrawInputGlob->x = Game::conDrawInputGlob->leftX;
#pragma endregion
		
		// --------------------------------------------------------------------------- 
		// :: HINTBOX

		// copy "con_inputHintBoxColor" to dynamically change its alpha without changing the dvar
		float loc_inputHintBoxColor[4]; 
		memcpy(&loc_inputHintBoxColor, Game::Dvar_FindVar("con_inputHintBoxColor")->current.vector, sizeof(loc_inputHintBoxColor));

		// reduce alpha on fullscreen to make text more readable - will reset itself when small again
		if (loc_inputHintBoxColor[3] < 1.0f && Game::con->outputVisible) {
			loc_inputHintBoxColor[3] = 0.95f; // alpha
		}

		// --------------------------------------------------------------------------- 
		// :: Too many matches to show ( 1 line )

		float outputSliderWidth = 16.0f;

		if(Game::conDrawInputGlob->matchCount > Dvars::xo_con_maxMatches->current.integer)
		{
			// create the text and get its width in pixels so we can center it
			Game::Font_s *fontHandle = reinterpret_cast<Game::Font_s *>(Game::R_RegisterFont(FONT_CONSOLE, sizeof(FONT_CONSOLE)));
			const char* tooManyTxt = Utils::VA("%i matches (too many to show here, press TAB to print all, press SHIFT + TILDE to open full console)", var_con_currentMatchCount);
			
			int txtLength = Game::R_TextWidth(tooManyTxt, 0, fontHandle);

			// small console --------------------------------------------------------

			// hint box
			conAddon.conItems.smallCon.hintBoxUpper.x = x;
			conAddon.conItems.smallCon.hintBoxUpper.y = Game::conDrawInputGlob->y - Game::conDrawInputGlob->fontHeight - 7.0f;
			conAddon.conItems.smallCon.hintBoxUpper.w = w;
			conAddon.conItems.smallCon.hintBoxUpper.h = Game::conDrawInputGlob->fontHeight + 17.0f;

			// hint box text (centered)
			conAddon.conItems.smallCon.hintBoxUpperText.x = floorf((conAddon.conItems.smallCon.hintBoxUpper.w * 0.5f) - ((float)txtLength * 0.5f) + conAddon.conItems.smallCon.inputBox.x);
			//conAddon.conItems.smallCon.hintBoxUpperText.x = x + 18.0f; // left aligned
			conAddon.conItems.smallCon.hintBoxUpperText.y = Game::conDrawInputGlob->y;

			// full console --------------------------------------------------------

			// hint box
			conAddon.conItems.fullCon.hintBoxUpper.x = x;
			conAddon.conItems.fullCon.hintBoxUpper.y = Game::conDrawInputGlob->y - Game::conDrawInputGlob->fontHeight - 5.0f;
			conAddon.conItems.fullCon.hintBoxUpper.w = w - outputSliderWidth;
			conAddon.conItems.fullCon.hintBoxUpper.h = Game::conDrawInputGlob->fontHeight + 17.0f;

			// hint box text (centered)
			conAddon.conItems.fullCon.hintBoxUpperText.x = (conAddon.conItems.fullCon.hintBoxUpper.w * 0.5f) - ((float)txtLength * 0.5f) + conAddon.conItems.fullCon.inputBox.x;
			//conAddon.conItems.fullCon.hintBoxUpperText.x = x + 10.0f + 34.0f; // left aligned
			conAddon.conItems.fullCon.hintBoxUpperText.y = Game::conDrawInputGlob->y + 2.0f;

			// ---------------------------------------------------------------------

			// if small console is near the bottom or clipped at the bottom, put hintboxes above the output window
			if (!Game::con->outputVisible && conAddon.fltCon.flt_enabled && (conAddon.fltCon.fltClippedAtBorder.bottom || conAddon.fltCon.fltAnker.bottom + conAddon.conItems.smallCon.hintBoxUpper.h > conAddon.viewportRes.height))
			{
				// calculate the new y
				conAddon.conItems.smallCon.hintBoxUpper.y		= conAddon.fltCon.fltAnker.bottom - conAddon.conItems.smallCon.inputBox.h - conAddon.conItems.smallCon.outputBox.h - conAddon.conItems.smallCon.hintBoxUpper.h;
				conAddon.conItems.smallCon.hintBoxUpperText.y	= conAddon.conItems.smallCon.hintBoxUpper.y + ((conAddon.conItems.smallCon.hintBoxUpper.h * 0.5f) + (Game::conDrawInputGlob->fontHeight * 0.5f) + 0.5f); // offset by half a pixel (blurriness)
			}

			ConDraw_Box(
			/* col */ loc_inputHintBoxColor,
			/*  x  */ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.x : conAddon.conItems.smallCon.hintBoxUpper.x,
			/*  y  */ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.y : conAddon.conItems.smallCon.hintBoxUpper.y,
			/*  w  */ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.w : conAddon.conItems.smallCon.hintBoxUpper.w,
			/*  h  */ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.h : conAddon.conItems.smallCon.hintBoxUpper.h,
			/* dep */ false,
			/* iFS */ true);

			// hintbox shadow --------------------------------------------------------
			float shadow_color[4] = { 0.0f, 0.0f, 0.0f, 0.125f };

			ConDraw_Box(
				/*    c		*/ shadow_color,
				/*	  x		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.x : conAddon.conItems.smallCon.hintBoxUpper.x,
				/*	  y		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.y : conAddon.conItems.smallCon.hintBoxUpper.y,
				/*	  w		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.w + outputSliderWidth : conAddon.conItems.smallCon.hintBoxUpper.w,
				/*	  h		*/ 3.0f,
				/*  depth	*/ false,
				/* ignoreFS	*/ true);

			ConDrawInput_Text(
			/*  x  */ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpperText.x : conAddon.conItems.smallCon.hintBoxUpperText.x,
			/*  y  */ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpperText.y : conAddon.conItems.smallCon.hintBoxUpperText.y,
			/* scX */ 1.0f,
			/* scY */ 1.0f,
			/* fon */ fontHandle,
			/* col */ Dvars::xo_con_hintBoxTxtColor_currentDvar->current.vector,
			/* txt */ tooManyTxt);
			
			Game::Cmd_EndTokenizedString();
			goto REDRAW_CURSOR_RETURN;
		}

		// --------------------------------------------------------------------------- 
		// :: Exact Match - Draw Details

		if(Game::conDrawInputGlob->matchCount == 1 || Game::conDrawInputGlob->hasExactMatch && Game::Con_AnySpaceAfterCommand())
		{
			glob_con_col = loc_inputHintBoxColor;

			// small console --------------------------------------------------------

			// hint box -- we calculate y & h within DetailedMatch_UpperBox && DetailedMatch_LowerBox
			conAddon.conItems.smallCon.hintBoxUpper.x = x;
			conAddon.conItems.smallCon.hintBoxUpper.w = w;

			// hint box text
			// we do not handle text for dvar matches

			// full console --------------------------------------------------------

			// hint box -- we calculate y & h within DetailedMatch_UpperBox && DetailedMatch_LowerBox
			conAddon.conItems.fullCon.hintBoxUpper.x = x;
			conAddon.conItems.fullCon.hintBoxUpper.w = w - outputSliderWidth;

			// hint box text
			// we do not handle text for dvar matches

			// ---------------------------------------------------------------------

			// if small console is near the bottom or clipped at the bottom, put hintboxes above the output window
			if (!Game::con->outputVisible && (conAddon.fltCon.fltClippedAtBorder.bottom || conAddon.fltCon.fltAnker.bottom + conAddon.conItems.smallCon.hintBoxTotal.h > conAddon.viewportRes.height))
			{
				// we calculate the box offsets in DetailedMatch_UpperBox && DetailedMatch_LowerBox
				// but we have to offset Game::conDrawInputGlob->y here. Unfortunately, we do not know the height, so we have to "think" a frame later 
				//Game::conDrawInputGlob->y = conAddon.fltCon.fltAnker.bottom - conAddon.conItems.smallCon.inputBox.h - conAddon.conItems.smallCon.outputBox.h - conAddon.conItems.smallCon.hintBoxTotal.h + conDrawInputGlobY_fuckery + 8.0f;
			
				float tmp_topSpace = conAddon.fltCon.fltAnker.top;
				float tmp_bottomSpace = conAddon.viewportRes.height - conAddon.fltCon.fltAnker.bottom;
			
				// compare top with bottom
				if (tmp_topSpace > tmp_bottomSpace)
				{
					// top now offers more space, so move the hintbox above the output box
					Game::conDrawInputGlob->y = conAddon.fltCon.fltAnker.bottom - conAddon.conItems.smallCon.inputBox.h - conAddon.conItems.smallCon.outputBox.h - conAddon.conItems.smallCon.hintBoxTotal.h + conDrawInputGlobY_fuckery + 7.0f;

					// calculate the new y
					conAddon.conItems.smallCon.hintBoxUpper.y = conAddon.fltCon.fltAnker.bottom - conAddon.conItems.smallCon.inputBox.h - conAddon.conItems.smallCon.outputBox.h - conAddon.conItems.smallCon.hintBoxUpper.h;
				
					// lets offset text in the to the left if we display hintboxes over the console ;)
					Game::conDrawInputGlob->x -= 36.0f;
					Game::conDrawInputGlob->leftX = Game::conDrawInputGlob->x;
				}
			}

			// we have to subtract from Game::conDrawInputGlob->y because we want to offset the match text in "ConDrawInput_Detailed..." (we add it back when drawing the box)
			// this is because we only hook the box drawing part of "ConDrawInput_Detailed..." and not the text drawing
			Game::conDrawInputGlob->y -= conDrawInputGlobY_fuckery;

			// mid-hook to change box drawing
			// -> ConDrawInput_Box_DetailedMatch_UpperBox && ConDrawInput_Box_DetailedMatch_LowerBox
			Game::Dvar_ForEachName(Game::ConDrawInput_DetailedDvarMatch); 

			if (!CmdOrDvar) 
			{
				Game::Cmd_ForEachXO(Game::ConDrawInput_DetailedCmdMatch);
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
			conAddon.conItems.smallCon.hintBoxUpper.x = x;
			conAddon.conItems.smallCon.hintBoxUpper.y = y + h; // bad
			conAddon.conItems.smallCon.hintBoxUpper.w = w;
			conAddon.conItems.smallCon.hintBoxUpper.h = Game::conDrawInputGlob->matchCount * Game::conDrawInputGlob->fontHeight + 20.0f;//var_con_currentMatchCount * Game::conDrawInputGlob->fontHeight + 20.0f; // bad

			// hint box text
			// we do not handle text for dvar matches

			// full console --------------------------------------------------------

			// hint box -- we calculate y & h within DetailedMatch_UpperBox && DetailedMatch_LowerBox
			conAddon.conItems.fullCon.hintBoxUpper.x = x;
			conAddon.conItems.fullCon.hintBoxUpper.y = y + h; // bad
			conAddon.conItems.fullCon.hintBoxUpper.w = w - outputSliderWidth;
			conAddon.conItems.fullCon.hintBoxUpper.h = Game::conDrawInputGlob->matchCount * Game::conDrawInputGlob->fontHeight + 20.0f;//var_con_currentMatchCount * Game::conDrawInputGlob->fontHeight + 20.0f; // bad

			// hint box text
			// we do not handle text for dvar matches

			// ---------------------------------------------------------------------

			// check if hintbox goes out of bounds
			if (!Game::con->outputVisible && (conAddon.fltCon.fltClippedAtBorder.bottom || conAddon.fltCon.fltAnker.bottom + conAddon.conItems.smallCon.hintBoxUpper.h > conAddon.viewportRes.height))
			{
				// check if we have more "space" above the console, if not, allow a part of the hintbox to move out of bounds till the top has more space then the bottom
				float tmp_topSpace = conAddon.fltCon.fltAnker.top;
				float tmp_bottomSpace = conAddon.viewportRes.height - conAddon.fltCon.fltAnker.bottom;

				// compare top with bottom
				if (tmp_topSpace > tmp_bottomSpace) 
				{
					// top now offers more space, so move the hintbox above the output box
					Game::conDrawInputGlob->y = conAddon.fltCon.fltAnker.bottom - conAddon.conItems.smallCon.inputBox.h - conAddon.conItems.smallCon.outputBox.h - conAddon.conItems.smallCon.hintBoxUpper.h + conDrawInputGlobY_fuckery + 8.0f;
				
					// calculate the new y
					conAddon.conItems.smallCon.hintBoxUpper.y = conAddon.fltCon.fltAnker.bottom - conAddon.conItems.smallCon.inputBox.h - conAddon.conItems.smallCon.outputBox.h - conAddon.conItems.smallCon.hintBoxUpper.h;
				
					// lets offset text in the to the left if we display hintboxes over the console ;)
					Game::conDrawInputGlob->x -= 36.0f;
					Game::conDrawInputGlob->leftX = Game::conDrawInputGlob->x;
				}
			}

			// we have to subtract from Game::conDrawInputGlob->y because we want to offset the match text in "ConDrawInput_Detailed..." (we add it back when drawing the box)
			// this is because we only hook the box drawing part of "ConDrawInput_Detailed..." and not the text drawing
			Game::conDrawInputGlob->y -= conDrawInputGlobY_fuckery;

			ConDraw_Box(
				/* col */ loc_inputHintBoxColor,
				/*  x  */ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.x : conAddon.conItems.smallCon.hintBoxUpper.x,//x,
				/*  y  */ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.y : conAddon.conItems.smallCon.hintBoxUpper.y,//y + h,
				/*  w  */ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.w : conAddon.conItems.smallCon.hintBoxUpper.w,//w,
				/*  h  */ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.h : conAddon.conItems.smallCon.hintBoxUpper.h,//var_con_currentMatchCount * Game::conDrawInputGlob->fontHeight + 20.0f,
				/* dep */ false,
				/* iFS */ true );

			// hintbox shadow --------------------------------------------------------
			float shadow_color[4] = { 0.0f, 0.0f, 0.0f, 0.125f };

			ConDraw_Box(
				/*    c		*/ shadow_color,
				/*	  x		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.x : conAddon.conItems.smallCon.hintBoxUpper.x,
				/*	  y		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.y : conAddon.conItems.smallCon.hintBoxUpper.y,
				/*	  w		*/ Game::con->outputVisible ? conAddon.conItems.fullCon.hintBoxUpper.w + outputSliderWidth : conAddon.conItems.smallCon.hintBoxUpper.w,
				/*	  h		*/ 3.0f,
				/*  depth	*/ false,
				/* ignoreFS	*/ true);

			Game::Dvar_ForEachName(Game::ConDrawInput_DvarMatch);
			
			if (!CmdOrDvar)
			{
				Game::Cmd_ForEachXO(Game::ConDrawInput_CmdMatch);
				Game::Cmd_EndTokenizedString();

				goto REDRAW_CURSOR_RETURN;
			}
		}

		Game::Cmd_EndTokenizedString();

		// *
		// -----------------
		REDRAW_CURSOR_RETURN:

		// Overdraw the cursor so its above the console so we obv. can not return early ;)
		if ((Game::_uiContext->openMenuCount || Dvars::xo_con_cursorState->current.enabled) && Dvars::xo_con_cursorOverdraw->current.enabled)
		{
			// get material handle
			void *cur_material = Game::Material_RegisterHandle("ui_cursor", 3);

			float cur_w = (32.0f * Game::scrPlace->scaleVirtualToReal[0]) / Game::scrPlace->scaleVirtualToFull[0];
			float cur_h = (32.0f * Game::scrPlace->scaleVirtualToReal[1]) / Game::scrPlace->scaleVirtualToFull[1];
			float cur_x = Game::_uiContext->cursor.x - 0.5f * cur_w;
			float cur_y = Game::_uiContext->cursor.y - 0.5f * cur_h;

			// set up texcoords
			float s0, s1;
			float t0, t1;

			if (cur_w >= 0.0f) 
			{
				s0 = 0.0f;
				s1 = 1.0f;
			}
			else 
			{
				cur_w = -cur_w;
				s0 = 1.0f;
				s1 = 0.0f;
			}

			if (cur_h >= 0.0f) 
			{
				t0 = 0.0f;
				t1 = 1.0f;
			}
			else 
			{
				cur_h = -cur_h;
				t0 = 1.0f;
				t1 = 0.0f;
			}

			// scale 640x480 rect to viewport resolution and draw the cursor
			_UI::ScrPlace_ApplyRect(&cur_x, &cur_w, &cur_y, &cur_h, VERTICAL_ALIGN_FULLSCREEN, VERTICAL_ALIGN_FULLSCREEN);
			Game::R_AddCmdDrawStretchPic(cur_material, cur_x, cur_y, cur_w, cur_h, s0, t0, s1, t1, 0);
		}
	}

	#pragma endregion

	#pragma region CON-ASM // ++++++

	// Replace ConDrawInput_Box in ConDrawInput_DetailedDvarMatch ( mid-hook ) -- upper box
	__declspec(naked) void detailed_dvar_match_stub_01()
	{
		const static uint32_t RetAddr = 0x460022;
		__asm
		{
			push	eax		// overwritten op
			push	esi
			
			call	ConDrawInput_Box_DetailedMatch_UpperBox
			
			pop		esi
			add		esp, 4h

			jmp		RetAddr
		}
	}

	// Replace ConDrawInput_Box in ConDrawInput_DetailedDvarMatch ( mid-hook ) -- lower box
	__declspec(naked) void detailed_dvar_match_stub_02()
	{
		const static uint32_t RetAddr = 0x4601FB;
		__asm
		{
			push	eax		// overwritten op
			push	esi

			Call	ConDrawInput_Box_DetailedMatch_LowerBox

			pop		esi
			add		esp, 4h

			jmp		RetAddr
		}
	}

	// Replace ConDrawInput_Box in ConDrawInput_DetailedCmdMatch
	__declspec(naked) void detailed_cmd_match_stub()
	{
		const static uint32_t RetAddr = 0x4603BF;
		__asm
		{
			push	1		// overwritten op
			push	esi

			Call	ConDrawInput_Box_DetailedMatch_UpperBox

			pop		esi
			add		esp, 4h

			jmp		RetAddr
		}
	}

	// helper func to offset matchbox values
	void Con_OffsetMatchValues()
	{
		if (*Game::conScreenMin0)
			Game::conDrawInputGlob->x += 500.0f + *Game::conScreenMin0;

		else 
			Game::conDrawInputGlob->x += 500.0f;
	}

	// offset values for multiple matches, as we increased the maximum amount of dvar chars displayed 
	__declspec(naked) void matchbox_offset_values_stub_01()
	{
		__asm
		{
			pushad
			Call	Con_OffsetMatchValues
			popad
			
			add     esp, 4		// stock
			push	0x45FAFA;
			retn
		}
	}

	// offset values for detailed matches, as we increased the maximum amount of dvar chars displayed 
	__declspec(naked) void matchbox_offset_values_stub_02()
	{
		__asm 
		{
			pushad
			Call	Con_OffsetMatchValues
			popad

			add     esp, 4		// stock
			push    6BDF24h		// stock
			sub     esp, 10h	// stock

			push	0x460054;
			retn
		}
	}

	// ^
	__declspec(naked) void matchbox_offset_values_stub_03()
	{
		__asm
		{
			pushad
			Call	Con_OffsetMatchValues
			popad

			add     esp, 4		// stock
			push    6BDF34h		// stock
			sub     esp, 10h	// stock

			push	0x4600D4;
			retn
		}
	}

	// ^
	__declspec(naked) void matchbox_offset_values_stub_04()
	{
		__asm
		{
			pushad
			Call	Con_OffsetMatchValues
			popad

			add     esp, 4		// stock
			push    6BDF34h		// stock
			sub     esp, 10h	// stock

			push	0x460150;
			retn
		}
	}

	// fully disable cmd autocompletion box drawing
	__declspec(naked) void xo_con_disableCmdAutocomplBox_stub()
	{
		const static uint32_t RetAddr = 0x460433;
		__asm
		{
			jmp		RetAddr
		}
	}

	// change devmap autocomplete extension to "autocomplete" :: files are in main/maps/mp
	__declspec(naked) void con_devmap_autocompl_stub()
	{
		__asm
		{
			push	ecx
			mov		ecx, con_mapdir
			mov		[eax + 8], ecx

			mov		ecx, con_mapext
			mov		[eax + 0Ch], ecx

			pop		ecx
			push	0x528F96;
			retn
		}
	}

	// change map autocomplete extension to "autocomplete" :: files are in main/maps/mp
	__declspec(naked) void con_map_autocompl_stub()
	{
		__asm
		{
			push	ecx

			mov		ecx, con_mapext
			mov		ebx, ecx

			mov     esi, edi	// stock op inbetween

			mov		ecx, con_mapdir
			mov		[eax + 8], ecx

			pop		ecx
			push	0x528E5D;
			retn
		}
	}

	// *
	// minicon drawing
	void Con_DrawMessageWindowOldToNew_Proxy(DWORD* msgWnd, int s_xPos, int s_yPos, int s_charHeight, int s_horzAlign, int s_vertAlign, int s_mode, Game::Font_s* s_font, const float* s_color, int s_textStyle, float s_msgwndScale, int s_textAlignMode)
	{
		// get font handle
		auto _font	= _CG::GetFontForStyle(Dvars::con_minicon_font->current.integer);
		auto  font	= reinterpret_cast<Game::Font_s*>(Game::R_RegisterFont(_font, sizeof(_font)));

		Game::Con_DrawMessageWindowOldToNew(
			msgWnd,
			0,
			static_cast<int>(Dvars::con_minicon_position->current.vector[0]),
			static_cast<int>(Dvars::con_minicon_position->current.vector[1]),
			Dvars::con_minicon_fontHeight->current.integer,
			s_horzAlign,
			s_vertAlign,
			s_mode,
			font,
			Dvars::con_minicon_fontColor->current.vector,
			Dvars::con_minicon_fontStyle->current.integer,
			s_msgwndScale,
			s_textAlignMode
		);
	}

	// wrapper (the game did not push any args besides esi)
	__declspec(naked) void con_minicon_stub()
	{
		const static uint32_t retnPt = 0x4615CA;
		__asm
		{
			push	4	// textAlignMode
			//push	ecx	// whatever that is, we push it in the Con_DrawMessageWindowOldToNew wrapper

			fstp	dword ptr[esp]	// msgwndScale
			push	3				// textStyle

			lea     eax, [esp + 1Ch]// color
			push    eax				// color

			push    edi				// font
			push    edx				// mode
			push    1				// vert
			push    1				// horz
			push    12				// charHeight
			push    4				// yPos
			push    2				// xPos

			//push    ecx			// localClientNum
			push	esi				// esi holds msgWnd
			
			Call	Con_DrawMessageWindowOldToNew_Proxy
			add     esp, 2Ch

			jmp		retnPt // after <add esp, 30h>
		}
	}

	#pragma endregion

	XO_Console::XO_Console()
	{
		// -----
		// DVARS

		Dvars::con_minicon_position = Game::Dvar_RegisterVec2(
			/* name		*/ "con_minicon_position",
			/* desc		*/ "minicon position (int)",
			/* x		*/ 115.0f,
			/* y		*/ 17.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 640.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::con_minicon_font = Game::Dvar_RegisterInt(
			/* name		*/ "con_minicon_font",
			/* desc		*/ "minicon font",
			/* default	*/ 8,
			/* minVal	*/ 0,
			/* maxVal	*/ 8,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::con_minicon_fontHeight = Game::Dvar_RegisterInt(
			/* name		*/ "con_minicon_fontHeight",
			/* desc		*/ "minicon char height",
			/* default	*/ 11,
			/* minVal	*/ 0,
			/* maxVal	*/ 64,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::con_minicon_fontColor = Game::Dvar_RegisterVec4(
			/* name		*/ "con_minicon_fontColor",
			/* desc		*/ "minicon font color",
			/* x		*/ 1.0f,
			/* y		*/ 1.0f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::con_minicon_fontStyle = Game::Dvar_RegisterInt(
			/* name		*/ "con_minicon_fontStyle",
			/* desc		*/ "3 = <renderFlags 4>; 6 = <renderFlags 12>; 128 = <renderFlags 1>",
			/* default	*/ 3,
			/* minVal	*/ 0,
			/* maxVal	*/ 128,
			/* flags	*/ Game::dvar_flags::saved);
		
		Dvars::xo_con_fltCon = Game::Dvar_RegisterBool(
			/* name		*/ "xo_con_fltCon",
			/* desc		*/ "console :: floating console state",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_fltConLeft = Game::Dvar_RegisterFloat(
			/* name		*/ "xo_con_fltConLeft",
			/* desc		*/ "console :: floating console left anker",
			/* default	*/ 0.0f,
			/* minVal	*/ -7680.0f,
			/* maxVal	*/ 7680.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_fltConTop = Game::Dvar_RegisterFloat(
			/* name		*/ "xo_con_fltConTop",
			/* desc		*/ "console :: floating console top anker",
			/* default	*/ 0.0f,
			/* minVal	*/ -4320.0f,
			/* maxVal	*/ 4320.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_fltConRight = Game::Dvar_RegisterFloat(
			/* name		*/ "xo_con_fltConRight",
			/* desc		*/ "console :: floating console right anker",
			/* default	*/ 0.0f,
			/* minVal	*/ -7680.0f,
			/* maxVal	*/ 7680.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_fltConBottom = Game::Dvar_RegisterFloat(
			/* name		*/ "xo_con_fltConBottom",
			/* desc		*/ "console :: floating console bottom anker",
			/* default	*/ 0.0f,
			/* minVal	*/ -4320.0f,
			/* maxVal	*/ 4320.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_outputHeight = Game::Dvar_RegisterInt(
			/* name		*/ "xo_con_outputHeight",
			/* desc		*/ "console :: height / lines of console output.",
			/* default	*/ 8,
			/* minVal	*/ 1,
			/* maxVal	*/ 30,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_maxMatches = Game::Dvar_RegisterInt(
			/* name		*/ "xo_con_maxMatches",
			/* desc		*/ "console :: maximum amout of matches to show till \"too many to show here\" is drawn.",
			/* default	*/ 24,
			/* minVal	*/ 0,
			/* maxVal	*/ 50,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_useDepth = Game::Dvar_RegisterBool(
			/* name		*/ "xo_con_useDepth",
			/* desc		*/ "console :: use scene depth as inputBox background.",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_padding = Game::Dvar_RegisterFloat(
			/* name		*/ "xo_con_padding",
			/* desc		*/ "console :: distance screen border <-> console (all sides)",
			/* default	*/ 0.0f,
			/* minVal	*/ -2000.0f,
			/* maxVal	*/ 2000.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_fontSpacing = Game::Dvar_RegisterFloat(
			/* name		*/ "xo_con_fontSpacing",
			/* desc		*/ "console output :: space between lines",
			/* default	*/ 0.8f,
			/* minVal	*/ 0.8f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_cursorOverdraw = Game::Dvar_RegisterBool(
			/* name		*/ "xo_con_cursorOverdraw",
			/* desc		*/ "console :: redraw the menu cursor so its above the console.",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_cursorState = Game::Dvar_RegisterBool(
			/* name		*/ "xo_con_cursorState",
			/* desc		*/ "console :: current state of the cursor.",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::xo_con_hintBoxTxtColor_currentDvar = Game::Dvar_RegisterVec4(
			/* name		*/ "xo_con_hintBoxTxtColor_currentDvar",
			/* desc		*/ "console :: color of dvar names in the hintbox",
			/* x		*/ 0.8f,
			/* y		*/ 0.8f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_hintBoxTxtColor_currentValue = Game::Dvar_RegisterVec4(
			/* name		*/ "xo_con_hintBoxTxtColor_currentValue",
			/* desc		*/ "console :: color of dvar values in the hintbox",
			/* x		*/ 0.8f,
			/* y		*/ 0.8f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_hintBoxTxtColor_defaultValue = Game::Dvar_RegisterVec4(
			/* name		*/ "xo_con_hintBoxTxtColor_defaultValue",
			/* desc		*/ "console :: color of default dvar value in the hintbox",
			/* x		*/ 0.8f,
			/* y		*/ 0.8f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_hintBoxTxtColor_dvarDescription = Game::Dvar_RegisterVec4(
			/* name		*/ "xo_con_hintBoxTxtColor_dvarDescription",
			/* desc		*/ "console :: color of dvar description in the hintbox",
			/* x		*/ 0.8f,
			/* y		*/ 0.8f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::xo_con_hintBoxTxtColor_domainDescription = Game::Dvar_RegisterVec4(
			/* name		*/ "xo_con_hintBoxTxtColor_domainDescription",
			/* desc		*/ "console :: color of dvar domain in the hintbox",
			/* x		*/ 0.8f,
			/* y		*/ 0.8f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* minValue	*/ 0.0f,
			/* maxValue	*/ 1.0f,
			/* flags	*/ Game::dvar_flags::saved);

		// --------
		// Commands

		// activate cursor in-game with empty helper menu "pregame_loaderror_mp" in xcommon_iw3xo
		//Command::Add("con_toggle_cursor", [](Command::Params)
		//{
		//	if (!Dvars::xo_con_cursorState->current.enabled)
		//	{
		//		Game::Dvar_SetValue(Dvars::xo_con_cursorState, true);
		//		Game::UI_SetActiveMenu(0, 5);
		//	}

		//	else 
		//	{
		//		Game::Dvar_SetValue(Dvars::xo_con_cursorState, false);

		//		Game::UiContext *ui = &Game::_uiContext[0];
		//		Game::Menus_CloseByName("pregame_loaderror_mp", ui);
		//	}
		//});

		// -----
		// Hooks
		
		// Minicon drawing - wrapper to proxy
		Utils::Hook::Nop(0x4615A8, 6);
		Utils::Hook(0x4615A8, con_minicon_stub, HOOK_JUMP).install()->quick(); // in Con_DrawMiniConsole

		// Con_CheckResize - main position
		//Utils::Hook(0x475052, xo_con_CheckResize, HOOK_CALL).install()->quick(); // in SCR_UpdateFrame
		Utils::Hook(0x45EB81, xo_con_CheckResize_On_Init, HOOK_CALL).install()->quick(); // in Con_OneTimeInit
		Utils::Hook(0x46CC6B, xo_con_CheckResize_On_Init, HOOK_CALL).install()->quick(); // in CL_InitRenderer
		

		// Con_DrawOuputWindow -- in "Con_DrawSolidConsole" before "Con_DrawInput"
		Utils::Hook(0x461D44, xo_con_DrawOutputWindow, HOOK_CALL).install()->quick(); // no flicker but latched value bug on vec4

		// Con_DrawInput ( 2x IfElse )
		Utils::Hook(0x461D34, xo_con_DrawInput, HOOK_CALL).install()->quick();
		Utils::Hook(0x461D49, xo_con_DrawInput, HOOK_CALL).install()->quick(); // our active func

		// Disable the need for Forward/Backslash for console cmds
		Utils::Hook::Nop(0x46752F, 5);

		// Replace ConDrawInput_Box in ConDrawInput_DetailedDvarMatch ( mid-hook ) -- upper box
		Utils::Hook(0x46001C, detailed_dvar_match_stub_01, HOOK_CALL).install()->quick();
		Utils::Hook::Nop(0x460021, 1);

		// Replace ConDrawInput_Box in ConDrawInput_DetailedDvarMatch ( mid-hook ) -- lower box
		Utils::Hook(0x4601F5, detailed_dvar_match_stub_02, HOOK_CALL).install()->quick();
		Utils::Hook::Nop(0x4601FA, 1);

		// Replace ConDrawInput_Box in ConDrawInput_DetailedCmdMatch
		Utils::Hook(0x4603B8, detailed_cmd_match_stub, HOOK_CALL).install()->quick();
		Utils::Hook::Nop(0x4603BD, 2);

		// Increase max chars to draw for dvar matches
		Utils::Hook::Set<BYTE>(0x45FADA, 0x40); // Stock 0x18

		// Increase draw width for matched dvar - dvar string
		Utils::Hook::Set<BYTE>(0x46002E, 0x40); // Stock 0x18

		// Increase distance to matched dvars as we increased the maximum amount of chars for found matches
		Utils::Hook::Nop(0x45FAE5, 6); // Nop the instruction first, then install our jmp
		Utils::Hook(0x45FAE5, matchbox_offset_values_stub_01, HOOK_JUMP).install()->quick();
		
		// same for detailed match (3 times for current, latched, default)
		Utils::Hook::Nop(0x460037, 6); // Nop the instruction first, then install our jmp
		Utils::Hook(0x460037, matchbox_offset_values_stub_02, HOOK_JUMP).install()->quick();

		Utils::Hook::Nop(0x4600B7, 6); // Nop the instruction first, then install our jmp
		Utils::Hook(0x4600B7, matchbox_offset_values_stub_03, HOOK_JUMP).install()->quick();

		Utils::Hook::Nop(0x460133, 6); // Nop the instruction first, then install our jmp
		Utils::Hook(0x460133, matchbox_offset_values_stub_04, HOOK_JUMP).install()->quick();
		
		// fully disable cmd autocomplete box with dir searching
		Utils::Hook::Nop(0x4603FC, 6); Utils::Hook(0x4603FC, xo_con_disableCmdAutocomplBox_stub, HOOK_JUMP).install()->quick();
		
		//// devmap autocomplete :: change extentsion to .autocomplete and place nullfiles in main/maps/mp/mp_mapname.autocomplete
		//Utils::Hook(0x528F8C, con_devmap_autocompl_stub, HOOK_JUMP).install()->quick();
		//Utils::Hook::Nop(0x528F91, 2);
		//// map autocomplete :: change extentsion to .autocomplete and place nullfiles in main/maps/mp/mp_mapname.autocomplete
		//Utils::Hook(0x528E4F, con_map_autocompl_stub, HOOK_JUMP).install()->quick();
	}

	XO_Console::~XO_Console()
	{ }
}
