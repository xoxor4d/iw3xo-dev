#pragma once

namespace Components
{
	class _Pmove : public Component
	{
	public:
		_Pmove();
		~_Pmove();
		const char* getName() override { return "_Pmove"; };

		static void PM_DrawDebug();

	private:
	};
}
