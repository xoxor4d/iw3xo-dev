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

		writeMenuDef(menu);

		//close menu
		menuFile << "}\n";

		menuFile.close();
	}


	void MenuExport::writeMenuDef(Game::menuDef_t *menuDef)
	{

		//open menudef
		menuFile << tabbing << "menuDef" << "\n" << tabbing << "{\n";
		tabbing = MENUDEF_TABBING;

		writeText("name", menuDef->window.name);

		writeRect(menuDef->window.rect);

		writeVisible(menuDef->visibleExp);

		writeNumber("fullScreen", menuDef->fullScreen);

		if(menuDef->blurRadius != 0)
		{ 
			writeNumber("blurWorld", menuDef->blurRadius);
		}

		if (menuDef->fadeCycle != 0) {
			writeNumber("fadeCycle", menuDef->fadeCycle);
		}

		if (menuDef->fadeClamp != 0) {
			writeNumber("fadeClamp", menuDef->fadeClamp);
		}

		if (menuDef->fadeAmount != 0) {
			writeNumber("fadeAmount", menuDef->fadeAmount);
		}

		if (menuDef->fadeInAmount != 0) {
			writeNumber("fadeInAmount", menuDef->fadeInAmount);
		}
		
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

		if (menuDef->allowedBinding) {
			writeText("allowBinding", menuDef->allowedBinding);
		}

		if (menuDef->soundName) {
			writeText("soundLoop", menuDef->soundName);
		}

		if (menuDef->window.background) {

			writeText("background", menuDef->window.background->info.name);
		}

		if (menuDef->window.staticFlags) {
			writeFlag(menuDef->window.staticFlags);
		}

		
		
		if (menuDef->onOpen) {
			writeAction("onOpen", menuDef->onOpen);
		}

		if (menuDef->onClose) {
			writeAction("onClose", menuDef->onClose);
		}

		if (menuDef->onESC) {
			writeAction("onESC", menuDef->onESC);
		}

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

		if (itemDef.window.name) {
			writeText("name", itemDef.window.name);
		}

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

		if (itemDef.window.group) {
			writeText("group", itemDef.window.group);
		}

		if (itemDef.window.background) {
			writeText("background", itemDef.window.background->info.name);
		}

		writeNumber("type", itemDef.type, itemTypeMap);


		if (itemDef.fontEnum != 0) {
			writeNumber("textfont", itemDef.fontEnum, fontTypeMap);
		}

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

		if (itemDef.type == itemType::listBox) {
			writeNumber("feeder", itemDef.feeder, feederMap);
			writeNumber("elementWidth", itemDef.typeData.listBox->elementWidth);
			writeNumber("elementHeight", itemDef.typeData.listBox->elementHeight);		
			writeNumber("elementType", itemDef.typeData.listBox->elementStyle, listBoxTypeMap);
			if (itemDef.typeData.listBox->doubleClick) {
				writeAction("doubleclick", itemDef.typeData.listBox->doubleClick);
			}
			writeVec4("selectBorder", itemDef.typeData.listBox->selectBorder);
			writeVec4("disableColor", itemDef.typeData.listBox->disableColor);

			//fix this
			//if (itemDef.typeData.listBox->selectIcon->info.name) {
			//	writeText("selectIcon", itemDef.typeData.listBox->selectIcon->info.name);
			//}
			if (itemDef.typeData.listBox->notselectable) {
				writeLine("notselectable");
			}
			if (itemDef.typeData.listBox->noScrollBars) {
				writeLine("noScrollBars");
			}
			if (itemDef.typeData.listBox->usePaging) {
				writeLine("usePaging");
			}

			padding = PADDING(5);
			menuFile << tabbing << "columns" << setw(padding) << itemDef.typeData.listBox->numColumns << "\n";
			for (int i = 0; i < itemDef.typeData.listBox->numColumns; i++) {
				menuFile << tabbing << setw(padding) << itemDef.typeData.listBox->columnInfo[i].pos
					<< " " << itemDef.typeData.listBox->columnInfo[i].width
					<< " " << itemDef.typeData.listBox->columnInfo[i].maxChars << "\n";
			}
		}
		else if (itemDef.type == itemType::editField) {
			writeNumber("maxChars", itemDef.typeData.editField->maxChars);
			writeNumber("maxPaintChars", itemDef.typeData.editField->maxPaintChars);
			if (itemDef.typeData.editField->maxCharsGotoNext) {
				writeLine("maxCharsGotoNext");
			}
		}
		else if (itemDef.type == itemType::multi) {

		}
		else if (itemDef.type == itemType::enumDvar) {

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
		
		writeStatement("text", itemDef.textExp);

		writeStatement("material", itemDef.materialExp);

		writeStatement("rect X", itemDef.rectXExp);

		writeStatement("rect Y", itemDef.rectYExp);

		writeStatement("rect W", itemDef.rectWExp);

		writeStatement("rect H", itemDef.rectHExp);

		writeStatement("foreColor A", itemDef.forecolorAExp);
		

		//dvar

		//dvartest

		//enable dvar

		//dvarflags

		//focussound

		//textSavegame

		//imagetrack

		//gameMsgWindowIndex

		//gameMsgWindowMode


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

		padding = PADDING(4);
		menuFile << tabbing << "rect" << setw(padding)
			<< rect.x << " "
			<< rect.y << " "
			<< rect.w << " "
			<< rect.h << " "
			<< hAlign << " "
			<< vAlign << "\n";
		
	}

	//Write text entries
	void MenuExport::writeText(std::string name, const char* text) {	
			padding = PADDING(name.length());
			menuFile << tabbing << name << std::setw(padding) << "\"" << text << "\"\n";
	}

	//write single line
	void MenuExport::writeLine(std::string name) {
		padding = PADDING(name.length());
		menuFile << tabbing << name << "\n";
	}

	//Write number entries
	template <typename T>
	void MenuExport::writeNumber(std::string name, T num) {
		padding = PADDING(name.length());
		menuFile << tabbing << name << std::setw(padding) << num << "\n";
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

		padding = PADDING(name.length());
		menuFile << tabbing << name << std::setfill(' ') << std::setw(padding) << number << "\n";
	}

	//write flags
	void MenuExport::writeFlag(int num) {
		find = staticFlagsMap.find(num);

		if (find != staticFlagsMap.end()) {
			menuFile << tabbing << find->second << "\n";
		}
	}

	//might need to check for string in scriptactions :pepethink:
	//Write menu actions, mousehover, onopen, onclose etc
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

	//write key actions, execkey, execkeyint
	void MenuExport::writeKeyAction(int key, std::string action) {
		std::string name = "execKey \"" + std::to_string(key) + "\" ";
		writeAction(name, action);
	}

	//Write a float4
	void MenuExport::writeVec4(std::string name, float vec4[]) {

		padding = PADDING(name.length());
		menuFile << tabbing << name << std::setw(padding)
			<< vec4[0] << " "
			<< vec4[1] << " "
			<< vec4[2] << " "
			<< vec4[3] << "\n";
	}

	//special case for visible
	void MenuExport::writeVisible(Game::statement_s stmt) {
		if (stmt.numEntries == 0) {
			padding = PADDING(7);
			menuFile << tabbing << "visible" << std::setw(padding) << "1\n";
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
		padding = PADDING(name.length());
		menuFile << tabbing << "exp " << setw(padding) << name << " ";

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



		menuFile << statement << "\n";//does not save last lose bracket as operand ?

	}

}