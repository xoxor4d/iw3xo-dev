#pragma once

namespace components
{
	class flags : public component
	{
	public:
		static bool has_flag(const std::string& flag);

	private:
		static std::vector<std::string> enabled_flags;

		static void parse_flags();
	};
}