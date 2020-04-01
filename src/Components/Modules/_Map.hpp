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

	private:
	};
}
