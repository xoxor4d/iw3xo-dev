#pragma once

namespace Components
{
	class Mvm : public Component
	{
	public:
		Mvm();
		~Mvm();
		const char* getName() override { return "Mvm"; };
        
		static void avidemo_streams();

	private:
	};
}
