#pragma once

namespace vge
{
	struct CameraComponent
	{
	public:
		CameraComponent() = default;
		CameraComponent(bool observer) : m_Observer(observer) {}

		// Whether entity that holds this component should be used as observer.
		inline bool ShouldObserve() const { return m_Observer; }

	private:
		bool m_Observer = false;
	};
}
