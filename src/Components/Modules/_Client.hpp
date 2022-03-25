#pragma once

namespace Components
{
	class _client final : public Component
	{
	public:
		_client();
		const char* getName() override { return "_client"; };
	};
}
