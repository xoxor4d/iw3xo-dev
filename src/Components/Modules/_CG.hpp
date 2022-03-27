#pragma once

namespace components
{
	class _cg final : public component
	{
	public:
		_cg();
		const char* get_name() override { return "_cg"; };

		static const char* get_font_for_style(int font_style);
	};
}
