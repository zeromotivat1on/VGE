#pragma once

#include "ECS/Systems/RenderSystem.h"

namespace vge
{
	class RenderLoop
	{
	public:
		RenderLoop() = default;

		void Initialize();
		void Tick(f32 deltaTime);
		void Destroy();

		inline RenderSystem* GetRenderSystem() { return m_RenderSystem.get(); }

	private:
		void RegisterDefaultComponents() const;
		void RegisterSystems();

	private:
		std::shared_ptr<RenderSystem> m_RenderSystem = nullptr;
	};
}
