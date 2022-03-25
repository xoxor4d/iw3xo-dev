#pragma once

namespace Components
{
	class compass final : public Component
	{

	public:

		compass();
		const char* getName() override { return "compass"; };

		static void main();
	};
}