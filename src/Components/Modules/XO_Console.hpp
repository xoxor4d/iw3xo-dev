#pragma once

namespace Components
{
	class XO_Console : public Component
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
