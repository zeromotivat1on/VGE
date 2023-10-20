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
		void Tick(float deltaTime);

	private:
		InputController m_InputController = {};
	};
}
