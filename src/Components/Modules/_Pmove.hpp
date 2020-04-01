#pragma once

namespace Components
{
	class _Pmove : public Component
	{
	public:
		_Pmove();
		~_Pmove();
		const char* getName() override { return "_Pmove"; };

	private:
	};
}
