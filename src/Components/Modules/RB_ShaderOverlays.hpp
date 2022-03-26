#pragma once

namespace components
{
	class RB_ShaderOverlays : public component
	{
	public:
		RB_ShaderOverlays();
		~RB_ShaderOverlays();
		const char* getName() override { return "RB_ShaderOverlays"; };

		static void register_dvars();

	private:
	};
}
