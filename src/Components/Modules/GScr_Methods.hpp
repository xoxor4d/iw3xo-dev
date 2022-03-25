#pragma once

namespace Components
{
	typedef void(*xfunction_t)();

	typedef struct scr_function_s
	{
		scr_function_s*	next;
		char* name;
		xfunction_t	function;
		bool developer;
	} scr_function_t;

	typedef int scr_entref_t;

	class gscr_methods final : public Component
	{
	public:
		gscr_methods();
		const char* getName() override { return "gscr_methods"; };
	};
}
