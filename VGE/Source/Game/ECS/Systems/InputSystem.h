#pragma once

#include "ECS/System.h"
#include "InputController.h"

namespace vge
{
	class InputSystem : public System
	{
	public:
		InputSystem() = default;

		void Initialize();
		void Tick(f32 deltaTime);

	private:
		InputController m_InputController = {};
	};
}