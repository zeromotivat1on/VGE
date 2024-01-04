#pragma once

#include "ECS/Systems/InputSystem.h"
#include "ECS/Systems/CameraSystem.h"

namespace vge
{
	class GameLoop
	{
	public:
		GameLoop() = default;

		void Initialize();
		void Tick(f32 deltaTime);
		void Destroy();

	private:
		void RegisterDefaultComponents() const;
		void RegisterSystems();

	private:
		std::shared_ptr<InputSystem> m_InputSystem = nullptr;
		std::shared_ptr<CameraSystem> m_CameraSystem = nullptr;
	};
}
