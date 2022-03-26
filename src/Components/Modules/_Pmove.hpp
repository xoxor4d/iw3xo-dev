#pragma once

namespace components
{
	class _pmove final : public component
	{
	public:
		_pmove();
		const char* getName() override { return "_pmove"; };

		static void draw_debug();
	};
}
