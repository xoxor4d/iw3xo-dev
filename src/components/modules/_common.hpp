#pragma once

namespace components
{
	class _common final : public component
	{
	public:
		_common();
		const char* get_name() override { return "_common"; };

	private:
		void db_realloc_entry_pool();
	};
}
