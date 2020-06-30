// *
// RadiantRemote
#define RADIANT_MAX_SEL_BRUSHES 16 // max amount of selected brushes (from radiant) to parse (should always match REMOTE_MAX_SEL_BRUSHES in radiant)

// *
// Common
#define FF_LOAD_ADDON_MENU		true
#define FF_ADDON_MENU_NAME		"xcommon_iw3xo_menu"

#define FF_LOAD_ADDON_REQ		true
#define FF_ADDON_REQ_NAME		"xcommon_iw3xo"

#define FF_LOAD_ADDON_OPT		true
#define FF_ADDON_OPT_NAME		"xcommon_iw3xo_addon"

// *
// Version
#define IW3XO_BUILDVERSION_DATE __TIMESTAMP__  
#define IW3XO_BUILDSTRING "IW3xo :: build %.lf :: %s", IW3X_BUILDNUMBER, __TIMESTAMP__
#define IW3XO_BUILDSTRING_CHANGELOG "IW3xo :: build %.lf :: %s %s", IW3X_BUILDNUMBER, __TIMESTAMP__

// *
// Gui
#define GET_GGUI Game::Globals::gui
#define GGUI_READY Game::Globals::gui.imgui_initialized
#define GGUI_READY_DVARS Game::Globals::gui.dvars_initialized
#define GGUI_ANY_MENUS_OPEN Game::Globals::gui.any_menus_open
#define GGUI_MENU_COUNT 4

#define DEVGUI_OCEAN // comment to disable tab

// *
// Screen
#define SCREEN_WIDTH 640	// all drawing is done to a 640*480 virtual screen size
#define SCREEN_HEIGHT 480	// and will be automatically scaled to the real resolution

// *
// Changelog (avoid tabs (will result in dots); empty strings "." + large y offset to move them out of visible space)
#if DEBUG
	#define IW3XO_CHANGELOG_TITLE_FMT	"IW3XO :: %.lf :: %s :: ^1DEBUG\n"
#else
	#define IW3XO_CHANGELOG_TITLE_FMT	"IW3XO :: %.lf :: %s\n"
#endif

#define IW3XO_CHANGELOG_01_Y_OFFS	10
#define IW3XO_CHANGELOG_01_SEP		1
#define IW3XO_CHANGELOG_01_TITLE	"[General]\n"
#define IW3XO_CHANGELOG_01			"-  Added dvar r_aspectRatio_custom (21:9 = 2.333) - Use r_aspectRatio custom to enable\n" \
									"-  Automatically load xcommon_iw3xo_addon.ff if found in zone/language (user fastfile to overwrite loaded assets)\n" \
									"-  cubeMapShot fixed (disable r_smp_backend!)"

#define IW3XO_CHANGELOG_02_Y_OFFS	0
#define IW3XO_CHANGELOG_02_SEP		1
#define IW3XO_CHANGELOG_02_TITLE	"[Map Export]"
#define IW3XO_CHANGELOG_02			"-  Fixed brush indices drawing when using r_drawCollision_brushIndexVisible\n"	\
									"-  Minor fixes to texture scaling when exporting maps\n" \
									"-  Use dvar mapexport_brush5Sides to export brushes with only 5 sides"

#define IW3XO_CHANGELOG_03_Y_OFFS	-8
#define IW3XO_CHANGELOG_03_SEP		1
#define IW3XO_CHANGELOG_03_TITLE	"[ImGui / Ocean Shader]"
#define IW3XO_CHANGELOG_03			"-  Implemented Ocean tab to tweak ocean shader settings in realtime\n" \
									"-  Use the export button to export shader settings\n" \
									"-  Needs #define USE_CUSTOM_CONSTANTS to work"

#define IW3XO_CHANGELOG_04_Y_OFFS	-24
#define IW3XO_CHANGELOG_04_SEP		1
#define IW3XO_CHANGELOG_04_TITLE	"[Shader Overlays]"
#define IW3XO_CHANGELOG_04			"-  xo_shaderoverlay can now be set to custom (define postfx material with xo_shaderoverlay_custom)\n" \
									"-  Added dvar xo_shaderdbg_matrix to debug transformation matrices"


// *
// General
#define FONT_SMALL_DEV  "fonts/smallDevFont"
#define FONT_BIG_DEV    "fonts/bigDevFont"
#define FONT_CONSOLE    "fonts/consoleFont"
#define FONT_BIG        "fonts/bigFont"
#define FONT_SMALL      "fonts/smallFont"
#define FONT_BOLD       "fonts/boldFont"
#define FONT_NORMAL     "fonts/normalFont"
#define FONT_EXTRA_BIG  "fonts/extraBigFont"
#define FONT_OBJECTIVE  "fonts/objectiveFont"

#define KEYCATCHER_NONE	0x0
#define KEYCATCHER_TAB	0x9
#define KEYCATCHER_ENTER 0xD
#define KEYCATCHER_ESCAPE 0x1B
#define KEYCATCHER_SPACE 0x20
#define KEYCATCHER_GRAVE 0x60
#define KEYCATCHER_TILDE 0x7E
#define KEYCATCHER_BACKSPACE 0x7F
#define KEYCATCHER_ASCII_FIRST 0x80
#define KEYCATCHER_ASCII_181 0x80
#define KEYCATCHER_ASCII_191 0x81
#define KEYCATCHER_ASCII_223 0x82
#define KEYCATCHER_ASCII_224 0x83
#define KEYCATCHER_ASCII_225 0x84
#define KEYCATCHER_ASCII_228 0x85
#define KEYCATCHER_ASCII_229 0x86
#define KEYCATCHER_ASCII_230 0x87
#define KEYCATCHER_ASCII_231 0x88
#define KEYCATCHER_ASCII_232 0x89
#define KEYCATCHER_ASCII_233 0x8A
#define KEYCATCHER_ASCII_236 0x8B
#define KEYCATCHER_ASCII_241 0x8C
#define KEYCATCHER_ASCII_242 0x8D
#define KEYCATCHER_ASCII_243 0x8E
#define KEYCATCHER_ASCII_246 0x8F
#define KEYCATCHER_ASCII_248 0x90
#define KEYCATCHER_ASCII_249 0x91
#define KEYCATCHER_ASCII_250 0x92
#define KEYCATCHER_ASCII_252 0x93
#define KEYCATCHER_END_ASCII_CHARS 0x94
#define KEYCATCHER_COMMAND 0x96
#define KEYCATCHER_CAPSLOCK 0x97
#define KEYCATCHER_POWER 0x98
#define KEYCATCHER_PAUSE 0x99
#define KEYCATCHER_UPARROW 0x9A
#define KEYCATCHER_DOWNARROW 0x9B
#define KEYCATCHER_LEFTARROW 0x9C
#define KEYCATCHER_RIGHTARROW 0x9D
#define KEYCATCHER_ALT 0x9E
#define KEYCATCHER_CTRL 0x9F
#define KEYCATCHER_SHIFT 0xA0
#define KEYCATCHER_INS 0xA1
#define KEYCATCHER_DEL 0xA2
#define KEYCATCHER_PGDN 0xA3
#define KEYCATCHER_PGUP 0xA4
#define KEYCATCHER_HOME 0xA5
#define KEYCATCHER_END 0xA6
#define KEYCATCHER_F1 0xA7
#define KEYCATCHER_F2 0xA8
#define KEYCATCHER_F3 0xA9
#define KEYCATCHER_F4 0xAA
#define KEYCATCHER_F5 0xAB
#define KEYCATCHER_F6 0xAC
#define KEYCATCHER_F7 0xAD
#define KEYCATCHER_F8 0xAE
#define KEYCATCHER_F9 0xAF
#define KEYCATCHER_F10 0xB0
#define KEYCATCHER_F11 0xB1
#define KEYCATCHER_F12 0xB2
#define KEYCATCHER_F13 0xB3
#define KEYCATCHER_F14 0xB4
#define KEYCATCHER_F15 0xB5
#define KEYCATCHER_KP_HOME 0xB6
#define KEYCATCHER_KP_UPARROW 0xB7
#define KEYCATCHER_KP_PGUP 0xB8
#define KEYCATCHER_KP_LEFTARROW 0xB9
#define KEYCATCHER_KP_5 0xBA
#define KEYCATCHER_KP_RIGHTARROW 0xBB
#define KEYCATCHER_KP_END 0xBC
#define KEYCATCHER_KP_DOWNARROW 0xBD
#define KEYCATCHER_KP_PGDN 0xBE
#define KEYCATCHER_KP_ENTER 0xBF
#define KEYCATCHER_KP_INS 0xC0
#define KEYCATCHER_KP_DEL 0xC1
#define KEYCATCHER_KP_SLASH 0xC2
#define KEYCATCHER_KP_MINUS 0xC3
#define KEYCATCHER_KP_PLUS 0xC4
#define KEYCATCHER_KP_NUMLOCK 0xC5
#define KEYCATCHER_KP_STAR 0xC6
#define KEYCATCHER_KP_EQUALS 0xC7
#define KEYCATCHER_MOUSE1 0xC8
#define KEYCATCHER_MOUSE2 0xC9
#define KEYCATCHER_MOUSE3 0xCA
#define KEYCATCHER_MOUSE4 0xCB
#define KEYCATCHER_MOUSE5 0xCC
#define KEYCATCHER_MWHEELDOWN 0xCD
#define KEYCATCHER_MWHEELUP 0xCE
#define KEYCATCHER_AUX1 0xCF
#define KEYCATCHER_AUX2 0xD0
#define KEYCATCHER_AUX3 0xD1
#define KEYCATCHER_AUX4 0xD2
#define KEYCATCHER_AUX5 0xD3
#define KEYCATCHER_AUX6 0xD4
#define KEYCATCHER_AUX7 0xD5
#define KEYCATCHER_AUX8 0xD6
#define KEYCATCHER_AUX9 0xD7
#define KEYCATCHER_AUX10 0xD8
#define KEYCATCHER_AUX11 0xD9
#define KEYCATCHER_AUX12 0xDA
#define KEYCATCHER_AUX13 0xDB
#define KEYCATCHER_AUX14 0xDC
#define KEYCATCHER_AUX15 0xDD
#define KEYCATCHER_AUX16 0xDE
#define KEYCATCHER_LAST_KEY 0xDF