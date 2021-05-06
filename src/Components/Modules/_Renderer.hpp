#pragma once

namespace Components
{
	class _Renderer : public Component
	{
	public:
		_Renderer();
		~_Renderer();
		const char* getName() override { return "_Renderer"; };

		static void R_RegisterBufferDvars();
	private:
	};
}
