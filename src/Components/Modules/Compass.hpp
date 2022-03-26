#pragma once

namespace components
{
	class compass final : public component
	{

	public:

		compass();
		const char* getName() override { return "compass"; };

		static void main();
	};
}