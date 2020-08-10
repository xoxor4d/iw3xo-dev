#pragma once

namespace Components
{
	class RB_ShaderOverlays : public Component
	{
	public:
		RB_ShaderOverlays();
		~RB_ShaderOverlays();
		const char* getName() override { return "RB_ShaderOverlays"; };

		static void Register_StringDvars();

	private:
	};
}
