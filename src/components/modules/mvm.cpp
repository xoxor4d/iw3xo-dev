#include "std_include.hpp"

namespace components
{
	mvm::mvm()
	{
		dvars::load_iw3mvm = game::Dvar_RegisterBool(
			/* name		*/ "load_iw3mvm",
			/* desc		*/ "load iw3mvm on startup",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);

		// fix out of memory error when trying to take screenshots with R_TakeScreenshot when using heavy mods
		utils::hook::set<BYTE>(0x60E624 + 1, 0x2); // CreateOffscreenPlainSurface D3DPOOL_SCRATCH -> D3DPOOL_SYSTEMMEM
	}
}