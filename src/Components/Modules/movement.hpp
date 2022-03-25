#pragma once

namespace Components
{
	class movement final : public Component
	{
	public:
		movement();
		const char* getName() override { return "movement"; };
	};
}
