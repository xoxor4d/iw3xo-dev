#pragma once

namespace components
{
	class movement final : public component
	{
	public:
		movement();
		const char* getName() override { return "movement"; };
	};
}
