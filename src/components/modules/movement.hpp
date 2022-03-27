#pragma once

namespace components
{
	class movement final : public component
	{
	public:
		movement();
		const char* get_name() override { return "movement"; };
	};
}
