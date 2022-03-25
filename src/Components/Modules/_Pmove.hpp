#pragma once

namespace Components
{
	class _pmove final : public Component
	{
	public:
		_pmove();
		const char* getName() override { return "_pmove"; };

		static void draw_debug();
	};
}
