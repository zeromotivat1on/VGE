#pragma once

#include "ECS/GameSystem.h"

namespace vge
{
	class GameLoop
	{
	public:
		GameLoop() = default;

		void Initialize();
		void Tick(f32 deltaTime);
		void Destroy();

		inline GameSystem* GetGameSystem() { return m_GameSystem.get(); }

	private:
		void RegisterDefaultComponents() const;
		void RegisterGameSystem();

	private:
		std::shared_ptr<GameSystem> m_GameSystem = nullptr;
	};
}
