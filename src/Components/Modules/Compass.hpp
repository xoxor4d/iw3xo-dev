#pragma once

namespace Components
{
	class Compass : public Component
	{

	public:

		Compass();
		~Compass();
		const char* getName() override { return "Compass"; };

		static void main();

	private:
	};
}