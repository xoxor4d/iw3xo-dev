#pragma once

namespace Components
{
	class mvm final : public Component
	{
	public:
		mvm();
		const char* getName() override { return "mvm"; };
        
		static void avidemo_streams();
	};
}
