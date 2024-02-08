#include "std_include.hpp"

namespace components
{
	std::vector<std::string> flags::enabled_flags;

	bool flags::has_flag(const std::string& flag)
	{
		parse_flags();

		for (const auto& entry : enabled_flags)
		{
			if (utils::str_to_lower(entry) == utils::str_to_lower(flag))
			{
				return true;
			}
		}

		return false;
	}

	void flags::parse_flags()
	{
		static auto flags_parsed = false;
		if (flags_parsed)
		{
			return;
		}

		// only parse flags once
		flags_parsed = true;
		int num_args;
		auto* const argv = CommandLineToArgvW(GetCommandLineW(), &num_args);

		if (argv)
		{
			for (auto i = 0; i < num_args; ++i)
			{
				std::wstring wFlag(argv[i]);
				if (wFlag[0] == L'-')
				{
					wFlag.erase(wFlag.begin());
					enabled_flags.emplace_back(utils::convert_wstring(wFlag));
				}
			}

			LocalFree(argv);
		}
	}
}