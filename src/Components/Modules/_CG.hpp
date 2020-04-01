#pragma once

namespace Components
{
	class _CG : public Component
	{
	public:
		_CG();
		~_CG();
		const char* getName() override { return "_CG"; };

		static char* GetFontForStyle(int fontStyle);

	private:
	};
}
