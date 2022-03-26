#pragma once

namespace components
{
	class mvm final : public component
	{
	public:
		mvm();
		const char* getName() override { return "mvm"; };
        
		static void avidemo_streams();
	};
}
