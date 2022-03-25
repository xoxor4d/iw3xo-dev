#pragma once

namespace Components
{
	class _common final : public Component
	{
	public:
		_common();
		const char* getName() override { return "_common"; };

	private:
		void db_realloc_entry_pool();
	};
}
