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

		inline RenderSystem* GetRenderSystem() { return m_RenderSystem.get(); }

	private:
		void RegisterDefaultComponents() const;
		void RegisterRenderSystem();

	private:
		std::shared_ptr<RenderSystem> m_RenderSystem = nullptr;
	};
}
