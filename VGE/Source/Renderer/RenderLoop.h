#pragma once

#include "ECS/RenderSystem.h"

namespace vge
{
	class RenderLoop
	{
	public:
		RenderLoop() = default;

		void Initialize();
		void Tick(float deltaTime);
		void Destroy();

	private:
		std::shared_ptr<RenderSystem> m_RenderSystem = nullptr;
	};
}
