#pragma once
#include <fstream>
#include <iomanip>

#define PADDING(x) maxPadding - x
#define MENU_TABBING "\t"
#define MENUDEF_TABBING "\t\t"
#define ITEMDEF_TABBING "\t\t\t"


namespace Components
{
	
	class MenuExport : public Component
	{
	public:
		MenuExport();
		~MenuExport();
		const char* getName() override { return "Menu Exporter"; };

	private:
		typedef std::unordered_map<unsigned int, std::string> identifierMap;
		typedef std::unordered_map<unsigned int, std::string>::const_iterator identifierMapIt;

		void exportMenu(Game::menuDef_t *menu);
		std::string statementToText(Game::statement_s stmt);
		void writeVec4(std::string name, float vec4[]);
		void writeItemDef(Game::itemDef_s &itemDef);
		void writeRect(Game::rectDef_s rect);
		template <typename T>
		void writeNumber(std::string name, T num);
		template <typename T>
		void writeNumber(std::string name, T num, identifierMap &map);
		void writeText(std::string name, const char* text);
		void writeFlag(int num);
		void writeAction(std::string name, std::string action);
		void writeKeyAction(int key, std::string action);


		int padding;
		int tabs;
		const float defaultTextSize = 0.55f;
		std::string tabbing;
		int maxPadding;
		std::ofstream menuFile;		
		identifierMapIt find;
		

		identifierMap itemTypeMap = {
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

		identifierMap itemTextStyleMap = {
			{0, "ITEM_TEXTSTYLE_NORMAL"},
			{1, "ITEM_TEXTSTYLE_BLINK"},
			{3, "ITEM_TEXTSTYLE_SHADOWED"},
			{5, "ITEM_TEXTSTYLE_SHADOWEDMORE"},
			{128, "ITEM_TEXTSTYLE_MONOSPACE"},
		};

		identifierMap itemTextAlignMap = {
			{0, "ITEM_ALIGN_LEFT"},
			{1, "ITEM_ALIGN_CENTER"},
			{2, "ITEM_ALIGN_RIGHT"},
			{3, "ITEM_ALIGN_X_MASK"},
		};

		identifierMap listBoxTypeMap = {
			{0, "LISTBOX_TEXT"},
			{1, "LISTBOX_IMAGE"}
		};

		identifierMap borderTypeMap = {
			{0, "WINDOW_BORDER_NONE"},
			{1, "WINDOW_BORDER_FULL"},
			{2, "WINDOW_BORDER_HORZ"},
			{3, "WINDOW_BORDER_VERT"},
			{4, "WINDOW_BORDER_KCGRADIENT"},
			{5, "WINDOW_BORDER_RAISED"},
			{6, "WINDOW_BORDER_SUNKEN"},
		};

		identifierMap windowStyleMap = {
			{0, "WINDOW_STYLE_EMPTY"},
			{1, "WINDOW_STYLE_FILLED"},
			{2, "WINDOW_STYLE_GRADIENT"},
			{3, "WINDOW_STYLE_SHADER"},
			{4, "WINDOW_STYLE_TEAMCOLOR"},
			{5, "WINDOW_STYLE_DVAR_SHADER"},
			{6, "WINDOW_STYLE_LOADBAR"},
		};


		identifierMap feederMap = {
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


		identifierMap fontTypeMap = {
			{0, "UI_FONT_DEFAULT"},
			{1, "UI_FONT_NORMAL"},
			{2, "UI_FONT_BIG"},
			{3, "UI_FONT_SMALL"},
			{4, "UI_FONT_BOLD"},
			{5, "UI_FONT_CONSOLE"},
			{6, "UI_FONT_OBJECTIVE"},
		};

		identifierMap horizontalAlignmentMap = {
			{0, "HORIZONTAL_ALIGN_SUBLEFT"},
			{1, "HORIZONTAL_ALIGN_LEFT"},
			{2, "HORIZONTAL_ALIGN_CENTER"},
			{3, "HORIZONTAL_ALIGN_RIGHT"},
			{4, "HORIZONTAL_ALIGN_FULLSCREEN"},
			{5, "HORIZONTAL_ALIGN_NOSCALE"},
			{6, "HORIZONTAL_ALIGN_TO640"},
			{7, "HORIZONTAL_ALIGN_CENTER_SAFEAREA"},
		};


		identifierMap verticalAlignmentMap = {
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
		identifierMap staticFlagsMap = {
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

	};
}