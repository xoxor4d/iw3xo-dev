#include "STDInclude.hpp"
#include "MenuExport.hpp"


namespace Components
{

	MenuExport::MenuExport() {


		maxPadding = 16;
		tabs = 0;
		padding = 0;
		tabbing = MENU_TABBING;

		Command::Add("menuList", [this](Command::Params) {
			for (int i = 0; i < Game::_uiContext->menuCount; i++) {
				Game::Com_PrintMessage(0, Utils::VA("Menu: %s \n", Game::_uiContext->Menus[i]->window.name), 0);
			}
		});

		Command::Add("exportMenu", [this](Command::Params params) {
			Game::menuDef_t *menu;
			if (params.Length() < 2) {
				return;
			}
			const char * name = params[1];
			for (int i = 0; i < Game::_uiContext->menuCount; i++) {
				if (strcmp(Game::_uiContext->Menus[i]->window.name, name) == 0) {//might be wrong
					exportMenu(Game::_uiContext->Menus[i]);
					break;
				}
			}
		});

		

	}

	MenuExport::~MenuExport() {

	}

	void MenuExport::exportMenu(Game::menuDef_t *menu) {
		using namespace std;
		
		string basePath = Game::Dvar_FindVar("fs_basepath")->current.string;
		basePath += "\\menu_export\\";

		string filePath = basePath + menu->window.name + ".menu";

		if (!CreateDirectoryA(basePath.c_str(), NULL))
		{
			//Game::Com_PrintMessage(0, "|- ^1Failed to create directory \"root/^3menu_export\"\n", 0);
			//return;
		}

		menuFile.open(filePath.c_str());

		//header
		
		menuFile << "//This menu was generated using iw3xo Client\n";
		menuFile << "//If you find any bug please report them here: https://github.com/xoxor4d/iw3xo-dev/issues\n\n";
		menuFile << "#include \"ui/menudefinition.h\"\n\n";

		//open menu
		menuFile << "{\n";

		//open menudef
		menuFile << tabbing << "menuDef" << "\n" << tabbing << "{\n";
		tabbing = MENUDEF_TABBING;

		//name
		writeText("name", menu->window.name);

		//rectangle
		writeRect(menu->window.rect);

		//fullscreen
		writeNumber("fullScreen", menu->fullScreen);

		//blurworld
		if(menu->blurRadius != 0)
		{ 
			writeNumber("blurWorld", menu->blurRadius);
		}
		
		if (menu->window.border != 0 && menu->window.borderSize != 0) {

			//border
			writeNumber("border", menu->window.border, borderTypeMap);

			//bordersize
			writeNumber("borderSize", menu->window.borderSize);

			//bordercolor
			writeVec4("borderColor", menu->window.borderColor);
			
		}

		//forecolor
		writeVec4("foreColor", menu->window.foreColor);
	
		//backcolor
		writeVec4("backColor", menu->window.backColor);

		//outlinecolor
		writeVec4("outlineColor", menu->window.outlineColor);

		//disablecolor
		writeVec4("disableColor", menu->disableColor);

		//focuscolor
		writeVec4("focusColor", menu->focusColor);

		if (menu->window.background) {
			//background
			writeText("background", menu->window.background->info.name);
		}

		if (menu->window.staticFlags) {
			writeFlag(menu->window.staticFlags);
		}

		//onopen
		
		if (menu->onOpen) {
			writeAction("onOpen", menu->onOpen);
		}

		if (menu->onClose) {
			writeAction("onClose", menu->onClose);
		}

		if (menu->onESC) {
			writeAction("onESC", menu->onESC);
		}

		while (menu->onKey) {
			writeKeyAction(menu->onKey->key, menu->onKey->action);
			menu->onKey = menu->onKey->next;
		}

		for (int i = 0; i < menu->itemCount; i++) {
			writeItemDef(*menu->items[i]);
		}
		


		tabbing = MENU_TABBING;
		//close menudef
		menuFile << tabbing << "}\n";

		//close menu
		menuFile << "}\n";

		menuFile.close();


	}

	void MenuExport::writeItemDef(Game::itemDef_s &itemDef) {
		using namespace std;

		//itemdef open
		menuFile << "\n" << tabbing << "itemDef\n";
		menuFile << tabbing << "{\n";
		tabbing = ITEMDEF_TABBING;

		//name
		if (itemDef.window.name) {
			writeText("name", itemDef.window.name);
		}

		//rect
		writeRect(itemDef.window.rectClient);
		
		writeNumber("style", itemDef.window.style, windowStyleMap);

		if (itemDef.window.border != 0 && itemDef.window.borderSize != 0) {

			//border
			writeNumber("border", itemDef.window.border, borderTypeMap);

			//bordersize
			writeNumber("borderSize", itemDef.window.borderSize);

			//bordercolor
			writeVec4("borderColor", itemDef.window.borderColor);

		}

		//forecolor
		writeVec4("foreColor", itemDef.window.foreColor);

		//backcolor
		writeVec4("backColor", itemDef.window.backColor);

		if (itemDef.window.outlineColor[3] != 0) {
			writeVec4("outlineColor", itemDef.window.outlineColor);
		}


		if (itemDef.window.background) {
			//background
			writeText("background", itemDef.window.background->info.name);
		}

		writeNumber("type", itemDef.type, itemTypeMap);

		//datatype

		//alignment

		//fontenum

		if (itemDef.textAlignMode != 0) {
			writeNumber("textAlign", itemDef.textAlignMode, itemTextAlignMap);
		}

		if (itemDef.textaligny != 0) {
			writeNumber("textAlignX", itemDef.textalignx);
		}

		if (itemDef.textalignx != 0) {
			writeNumber("textAlignY", itemDef.textaligny);
		}
		
		if (itemDef.textscale != defaultTextSize) {
			writeNumber("textScale", itemDef.textscale);
		}
		
		if (itemDef.textStyle != 0) {
			writeNumber("textStyle", itemDef.textStyle, itemTextStyleMap);
		}

		if (itemDef.text) {
			writeText("text", itemDef.text);
		}

		//listbox
		if (itemDef.type == 6) {
			writeNumber("elementHeight", itemDef.typeData.listBox->elementHeight);
			writeNumber("elementWidth", itemDef.typeData.listBox->elementWidth);
			writeNumber("elementType", itemDef.typeData.listBox->elementStyle, listBoxTypeMap);
			writeAction("doubleclick", itemDef.typeData.listBox->doubleClick);
			writeVec4("selectBoder", itemDef.typeData.listBox->selectBorder);
			writeVec4("disableColor", itemDef.typeData.listBox->disableColor);
			if (itemDef.typeData.listBox->notselectable) {
				writeText("notselect", "");
			}
			if (itemDef.typeData.listBox->noScrollBars) {
				writeText("noScrollBars", "");
			}
			if (itemDef.typeData.listBox->usePaging) {
				writeText("usePaging", "");
			}

			padding = PADDING(5);
			menuFile << tabbing << "columns" << setw(padding) << itemDef.typeData.listBox->numColumns << " ";
			for (int i = 0; i < itemDef.typeData.listBox->numColumns; i++) {
				menuFile << tabbing << tabbing << itemDef.typeData.listBox->columnInfo->pos
					<< " " << itemDef.typeData.listBox->columnInfo->width
					<< " " << itemDef.typeData.listBox->columnInfo->maxChars << "\n";
			}
			//feeder
		}

		if (itemDef.mouseEnterText) {
			writeAction("mouseEnterText", itemDef.mouseEnterText);
		}

		if (itemDef.mouseExitText) {
			writeAction("mouseExitText", itemDef.mouseExitText);
		}

		if (itemDef.mouseEnter) {
			writeAction("mouseEnter", itemDef.mouseEnter);
		}

		if (itemDef.mouseExit) {
			writeAction("mouseExit", itemDef.mouseExit);
		}
		
		if (itemDef.action) {
			writeAction("action", itemDef.action);
		}

		if (itemDef.onAccept) {
			writeAction("onAccept", itemDef.onAccept);
		}

		if (itemDef.onFocus) {
			writeAction("onFocus", itemDef.onFocus);
		}

		if (itemDef.leaveFocus) {
			writeAction("leaveFocus", itemDef.mouseExitText);
		}

		while (itemDef.onKey) {
			writeKeyAction(itemDef.onKey->key, itemDef.onKey->action);
			itemDef.onKey = itemDef.onKey->next;
		}

		

		

		//group

		//dvar

		//dvartest

		//enable dvar

		//dvarflags

		//focussound

		//special

		//typedata

		//imagetrack

		//visible exp

		//text exp

		//material exp

		//rectXexp

		//rectYexp

		//recWXexp

		//rectHexp

		//forecolor exp

		//static flags
		if (itemDef.window.staticFlags) {
			writeFlag(itemDef.window.staticFlags);
		}



		tabbing = MENUDEF_TABBING;
		//close itemDef
		menuFile << tabbing << "}\n";

	}

	void MenuExport::writeRect(Game::rectDef_s rect) {
		using namespace std;
		string hAlign = to_string(rect.horzAlign);
		find = horizontalAlignmentMap.find(rect.horzAlign);

		if (find != horizontalAlignmentMap.end()) {
			hAlign = find->second;
		}

		string vAlign = to_string(rect.vertAlign);
		find = verticalAlignmentMap.find(rect.vertAlign);

		if (find != verticalAlignmentMap.end()) {
			vAlign = find->second;
		}

		padding = PADDING(4);
		menuFile << tabbing << "rect" << setw(padding)
			<< rect.x << " "
			<< rect.y << " "
			<< rect.w << " "
			<< rect.h << " "
			<< hAlign << " "
			<< vAlign << "\n";
		
	}

	void MenuExport::writeText(std::string name, const char* text) {	
			padding = PADDING(name.length());
			menuFile << tabbing << name << std::setw(padding) << "\"" << text << "\"\n";
	}


	template <typename T>
	void MenuExport::writeNumber(std::string name, T num) {
		padding = PADDING(name.length());
		menuFile << tabbing << name << std::setw(padding) << num << "\n";
	}

	template <typename T>
	void MenuExport::writeNumber(std::string name, T num, identifierMap &map) {
		using namespace std;

		string number = to_string(num);
		find = map.find(num);

		if (find != map.end()) {
			number = find->second;
		}


		padding = PADDING(name.length());
		menuFile << tabbing << name << std::setw(padding) << number << "\n";
	}

	void MenuExport::writeFlag(int num) {
		find = staticFlagsMap.find(num);

		if (find != staticFlagsMap.end()) {
			menuFile << tabbing << find->second << "\n";
		}
	}

	//might need to check for string in scriptactions :pepethink:
	void MenuExport::writeAction(std::string name, std::string action) {
		using namespace std;
		
		bool stringOpen = false;
		string newText = "";

		for (int i = 0; i < action.length(); i++) {
			if (action.at(i) == '"'){
				continue;
			}
			newText += action.at(i);
		}

		menuFile << tabbing << name << "{ " << newText << " }" << "\n";
	}

	void MenuExport::writeKeyAction(int key, std::string action) {
		std::string name = "execKey \"" + std::to_string(key) + "\" ";
		writeAction(name, action);
	}

	void MenuExport::writeVec4(std::string name, float vec4[]) {

		padding = PADDING(name.length());
		menuFile << tabbing << name << std::setw(padding)
			<< vec4[0] << " "
			<< vec4[1] << " "
			<< vec4[2] << " "
			<< vec4[3] << "\n";
	}

	std::string MenuExport::statementToText(Game::statement_s stmt) {
		Game::Com_PrintMessage(0, Utils::VA("%d", stmt.numEntries), 0);
		return "";// (**stmt.entries).type 
	}

}