#pragma once

namespace components
{
	class compass final : public component
	{

	public:

		compass();
		const char* get_name() override { return "compass"; };

		static void main();
	};
}