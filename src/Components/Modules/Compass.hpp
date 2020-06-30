#pragma once

namespace Components
{
	class Compass : public Component
	{
	public:
		static void draw_compass();
		Compass();
		~Compass();
		const char* getName() override { return "Compass"; };
	private:
	};
}