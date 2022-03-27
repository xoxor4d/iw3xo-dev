#pragma once

namespace components
{
	class postfx_shaders final : public component
	{
	public:
		postfx_shaders();
		const char* get_name() override { return "postfx_shaders"; };

		static void register_dvars();
	};
}
