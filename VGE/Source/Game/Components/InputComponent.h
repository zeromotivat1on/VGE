#pragma once

namespace vge
{
	struct InputComponent
	{
	public:
		InputComponent() = default;
		InputComponent(bool controllable) : m_Controllable(controllable) {}

		// Whether entity that holds this component can be controlled by player.
		inline bool IsControllable() const { return m_Controllable; }

	private:
		bool m_Controllable = false;
	};
}
