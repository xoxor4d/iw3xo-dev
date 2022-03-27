#pragma once

namespace components
{
	class console final : public component
	{
	public:
		console();
		const char* get_name() override { return "console"; };

		static void		check_resize();
		static int		cmd_argc();
	};
}
