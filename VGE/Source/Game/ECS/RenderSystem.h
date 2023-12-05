#pragma once

#include "Renderer/RenderCommon.h"
#include "System.h"

namespace vge
{
	class Renderer;
	class Camera;
	class CommandBuffer;

	class RenderSystem : public System
	{
	public:
		RenderSystem() = default;
		void Initialize(Renderer* renderer, Camera* camera);
		void Tick(f32 deltaTime);

	private:
		Renderer* m_Renderer = nullptr;
		Camera* m_Camera = nullptr;
	};
}
