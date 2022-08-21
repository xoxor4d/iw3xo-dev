#pragma once

namespace components
{
	class mvm final : public component
	{
	public:
		mvm();
		const char* get_name() override { return "mvm"; };
	};
}
