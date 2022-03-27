#pragma once

namespace utils
{
	class entities
	{
	public:
		entities() {};
		entities(const char* string, size_t lenPlusOne) : entities(std::string(string, lenPlusOne - 1)) {}
		entities(std::string buffer) : entities() { this->parse(buffer); };
		entities(const entities &obj) : entities_(obj.entities_) {};

		std::string build_all();
		std::string buildAll_FixBrushmodels(const std::vector<game::brushmodel_entity_s>& bmodel_list);
		std::string buildSelection_FixBrushmodels(const game::boundingbox_s* sbox, const std::vector<game::brushmodel_entity_s>& bmodel_list);
		std::string build_all_script_structs();
		std::string build_worldspawn();

		std::vector<std::string> get_models();
		std::vector<game::brushmodel_entity_s> get_brushmodels();

		void delete_worldspawn();
		void delete_triggers();
		void delete_weapons(bool keepTurrets);

	private:
		enum
		{
			PARSE_AWAIT_KEY,
			PARSE_READ_KEY,
			PARSE_AWAIT_VALUE,
			PARSE_READ_VALUE,
		};

		std::vector<std::unordered_map<std::string, std::string>> entities_;
		void parse(std::string buffer);
	};
}
