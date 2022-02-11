#pragma once

namespace Components
{
	class _Map : public Component
	{
	public:
		_Map();
		~_Map();
		const char* getName() override { return "_Map"; };

		static	void OnLoad();
		static	void OnUnload();

		inline static bool mpsp_is_sp_map = false;
		inline static char* mpsp_mapents_original = nullptr;

	private:
	};
}
