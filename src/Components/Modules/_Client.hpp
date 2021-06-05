#pragma once

namespace Components
{
	class _Client : public Component
	{
	public:
		_Client();
		~_Client();
		const char* getName() override { return "_Client"; };

		static	void OnDisconnect();

	private:
	};
}
