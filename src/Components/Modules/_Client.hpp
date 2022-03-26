#pragma once

namespace components
{
	class _client final : public component
	{
	public:
		_client();
		const char* getName() override { return "_client"; };
	};
}
