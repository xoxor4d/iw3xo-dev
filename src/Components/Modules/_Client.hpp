#pragma once

namespace components
{
	class _client final : public component
	{
	public:
		_client();
		const char* get_name() override { return "_client"; };
	};
}
