#include "std_include.hpp"
#include "menu_export.hpp"

namespace components
{
	menu_export::menu_export() 
	{
		maxPadding = 20;
		tabs	= 0;
		padding = 0;
		tabbing = MENU_TABBING;

		command::add("menu_list", "", "print all loaded menus to the console", [this](command::params) 
		{
			if (!game::ui_context || !*game::ui_context->Menus) 
			{
				return;
			}

			for (int i = 0; i < game::ui_context->menuCount; i++) 
			{
				game::Com_PrintMessage(0, utils::va("Menu: %s \n", game::ui_context->Menus[i]->window.name), 0);
			}
		});

		command::add("menu_export", "<menu name> <[optional] subdir>", "exports the specified menu to root\\menu_export\\menus\\subdir\\", [this](command::params params) 
		{
			if (params.length() < 2)
			{
				game::Com_PrintMessage(0, "^1No menu name given.^7\n", 0);
				return;
			}

			if (!game::ui_context || !*game::ui_context->Menus) 
			{
				return;
			}

			for (int i = 0; i < game::ui_context->menuCount; i++) 
			{
				if (strcmp(game::ui_context->Menus[i]->window.name, params[1]) == 0)
				{
					export_menu(game::ui_context->Menus[i], params.length() == 2 ? "" : params[2]);
					return;
				}
			}

			game::Com_PrintMessage(0, utils::va("^1WARNING: ^7Menu %s was not found.\n", params[1]), 0);
		});

		command::add("menu_export_itemdefs", "<menu name> <[optional] subdir>", "exports all itemdefs of the specified menu to root\\menu_export\\itemdefs\\subdir\\", [this](command::params params) 
		{
			if (params.length() < 2)
			{
				game::Com_PrintMessage(0, "^1No menu name given.^7\n", 0);
				return;
			}

			if (!game::ui_context || !*game::ui_context->Menus) {
				return;
			}

			for (int i = 0; i < game::ui_context->menuCount; i++) 
			{
				if (strcmp(game::ui_context->Menus[i]->window.name, params[1]) == 0)
				{
					export_menu_itemdefs(game::ui_context->Menus[i], params.length() == 2 ? "" : params[2]);
					return;
				}
			}

			game::Com_PrintMessage(0, utils::va("^1WARNING: ^7Menu %s was not found.\n", params[1]), 0);
		});
	}

	//Export menudef and all its itemdef to specified folder and file name
	void menu_export::export_menu(game::menuDef_t *menu, std::string subdir) 
	{
		if (!menu || !set_path("menus\\"s + subdir, menu->window.name + ".menu"s))
		{
			return;
		}

		game::Com_PrintMessage(0, "Exporting menu ...\n", 0);

		menu_file_.open(writeFilePath.c_str());

		//header
		menu_file_ << headerText;

		//open menu
		menu_file_ << "{\n";

		write_menu_def(menu);

		//close menu
		menu_file_ << "}\n";
		menu_file_.close();

		game::Com_PrintMessage(0, utils::va("Menu exported to: %s\n", writeFilePath.c_str()), 0);
	}

	//Export all itemdef from a menudef to a specified folder and file name
	void menu_export::export_menu_itemdefs(game::menuDef_t *menu, std::string subdir)
	{
		if (!menu || !set_path("itemdefs\\"s + subdir, std::string(menu->window.name + "_itemdefs.menu"s)))
		{
			return;
		}

		game::Com_PrintMessage(0, "Exporting itemDefs ...\n", 0);

		menu_file_.open(writeFilePath.c_str());

		//header
		menu_file_ << headerText;

		for (int i = 0; i < menu->itemCount; i++) 
		{
			write_item_def(*menu->items[i]);
		}

		menu_file_.close();

		game::Com_PrintMessage(0, utils::va("ItemDefs exported to: %s\n", writeFilePath.c_str()), 0);
	}

	bool menu_export::set_path(std::string subdir, std::string name) 
	{
		// client is only able to export to a sub-directory of "menu_export"
		const auto& base_path = game::Dvar_FindVar("fs_basepath");
		
		if (!base_path) 
		{
			return false;
		}

		std::string file_path = base_path->current.string + "\\iw3xo\\menu_export\\"s + subdir;
					file_path += utils::ends_with(subdir, "\\"s) ? "" : "\\";

		std::filesystem::create_directories(file_path);
		writeFilePath = file_path + name;

		return true;
	}

	void menu_export::write_menu_def(game::menuDef_t *menuDef)
	{
		tabbing = MENU_TABBING;

		//open menudef
		menu_file_ << tabbing << "menuDef" << "\n" << tabbing << "{\n";
		tabbing = MENUDEF_TABBING;

		WRITE_TEXT("name", menuDef->window.name);

		write_rect(menuDef->window.rect);
		write_visible(menuDef->visibleExp);
		write_number("fullScreen", menuDef->fullScreen);

		WRITE_NUMBER("blurWorld", 0, menuDef->blurRadius);
		WRITE_NUMBER("fadeCycle", 0, menuDef->fadeCycle);
		WRITE_NUMBER("fadeClamp", 0, menuDef->fadeClamp);
		WRITE_NUMBER("fadeAmount", 0, menuDef->fadeAmount);
		WRITE_NUMBER("fadeInAmount", 0, menuDef->fadeInAmount);
		
		if (menuDef->window.border != 0 && menuDef->window.borderSize != 0) {

			write_number("border", menuDef->window.border, borderTypeMap);
			write_number("borderSize", menuDef->window.borderSize);
			write_vec4("borderColor", menuDef->window.borderColor);
		}

		write_vec4("foreColor", menuDef->window.foreColor);
		write_vec4("backColor", menuDef->window.backColor);
		write_vec4("outlineColor", menuDef->window.outlineColor);
		write_vec4("disableColor", menuDef->disableColor);
		write_vec4("focusColor", menuDef->focusColor);

		if (menuDef->window.ownerDraw) 
		{
			write_number("ownerDraw", menuDef->window.ownerDraw, ownerDrawTypeMap);
			write_number("ownerDrawFlag", menuDef->window.ownerDrawFlags);
		}

		WRITE_TEXT("allowBinding", menuDef->allowedBinding);
		WRITE_TEXT("soundLoop", menuDef->soundName);

		if (menuDef->window.background) 
		{
			write_text("background", menuDef->window.background->info.name);
		}

		if (menuDef->window.staticFlags) 
		{
			write_flag(menuDef->window.staticFlags);
		}

		WRITE_ACTION("onOpen", menuDef->onOpen);
		WRITE_ACTION("onClose", menuDef->onClose);
		WRITE_ACTION("onESC", menuDef->onESC);

		while (menuDef->onKey) 
		{
			write_key_action(menuDef->onKey->key, menuDef->onKey->action);
			menuDef->onKey = menuDef->onKey->next;
		}

		write_statement("rect X", menuDef->rectXExp);
		write_statement("rect Y", menuDef->rectYExp);

		for (int i = 0; i < menuDef->itemCount; i++) 
		{
			write_item_def(*menuDef->items[i]);
		}
		
		tabbing = MENU_TABBING;

		//close menudef
		menu_file_ << tabbing << "}\n";
	}

	void menu_export::write_item_def(game::itemDef_s &itemDef) 
	{
		//itemdef open
		menu_file_ << "\n" << tabbing << "itemDef\n";
		menu_file_ << tabbing << "{\n";
		tabbing = ITEMDEF_TABBING;

		WRITE_TEXT("name", itemDef.window.name);

		write_rect(itemDef.window.rectClient);
		write_visible(itemDef.visibleExp);

		if (itemDef.window.ownerDraw) 
		{
			write_number("ownerDraw", itemDef.window.ownerDraw, ownerDrawTypeMap);
			write_number("ownerDrawFlag", itemDef.window.ownerDrawFlags);
		}
		
		write_number("style", itemDef.window.style, windowStyleMap);

		if (itemDef.window.border != 0 && itemDef.window.borderSize != 0) 
		{
			write_number("border", itemDef.window.border, borderTypeMap);
			write_number("borderSize", itemDef.window.borderSize);
			write_vec4("borderColor", itemDef.window.borderColor);
		}

		write_vec4("foreColor", itemDef.window.foreColor);
		write_vec4("backColor", itemDef.window.backColor);

		if (itemDef.window.outlineColor[3] != 0) 
		{
			write_vec4("outlineColor", itemDef.window.outlineColor);
		}

		WRITE_TEXT("group", itemDef.window.group);

		if (itemDef.window.background) 
		{
			write_text("background", itemDef.window.background->info.name);
		}

		write_number("type", itemDef.type, itemTypeMap);

		WRITE_NUMBER_MAP("textfont", 0, itemDef.fontEnum, fontTypeMap);
		WRITE_NUMBER_MAP("textAlign", 0, itemDef.textAlignMode, itemTextAlignMap);
		WRITE_NUMBER("textAlignX", 0, itemDef.textalignx);
		WRITE_NUMBER("textAlignY", 0, itemDef.textaligny);
		WRITE_NUMBER("textScale", defaultTextSize, itemDef.textscale);
		WRITE_NUMBER_MAP("textStyle", 0, itemDef.textStyle, itemTextStyleMap)
		WRITE_TEXT("text", itemDef.text);

		if (itemDef.type == ITEM_TYPE_LISTBOX) 
		{
			write_number("feeder", itemDef.feeder, feederMap);
			write_number("elementWidth", itemDef.typeData.listBox->elementWidth);
			write_number("elementHeight", itemDef.typeData.listBox->elementHeight);		
			write_number("elementType", itemDef.typeData.listBox->elementStyle, listBoxTypeMap);
			WRITE_ACTION("doubleclick", itemDef.typeData.listBox->doubleClick);
			write_vec4("selectBorder", itemDef.typeData.listBox->selectBorder);
			write_vec4("disableColor", itemDef.typeData.listBox->disableColor);

			if (itemDef.typeData.listBox->selectIcon) 
			{
				write_text("selectIcon", itemDef.typeData.listBox->selectIcon->info.name);
			}

			if (itemDef.typeData.listBox->notselectable) 
			{
				write_line("notselectable");
			}

			if (itemDef.typeData.listBox->noScrollBars) 
			{
				write_line("noScrollBars");
			}

			if (itemDef.typeData.listBox->usePaging) 
			{
				write_line("usePaging");
			}

			menu_file_ << tabbing << "columns" << PADDING(7) << itemDef.typeData.listBox->numColumns << "\n";

			for (int i = 0; i < itemDef.typeData.listBox->numColumns; i++) 
			{
				menu_file_ << tabbing << "\t\t\t\t" << std::setw(padding) << itemDef.typeData.listBox->columnInfo[i].pos
					<< " " << itemDef.typeData.listBox->columnInfo[i].width
					<< " " << itemDef.typeData.listBox->columnInfo[i].maxChars << "\n";
			}
		}
		else if (itemDef.type == ITEM_TYPE_EDITFIELD) 
		{
			write_number("maxChars", itemDef.typeData.editField->maxChars);
			write_number("maxPaintChars", itemDef.typeData.editField->maxPaintChars);

			if (itemDef.typeData.editField->maxCharsGotoNext) 
			{
				write_line("maxCharsGotoNext");
			}
		}
		else if (itemDef.type == ITEM_TYPE_MULTI) 
		{

			if (itemDef.typeData.multi->strDef == 1) 
			{
				menu_file_ << tabbing << "dvarStrList" << PADDING(13) << "{ ";
			}
			else 
			{
				menu_file_ << tabbing << "dvarFloatList" << PADDING(13) << "{ ";
			}

			for (int i = 0; i < itemDef.typeData.multi->count; i++) 
			{
				if (itemDef.typeData.multi->strDef == 1) 
				{
					menu_file_ << itemDef.typeData.multi->dvarList[i] << " " << itemDef.typeData.multi->dvarStr[i] << " ";
				}
				else 
				{
					menu_file_ << itemDef.typeData.multi->dvarList[i] << " " << itemDef.typeData.multi->dvarValue[i] << " ";
				}
			}

			menu_file_ << " }\n";
		}
		else if (itemDef.type == ITEM_TYPE_DVARENUM) 
		{
			//idk what this does
		}
		else if (itemDef.type == ITEM_TYPE_GAME_MESSAGE_WINDOW) 
		{
			write_number("gameMsgWindowIndex", itemDef.gameMsgWindowIndex);
			write_number("gameMsgWindowMode", itemDef.gameMsgWindowMode, messageMap);
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

		while (itemDef.onKey) 
		{
			write_key_action(itemDef.onKey->key, itemDef.onKey->action);
			itemDef.onKey = itemDef.onKey->next;
		}
		
		write_statement("text", itemDef.textExp);
		write_statement("material", itemDef.materialExp);
		write_statement("rect X", itemDef.rectXExp);
		write_statement("rect Y", itemDef.rectYExp);
		write_statement("rect W", itemDef.rectWExp);
		write_statement("rect H", itemDef.rectHExp);
		write_statement("foreColor A", itemDef.forecolorAExp);
		
		WRITE_TEXT("dvar", itemDef.dvar);
		WRITE_TEXT("dvartest", itemDef.dvarTest);

		if (itemDef.dvarFlags != 0) 
		{
			std::string dvarFlag = "";

			switch (itemDef.dvarFlags) 
			{
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

			write_action(dvarFlag, itemDef.enableDvar);
		}
		
		if (itemDef.window.staticFlags) 
		{
			write_flag(itemDef.window.staticFlags);
		}

		tabbing = MENUDEF_TABBING;

		//close itemDef
		menu_file_ << tabbing << "}\n";
	}

	//Write rectangle
	void menu_export::write_rect(game::rectDef_s rect) 
	{
		auto hAlign = std::to_string(rect.horzAlign);
		find = horizontalAlignmentMap.find(rect.horzAlign);

		if (find != horizontalAlignmentMap.end()) 
		{
			hAlign = find->second;
		}

		auto vAlign = std::to_string(rect.vertAlign);
		find = verticalAlignmentMap.find(rect.vertAlign);

		if (find != verticalAlignmentMap.end()) 
		{
			vAlign = find->second;
		}

		menu_file_ << tabbing << "rect" << PADDING(4)
			<< rect.x << " "
			<< rect.y << " "
			<< rect.w << " "
			<< rect.h << " "
			<< hAlign << " "
			<< vAlign << "\n";
	}

	//Write text entries
	void menu_export::write_text(std::string name, const char* text) 
	{
		menu_file_ << tabbing << name << PADDING(name.length()) << "\"" << text << "\"\n";
	}

	//write single line
	void menu_export::write_line(std::string name) 
	{
		menu_file_ << tabbing << name << "\n";
	}

	//Write number entries
	template <typename T>
	void menu_export::write_number(std::string name, T num) 
	{
		menu_file_ << tabbing << name << PADDING(name.length()) << num << "\n";
	}

	//Write number entries that have a definiton in menudefinition.h
	//template <typename T>
	void menu_export::write_number(std::string name, int num, identifierMap &map)
	{
		auto number = std::to_string(num);
		find = map.find(num);

		if (find != map.end()) 
		{
			number = find->second;
		}

		menu_file_ << tabbing << name << PADDING(name.length()) << number << "\n";
	}

	//write flags
	void menu_export::write_flag(int num) 
	{
		find = staticFlagsMap.find(num);

		if (find != staticFlagsMap.end()) 
		{
			menu_file_ << tabbing << find->second << "\n";
		}
	}

	//Write menu actions, mousehover, onopen, onclose etc
	void menu_export::write_action(std::string name, std::string action) 
	{
		bool string_open = false;
		std::string word;

		// I dont think this is the best solution
		// Need to remove quote character around script actions but leave quotes everywhere else
		int start = 0;

		for (auto i = 0; i < static_cast<int>(action.length()); i++)
		{
			if (action.at(i) == '"') 
			{
				if (!string_open) 
				{
					string_open = true;
					start = i;

					continue;
				}
				else 
				{
					string_open = false;

					for (int z = 0; z < script_action_length_; z++) 
					{
						utils::str_to_lower(word);

						if (word.compare(script_actions_[z]) == 0) 
						{
							action.replace(start, 1, " ");
							action.replace(i, 1, " ");
							z = script_action_length_;
						}
					}

					word = "";
				}
			}
			
			if (string_open) 
			{
				word += action.at(i);
			}
		}

		menu_file_ << tabbing << name << PADDING(name.length()) <<  "{ " << action << " }" << "\n";
	}

	//write key actions, execkey, execkeyint
	void menu_export::write_key_action(int key, std::string action) 
	{
		std::string name;

		//range for single character keys
		if (key >= KEY_MULTIPLY && key <= KEY_Z ) 
		{
			name = "execKey \"";
			name += (char)key;
			name += "\" ";
		}
		else
		{
			name = "execKeyInt " + std::to_string(key) + " ";
		}

		write_action(name, action);
	}

	//Write a float4
	void menu_export::write_vec4(std::string name, float vec4[]) 
	{
		menu_file_ << tabbing << name << PADDING(name.length())
			<< vec4[0] << " "
			<< vec4[1] << " "
			<< vec4[2] << " "
			<< vec4[3] << "\n";
	}

	//special case for visible
	void menu_export::write_visible(game::statement_s stmt)
	{
		if (stmt.numEntries == 0) 
		{
			menu_file_ << tabbing << "visible" << PADDING(7) << "1\n";
		}
		else 
		{
			write_statement("visible", stmt);
		}
	}

	//Write statement_s
	void menu_export::write_statement(std::string name, game::statement_s stmt)
	{
		if (stmt.numEntries == 0)
		{
			return;
		}

		std::string statement;
		menu_file_ << tabbing << "exp " << PADDING(name.length() + 3/*for the exp*/) << name << " ";

		for (int i = 0; i < stmt.numEntries; i++) 
		{
			if (stmt.entries[i]->type == 0) 
			{
				find = operandMap.find(stmt.entries[i]->data.operationEnum);

				if (find != operandMap.end()) 
				{
					statement += find->second;
				}
			}
			else if (stmt.entries[i]->type == 1) 
			{
				switch (stmt.entries[i]->data.operand.dataType) 
				{
					case game::expDataType::VAL_INT:
					{
						statement += std::to_string(stmt.entries[i]->data.operand.internals.intVal);
						break;
					}
					case game::expDataType::VAL_FLOAT:
					{
						statement += std::to_string(stmt.entries[i]->data.operand.internals.floatVal);
						break;
					}
					case game::expDataType::VAL_STRING:
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
		int open_brackets = 0;
		for (auto i = 0; i < static_cast<int>(statement.length()); i++) 
		{
			if (statement.at(i) == '(') 
			{
				open_brackets++;
			}
			else if(statement.at(i) == ')') 
			{
				open_brackets--;
			}
		}

		if (open_brackets < 0) 
		{
			statement += "//More closed brackets ')' then open.";
		}
		else 
		{
			for (int i = 0; i < open_brackets; i++) 
			{
				statement += ')';
			}
		}

		menu_file_ << statement << "\n";
	}
}