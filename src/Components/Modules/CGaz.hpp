#pragma once

namespace Components
{
	class CGaz : public Component
	{
	public:
		static void draw_cgaz();
		CGaz();
		~CGaz();
		const char* getName() override { return "CGaz"; };
	private:
	};
}