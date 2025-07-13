#pragma once

namespace components
{
	typedef void* (__cdecl* Scr_GetFunction_hook)(const char** name, int* type);
	inline Scr_GetFunction_hook Scr_GetFunction_f = (Scr_GetFunction_hook)(0x4D8470);

	typedef void(*xfunction_t)();

	/*typedef struct scr_function_s
	{
		scr_function_s*	next;
		char* name;
		xfunction_t	function;
		bool developer;
	} scr_function_t;*/

	typedef int scr_entref_t;

	class gscr_methods final : public component
	{
	public:
		gscr_methods();
		const char* get_name() override { return "gscr_methods"; };
	};
}
