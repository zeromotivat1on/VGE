#pragma once

#include "ECS/System.h"

namespace vge
{
	class Camera;

	class CameraSystem : public System
	{
	public:
		CameraSystem() = default;

		void Initialize(Camera* camera);
		void Tick(f32 deltaTime);

	private:
		Camera* m_Camera = nullptr;
	};
}
