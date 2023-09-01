#pragma once

#include "Game/GameLoop.h"
#include "Renderer/RenderLoop.h"

namespace vge
{
	class EngineLoop
	{
	public:
		EngineLoop() = default;

		void Initialize();
		void Start();
		void Destroy();

	public:
		float StartTime = 0.0f;
		float LastTime = 0.0f;
		float DeltaTime = 0.0f;

	private:
		void Tick();
		void UpdateTime();

	private:
		GameLoop m_GameLoop = {};
		RenderLoop m_RenderLoop = {};
	};
}