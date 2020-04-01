#pragma once

namespace Components
{
	typedef void(*xfunction_t)();

	typedef struct scr_function_s
	{
		struct scr_function_s	*next;
		char			*name;
		xfunction_t		function;
		bool			developer;
	} scr_function_t;

	typedef int scr_entref_t;

	class GScr_Methods : public Component
	{
	public:
		GScr_Methods();
		~GScr_Methods();
		const char* getName() override { return "GScr_Methods"; };

	private:
	};
}
