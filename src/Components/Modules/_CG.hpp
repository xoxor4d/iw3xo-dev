#pragma once

namespace Components
{
	class _cg final : public Component
	{
	public:
		_cg();
		const char* getName() override { return "_cg"; };

		static const char* get_font_for_style(int font_style);
	};
}
