#pragma once
#include <fstream>
#include <iomanip>

#define MENU_TABBING "\t"
#define MENUDEF_TABBING "\t\t"
#define ITEMDEF_TABBING "\t\t\t"

//some macros to make code look neater
#define PADDING(x) std::left << std::setw(maxPadding - x) << std::cout.fill(' ')
#define WRITE_TEXT(name, value)	\
	if (value) {				\
		write_text(name, value);	\
	}

#define WRITE_ACTION(name, value)	\
	if(value) {						\
		write_action(name, value);	\
	}

//write number if not default value
#define WRITE_NUMBER(name, def, value)	\
	if(value != def) {					\
		write_number(name,value);		\
	}

#define WRITE_NUMBER_MAP(name, def, value, map)	\
	if(value != def) {							\
		write_number(name, value, map);			\
	}

namespace Components
{
	class menu_export final : public Component
	{
	public:
		menu_export();
		const char* getName() override { return "menu_export"; };

		void export_menu(Game::menuDef_t* menu, std::string subdir);
		void export_menu_itemdefs(Game::menuDef_t* menu, std::string subdir);

	private:
		typedef const std::unordered_map<unsigned int, std::string> identifierMap;
		typedef std::unordered_map<unsigned int, std::string>::const_iterator identifierMapIt;

		bool set_path(std::string subdir, std::string name);
		void write_menu_def(Game::menuDef_t *menuDef);
		void write_statement(std::string name, Game::statement_s stmt);
		void write_vec4(std::string name, float vec4[]);
		void write_item_def(Game::itemDef_s &itemDef);
		void write_line(std::string name);
		void write_rect(Game::rectDef_s rect);
		template <typename T>
		void write_number(std::string name, T num);
		void write_number(std::string name, int num, identifierMap& map);
		void write_text(std::string name, const char* text);
		void write_flag(int num);
		void write_action(std::string name, std::string action);
		void write_visible(Game::statement_s stmt);
		void write_key_action(int key, std::string action);


		std::string writeFilePath;
		int padding;
		int tabs;
		const float defaultTextSize = 0.55f;
		std::string tabbing;
		int maxPadding;
		std::ofstream menu_file_;		
		identifierMapIt find;
		const std::string headerText = "//This menu was generated using iw3xo Client\n//If you find any bug please report them here: https://github.com/xoxor4d/iw3xo-dev/issues\n\n#include \"ui/menudefinition.h\"\n\n";


		const enum itemType 
		{
			ITEM_TYPE_EDITFIELD = 4,
			ITEM_TYPE_LISTBOX = 6,	
			ITEM_TYPE_MULTI = 12,
			ITEM_TYPE_DVARENUM = 13,
			ITEM_TYPE_GAME_MESSAGE_WINDOW = 19,
		};

		const enum keyCodes 
		{
			KEY_MULTIPLY = 42,
			KEY_Z = 122,
		};

		const enum dvarFlags
		{
			DISABLEDVAR = 2,
			SHOWDVAR = 4,
			HIDEDVAR = 8,
			FOCUSDVAR = 16,
		};

		const std::string script_actions_[34] =
		{
			"fadein",
			"fadeout",
			"show",
			"hide",
			"hidemenu",
			"open",
			"close",
			"setasset",
			"setbackground",
			"setteamcolor",
			"setitemcolor",
			"setfocus",
			"setplayermodel",
			"setplayerhead",
			"transition",
			"setcvar",
			"exec",
			"play",
			"playlooped",
			"orbit",
			"uiscript",
			"scriptmenuresponse",
			"setlocalvarint",
			"setlocalvarstring",
			"setlocalvarfloat",
			"setlocalvarbool",
			"execnow",
			"execondvarintvalue",
			"execondvarstringvalue",
			"showmenu",
			"focusfirst",
			"setfocusbydvar",
			"setdvar",
			"execnowOndvarstringvalue",
		};
		const int script_action_length_ = 34;
		

		identifierMap itemTypeMap = 
		{
			{0, "ITEM_TYPE_TEXT"},
			{1, "ITEM_TYPE_BUTTON"},
			{2, "ITEM_TYPE_RADIOBUTTON"},
			{3, "ITEM_TYPE_CHECKBOX"},
			{4, "ITEM_TYPE_EDITFIELD"},
			{5, "ITEM_TYPE_COMBO"},
			{6, "ITEM_TYPE_LISTBOX"},
			{7, "ITEM_TYPE_MODEL"},
			{8, "ITEM_TYPE_OWNERDRAW"},
			{9, "ITEM_TYPE_NUMERICFIELD"},
			{10, "ITEM_TYPE_SLIDER"},
			{11, "ITEM_TYPE_YESNO"},
			{12, "ITEM_TYPE_MULTI"},
			{13, "ITEM_TYPE_DVARENUM"},
			{14, "ITEM_TYPE_BIND"},
			{15, "ITEM_TYPE_MENUMODEL"},
			{16, "ITEM_TYPE_VALIDFILEFIELD"},
			{17, "ITEM_TYPE_DECIMALFIELD"},
			{18, "ITEM_TYPE_UPREDITFIELD"},
			{19, "ITEM_TYPE_GAME_MESSAGE_WINDOW"},
		};

		identifierMap itemTextStyleMap = 
		{
			{0, "ITEM_TEXTSTYLE_NORMAL"},
			{1, "ITEM_TEXTSTYLE_BLINK"},
			{3, "ITEM_TEXTSTYLE_SHADOWED"},
			{5, "ITEM_TEXTSTYLE_SHADOWEDMORE"},
			{128, "ITEM_TEXTSTYLE_MONOSPACE"},
		};

		identifierMap itemTextAlignMap = 
		{
			{0, "ITEM_ALIGN_LEFT"},
			{1, "ITEM_ALIGN_CENTER"},
			{2, "ITEM_ALIGN_RIGHT"},
			{3, "ITEM_ALIGN_X_MASK"},
		};

		identifierMap listBoxTypeMap = 
		{
			{0, "LISTBOX_TEXT"},
			{1, "LISTBOX_IMAGE"}
		};

		identifierMap borderTypeMap = 
		{
			{0, "WINDOW_BORDER_NONE"},
			{1, "WINDOW_BORDER_FULL"},
			{2, "WINDOW_BORDER_HORZ"},
			{3, "WINDOW_BORDER_VERT"},
			{4, "WINDOW_BORDER_KCGRADIENT"},
			{5, "WINDOW_BORDER_RAISED"},
			{6, "WINDOW_BORDER_SUNKEN"},
		};

		identifierMap windowStyleMap = 
		{
			{0, "WINDOW_STYLE_EMPTY"},
			{1, "WINDOW_STYLE_FILLED"},
			{2, "WINDOW_STYLE_GRADIENT"},
			{3, "WINDOW_STYLE_SHADER"},
			{4, "WINDOW_STYLE_TEAMCOLOR"},
			{5, "WINDOW_STYLE_DVAR_SHADER"},
			{6, "WINDOW_STYLE_LOADBAR"},
		};

		identifierMap ownerDrawTypeMap = 
		{
			{1, "CG_OWNERDRAW_BASE"},
			{5, "CG_PLAYER_AMMO_VALUE"},
			{6, "CG_PLAYER_AMMO_BACKDROP"},
			{20, "CG_PLAYER_STANCE"},
			{60, "CG_SPECTATORS"},
			{71, "CG_HOLD_BREATH_HINT"},
			{72, "CG_CURSORHINT"},
			{73, "CG_PLAYER_POWERUP"},
			{74, "CG_PLAYER_HOLDABLE"},
			{75, "CG_PLAYER_INVENTORY"},
			{78, "CG_CURSORHINT_STATUS"},
			{79, "CG_PLAYER_BAR_HEALTH"},
			{80, "CG_MANTLE_HINT"},
			{81, "CG_PLAYER_WEAPON_NAME"},
			{82, "CG_PLAYER_WEAPON_NAME_BACK"},
			{90, "CG_CENTER_MESSAGE"},
			{95, "CG_TANK_BODY_DIR"},
			{96, "CG_TANK_BARREL_DIR"},
			{97, "CG_DEADQUOTE"},
			{98, "CG_PLAYER_BAR_HEALTH_BACK"},
			{99, "CG_MISSION_OBJECTIVE_HEADER"},
			{100, "CG_MISSION_OBJECTIVE_LIST"},
			{101, "CG_MISSION_OBJECTIVE_BACKDROP"},
			{102, "CG_PAUSED_MENU_LINE"},
			{103, "CG_OFFHAND_WEAPON_ICON_FRAG"},
			{104, "CG_OFFHAND_WEAPON_ICON_SMOKEFLASH"},
			{105, "CG_OFFHAND_WEAPON_AMMO_FRAG"},
			{106, "CG_OFFHAND_WEAPON_AMMO_SMOKEFLASH"},
			{107, "CG_OFFHAND_WEAPON_NAME_FRAG"},
			{108, "CG_OFFHAND_WEAPON_NAME_SMOKEFLASH"},
			{109, "CG_OFFHAND_WEAPON_SELECT_FRAG"},
			{110, "CG_OFFHAND_WEAPON_SELECT_SMOKEFLASH"},
			{111, "CG_SAVING"},
			{112, "CG_PLAYER_LOW_HEALTH_OVERLAY"},
			{113, "CG_INVALID_CMD_HINT"},
			{114, "CG_PLAYER_SPRINT_METER"},
			{115, "CG_PLAYER_SPRINT_BACK"},
			{116, "CG_PLAYER_WEAPON_BACKGROUND"},
			{117, "CG_PLAYER_WEAPON_AMMO_CLIP_GRAPHIC"},
			{118, "CG_PLAYER_WEAPON_PRIMARY_ICON"},
			{119, "CG_PLAYER_WEAPON_AMMO_STOCK"},
			{120, "CG_PLAYER_WEAPON_LOW_AMMO_WARNING"},
			{145, "CG_PLAYER_COMPASS_TICKERTAPE"},
			{146, "CG_PLAYER_COMPASS_TICKERTAPE_NO_OBJ"},
			{150, "CG_PLAYER_COMPASS_PLAYER"},
			{151, "CG_PLAYER_COMPASS_BACK"},
			{152, "CG_PLAYER_COMPASS_POINTERS"},
			{153, "CG_PLAYER_COMPASS_ACTORS"},
			{154, "CG_PLAYER_COMPASS_TANKS"},
			{155, "CG_PLAYER_COMPASS_HELICOPTERS"},
			{156, "CG_PLAYER_COMPASS_PLANES"},
			{157, "CG_PLAYER_COMPASS_AUTOMOBILES"},
			{158, "CG_PLAYER_COMPASS_FRIENDS"},
			{159, "CG_PLAYER_COMPASS_MAP"},
			{160, "CG_PLAYER_COMPASS_NORTHCOORD"},
			{161, "CG_PLAYER_COMPASS_EASTCOORD"},
			{162, "CG_PLAYER_COMPASS_NCOORD_SCROLL"},
			{163, "CG_PLAYER_COMPASS_ECOORD_SCROLL"},
			{164, "CG_PLAYER_COMPASS_GOALDISTANCE"},
			{165, "CG_PLAYER_ACTIONSLOT_DPAD"},
			{166, "CG_PLAYER_ACTIONSLOT_1"},
			{167, "CG_PLAYER_ACTIONSLOT_2"},
			{168, "CG_PLAYER_ACTIONSLOT_3"},
			{169, "CG_PLAYER_ACTIONSLOT_4"},
			{170, "CG_PLAYER_COMPASS_ENEMIES"},
			{180, "CG_PLAYER_FULLMAP_BACK"},
			{181, "CG_PLAYER_FULLMAP_MAP"},
			{182, "CG_PLAYER_FULLMAP_POINTERS"},
			{183, "CG_PLAYER_FULLMAP_PLAYER"},
			{184, "CG_PLAYER_FULLMAP_ACTORS"},
			{185, "CG_PLAYER_FULLMAP_FRIENDS"},
			{186, "CG_PLAYER_FULLMAP_LOCATION_SELECTOR"},
			{187, "CG_PLAYER_FULLMAP_BORDER"},
			{188, "CG_PLAYER_FULLMAP_ENEMIES"},
			{190, "CG_VEHICLE_RETICLE"},
			{191, "CG_HUD_TARGETS_VEHICLE"},
			{192, "CG_HUD_TARGETS_JAVELIN"},
			{193, "CG_TALKER1"},
			{194, "CG_TALKER2"},
			{195, "CG_TALKER3"},
			{196, "CG_TALKER4"},
			{200, "UI_OWNERDRAW_BASE"},
			//200 is also UI_HANDICAP

			{201, "UI_EFFECTS"},
			{202, "UI_PLAYERMODEL"},
			{205, "UI_GAMETYPE"},
			{207, "UI_SKILL"},
			{220, "UI_NETSOURCE"},
			{222, "UI_NETFILTER"},
			{238, "UI_VOTE_KICK"},
			{245, "UI_NETGAMETYPE"},
			{247, "UI_SERVERREFRESHDATE"},
			{248, "UI_SERVERMOTD"},
			{249, "UI_GLINFO"},
			{250, "UI_KEYBINDSTATUS"},
			{253, "UI_JOINGAMETYPE "},
			{254, "UI_MAPPREVIEW"},
			{257, "UI_MENUMODEL"},
			{258, "UI_SAVEGAME_SHOT"},
			{262, "UI_SAVEGAMENAME"},
			{263, "UI_SAVEGAMEINFO"},
			{264, "UI_LOADPROFILING"},
			{265, "UI_RECORDLEVEL"},
			{266, "UI_AMITALKING"},
			{267, "UI_TALKER1"},
			{268, "UI_TALKER2"},
			{269, "UI_TALKER3"},
			{270, "UI_TALKER4"},
			{271, "UI_PARTYSTATUS"},
			{272, "UI_LOGGEDINUSER"},
			{273, "UI_RESERVEDSLOTS"},
			{274, "UI_PLAYLISTNAME"},
			{275, "UI_PLAYLISTDESCRIPTION"},
			{276, "UI_USERNAME"},
			{277, "UI_CINEMATIC"},
		};

		identifierMap messageMap = 
		{
			{0, "MODE_BOTTOMUP_ALIGN_TOP"},
			{1, "MODE_BOTTOMUP_ALIGN_BOTTOM"},
			{2, "MODE_TOPDOWN_ALIGN_TOP"},
			{3, "MODE_TOPDOWN_ALIGN_BOTTOM"}
		};

		identifierMap feederMap = 
		{
			{0, "FEEDER_HEADS"},
			{1, "FEEDER_MAPS"},
			{2, "FEEDER_SERVERS"},
			{3, "FEEDER_CLAN_MEMBERS"},
			{4, "FEEDER_ALLMAPS"},
			{5, "FEEDER_REDTEAM_LIST"},
			{6, "FEEDER_BLUETEAM_LIST"},
			{7, "FEEDER_PLAYER_LIST"},
			{8, "FEEDER_TEAM_LIST"},
			{9, "FEEDER_MODS"},
			{10, "FEEDER_DEMOS"},
			{11, "FEEDER_SCOREBOARD"},
			{12, "FEEDER_Q3HEADS"},
			{13, "FEEDER_SERVERSTATUS"},
			{14, "FEEDER_FINDPLAYER"},
			{15, "FEEDER_CINEMATICS"},
			{16, "FEEDER_SAVEGAMES"},
			{17, "FEEDER_PICKSPAWN"},
			{18, "FEEDER_LOBBY_MEMBERS"},
			{19, "FEEDER_LOBBY_MEMBERS_TALK"},
			{20, "FEEDER_MUTELIST"},
			{21, "FEEDER_PLAYERSTALKING"},
			{22, "FEEDER_SPLITSCREENPLAYERS"},
			{23, "FEEDER_LOBBY_MEMBERS_READY"},
			{24, "FEEDER_PLAYER_PROFILES"},
			{25, "FEEDER_PARTY_MEMBERS"},
			{26, "FEEDER_PARTY_MEMBERS_TALK"},
			{27, "FEEDER_PARTY_MEMBERS_READY"},
			{28, "FEEDER_PLAYLISTS"},
			{29, "FEEDER_GAMEMODES"},
			{30, "FEEDER_LEADERBOARD"},
			{32, "FEEDER_MYTEAM_MEMBERS"},
			{33, "FEEDER_MYTEAM_MEMBERS_TALK"},
			{34, "FEEDER_ENEMY_MEMBERS"},
			{35, "FEEDER_ENEMY_MEMBERS_TALK"},
			{36, "FEEDER_LOBBY_MEMBERS_STAT"},
			{37, "FEEDER_MYTEAM_MEMBERS_STAT"},
			{38, "FEEDER_ENEMY_MEMBERS_STAT"},
			{39, "FEEDER_ONLINEFRIENDS"},
			{40, "FEEDER_LOBBY_MEMBERS_RANK"},
			{41, "FEEDER_PARTY_MEMBERS_RANK"},
			{48, "FEEDER_ENEMY_MEMBERS_RANK"},
			{49, "FEEDER_MYTEAM_MEMBERS_RANK"},
		};


		identifierMap fontTypeMap = 
		{
			{0, "UI_FONT_DEFAULT"},
			{1, "UI_FONT_NORMAL"},
			{2, "UI_FONT_BIG"},
			{3, "UI_FONT_SMALL"},
			{4, "UI_FONT_BOLD"},
			{5, "UI_FONT_CONSOLE"},
			{6, "UI_FONT_OBJECTIVE"},
		};

		identifierMap horizontalAlignmentMap =
		{
			{0, "HORIZONTAL_ALIGN_SUBLEFT"},
			{1, "HORIZONTAL_ALIGN_LEFT"},
			{2, "HORIZONTAL_ALIGN_CENTER"},
			{3, "HORIZONTAL_ALIGN_RIGHT"},
			{4, "HORIZONTAL_ALIGN_FULLSCREEN"},
			{5, "HORIZONTAL_ALIGN_NOSCALE"},
			{6, "HORIZONTAL_ALIGN_TO640"},
			{7, "HORIZONTAL_ALIGN_CENTER_SAFEAREA"},
		};


		identifierMap verticalAlignmentMap = 
		{
			{0, "VERTICAL_ALIGN_SUBTOP"},
			{1, "VERTICAL_ALIGN_TOP"},
			{2, "VERTICAL_ALIGN_CENTER"},
			{3, "VERTICAL_ALIGN_RIGHT"},
			{4, "VERTICAL_ALIGN_FULLSCREEN"},
			{5, "VERTICAL_ALIGN_NOSCALE"},
			{6, "VERTICAL_ALIGN_TO640"},
			{7, "VERTICAL_ALIGN_CENTER_SAFEAREA"},
		};

		//Found at https://github.com/callofduty4x/CoD4x_Server/blob/98ee29d49960ba248977b5e413dd93e6c0670fdd/src/xassets/menu.h#L129
		identifierMap staticFlagsMap = 
		{
			{1048576, "decoration"},
			{1048576, "horizontalscroll"},
			{1048576, "autowrapped"},
			{1048576, "popup"},
			{1048576, "outOfBoundsClick"},
			{1048576, "legacySplitScreenScale"},
			{1048576, "hiddenDuringFlashbang"},
			{1048576, "hiddenDuringScope"},
			{1048576, "hiddenDuringGUI"},
		};

		//Found at https://github.com/callofduty4x/CoD4x_Server/blob/98ee29d49960ba248977b5e413dd93e6c0670fdd/src/xassets/menu.c#L7
		//thank you T-max or whoever made this list :)
		identifierMap operandMap = 
		{
			{0, "NOOP"},
			{1, ")"},
			{2, "*"},
			{3, "/"},
			{4, "%"},
			{5, "+"},
			{6, "-"},
			{7, "!"},
			{8, "<"},
			{9, "<="},
			{10, ">"},
			{11, ">="},
			{12, "=="},
			{13, "!="},
			{14, "&&"},
			{15, "||"},
			{16, "("},
			{17, ","},
			{18, "&"},
			{19, "|"},
			{20, "~"},
			{21, "<<"},
			{22, ">>"},
			{23, "sin("},
			{24, "cos("},
			{25, "min("},
			{26, "max("},
			{27, "milliseconds("},
			{28, "dvarint("},
			{29, "dvarbool("},
			{30, "dvarfloat("},
			{31, "dvarstring("},
			{32, "stat("},
			{33, "ui_active("},
			{34, "flashbanged("},
			{35, "scoped("},
			{36, "scoreboard_visible("},
			{37, "inkillcam("},
			{38, "player("},
			{39, "selecting_location("},
			{40, "team("},
			{41, "otherteam("},
			{42, "marinesfield("},
			{43, "opforfield("},
			{44, "menuisopen("},
			{45, "writingdata("},
			{46, "inlobby("},
			{47, "inprivateparty("},
			{48, "privatepartyhost("},
			{49, "privatepartyhostinlobby("},
			{50, "aloneinparty("},
			{51, "adsjavelin("},
			{52, "weaplockblink("},
			{53, "weapattacktop("},
			{54, "weapattackdirect("},
			{55, "secondsastime("},
			{56, "tablelookup("},
			{57, "locstring("},
			{58, "localvarint("},
			{59, "localvarbool("},
			{60, "localvarfloat("},
			{61, "localvarstring("},
			{62, "timeleft("},
			{63, "secondsascountdown("},
			{64, "gamemsgwndactive("},
			{65, "int("},
			{66, "string("},
			{67, "float("},
			{68, "gametypename("},
			{69, "gametype("},
			{70, "gametypedescription("},
			{71, "scoreatrank("},
			{72, "friendsonline("},
			{73, "spectatingclient("},
			{74, "statrangeanybitsset("},
			{75, "keybinding("},
			{76, "actionslotusable("},
			{77, "hudfade("},
			{78, "maxrecommendedplayers("},
			{79, "acceptinginvite("},
			{80, "isintermission("},
		};
	};
}