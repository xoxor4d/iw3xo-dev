#include "STDInclude.hpp"

namespace Utils
{
	// build all entities (includes brushmodel pointers "*")
	std::string Entities::buildAll()
	{
		std::string entityString;

		for (auto& entity : this->entities)
		{
			entityString.append("{\n");

			for (auto& property : entity)
			{
				entityString.push_back('"');
				entityString.append(property.first);
				entityString.append("\" \"");
				entityString.append(property.second);
				entityString.append("\"\n");
			}

			entityString.append("}\n");
		}

		return entityString;
	}

	// build all entities and fix brushmodels
	std::string Entities::buildAll_script_structs()
	{
		std::string entityString;

		for (auto& entity : this->entities)
		{
			std::string classname = entity["classname"];

			// if ent is a brushmodel/submodel
			if (!classname.empty() && Utils::Contains(classname, "script_struct"))
			{
				// start entity
				//entityString.append(Utils::VA("// entity %d\n", entityNum));
				entityString.append("{\n");

				for (auto& property : entity)
				{
					entityString.push_back('"');
					entityString.append(property.first);
					entityString.append("\" \"");
					entityString.append(property.second);
					entityString.append("\"\n");
				}

				// close entity
				entityString.append("}\n");
			}
		}

		return entityString;
	}

	// build all entities and fix brushmodels
	std::string Entities::buildAll_FixBrushmodels(const std::vector<Game::brushmodelEnt_t> &bModelList)
	{
		int entityNum = 1; // worldspawn is 0
		int submodelNum = 0;
		std::string entityString;

		for (auto& entity : this->entities)
		{
			std::string model = entity["model"];

			// if ent is a brushmodel/submodel
			if (!model.empty() && model[0] == '*')
			{
				// get the submodel index 
				auto p_index = std::stoi(model.erase(0, 1));

				if (p_index < static_cast<int>(bModelList.size()))
				{
					auto bModelSideCount = static_cast<int>(bModelList[p_index].brushSides.size());
					
					// skip submodel entity if we have less then 6 brushsides (we could also create a temp. cube at its origin)
					if (bModelSideCount < 6)
					{
						continue;
					}

					// start submodel
					entityString.append(Utils::VA("// submodel %d\n", submodelNum));
					entityString.append("{\n");

					// write submodel keys
					for (auto& property : entity)
					{
						// do not write model/origin keys
						if (property.first == "model" || property.first == "origin")
						{
							continue;
						}

						entityString.push_back('"');
						entityString.append(property.first);
						entityString.append("\" \"");
						entityString.append(property.second);
						entityString.append("\"\n");
					}

					// start submodel brush
					entityString.append("{\n");

					for (auto bSide = 0; bSide < bModelSideCount; bSide++)
					{
						entityString.append(bModelList[p_index].brushSides[bSide]);
					}

					// close submodel brush
					entityString.append("}\n");

					// close submodel
					entityString.append("}\n");

					submodelNum++;
				}
			}

			else
			{
				// start entity
				entityString.append(Utils::VA("// entity %d\n", entityNum));
				entityString.append("{\n");

				for (auto& property : entity)
				{
					entityString.push_back('"');
					entityString.append(property.first);
					entityString.append("\" \"");
					entityString.append(property.second);
					entityString.append("\"\n");
				}

				// close entity
				entityString.append("}\n");

				entityNum++;
			}
		}

		return entityString;
	}

	// build all selected entities and fix brushmodels
	std::string Entities::buildSelection_FixBrushmodels(const Game::boundingBox_t* box, const std::vector<Game::brushmodelEnt_t>& bModelList)
	{
		int entityNum = 1; // worldspawn is 0
		int submodelNum = 0;
		std::string entityString;

		for (auto& entity : this->entities)
		{
			std::string model = entity["model"];
			std::string origin = entity["origin"];

			// if ent is a brushmodel/submodel
			if (!model.empty() && model[0] == '*')
			{
				// get the submodel index 
				auto p_index = std::stoi(model.erase(0, 1));

				if (p_index < static_cast<int>(bModelList.size()))
				{
					auto bModelSideCount = static_cast<int>(bModelList[p_index].brushSides.size());

					// skip submodel entity if we have less then 6 brushsides (we could also create a temp. cube at its origin)
					if (bModelSideCount < 6)
					{
						continue;
					}

					// start submodel
					entityString.append(Utils::VA("// submodel %d\n", submodelNum));
					entityString.append("{\n");

					// write submodel keys
					for (auto& property : entity)
					{
						// do not write model/origin keys
						if (property.first == "model" || property.first == "origin")
						{
							continue;
						}

						entityString.push_back('"');
						entityString.append(property.first);
						entityString.append("\" \"");
						entityString.append(property.second);
						entityString.append("\"\n");
					}

					// start submodel brush
					entityString.append("{\n");

					for (auto bSide = 0; bSide < bModelSideCount; bSide++)
					{
						entityString.append(bModelList[p_index].brushSides[bSide]);
					}

					// close submodel brush
					entityString.append("}\n");

					// close submodel
					entityString.append("}\n");

					submodelNum++;
				}
			}

			else
			{
				float tempOrigin[3] = {0.0f, 0.0f, 0.0f};

				if (!sscanf_s(origin.c_str(), "%f %f %f", &tempOrigin[0], &tempOrigin[1], &tempOrigin[2]))
				{
					Game::Com_PrintMessage(0, Utils::VA("[!]: sscanf failed for entity %d", entityNum), 0);
				}

				if (Utils::polylib::PointWithinBounds(glm::toVec3(tempOrigin), box->mins, box->maxs, 0.25f))
				{
					// start entity
					entityString.append(Utils::VA("// entity %d\n", entityNum));
					entityString.append("{\n");

					for (auto& property : entity)
					{
						entityString.push_back('"');
						entityString.append(property.first);
						entityString.append("\" \"");
						entityString.append(property.second);
						entityString.append("\"\n");
					}

					// close entity
					entityString.append("}\n");
					entityNum++;
				}
			}
		}

		return entityString;
	}

	// only build worldspawn keys/values without opening/closing brackets
	std::string Entities::buildWorldspawnKeys()
	{
		std::string entityString;

		for (auto& entity : this->entities)
		{
			if (entity.find("classname") != entity.end())
			{
				if (entity["classname"] == "worldspawn"s)
				{
					for (auto& property : entity)
					{
						entityString.push_back('"');
						entityString.append(property.first);
						entityString.append("\" \"");
						entityString.append(property.second);
						entityString.append("\"\n");
					}

					break;
				}
			}
		}

		return entityString;
	}

	std::vector<std::string> Entities::getModels()
	{
		std::vector<std::string> models;

		for (auto& entity : this->entities)
		{
			if (entity.find("model") != entity.end())
			{
				std::string model = entity["model"];

				if (!model.empty() && model[0] != '*' && model[0] != '?') // Skip brushmodels
				{
					if (std::find(models.begin(), models.end(), model) == models.end())
					{
						models.push_back(model);
					}
				}
			}
		}

		return models;
	}

	std::vector<Game::brushmodelEnt_t> Entities::getBrushModels()
	{
		std::vector<Game::brushmodelEnt_t> bModels;

		// geting the total clipmap size would prob. be better
		uintptr_t leafBrushesStart = reinterpret_cast<uintptr_t>(&*Game::cm->leafbrushNodes);
		uintptr_t leafBrushesEnd = leafBrushesStart + sizeof(Game::cLeafBrushNode_s) * (Game::cm->leafbrushNodesCount + Game::cm->numLeafBrushes); // wrong

		// first element is always empty because
		// the first submodel within the entsMap starts at 1 and we want to avoid subtracting - 1 everywhere 
		bModels.push_back(Game::brushmodelEnt_t()); 

		for (auto& entity : this->entities)
		{
			if (entity.find("model") != entity.end())
			{
				std::string model = entity["model"];
				std::string origin = entity["origin"];

				// if ent is a brushmodel/submodel
				if (!model.empty() && model[0] == '*' && !origin.empty())
				{
					auto currBModel = Game::brushmodelEnt_t();

					// get the submodel index 
					auto p_index = std::stoi(model.erase(0, 1));

					// the index should always match the size of our vector or we did something wrong
					if (p_index != (int)bModels.size())
					{
						Game::Com_PrintMessage(0, Utils::VA("[Entities::getBrushModels]: Something went wrong while parsing submodels. (%d != %d)", p_index, bModels.size()), 0);
					}

					if (p_index >= static_cast<int>(Game::cm->numSubModels))
					{
						Game::Com_PrintMessage(0, Utils::VA("[Entities::getBrushModels]: Something went wrong while parsing submodels. (%d >= %d numSubModels)", p_index, Game::cm->numSubModels), 0);
						break;
					}

					// assign indices and pointers to both the brush and the submodel
					currBModel.cmSubmodelIndex = p_index;
					
					if (&Game::cm->cmodels[p_index])
					{
						currBModel.cmSubmodel = &Game::cm->cmodels[p_index];
					}

					// fix me daddy
					auto brushIdx_ptr = Game::cm->leafbrushNodes[Game::cm->cmodels[p_index].leaf.leafBrushNode].data.leaf.brushes;
					currBModel.cmBrushIndex = 0;

					// this is giving me cancer
					if (Game::cm->cmodels[p_index].leaf.leafBrushNode != 0 && brushIdx_ptr)
					{
						if ((uintptr_t)&*brushIdx_ptr >= leafBrushesStart && (uintptr_t) & *brushIdx_ptr < leafBrushesEnd)
						{
							currBModel.cmBrushIndex = static_cast<int>(*Game::cm->leafbrushNodes[Game::cm->cmodels[p_index].leaf.leafBrushNode].data.leaf.brushes);
						}
						else
						{
							Game::Com_PrintMessage(0, Utils::VA("[Entities::getBrushModels]: Skipping faulty brush-index pointer at leafbrushNodes[%d].data.leaf.brushes ...\n", p_index), 0);
						}
						
						//currBModel.cmBrush = &Game::cm->brushes[*Game::cm->leafbrushNodes[Game::cm->cmodels[p_index].leaf.leafBrushNode].data.leaf.brushes];
						currBModel.cmBrush = &Game::cm->brushes[currBModel.cmBrushIndex];

						// add the submodel index to the clipmap brush
						currBModel.cmBrush->isSubmodel = true;
						currBModel.cmBrush->cmSubmodelIndex = static_cast<__int16>(p_index);
					}


					// save entity origin
					if (!sscanf_s(origin.c_str(), "%f %f %f", &currBModel.cmSubmodelOrigin[0], &currBModel.cmSubmodelOrigin[1], &currBModel.cmSubmodelOrigin[2]))
					{
						Game::Com_PrintMessage(0, Utils::VA("[!]: sscanf failed for submodel %d", p_index), 0);
						currBModel.cmSubmodelOrigin[0] = 0.0f;
						currBModel.cmSubmodelOrigin[1] = 0.0f;
						currBModel.cmSubmodelOrigin[2] = 0.0f;
					}

					bModels.push_back(currBModel);
				}
			}
		}

		return bModels;
	}

	void Entities::deleteWorldspawn()
	{
		for (auto i = this->entities.begin(); i != this->entities.end();)
		{
			if (i->find("classname") != i->end())
			{
				std::string classname = (*i)["classname"];
				if (Utils::StartsWith(classname, "worldspawn"))
				{
					i = this->entities.erase(i);
					continue;
				}
			}

			++i;
		}
	}

	void Entities::deleteTriggers()
	{
		for (auto i = this->entities.begin(); i != this->entities.end();)
		{
			if (i->find("classname") != i->end())
			{
				std::string classname = (*i)["classname"];
				if (Utils::StartsWith(classname, "trigger_"))
				{
					i = this->entities.erase(i);
					continue;
				}
			}

			++i;
		}
	}

	void Entities::deleteWeapons(bool keepTurrets)
	{
		for (auto i = this->entities.begin(); i != this->entities.end();)
		{
			if (i->find("weaponinfo") != i->end() || (i->find("targetname") != i->end() && (*i)["targetname"] == "oldschool_pickup"s))
			{
				if (!keepTurrets || i->find("classname") == i->end() || (*i)["classname"] != "misc_turret"s)
				{
					i = this->entities.erase(i);
					continue;
				}
			}

			++i;
		}
	}

	void Entities::parse(std::string buffer)
	{
		int parseState = 0;
		std::string key;
		std::string value;
		std::unordered_map<std::string, std::string> entity;

		for (unsigned int i = 0; i < buffer.size(); ++i)
		{
			char character = buffer[i];
			if (character == '{')
			{
				entity.clear();
			}

			switch (character)
			{
				case '{':
				{
					entity.clear();
					break;
				}

				case '}':
				{
					this->entities.push_back(entity);
					entity.clear();
					break;
				}

				case '"':
				{
					if (parseState == PARSE_AWAIT_KEY)
					{
						key.clear();
						parseState = PARSE_READ_KEY;
					}
					else if (parseState == PARSE_READ_KEY)
					{
						parseState = PARSE_AWAIT_VALUE;
					}
					else if (parseState == PARSE_AWAIT_VALUE)
					{
						value.clear();
						parseState = PARSE_READ_VALUE;
					}
					else if (parseState == PARSE_READ_VALUE)
					{
						entity[Utils::StrToLower(key)] = value;
						parseState = PARSE_AWAIT_KEY;
					}
					else
					{
						throw std::runtime_error("Parsing error!");
					}
					break;
				}

				default:
				{
					if (parseState == PARSE_READ_KEY) key.push_back(character);
					else if (parseState == PARSE_READ_VALUE) value.push_back(character);

					break;
				}
			}
		}
	}
}
