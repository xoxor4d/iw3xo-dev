#include "STDInclude.hpp"

namespace Utils
{
	std::string Entities::build()
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
						Game::Com_PrintMessage(0, Utils::VA("[!]: Something went wrong while parsing submodels. (%d != %d)", p_index, bModels.size()), 0);
					}

					if (p_index >= static_cast<int>(Game::cm->numSubModels))
					{
						Game::Com_PrintMessage(0, Utils::VA("[!]: Something went wrong while parsing submodels. (%d >= %d numSubModels)", p_index, Game::cm->numSubModels), 0);
						break;
					}

					// assign indices and pointers to both the brush and the submodel
					currBModel.cmSubmodelIndex = p_index;
					
					if (&Game::cm->cmodels[p_index])
					{
						currBModel.cmSubmodel = &Game::cm->cmodels[p_index];
					}
					
					if (Game::cm->cmodels[p_index].leaf.leafBrushNode != 0 && Game::cm->leafbrushNodes[Game::cm->cmodels[p_index].leaf.leafBrushNode].data.leaf.brushes)
					{
						currBModel.cmBrushIndex = (int)*Game::cm->leafbrushNodes[Game::cm->cmodels[p_index].leaf.leafBrushNode].data.leaf.brushes;
							
						//currBModel.cmBrush = &Game::cm->brushes[*Game::cm->leafbrushNodes[Game::cm->cmodels[p_index].leaf.leafBrushNode].data.leaf.brushes];
						currBModel.cmBrush = &Game::cm->brushes[currBModel.cmBrushIndex];

						// add the submodel index to the clipmap brush
						currBModel.cmBrush->isSubmodel = true;
						currBModel.cmBrush->cmSubmodelIndex = static_cast<__int16>(p_index);
					}
					else
					{
						currBModel.cmBrushIndex = 0;
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

	void Entities::convertTurrets()
	{
		for (auto& entity : this->entities)
		{
			if (entity.find("classname") != entity.end())
			{
				if (entity["classname"] == "misc_turret"s)
				{
					entity["weaponinfo"] = "turret_minigun_mp";
					entity["model"] = "weapon_minigun";
				}
			}
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
