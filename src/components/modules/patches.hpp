#pragma once

namespace components
{
	class patches final : public component
	{
	public:
		patches();
		const char* get_name() override { return "patches"; };
	};
}
