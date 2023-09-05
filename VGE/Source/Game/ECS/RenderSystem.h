#pragma once

#include "SystemManager.h"

namespace vge
{
	class Renderer;

	class RenderSystem : public System
	{
	public:
		RenderSystem() = default;
		void Initialize(Renderer* renderer);
		void Tick(float deltaTime);

	private:
		Renderer* m_Renderer = nullptr;
	};
}

