#pragma once

namespace components
{
	class patches final : public component
	{
	public:
		patches();
		const char* getName() override { return "patches"; };
	};
}
