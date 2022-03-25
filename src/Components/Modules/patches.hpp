#pragma once

namespace Components
{
	class patches final : public Component
	{
	public:
		patches();
		const char* getName() override { return "patches"; };
	};
}
