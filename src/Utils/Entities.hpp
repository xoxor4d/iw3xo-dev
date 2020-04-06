#pragma once

namespace Utils
{
	class Entities
	{
	public:
		Entities() {};
		Entities(const char* string, size_t lenPlusOne) : Entities(std::string(string, lenPlusOne - 1)) {}
		Entities(std::string buffer) : Entities() { this->parse(buffer); };
		Entities(const Entities &obj) : entities(obj.entities) {};

		std::string buildAll();
		std::string buildAll_FixBrushmodels(const std::vector<Game::brushmodelEnt_t>& bModelList);
		std::string buildWorldspawnKeys();

		std::vector<std::string> getModels();
		std::vector<Game::brushmodelEnt_t> getBrushModels();

		void deleteWorldspawn();
		void deleteTriggers();
		void deleteWeapons(bool keepTurrets);

	private:
		enum
		{
			PARSE_AWAIT_KEY,
			PARSE_READ_KEY,
			PARSE_AWAIT_VALUE,
			PARSE_READ_VALUE,
		};

		std::vector<std::unordered_map<std::string, std::string>> entities;
		void parse(std::string buffer);
	};
}
