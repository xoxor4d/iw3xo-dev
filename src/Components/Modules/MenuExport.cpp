#include "STDInclude.hpp"
#include "MenuExport.hpp"


namespace Components
{

	MenuExport::MenuExport() {


		maxPadding = 20;
		tabs = 0;
		padding = 0;
		tabbing = MENU_TABBING;

		Command::Add("menuList", [this](Command::Params) {
			for (int i = 0; i < Game::_uiContext->menuCount; i++) {
				Game::Com_PrintMessage(0, Utils::VA("Menu: %s \n", Game::_uiContext->Menus[i]->window.name), 0);
			}
		});

		Command::Add("exportMenu", [this](Command::Params params) {

			if (params.Length() < 2) {
				Game::Com_PrintMessage(0, "^1No menu name given.^7\n", 0);
				return;
			}
			const char * name = params[1];
			bool found = false;
			for (int i = 0; i < Game::_uiContext->menuCount; i++) {
				if (strcmp(Game::_uiContext->Menus[i]->window.name, name) == 0) {
					exportMenu(Game::_uiContext->Menus[i]);
					found = true;
					break;
				}
			}
			if (!found) {
				Game::Com_PrintMessage(0, Utils::VA("^1WARNING: ^7Menu %s was not found.\n", name), 0);
			}
		});

		Command::Add("exportItemdefs", [this](Command::Params params) {
			if (params.Length() < 2) {
				Game::Com_PrintMessage(0, "^1No menu name given.^7\n", 0);
				return;
			}
			const char * name = params[1];
			bool found = false;
			for (int i = 0; i < Game::_uiContext->menuCount; i++) {
				if (strcmp(Game::_uiContext->Menus[i]->window.name, name) == 0) {
					exportMenuItemdefs(Game::_uiContext->Menus[i]);
					found = true;
					break;
				}
			}
			if (!found) {
				Game::Com_PrintMessage(0, Utils::VA("^1WARNING: ^7Menu %s was not found.\n", name), 0);
			}
		});
	}

	MenuExport::~MenuExport() {

	}

	//Export menudef and all its itemdef to specified folder and file name
	void MenuExport::exportMenu(Game::menuDef_t *menu, std::string path, std::string name) {

		setPath(path, name);

		menuFile.open(writeFilePath.c_str());

		Game::Com_PrintMessage(0, "Exporting menu.\n", 0);

		//header

		menuFile << headerText;


		//open menu
		menuFile << "{\n";

		writeMenuDef(menu);

		//close menu
		menuFile << "}\n";

		menuFile.close();

		Game::Com_PrintMessage(0, Utils::VA("Menu exported to: %s\n", writeFilePath.c_str()), 0);
	}

	//Export menudef and all its itemdefs to menu_export\menus folder in basepath
	void MenuExport::exportMenu(Game::menuDef_t *menu) {
		using namespace std;

		string basePath = Game::Dvar_FindVar("fs_basepath")->current.string;
		basePath += "\\menu_export\\menus\\";

		string fileName = menu->window.name;
		fileName += ".menu";
		
		exportMenu(menu, basePath, fileName);
	}

	//Export all itemdef from a menudef to a specified folder and file name
	void MenuExport::exportMenuItemdefs(Game::menuDef_t *menu, std::string path, std::string name) {
		setPath(path, name);

		Game::Com_PrintMessage(0, Utils::VA("Exporting itemDefs from %s.\n", menu->window.name), 0);

		menuFile.open(writeFilePath.c_str());

		//header
		menuFile << headerText;

		for (int i = 0; i < menu->itemCount; i++) {
			writeItemDef(*menu->items[i]);
		}

		menuFile.close();

		Game::Com_PrintMessage(0, Utils::VA("ItemDefs exported to: %s\n", writeFilePath.c_str()), 0);
	}

	//Export all itemdefs from a menudef to menu_export\itemdefs in basepath
	void MenuExport::exportMenuItemdefs(Game::menuDef_t *menu) {
		using namespace std;
		string basePath = Game::Dvar_FindVar("fs_basepath")->current.string;
		basePath += "\\menu_export\\itemdefs\\";

		string fileName = menu->window.name;
		fileName += "_itemdefs.menu";

		exportMenuItemdefs(menu, basePath, fileName);
	}

	//Export a single itemDef to a specified folder and file name
	void MenuExport::exportItemDef(Game::itemDef_s itemDef, std::string path, std::string name) {

		setPath(path, name);

		Game::Com_PrintMessage(0, "Exporting itemDef.", 0);

		menuFile.open(writeFilePath.c_str());

		//header
		menuFile << headerText;

		writeItemDef(itemDef);

		menuFile.close();

		Game::Com_PrintMessage(0, Utils::VA("ItemDef exported to: %s\n", writeFilePath.c_str()), 0);
	}

	void MenuExport::setPath(std::string path, std::string name) {
		std::string filePath = path + "\\" + name;
		writeFilePath = filePath;

		CreateDirectoryA(path.c_str(), NULL);

		return;
	}

	void MenuExport::writeMenuDef(Game::menuDef_t *menuDef)
	{
		tabbing = MENU_TABBING;
		//open menudef
		menuFile << tabbing << "menuDef" << "\n" << tabbing << "{\n";
		tabbing = MENUDEF_TABBING;

		WRITE_TEXT("name", menuDef->window.name);

		writeRect(menuDef->window.rect);

		writeVisible(menuDef->visibleExp);

		writeNumber("fullScreen", menuDef->fullScreen);


		WRITE_NUMBER("blurWorld", 0, menuDef->blurRadius);

		WRITE_NUMBER("fadeCycle", 0, menuDef->fadeCycle);

		WRITE_NUMBER("fadeClamp", 0, menuDef->fadeClamp);

		WRITE_NUMBER("fadeAmount", 0, menuDef->fadeAmount);

		WRITE_NUMBER("fadeInAmount", 0, menuDef->fadeInAmount);
		
		if (menuDef->window.border != 0 && menuDef->window.borderSize != 0) {

			writeNumber("border", menuDef->window.border, borderTypeMap);

			writeNumber("borderSize", menuDef->window.borderSize);

			writeVec4("borderColor", menuDef->window.borderColor);
			
		}

		writeVec4("foreColor", menuDef->window.foreColor);
	
		writeVec4("backColor", menuDef->window.backColor);

		writeVec4("outlineColor", menuDef->window.outlineColor);

		writeVec4("disableColor", menuDef->disableColor);

		writeVec4("focusColor", menuDef->focusColor);


		if (menuDef->window.ownerDraw) {
			writeNumber("ownerDraw", menuDef->window.ownerDraw, ownerDrawTypeMap);

			writeNumber("ownerDrawFlag", menuDef->window.ownerDrawFlags);
		}

		WRITE_TEXT("allowBinding", menuDef->allowedBinding);

		WRITE_TEXT("soundLoop", menuDef->soundName);

		if (menuDef->window.background) {
			writeText("background", menuDef->window.background->info.name);
		}

		if (menuDef->window.staticFlags) {
			writeFlag(menuDef->window.staticFlags);
		}

		

		WRITE_ACTION("onOpen", menuDef->onOpen);

		WRITE_ACTION("onClose", menuDef->onClose);

		WRITE_ACTION("onESC", menuDef->onESC);


		while (menuDef->onKey) {
			writeKeyAction(menuDef->onKey->key, menuDef->onKey->action);
			menuDef->onKey = menuDef->onKey->next;
		}

		writeStatement("rect X", menuDef->rectXExp);

		writeStatement("rect Y", menuDef->rectYExp);

		for (int i = 0; i < menuDef->itemCount; i++) {
			writeItemDef(*menuDef->items[i]);
		}
		


		tabbing = MENU_TABBING;
		//close menudef
		menuFile << tabbing << "}\n";



	}

	void MenuExport::writeItemDef(Game::itemDef_s &itemDef) {
		using namespace std;

		//itemdef open
		menuFile << "\n" << tabbing << "itemDef\n";
		menuFile << tabbing << "{\n";
		tabbing = ITEMDEF_TABBING;


		WRITE_TEXT("name", itemDef.window.name);

		writeRect(itemDef.window.rectClient);

		writeVisible(itemDef.visibleExp);

		if (itemDef.window.ownerDraw) {
			writeNumber("ownerDraw", itemDef.window.ownerDraw, ownerDrawTypeMap);

			writeNumber("ownerDrawFlag", itemDef.window.ownerDrawFlags);
		}
		
		writeNumber("style", itemDef.window.style, windowStyleMap);

		if (itemDef.window.border != 0 && itemDef.window.borderSize != 0) {

			writeNumber("border", itemDef.window.border, borderTypeMap);

			writeNumber("borderSize", itemDef.window.borderSize);

			writeVec4("borderColor", itemDef.window.borderColor);

		}

		writeVec4("foreColor", itemDef.window.foreColor);

		writeVec4("backColor", itemDef.window.backColor);

		if (itemDef.window.outlineColor[3] != 0) {
			writeVec4("outlineColor", itemDef.window.outlineColor);
		}

		WRITE_TEXT("group", itemDef.window.group);

		if (itemDef.window.background) {
			writeText("background", itemDef.window.background->info.name);
		}

		writeNumber("type", itemDef.type, itemTypeMap);

		WRITE_NUMBER_MAP("textfont", 0, itemDef.fontEnum, fontTypeMap);

		WRITE_NUMBER_MAP("textAlign", 0, itemDef.textAlignMode, itemTextAlignMap);

		WRITE_NUMBER("textAlignX", 0, itemDef.textalignx);

		WRITE_NUMBER("textAlignY", 0, itemDef.textaligny);
		
		WRITE_NUMBER("textScale", defaultTextSize, itemDef.textscale);
		
		WRITE_NUMBER_MAP("textStyle", 0, itemDef.textStyle, itemTextStyleMap)


		WRITE_TEXT("text", itemDef.text);

		if (itemDef.type == ITEM_TYPE_LISTBOX) {
			writeNumber("feeder", itemDef.feeder, feederMap);
			writeNumber("elementWidth", itemDef.typeData.listBox->elementWidth);
			writeNumber("elementHeight", itemDef.typeData.listBox->elementHeight);		
			writeNumber("elementType", itemDef.typeData.listBox->elementStyle, listBoxTypeMap);
			WRITE_ACTION("doubleclick", itemDef.typeData.listBox->doubleClick);
			writeVec4("selectBorder", itemDef.typeData.listBox->selectBorder);
			writeVec4("disableColor", itemDef.typeData.listBox->disableColor);


			if (itemDef.typeData.listBox->selectIcon) {
				writeText("selectIcon", itemDef.typeData.listBox->selectIcon->info.name);
			}
			if (itemDef.typeData.listBox->notselectable) {
				writeLine("notselectable");
			}
			if (itemDef.typeData.listBox->noScrollBars) {
				writeLine("noScrollBars");
			}
			if (itemDef.typeData.listBox->usePaging) {
				writeLine("usePaging");
			}

			menuFile << tabbing << "columns" << PADDING(7) << itemDef.typeData.listBox->numColumns << "\n";
			for (int i = 0; i < itemDef.typeData.listBox->numColumns; i++) {
				menuFile << tabbing << "\t\t\t\t" << setw(padding) << itemDef.typeData.listBox->columnInfo[i].pos
					<< " " << itemDef.typeData.listBox->columnInfo[i].width
					<< " " << itemDef.typeData.listBox->columnInfo[i].maxChars << "\n";
			}
		}
		else if (itemDef.type == ITEM_TYPE_EDITFIELD) {
			writeNumber("maxChars", itemDef.typeData.editField->maxChars);
			writeNumber("maxPaintChars", itemDef.typeData.editField->maxPaintChars);
			if (itemDef.typeData.editField->maxCharsGotoNext) {
				writeLine("maxCharsGotoNext");
			}
		}
		else if (itemDef.type == ITEM_TYPE_MULTI) {

			if (itemDef.typeData.multi->strDef == 1) {
				menuFile << tabbing << "dvarStrList" << PADDING(13) << "{ ";
			}
			else {
				menuFile << tabbing << "dvarFloatList" << PADDING(13) << "{ ";
			}

			

			for (int i = 0; i < itemDef.typeData.multi->count; i++) {
				if (itemDef.typeData.multi->strDef == 1) {
					menuFile << itemDef.typeData.multi->dvarList[i] << " " << itemDef.typeData.multi->dvarStr[i] << " ";
				}
				else {
					menuFile << itemDef.typeData.multi->dvarList[i] << " " << itemDef.typeData.multi->dvarValue[i] << " ";
				}
			}
			menuFile << " }\n";

		}
		else if (itemDef.type == ITEM_TYPE_DVARENUM) {
			//idk what this does
		}
		else if (itemDef.type == ITEM_TYPE_GAME_MESSAGE_WINDOW) {
			writeNumber("gameMsgWindowIndex", itemDef.gameMsgWindowIndex);
			writeNumber("gameMsgWindowMode", itemDef.gameMsgWindowMode, messageMap);
		}

		WRITE_ACTION("mouseEnterText", itemDef.mouseEnterText);

		WRITE_ACTION("mouseExitText", itemDef.mouseExitText);

		WRITE_ACTION("mouseEnter", itemDef.mouseEnter);

		WRITE_ACTION("mouseExit", itemDef.mouseExit);

		WRITE_ACTION("action", itemDef.action);

		WRITE_ACTION("onAccept", itemDef.onAccept);

		WRITE_ACTION("onAccept", itemDef.onAccept);

		WRITE_ACTION("onFocus", itemDef.onFocus);

		WRITE_ACTION("leaveFocus", itemDef.mouseExitText);


		while (itemDef.onKey) {
			writeKeyAction(itemDef.onKey->key, itemDef.onKey->action);
			itemDef.onKey = itemDef.onKey->next;
		}
		
		writeStatement("text", itemDef.textExp);

		writeStatement("material", itemDef.materialExp);

		writeStatement("rect X", itemDef.rectXExp);

		writeStatement("rect Y", itemDef.rectYExp);

		writeStatement("rect W", itemDef.rectWExp);

		writeStatement("rect H", itemDef.rectHExp);

		writeStatement("foreColor A", itemDef.forecolorAExp);
		
		WRITE_TEXT("dvar", itemDef.dvar);

		WRITE_TEXT("dvartest", itemDef.dvarTest);


		if (itemDef.dvarFlags != 0) {
			std::string dvarFlag = "";
			switch (itemDef.dvarFlags) {
				case DISABLEDVAR:
					dvarFlag = "disableDvar";
					break;
				case SHOWDVAR:
					dvarFlag = "showDvar";
					break;
				case HIDEDVAR:
					dvarFlag = "hideDvar";
					break;
				case FOCUSDVAR:
					dvarFlag = "focusDvar";
			}
			writeAction(dvarFlag, itemDef.enableDvar);
		}
		

		if (itemDef.window.staticFlags) {
			writeFlag(itemDef.window.staticFlags);
		}



		tabbing = MENUDEF_TABBING;
		//close itemDef
		menuFile << tabbing << "}\n";

	}

	//Write rectangle
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

		menuFile << tabbing << "rect" << PADDING(4)
			<< rect.x << " "
			<< rect.y << " "
			<< rect.w << " "
			<< rect.h << " "
			<< hAlign << " "
			<< vAlign << "\n";
		
	}

	//Write text entries
	void MenuExport::writeText(std::string name, const char* text) {
			menuFile << tabbing << name << PADDING(name.length()) << "\"" << text << "\"\n";
	}

	//write single line
	void MenuExport::writeLine(std::string name) {
		menuFile << tabbing << name << "\n";
	}

	//Write number entries
	template <typename T>
	void MenuExport::writeNumber(std::string name, T num) {
		menuFile << tabbing << name << PADDING(name.length()) << num << "\n";
	}

	//Write number entries that have a definiton in menudefinition.h
	template <typename T>
	void MenuExport::writeNumber(std::string name, T num, identifierMap &map) {
		using namespace std;

		string number = to_string(num);
		find = map.find(num);

		if (find != map.end()) {
			number = find->second;
		}

		menuFile << tabbing << name << PADDING(name.length()) << number << "\n";
	}

	//write flags
	void MenuExport::writeFlag(int num) {
		find = staticFlagsMap.find(num);

		if (find != staticFlagsMap.end()) {
			menuFile << tabbing << find->second << "\n";
		}
	}

	//Write menu actions, mousehover, onopen, onclose etc
	void MenuExport::writeAction(std::string name, std::string action) {
		using namespace std;
		
		bool stringOpen = false;
		string newText = "";
		string word = "";

		//I dont think this is the best solution
		//Need to remove quote character around script actions but leave quotes everywhere else
		int i = 0;
		int start = 0;
		while (i < action.length()) {
			if (action.at(i) == '"') {
				if (!stringOpen) {
					stringOpen = true;
					start = i;
					i++;
					continue;
				}
				else {
					stringOpen = false;
					for (int z = 0; z < scriptActionLength; z++) {
						transform(word.begin(), word.end(), word.begin(), [](char c) {return tolower(c); });
						if (word.compare(scriptActions[z]) == 0) {
							action.replace(start, 1, " ");
							action.replace(i, 1, " ");
							z = scriptActionLength;
						}
					}
					word = "";
				}
			}
			
			if (stringOpen) {
				word += action.at(i);
			}

			i++;
		}

		menuFile << tabbing << name << PADDING(name.length()) <<  "{ " << action << " }" << "\n";
	}

	//write key actions, execkey, execkeyint
	void MenuExport::writeKeyAction(int key, std::string action) {
		std::string name;
		//range for single character keys
		if (key >= KEY_MULTIPLY && key <= KEY_Z ) {
			name = "execKey \"";
			name += (char)key;
			name += "\" ";
		}
		else {
			name = "execKeyInt " + std::to_string(key) + " ";
		}

		writeAction(name, action);
	}

	//Write a float4
	void MenuExport::writeVec4(std::string name, float vec4[]) {

		menuFile << tabbing << name << PADDING(name.length())
			<< vec4[0] << " "
			<< vec4[1] << " "
			<< vec4[2] << " "
			<< vec4[3] << "\n";
	}

	//special case for visible
	void MenuExport::writeVisible(Game::statement_s stmt) {
		if (stmt.numEntries == 0) {
			menuFile << tabbing << "visible" << PADDING(7) << "1\n";
		}
		else {
			writeStatement("visible", stmt);
		}
	}

	//Write statement_s
	void MenuExport::writeStatement(std::string name, Game::statement_s stmt) {
		using namespace std;
		if (stmt.numEntries == 0) {
			return;
		}
		string statement = "";
		menuFile << tabbing << "exp " << PADDING(name.length() + 3/*for the exp*/) << name << " ";

		for (int i = 0; i < stmt.numEntries; i++) {
			if (stmt.entries[i]->type == 0) {
				find = operandMap.find(stmt.entries[i]->data.operationEnum);

				if (find != operandMap.end()) {
					statement += find->second;
				}
			}
			else if (stmt.entries[i]->type == 1) {
				switch (stmt.entries[i]->data.operand.dataType) {
					case Game::expDataType::VAL_INT:
					{
						statement += to_string(stmt.entries[i]->data.operand.internals.intVal);
						break;
					}
					case Game::expDataType::VAL_FLOAT:
					{
						statement += to_string(stmt.entries[i]->data.operand.internals.floatVal);
						break;
					}
					case Game::expDataType::VAL_STRING:
					{
						statement += "\"";
						statement += stmt.entries[i]->data.operand.internals.string;
						statement += "\"";
						break;
					}

				}
			}
		}

		//final close bracket does not get stored and sometimes open brackets do not get stored so we try to replace missing ones
		int openBrackets = 0;
		for (int i = 0; i < statement.length(); i++) {
			if (statement.at(i) == '(') {
				openBrackets++;
			}
			else if(statement.at(i) == ')') {
				openBrackets--;
			}
		}

		if (openBrackets < 0) {
			statement += "//More closed brackets ')' then open.";
		}
		else {
			for (int i = 0; i < openBrackets; i++) {
				statement += ')';
			}
		}



		menuFile << statement << "\n";

	}

}