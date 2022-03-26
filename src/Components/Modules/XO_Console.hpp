#pragma once

namespace components
{
	class XO_Console : public component
	{
	public:
		XO_Console();
		~XO_Console();
		const char* getName() override { return "XO_Console"; };

		static void		xo_con_CheckResize();
		static int		Cmd_Argc();

	private:
	};
}
