#pragma once

namespace Components
{
	class _map final : public Component
	{
	public:
		_map(); ~_map() override;
		const char* getName() override { return "_map"; };

		static	void on_load();
		static	void on_unload();

		inline static bool mpsp_is_sp_map = false;
		inline static char* mpsp_mapents_original = nullptr;

	private:
	};
}
