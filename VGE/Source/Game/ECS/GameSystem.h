#pragma once

#include "System.h"
#include "InputController.h"

namespace vge
{
	class GameSystem : public System
	{
	public:
		GameSystem() = default;

		void Initialize();
		void Tick(f32 deltaTime);

	private:
		InputController m_InputController = {};
	};
}
