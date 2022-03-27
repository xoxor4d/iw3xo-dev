#pragma once

namespace components
{
	class _pmove final : public component
	{
	public:
		_pmove();
		const char* get_name() override { return "_pmove"; };

		static void draw_debug();
	};
}
