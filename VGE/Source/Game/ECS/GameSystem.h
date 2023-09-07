#pragma once

#include "System.h"

namespace vge
{
	class GameSystem : public System
	{
	public:
		GameSystem() = default;
		void Initialize();
		void Tick(float deltaTime);
	};
}
