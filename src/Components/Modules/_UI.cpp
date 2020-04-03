#include "STDInclude.hpp"

namespace Components
{
	int _UI::GetTextHeight(Game::Font_s *font)
	{
		return font->pixelHeight;
	}

	float _UI::ScrPlace_ApplyX(int horzAlign, const float x, const float xOffs)
	{
		float result;
		switch (horzAlign)
		{
		case HORIZONTAL_APPLY_LEFT:
			result = Game::scrPlace->scaleVirtualToReal[0] * x + xOffs + Game::scrPlace->realViewableMin[0];
			break;

		case HORIZONTAL_APPLY_CENTER:
			result = Game::scrPlace->scaleVirtualToReal[0] * x + xOffs + Game::scrPlace->realViewportSize[0] * 0.5f;
			break;

		case HORIZONTAL_APPLY_RIGHT:
			result = Game::scrPlace->scaleVirtualToReal[0] * x + xOffs + Game::scrPlace->realViewableMax[0];
			break;

		case HORIZONTAL_APPLY_FULLSCREEN:
			result = Game::scrPlace->scaleVirtualToFull[0] * x + xOffs;
			break;

		case HORIZONTAL_APPLY_NONE:
			result = x;
			break;

		case HORIZONTAL_APPLY_TO640:
			result = Game::scrPlace->scaleRealToVirtual[0] * x + xOffs;
			break;

		case HORIZONTAL_APPLY_CENTER_SAFEAREA:
			result = (Game::scrPlace->realViewableMax[0] + Game::scrPlace->realViewableMin[0]) * 0.5f + xOffs + Game::scrPlace->scaleVirtualToReal[0] * x;
			break;

		case HORIZONTAL_APPLY_CONSOLE_SPECIAL:
			result = Game::scrPlace->realViewableMax[0] - xOffs + Game::scrPlace->scaleVirtualToReal[0] * x;
			break;

		default:
			result = x;
		}

		return result;
	}

	float _UI::ScrPlace_ApplyY(int vertAlign, const float y, const float yOffs)
	{
		float result;
		switch (vertAlign)
		{
		case VERTICAL_APPLY_TOP:
			result = Game::scrPlace->scaleVirtualToReal[1] * y + yOffs + Game::scrPlace->realViewableMin[1];
			break;

		case VERTICAL_APPLY_CENTER:
			result = Game::scrPlace->scaleVirtualToReal[1] * y + yOffs + Game::scrPlace->realViewportSize[1] * 0.5f;
			break;

		case VERTICAL_APPLY_BOTTOM:
			result = Game::scrPlace->scaleVirtualToReal[1] * y + yOffs + Game::scrPlace->realViewableMax[1];
			break;

		case VERTICAL_APPLY_FULLSCREEN:
			result = Game::scrPlace->scaleVirtualToFull[1] * y + yOffs;
			break;

		case VERTICAL_APPLY_NONE:
			result = y;
			break;

		case VERTICAL_APPLY_TO640:
			result = Game::scrPlace->scaleRealToVirtual[1] * y + yOffs;
			break;

		case VERTICAL_APPLY_CENTER_SAFEAREA:
			result = (Game::scrPlace->realViewableMax[1] + Game::scrPlace->realViewableMin[1]) * 0.5f + yOffs + Game::scrPlace->scaleVirtualToReal[1] * y;
			break;

		default:
			result = y;
		}

		return result;
	}

	void _UI::ScrPlace_ApplyRect(float *offs_x, float *w, float *offs_y, float *h, int horzAnker, int vertAnker)
	{
		float _x, _w, _y, _h;

		switch (horzAnker)
		{
		case HORIZONTAL_ALIGN_SUBLEFT:
			_x = Game::scrPlace->scaleVirtualToReal[0] * *offs_x + Game::scrPlace->subScreenLeft;
			_w = Game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_LEFT:
			_x = Game::scrPlace->scaleVirtualToReal[0] * *offs_x + Game::scrPlace->realViewableMin[0];
			_w = Game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_CENTER:
			_x = Game::scrPlace->scaleVirtualToReal[0] * *offs_x + Game::scrPlace->realViewportSize[0] * 0.5f;
			_w = Game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_RIGHT:
			_x = Game::scrPlace->scaleVirtualToReal[0] * *offs_x + Game::scrPlace->realViewableMax[0];
			_w = Game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_FULLSCREEN:
			_x = Game::scrPlace->scaleVirtualToFull[0] * *offs_x;
			_w = Game::scrPlace->scaleVirtualToFull[0] * *w;
			break;

		case HORIZONTAL_ALIGN_NOSCALE:
			goto USE_VERT_ALIGN; // we might wan't vertical alignment

		case HORIZONTAL_ALIGN_TO640:
			_x = Game::scrPlace->scaleRealToVirtual[0] * *offs_x;
			_w = Game::scrPlace->scaleRealToVirtual[0] * *w;
			break;

		case HORIZONTAL_ALIGN_CENTER_SAFEAREA:
			_x = (Game::scrPlace->realViewableMax[0] + Game::scrPlace->realViewableMin[0]) * 0.5f + Game::scrPlace->scaleVirtualToReal[0] * *offs_x;
			_w = Game::scrPlace->scaleVirtualToReal[0] * *w;
			break;

		default:
			goto USE_VERT_ALIGN; // we might wan't vertical alignment

		}

		*offs_x = _x;
		*w = _w;

	USE_VERT_ALIGN:
		switch (vertAnker)
		{
		case VERTICAL_ALIGN_TOP:
			_y = Game::scrPlace->scaleVirtualToReal[1] * *offs_y + Game::scrPlace->realViewableMin[1];
			_h = Game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_CENTER:
			_y = Game::scrPlace->scaleVirtualToReal[1] * *offs_y + Game::scrPlace->realViewportSize[1] * 0.5f;
			_h = Game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_BOTTOM:
			_y = Game::scrPlace->scaleVirtualToReal[1] * *offs_y + Game::scrPlace->realViewableMax[1];
			_h = Game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_FULLSCREEN:
			_y = Game::scrPlace->scaleVirtualToFull[1] * *offs_y;
			_h = Game::scrPlace->scaleVirtualToFull[1] * *h;
			break;

		case VERTICAL_ALIGN_NOSCALE:
			return; // don't do a thing

		case VERTICAL_ALIGN_TO480:
			_y = Game::scrPlace->scaleRealToVirtual[1] * *offs_y;
			_h = Game::scrPlace->scaleRealToVirtual[1] * *h;
			break;

		case VERTICAL_ALIGN_CENTER_SAFEAREA:
			_y = Game::scrPlace->scaleVirtualToReal[1] * *offs_y + (Game::scrPlace->realViewableMax[1] + Game::scrPlace->realViewableMin[1]) * 0.5f;
			_h = Game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_SUBTOP:
			_y = Game::scrPlace->scaleVirtualToReal[1] * *offs_y;
			_h = Game::scrPlace->scaleVirtualToReal[1] * *h;
			break;

		default:
			return; // don't do a thing
		}

		*offs_y = _y;
		*h = _h;
	}

	void _UI::ScrPlaceFull_ApplyRect(float *offs_x, float *w, float *offs_y, float *h, int horzAnker, int vertAnker)
	{
		float _x, _w, _y, _h;

		switch (horzAnker)
		{
		case HORIZONTAL_ALIGN_SUBLEFT:
			_x = Game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x + Game::scrPlaceFull->subScreenLeft;
			_w = Game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_LEFT:
			_x = Game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x + Game::scrPlaceFull->realViewableMin[0];
			_w = Game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_CENTER:
			_x = Game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x + Game::scrPlaceFull->realViewportSize[0] * 0.5f;
			_w = Game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_RIGHT:
			_x = Game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x + Game::scrPlaceFull->realViewableMax[0];
			_w = Game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		case HORIZONTAL_ALIGN_FULLSCREEN:
			_x = Game::scrPlaceFull->scaleVirtualToFull[0] * *offs_x;
			_w = Game::scrPlaceFull->scaleVirtualToFull[0] * *w;
			break;

		case HORIZONTAL_ALIGN_NOSCALE:
			goto USE_VERT_ALIGN; // we might wan't vertical alignment

		case HORIZONTAL_ALIGN_TO640:
			_x = Game::scrPlaceFull->scaleRealToVirtual[0] * *offs_x;
			_w = Game::scrPlaceFull->scaleRealToVirtual[0] * *w;
			break;

		case HORIZONTAL_ALIGN_CENTER_SAFEAREA:
			_x = (Game::scrPlaceFull->realViewableMax[0] + Game::scrPlaceFull->realViewableMin[0]) * 0.5f + Game::scrPlaceFull->scaleVirtualToReal[0] * *offs_x;
			_w = Game::scrPlaceFull->scaleVirtualToReal[0] * *w;
			break;

		default:
			goto USE_VERT_ALIGN; // we might wan't vertical alignment

		}

		*offs_x = _x;
		*w = _w;

	USE_VERT_ALIGN:
		switch (vertAnker)
		{
		case VERTICAL_ALIGN_TOP:
			_y = Game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y + Game::scrPlaceFull->realViewableMin[1];
			_h = Game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_CENTER:
			_y = Game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y + Game::scrPlaceFull->realViewportSize[1] * 0.5f;
			_h = Game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_BOTTOM:
			_y = Game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y + Game::scrPlaceFull->realViewableMax[1];
			_h = Game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_FULLSCREEN:
			_y = Game::scrPlaceFull->scaleVirtualToFull[1] * *offs_y;
			_h = Game::scrPlaceFull->scaleVirtualToFull[1] * *h;
			break;

		case VERTICAL_ALIGN_NOSCALE:
			return; // don't do a thing

		case VERTICAL_ALIGN_TO480:
			_y = Game::scrPlaceFull->scaleRealToVirtual[1] * *offs_y;
			_h = Game::scrPlaceFull->scaleRealToVirtual[1] * *h;
			break;

		case VERTICAL_ALIGN_CENTER_SAFEAREA:
			_y = Game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y + (Game::scrPlaceFull->realViewableMax[1] + Game::scrPlaceFull->realViewableMin[1]) * 0.5f;
			_h = Game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		case VERTICAL_ALIGN_SUBTOP:
			_y = Game::scrPlaceFull->scaleVirtualToReal[1] * *offs_y;
			_h = Game::scrPlaceFull->scaleVirtualToReal[1] * *h;
			break;

		default:
			return; // don't do a thing
		}

		*offs_y = _y;
		*h = _h;
	}
	// -------------------------------------------------------------------------------------------------------------------------------

	// fade from black into main menu on first start 
	#define mainmenu_init_fadeTime 50.0f

	bool	mainmenu_fadeDone = false;
	float	mainmenu_fadeTime = mainmenu_init_fadeTime;
	float	mainmenu_fadeColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	void UI_MainMenu_FadeIn()
	{
		if (!Game::Sys_IsMainThread() || mainmenu_fadeDone)
		{
			return;
		}
			
		if (mainmenu_fadeTime >= 0.0f)
		{
			mainmenu_fadeTime -= 0.5f;
			mainmenu_fadeColor[3] = (mainmenu_fadeTime * (1.0f / mainmenu_init_fadeTime));
			
			Game::ConDraw_Box(mainmenu_fadeColor, 0.0f, 0.0f, (float)Game::_uiContext->screenWidth, (float)Game::_uiContext->screenHeight);
			return;
		}

		mainmenu_fadeDone = true;
	}

	// draw version string in main menu
	void UI_MainMenu_Ver()
	{
		if (!Game::Sys_IsMainThread()) 
		{
			return;
		}

		float	scale, scale_x, scale_y, offs_x, offs_y;
		float	max = Game::scrPlace->scaleVirtualToReal[1] * 0.3f;

		char* font;

		if (Game::Dvar_FindVar("ui_smallFont")->current.value < max)
		{
			if (Game::Dvar_FindVar("ui_extraBigFont")->current.value > max)
			{
				font = FONT_BIG;

				if (Game::Dvar_FindVar("ui_bigFont")->current.value > max)
				{
					font = FONT_NORMAL;
				}
			}
			else 
				font = FONT_EXTRA_BIG;
		}
		else 
			font = FONT_SMALL;

		// get font handle
		void* fontHandle = Game::R_RegisterFont(font, sizeof(font));

		offs_x = 10.0f; offs_y = -10.0f; scale = 0.25f;

		scale_x = scale * 48.0f / static_cast<Game::Font_s*>(fontHandle)->pixelHeight;
		scale_y = scale_x;

		// place container
		_UI::ScrPlace_ApplyRect(&offs_x, &scale_x, &offs_y, &scale_y, HORIZONTAL_ALIGN_LEFT, VERTICAL_ALIGN_BOTTOM);

		const char* textForeground = Utils::VA("IW3xo :: %.lf :: %s", IW3X_BUILDNUMBER, IW3XO_BUILDVERSION_DATE);
		const char* textBackground = textForeground;

		if (DEBUG)
		{
			textForeground = Utils::VA("IW3xo :: %.lf :: %s :: %s", IW3X_BUILDNUMBER, IW3XO_BUILDVERSION_DATE, "^1DEBUG");
			textBackground = Utils::VA("IW3xo :: %.lf :: %s :: %s", IW3X_BUILDNUMBER, IW3XO_BUILDVERSION_DATE, "DEBUG");
		}

		// I was drawing 2 strings, because there was a problem with the R_AddCmdDrawText wrapper
		// I somehow fixed that .. but we are still drawing 2 because I like it with a backdrop shadow :x

		// Background String
		float colorBackground[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

		Game::R_AddCmdDrawTextASM(
			/* txt */ textBackground,
			/* max */ 0x7FFFFFFF,
			/* fot */ fontHandle,
			/*  x  */ offs_x + 3.0f,
			/*  y  */ offs_y + 3.0f,
			/* scX */ scale_x,
			/* scY */ scale_y,
			/* rot */ 0.0f,
			/* col */ colorBackground,
			/* sty */ 0);

		// Foreground String
		float colorForeground[4] = { 1.0f, 0.8f, 0.7f, 1.0f };

		Game::R_AddCmdDrawTextASM(
			/* txt */ textForeground,
			/* max */ 0x7FFFFFFF,
			/* fot */ fontHandle,
			/*  x  */ offs_x,
			/*  y  */ offs_y,
			/* scX */ scale_x,
			/* scY */ scale_y,
			/* rot */ 0.0f,
			/* col */ colorForeground,
			/* sty */ 0);

		// fade in the menu on first start
		UI_MainMenu_FadeIn();
	}

	// -- UI_MainMenu_Ver
	__declspec(naked) void UI_MainMenu_ver_stub()
	{
		__asm
		{
			Call	UI_MainMenu_Ver
			push	0x54353A;
			retn
		}
	}

	// --------------------------------------------------------------

	// gameTime constant is only updated in-game, so .. lets fix that
	float menu_gameTime = 0.0f;

	void UI_SetCustom_CodeConstants()
	{
		// check if we are in a menu to stop overwriting filtertap? needed?
		if (menu_gameTime >= 10000.0f) 
		{
			menu_gameTime = 0.0f;
		}

		menu_gameTime += 0.01f;

		// -- mouse position --
		glm::vec2 mousePos;

		// normalize mouse to ui resolution
		mousePos.x = (Game::_uiContext->cursor.x / 640.0f);
		mousePos.y = (Game::_uiContext->cursor.y / 480.0f);

		// screencenter will be 0.0 ( -1.0, 1.0 = bottom left )
		mousePos.x = mousePos.x * 2.0f - 1.0f;
		mousePos.y = mousePos.y * 2.0f - 1.0f;

		// filterTap 6
		Game::gfxCmdBufSourceState->input.consts[27][0] = Dvars::ui_button_highlight_radius->current.value;
		Game::gfxCmdBufSourceState->input.consts[27][1] = Dvars::ui_button_highlight_brightness->current.value;
		Game::gfxCmdBufSourceState->input.consts[27][2] = Dvars::ui_button_outline_radius->current.value;
		Game::gfxCmdBufSourceState->input.consts[27][3] = Dvars::ui_button_outline_brightness->current.value;

		// filterTap 7
		Game::gfxCmdBufSourceState->input.consts[28][0] = mousePos.x;
		Game::gfxCmdBufSourceState->input.consts[28][1] = mousePos.y;	
#if DEBUG		
		Game::gfxCmdBufSourceState->input.consts[28][2] = Dvars::xo_menu_dbg->current.value; 
#endif
		Game::gfxCmdBufSourceState->input.consts[28][3] = menu_gameTime;

		// get current viewport width / height and update ui_dvars
		int vpWidth = (int)floorf(_UI::ScrPlace_ApplyX(HORIZONTAL_APPLY_RIGHT, 0.0f, 0.0f));
		int vpHeight = (int)floorf(_UI::ScrPlace_ApplyY(VERTICAL_APPLY_BOTTOM, 0.0f, 0.0f));

		Game::Dvar_SetValue(Dvars::ui_renderWidth, vpWidth);
		Game::Dvar_SetValue(Dvars::ui_renderHeight, vpHeight);
	}

	// -- UI_SetCustom_CodeConstants
	__declspec(naked) void RB_Draw3DCommon_stub()
	{
		const static uint32_t R_RenderAllLeftovers_Func = 0x615570;
		__asm
		{
			pushad
			Call	UI_SetCustom_CodeConstants
			popad

			// args are on the stack
			Call	R_RenderAllLeftovers_Func

			push	6156F1h
			retn
		}
	}

	// the fuck did I do

	struct ui_material
	{
		Game::Material *material;
	};

	ui_material* mat_white_ptr = reinterpret_cast<ui_material*>(0xCAF06F0);
	Game::Material *ui_white_material;

	void UI_InitWhiteMaterial()
	{
		ui_white_material = mat_white_ptr->material;
	}
	
	// ITEM-SLIDER :: Background
	__declspec(naked) void Item_ListBox_Paint_SliderBackground_stub()
	{
		const static uint32_t returnPt = 0x55330D; // jump onto the call to UI_DrawHandlePic

		const static float xOffs = 11.0f;
		const static float yOffs = -1.0f;
		const static float hOffs =  0.0f; // add additional height
		const static float meme2 =  0.0f; // add additional height
		const static float width =  6.0f;

		const static float color[4] = LISTBOX_SLIDER_BACKGROUND_COLOR;
		const static float *colorPtr = color; // we need a ptr to our color

		// white material ptr location @ 0xCAF06F0 -points-to> 0xFA4EC8
		__asm
		{
			pushad
			Call	UI_InitWhiteMaterial
			popad

			fld     dword ptr[esp + 50h]		// stock op
			mov     eax, [edi]					// stock op
			fadd	yOffs						// stock op -- itemheight + yOffs
			mov		ecx, [ebp + eax * 4 + 4]	// stock op
			mov		[ebp + eax * 4 + 8], ecx	// stock op

			// ---------------------------------------------------
			// material <-> color
			mov     edx, ui_white_material//[0xFA4EC8]		// ui white material handle
			mov     eax, [ebx + 18h]		// stock op
			fstp    dword ptr[esp + 50h]	// stock op
			fld     dword ptr[ebx + 10h]	// stock op
			mov     ecx, [ebx + 14h]		// stock op
			fadd	hOffs					// stock op -- h - hOffs
			add     esp, 1Ch				// stock op
			push    edx						// stock op -- material
			push    eax						// stock op -- vertAlign
			fstp    dword ptr[esp + 20h]	// stock op
			push    ecx						// stock op -- horzAlign
			fld     dword ptr[esp + 24h]	// stock op
			sub     esp, 10h				// stock op
			fadd	meme2					// stock op -- new h + 1.0f
			mov     ecx, colorPtr			// color -- was "xor ecx, ecx"

			// ---------------------------------------------------
			// continue till we are at x
			mov     edx, esi				// stock op -- placement
			fstp    dword ptr[esp + 44h]	// stock op
			fld     dword ptr[esp + 44h]	// stock op
			fstp    dword ptr[esp + 0Ch]	// stock op -- h
			fld		width					// stock op
			fstp    dword ptr[esp + 8]		// stock op -- w
			fld     dword ptr[esp + 50h]	// stock op
			fstp    dword ptr[esp + 4]		// stock op -- y
			fld     dword ptr[esp + 30h]	// stock op
			fadd	xOffs
			fstp    dword ptr[esp]			// stock op -- x

			jmp		returnPt;
		}
	}

	// ITEM-SLIDER :: Thumb
	__declspec(naked) void Item_ListBox_Paint_SliderThumb_stub()
	{
		const static uint32_t returnPt = 0x5533C7; // jump onto the call to UI_DrawHandlePic

		const static float boxWH =  6.0f;
		const static float xOffs = 11.0f;
		const static float hOffs = 40.0f;
		const static float yOffs = -(hOffs * 0.5f) + 3.0f;
		

		const static float color[4] = LISTBOX_SLIDER_THUMB_COLOR;
		const static float *colorPtr = color; // we need a ptr to our color
		// white material ptr location @ 0xCAF06F0 -points-to> 0xFA4EC8
		__asm
		{
			pushad
			Call	UI_InitWhiteMaterial
			popad

			// material <-> color
			mov     edx, ui_white_material		// ui white material handle
			fld		boxWH				// stock op
			mov     eax, [ebx + 18h]	// stock op
			mov     ecx, [ebx + 14h]	// stock op
			push    edx					// stock op -- material
			push    eax					// stock op -- vertAlign
			push    ecx					// stock op -- horzAlign
			sub     esp, 10h				// stock op
			fadd	hOffs
			fst     dword ptr[esp + 0Ch]	// stock op -- h (uses boxWH)
			mov     ecx, colorPtr			// color -- was "xor ecx, ecx"
			fsub	hOffs
			fstp    dword ptr[esp + 8]		// stock op -- w (uses boxWH)
			mov     edx, esi				// stock op
			fld     dword ptr[esp + 34h]	// stock op
			fadd	yOffs
			fstp    dword ptr[esp + 4]		// stock op -- y
			fld     dword ptr[esp + 30h]	// stock op
			fadd	xOffs
			fstp    dword ptr[esp]			// stock op -- x

			jmp		returnPt;
		}
	}

	// --------------------------------------------------------------

	// initally reset ultrawide on each r_aspectRatio call
	void UltrawideDvarFalse()
	{
		Game::Dvar_SetValue(Dvars::ui_ultrawide, false);
	}

	// -- UltrawideDvarFalse
	__declspec(naked) void R_AspectRatio_Reset_Ultrawide_stub()
	{
		const static uint32_t returnPt = 0x5F3534;
		__asm
		{
			pushad
			Call	UltrawideDvarFalse
			popad

			mov     eax, [eax + 0Ch]	// overwritten op
			cmp     eax, 3				// overwritten op

			jmp		returnPt			// jump back to break op
		}
	}

	// set ultrawide if 21:9 aspect is used
	void UltrawideDvarTrue()
	{
		Game::Dvar_SetValue(Dvars::ui_ultrawide, true);
	}

	// -- UltrawideDvarTrue // use default switchcase to implement 21:9
	__declspec(naked) void R_AspectRatio_Ultrawide_stub()
	{
		const static float ultraWideAspect = 2.3333333f;
		const static uint32_t returnPt = 0x5F35E5;
		__asm
		{
			pushad
			Call	UltrawideDvarTrue
			popad

			push	eax							// push throwaway register
			//mov		eax, Dvars::ultrawide		// mov ultrawide dvar ptr into eax
			//mov		[eax + 12], 1				// set ultrawide to true
			mov		eax, 0xCC9D0F8				// move address of *Game::r_aspectRatio_float

			fld		ultraWideAspect		// load our custom aspect float
			fstp	[eax]				// set *Game::r_aspectRatio_float
			pop		eax					// pop

			mov     ecx, 1				// widescreen true
			jmp		returnPt			// jump back to break op
		}
	}

	// --------------------------------------------------------------

	// used to add custom uiScripts for menus 
	// :: returns true when it matched a valid uiScript string to skip the original function to avoid error prints and to make overwriting stock uiScripts a thing
	int UI_uiScriptAddons(const char *arg)
	{
		if (!Utils::Q_stricmpn(arg, "StartServerCustom", 0x7FFFFFFF))
		{
			const char* spawnServerStr;

			auto ui_dedicated			= Game::Dvar_FindVar("ui_dedicated");
			auto ui_developer			= Game::Dvar_FindVar("ui_developer");
			auto ui_developer_script	= Game::Dvar_FindVar("ui_developer_script");
			
			if (ui_dedicated->current.integer != 0) 
			{
				Game::Com_PrintMessage(0, Utils::VA("^3Warning ^7:: setting \"dedicated\" to 0 because advanced server settings only supports listen servers!\n"), 0);
			}

			Game::Dvar_SetValue(Game::Dvar_FindVar("dedicated"), 0);

			const char* gameTypeStr = (const char*)Game::gameTypeEnum[0x2 * Game::Dvar_FindVar("ui_netGameType")->current.integer];
			const char* mapNameStr	= (const char*)Game::mapNameEnum[0x28 * Game::Dvar_FindVar("ui_currentNetMap")->current.integer];

			if (!gameTypeStr) 
			{
				Game::Com_PrintMessage(0, Utils::VA("^3Warning ^7:: gameType was empty! Defaulting to dm!\n"), 0);
				gameTypeStr = "dm";
			}

			if (!mapNameStr) 
			{
				Game::Com_PrintMessage(0, Utils::VA("^3Warning ^7:: map name was empty! Defaulting to mp_backlot!\n"), 0);
				mapNameStr = "mp_backlot";
			}

			// set gametype
			Game::Dvar_SetString(gameTypeStr, Game::Dvar_FindVar("g_gametype"));
			
			// server spawn string
			spawnServerStr = Utils::VA("wait ; wait ; set developer %i; set developer_script %i; %s %s\n", 
																			ui_developer->current.integer, 
																			ui_developer_script->current.integer, 
																			Dvars::ui_devmap->current.enabled ? "devmap" : "map", 
																			mapNameStr);

			Game::Com_PrintMessage(0, Utils::VA("^2Spawning server ^7:: %s", spawnServerStr), 0);
			Game::Cbuf_AddText(spawnServerStr, 0);

			// return true to skip the stock function that looks for valid uiScripts
			return 1;
		}

		// return false and continue with the stock function
		return 0;
	}

	// -- UI_uiScriptsAddons()
	__declspec(naked) void UI_uiScriptsAddons_stub()
	{
		const static char* overwrittenStr = "StartServer";
		const static uint32_t stockScripts = 0x545BF2;  // next op after hook spot
		const static uint32_t ifAddonReturn = 0x546E52; // jump to the valid return point if we had a valid match in addons
		__asm
		{
			// call our addon function
			lea     edx, [esp + 58h]	// out arg // was at 5C but as we push, we offset esp ;)
										// we don't need to take care of edx after this so .. 
			pushad						
			push	edx					
			Call	UI_uiScriptAddons
			add		esp,4
			test	eax, eax
			je		STOCK_FUNC			// jump if UI_uiScriptsAddons was false
			popad

			jmp		ifAddonReturn		// return to skip the stock function

			STOCK_FUNC:
				popad
				push	overwrittenStr
				jmp		stockScripts	// jump back and exec the original function
		}
	}

	_UI::_UI()
	{
		// ---------------
		// Main Menu Hooks

		// Main Menu Version (UI_VersionNumber Call in UI_Refresh)
		Utils::Hook(0x543535, UI_MainMenu_ver_stub, HOOK_JUMP).install()->quick();

		// hook "R_RenderAllLeftovers" in RB_Draw3DCommon to set custom codeconstants
		Utils::Hook(0x6156EC, RB_Draw3DCommon_stub, HOOK_JUMP).install()->quick();

		// hook "Item_RunScript" to implement custom uiScripts callable from menu files
		/*Utils::Hook::Nop(0x54DF35, 8);*/	Utils::Hook(0x545BED, UI_uiScriptsAddons_stub, HOOK_JUMP).install()->quick();

		// *
		// List Box Slider

		// Change material "ui_scrollbar" to white and add a color (background)
		Utils::Hook::Nop(0x5532A0, 6);	Utils::Hook(0x5532A0, Item_ListBox_Paint_SliderBackground_stub, HOOK_JUMP).install()->quick();
		
		// disable drawing of upper arrow
		Utils::Hook::Nop(0x55329B, 5); // nop "UI_DrawHandlePic" call

		// disable drawing of lower arrow
		Utils::Hook::Nop(0x55335A, 5); // nop "UI_DrawHandlePic" call

		// Change material "material_ui_scrollbar_thumb" to white and add a color (thumb)
		Utils::Hook::Nop(0x553394, 6);	Utils::Hook(0x553394, Item_ListBox_Paint_SliderThumb_stub, HOOK_JUMP).install()->quick();

		// --------
		// Commands

		// menu_open menuName .. currently only opens menus when in a menu?
		Command::Add("menu_open", [](Command::Params params)
		{
			if (params.Length() < 2) 
			{
				return;
			}

			const char * name = params[1];
			Game::UiContext *ui = &Game::_uiContext[0];
			Game::clientUI->displayHUDWithKeycatchUI = true;

			Game::Menus_OpenByName(name, ui);
		});

		Command::Add("menu_openIngame", [](Command::Params params)
		{
			if (params.Length() < 2) 
			{
				return;
			}

			const char * name = params[1];
			Game::UiContext *ui = &Game::_uiContext[0];

			Game::Key_SetCatcher();
			Game::Menus_CloseAll(ui);
			Game::Menus_OpenByName(name, ui);
		});

		Command::Add("menu_closeAll", [](Command::Params params)
		{
			Game::UiContext *ui = &Game::_uiContext[0];
			Game::Menus_CloseAll(ui);
		});

		// menu_setActive 0 - 11 to open default ones
		Command::Add("menu_setActive", [](Command::Params params)
		{
			if (params.Length() < 2) 
			{
				return;
			}

			int menuNum = std::atoi(params[1]);
			if (menuNum == 0 || menuNum <= 12)
			{
				Game::UI_SetActiveMenu(0, menuNum);
			}
		});

		// -----
		// Dvars

		Dvars::ui_button_highlight_radius = Game::Dvar_RegisterFloat(
			/* name		*/ "ui_button_highlight_radius",
			/* desc		*/ "highlight radius (scale)",
			/* default	*/ 0.85f,
			/* minVal	*/ 0.0001f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_button_highlight_brightness = Game::Dvar_RegisterFloat(
			/* name		*/ "ui_button_highlight_brightness",
			/* desc		*/ "highlight brightness (lerp)",
			/* default	*/ 0.5f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 2.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_button_outline_radius = Game::Dvar_RegisterFloat(
			/* name		*/ "ui_button_outline_radius",
			/* desc		*/ "outline radius (scale)",
			/* default	*/ 0.95f,
			/* minVal	*/ 0.0001f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_button_outline_brightness = Game::Dvar_RegisterFloat(
			/* name		*/ "ui_button_outline_brightness",
			/* desc		*/ "outline brightness (lerp)",
			/* default	*/ 0.7f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 2.0f,
			/* flags	*/ Game::dvar_flags::saved);

#if DEBUG
		Dvars::xo_menu_dbg = Game::Dvar_RegisterFloat(
			/* name		*/ "xo_menu_dbg",
			/* desc		*/ "dvar to debug menu shaders",
			/* default	*/ 1.0f,
			/* minVal	*/ -10000.0f,
			/* maxVal	*/ 10000.0f,
			/* flags	*/ Game::dvar_flags::none);
#endif

		Dvars::ui_subnavbar = Game::Dvar_RegisterBool(
			/* name		*/ "ui_subnavbar",
			/* desc		*/ "menu helper",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::read_only);

		Dvars::ui_subnavbar_item = Game::Dvar_RegisterInt(
			/* name		*/ "ui_subnavbar_item",
			/* desc		*/ "menu helper",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 100,
			/* flags	*/ Game::dvar_flags::read_only);

		Dvars::ui_renderWidth = Game::Dvar_RegisterInt(
			/* name		*/ "ui_renderWidth",
			/* desc		*/ "menu helper",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 7680,
			/* flags	*/ Game::dvar_flags::read_only);

		Dvars::ui_renderHeight = Game::Dvar_RegisterInt(
			/* name		*/ "ui_renderHeight",
			/* desc		*/ "menu helper",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 4320,
			/* flags	*/ Game::dvar_flags::read_only);

		Dvars::ui_devmap = Game::Dvar_RegisterBool(
			/* name		*/ "ui_devmap",
			/* desc		*/ "menu helper",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_developer = Game::Dvar_RegisterInt(
			/* name		*/ "ui_developer",
			/* desc		*/ "menu helper",
			/* default	*/ 0,
			/* minVal	*/ 0,
			/* maxVal	*/ 2,
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_developer_script = Game::Dvar_RegisterBool(
			/* name		*/ "ui_developer_script",
			/* desc		*/ "menu helper",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::saved);

		// -------
		// Display

		// Hook R_AspectRatio to initially reset the ultrawide dvar
		Utils::Hook::Nop(0x5F352E, 6); // clear space
		Utils::Hook(0x5F352E, R_AspectRatio_Reset_Ultrawide_stub, HOOK_JUMP).install()->quick();

		// Set 21:9 aspect by using the default switchcase in R_AspectRatio
		Utils::Hook::Nop(0x5F35FA, 6); // clear space
		Utils::Hook(0x5F35FA, R_AspectRatio_Ultrawide_stub, HOOK_JUMP).install()->quick();

		// -----
		// Dvars

		static std::vector <char*> r_customAspectratio = 
		{ 
			"auto", 
			"4:3", 
			"16:10", 
			"16:9", 
			"21:9", 
		};

		Dvars::r_aspectRatio = Game::Dvar_RegisterEnum(
			/* name		*/ "r_aspectRatio",
			/* desc		*/ "Screen aspect ratio. \"auto\" does not choose 21:9 automatically!",
			/* default	*/ 0,
			/* enumSize	*/ r_customAspectratio.size(),
			/* enumData */ r_customAspectratio.data(),
			/* flags	*/ Game::dvar_flags::saved);

		Dvars::ui_ultrawide = Game::Dvar_RegisterBool(
			/* name		*/ "ui_ultrawide",
			/* desc		*/ "menu helper",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::read_only);
	}

	_UI::~_UI()
	{ }
}