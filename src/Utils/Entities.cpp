#include "STDInclude.hpp"

namespace utils
{
	// build all entities (includes brushmodel pointers "*")
	std::string entities::build_all()
	{
		std::string entity_string;

		for (auto& entity : this->entities_)
		{
			entity_string.append("{\n");

			for (auto& property : entity)
			{
				entity_string.push_back('"');
				entity_string.append(property.first);
				entity_string.append("\" \"");
				entity_string.append(property.second);
				entity_string.append("\"\n");
			}

			entity_string.append("}\n");
		}

		return entity_string;
	}

	// build all entities and fix brushmodels
	std::string entities::build_all_script_structs()
	{
		std::string entity_string;

		for (auto& entity : this->entities_)
		{
			std::string classname = entity["classname"];

			// if ent is a brushmodel/submodel
			if (!classname.empty() && utils::contains(classname, "script_struct"))
			{
				// start entity
				//entityString.append(utils::va("// entity %d\n", entityNum));
				entity_string.append("{\n");

				for (auto& property : entity)
				{
					entity_string.push_back('"');
					entity_string.append(property.first);
					entity_string.append("\" \"");
					entity_string.append(property.second);
					entity_string.append("\"\n");
				}

				// close entity
				entity_string.append("}\n");
			}
		}

		return entity_string;
	}

	// build all entities and fix brushmodels
	std::string entities::buildAll_FixBrushmodels(const std::vector<Game::brushmodel_entity_s> &bmodel_list)
	{
		int entity_num = 1; // worldspawn is 0
		int submodel_num = 0;

		std::string basic_string;

		for (auto& entity : this->entities_)
		{
			std::string model = entity["model"];

			// if ent is a brushmodel/submodel
			if (!model.empty() && model[0] == '*')
			{
				// get the submodel index 
				const auto p_index = std::stoi(model.erase(0, 1));

				if (p_index < static_cast<int>(bmodel_list.size()))
				{
					const auto bmodel_side_count = static_cast<int>(bmodel_list[p_index].brush_sides.size());
					
					// skip submodel entity if we have less then 6 brushsides (we could also create a temp. cube at its origin)
					if (bmodel_side_count < 6)
					{
						continue;
					}

					// start submodel
					basic_string.append(utils::va("// submodel %d\n", submodel_num));
					basic_string.append("{\n");

					// write submodel keys
					for (auto& property : entity)
					{
						// do not write model/origin keys
						if (property.first == "model" || property.first == "origin")
						{
							continue;
						}

						basic_string.push_back('"');
						basic_string.append(property.first);
						basic_string.append("\" \"");
						basic_string.append(property.second);
						basic_string.append("\"\n");
					}

					// start submodel brush
					basic_string.append("{\n");

					for (auto bSide = 0; bSide < bmodel_side_count; bSide++)
					{
						basic_string.append(bmodel_list[p_index].brush_sides[bSide]);
					}

					// close submodel brush
					basic_string.append("}\n");

					// close submodel
					basic_string.append("}\n");

					submodel_num++;
				}
			}

			else
			{
				// start entity
				basic_string.append(utils::va("// entity %d\n", entity_num));
				basic_string.append("{\n");

				for (auto& property : entity)
				{
					basic_string.push_back('"');
					basic_string.append(property.first);
					basic_string.append("\" \"");
					basic_string.append(property.second);
					basic_string.append("\"\n");
				}

				// close entity
				basic_string.append("}\n");

				entity_num++;
			}
		}

		return basic_string;
	}

	// build all selected entities and fix brushmodels
	std::string entities::buildSelection_FixBrushmodels(const Game::boundingbox_s* sbox, const std::vector<Game::brushmodel_entity_s>& bmodel_list)
	{
		int entity_num = 1; // worldspawn is 0
		int submodel_num = 0;

		std::string entity_string;

		for (auto& entity : this->entities_)
		{
			std::string model = entity["model"];
			std::string origin = entity["origin"];

			// if ent is a brushmodel/submodel
			if (!model.empty() && model[0] == '*')
			{
				// get the submodel index 
				const auto p_index = std::stoi(model.erase(0, 1));

				if (p_index < static_cast<int>(bmodel_list.size()))
				{
					const auto bmodel_side_count = static_cast<int>(bmodel_list[p_index].brush_sides.size());

					// skip submodel entity if we have less then 6 brushsides (we could also create a temp. cube at its origin)
					if (bmodel_side_count < 6)
					{
						continue;
					}

					// start submodel
					entity_string.append(utils::va("// submodel %d\n", submodel_num));
					entity_string.append("{\n");

					// write submodel keys
					for (auto& property : entity)
					{
						// do not write model/origin keys
						if (property.first == "model" || property.first == "origin")
						{
							continue;
						}

						entity_string.push_back('"');
						entity_string.append(property.first);
						entity_string.append("\" \"");
						entity_string.append(property.second);
						entity_string.append("\"\n");
					}

					// start submodel brush
					entity_string.append("{\n");

					for (auto bside = 0; bside < bmodel_side_count; bside++)
					{
						entity_string.append(bmodel_list[p_index].brush_sides[bside]);
					}

					// close submodel brush
					entity_string.append("}\n");

					// close submodel
					entity_string.append("}\n");

					submodel_num++;
				}
			}

			else
			{
				float t_origin[3] = {0.0f, 0.0f, 0.0f};

				if (!sscanf_s(origin.c_str(), "%f %f %f", &t_origin[0], &t_origin[1], &t_origin[2]))
				{
					Game::Com_PrintMessage(0, utils::va("[!]: sscanf failed for entity %d", entity_num), 0);
				}

				if (utils::polylib::is_point_within_bounds(glm::to_vec3(t_origin), sbox->mins, sbox->maxs, 0.25f))
				{
					// start entity
					entity_string.append(utils::va("// entity %d\n", entity_num));
					entity_string.append("{\n");

					for (auto& property : entity)
					{
						entity_string.push_back('"');
						entity_string.append(property.first);
						entity_string.append("\" \"");
						entity_string.append(property.second);
						entity_string.append("\"\n");
					}

					// close entity
					entity_string.append("}\n");
					entity_num++;
				}
			}
		}

		return entity_string;
	}

	// only build worldspawn keys/values without opening/closing brackets
	std::string entities::build_worldspawn()
	{
		std::string entityString;

		for (auto& entity : this->entities_)
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

	std::vector<std::string> entities::get_models()
	{
		std::vector<std::string> models;

		for (auto& entity : this->entities_)
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

	std::vector<Game::brushmodel_entity_s> entities::get_brushmodels()
	{
		std::vector<Game::brushmodel_entity_s> bmodels;

		// geting the total clipmap size would prob. be better
		const uintptr_t leaf_brushes_start = reinterpret_cast<uintptr_t>(&*Game::cm->leafbrushNodes);
		const uintptr_t leaf_brushes_end = leaf_brushes_start + sizeof(Game::cLeafBrushNode_s) * (Game::cm->leafbrushNodesCount + Game::cm->numLeafBrushes); // wrong

		// first element is always empty because
		// the first submodel within the entsMap starts at 1 and we want to avoid subtracting - 1 everywhere 
		bmodels.emplace_back(Game::brushmodel_entity_s()); 

		for (auto& entity : this->entities_)
		{
			if (entity.contains("model"))
			{
				std::string model = entity["model"];
				std::string origin = entity["origin"];

				// if ent is a brushmodel/submodel
				if (!model.empty() && model[0] == '*' && !origin.empty())
				{
					auto curr_bmodel = Game::brushmodel_entity_s();

					// get the submodel index 
					const auto p_index = std::stoi(model.erase(0, 1));

					// the index should always match the size of our vector or we did something wrong
					if (p_index != (int)bmodels.size())
					{
						Game::Com_PrintMessage(0, utils::va("[Entities::getBrushModels]: Something went wrong while parsing submodels. (%d != %d)", p_index, bmodels.size()), 0);
					}

					if (p_index >= static_cast<int>(Game::cm->numSubModels))
					{
						Game::Com_PrintMessage(0, utils::va("[Entities::getBrushModels]: Something went wrong while parsing submodels. (%d >= %d numSubModels)", p_index, Game::cm->numSubModels), 0);
						break;
					}

					// assign indices and pointers to both the brush and the submodel
					curr_bmodel.cm_submodel_index = p_index;
					
					if (&Game::cm->cmodels[p_index])
					{
						curr_bmodel.cm_submodel = &Game::cm->cmodels[p_index];
					}

					// fix me daddy
					auto brush_index_ptr = Game::cm->leafbrushNodes[Game::cm->cmodels[p_index].leaf.leafBrushNode].data.leaf.brushes;
					curr_bmodel.cm_brush_index = 0;

					// this is giving me cancer
					if (Game::cm->cmodels[p_index].leaf.leafBrushNode != 0 && brush_index_ptr)
					{
						if ((uintptr_t)&*brush_index_ptr >= leaf_brushes_start && (uintptr_t) & *brush_index_ptr < leaf_brushes_end)
						{
							curr_bmodel.cm_brush_index = static_cast<int>(*Game::cm->leafbrushNodes[Game::cm->cmodels[p_index].leaf.leafBrushNode].data.leaf.brushes);
						}
						else
						{
							Game::Com_PrintMessage(0, utils::va("[Entities::getBrushModels]: Skipping faulty brush-index pointer at leafbrushNodes[%d].data.leaf.brushes ...\n", p_index), 0);
						}
						
						//currBModel.cmBrush = &Game::cm->brushes[*Game::cm->leafbrushNodes[Game::cm->cmodels[p_index].leaf.leafBrushNode].data.leaf.brushes];
						curr_bmodel.cm_brush = &Game::cm->brushes[curr_bmodel.cm_brush_index];

						// add the submodel index to the clipmap brush
						curr_bmodel.cm_brush->isSubmodel = true;
						curr_bmodel.cm_brush->cmSubmodelIndex = static_cast<std::int16_t>(p_index);
					}


					// save entity origin
					if (!sscanf_s(origin.c_str(), "%f %f %f", &curr_bmodel.cm_submodel_origin[0], &curr_bmodel.cm_submodel_origin[1], &curr_bmodel.cm_submodel_origin[2]))
					{
						Game::Com_PrintMessage(0, utils::va("[!]: sscanf failed for submodel %d", p_index), 0);
						curr_bmodel.cm_submodel_origin[0] = 0.0f;
						curr_bmodel.cm_submodel_origin[1] = 0.0f;
						curr_bmodel.cm_submodel_origin[2] = 0.0f;
					}

					bmodels.push_back(curr_bmodel);
				}
			}
		}

		return bmodels;
	}

	void entities::delete_worldspawn()
	{
		for (auto i = this->entities_.begin(); i != this->entities_.end();)
		{
			if (i->find("classname") != i->end())
			{
				std::string classname = (*i)["classname"];
				if (utils::starts_with(classname, "worldspawn"))
				{
					i = this->entities_.erase(i);
					continue;
				}
			}

			++i;
		}
	}

	void entities::delete_triggers()
	{
		for (auto i = this->entities_.begin(); i != this->entities_.end();)
		{
			if (i->contains("classname"))
			{
				const std::string classname = (*i)["classname"];
				if (utils::starts_with(classname, "trigger_"))
				{
					i = this->entities_.erase(i);
					continue;
				}
			}

			++i;
		}
	}

	void entities::delete_weapons(bool keepTurrets)
	{
		for (auto i = this->entities_.begin(); i != this->entities_.end();)
		{
			if (   i->contains("weaponinfo") 
				|| i->contains("targetname") && (*i)["targetname"] == "oldschool_pickup"s)
			{
				if (   !keepTurrets 
					|| !i->contains("classname") 
					|| (*i)["classname"] != "misc_turret"s)
				{
					i = this->entities_.erase(i);
					continue;
				}
			}

			++i;
		}
	}

	void entities::parse(std::string buffer)
	{
		int parse_state= 0;

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
					this->entities_.push_back(entity);
					entity.clear();
					break;
				}

				case '"':
				{
					if (parse_state == PARSE_AWAIT_KEY)
					{
						key.clear();
						parse_state = PARSE_READ_KEY;
					}
					else if (parse_state == PARSE_READ_KEY)
					{
						parse_state = PARSE_AWAIT_VALUE;
					}
					else if (parse_state == PARSE_AWAIT_VALUE)
					{
						value.clear();
						parse_state = PARSE_READ_VALUE;
					}
					else if (parse_state == PARSE_READ_VALUE)
					{
						entity[utils::str_to_lower(key)] = value;
						parse_state = PARSE_AWAIT_KEY;
					}
					else
					{
						throw std::runtime_error("Parsing error!");
					}
					break;
				}

				default:
				{
					if (parse_state == PARSE_READ_KEY)
					{
						key.push_back(character);
					}
					else if (parse_state == PARSE_READ_VALUE)
					{
						value.push_back(character);
					}

					break;
				}
			}
		}
	}
}
