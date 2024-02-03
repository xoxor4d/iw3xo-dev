#pragma once

namespace components
{
	class rtx_fixed_function final : public component
	{
	public:
		rtx_fixed_function();
		const char* get_name() override { return "rtx_fixed_function"; };
	};
}
