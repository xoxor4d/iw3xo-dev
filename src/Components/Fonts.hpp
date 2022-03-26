#pragma once

namespace components
{
    enum FONTS
	{
		BOLD_LARGER		= 0,
		BOLD_LARGE		= 1,
		BOLD			= 2,
		REGULAR_LARGE	= 3,
		REGULAR			= 4,
	};

	namespace fonts
    {
        extern const unsigned int opensans_regular_compressed_size;
		extern const unsigned int opensans_regular_compressed_data[75360 / 4];

		extern const unsigned int opensans_bold_compressed_size;
		extern const unsigned int opensans_bold_compressed_data[79728 / 4];
    }
}
