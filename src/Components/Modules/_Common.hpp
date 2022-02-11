#pragma once

namespace Components
{
	class _Common : public Component
	{
	public:
		_Common();
		~_Common();
		const char* getName() override { return "_Common"; };

	private:
		void db_realloc_entry_pool();
	};
}
