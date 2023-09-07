#pragma once

#include "Game/GameLoop.h"
#include "Renderer/RenderLoop.h"

namespace vge
{
	inline class EngineLoop* GEngineLoop = nullptr;

	class EngineLoop
	{
	public:
		EngineLoop() = default;

		void Initialize();
		void Start();
		void Destroy();

		inline GameLoop& GetGameLoop() { return m_GameLoop; }
		inline RenderLoop& GetRenderLoop() { return m_RenderLoop; }

	public:
		float StartTime = 0.0f;
		float LastTime = 0.0f;
		float DeltaTime = 0.0f;

	private:
		void Tick();
		void UpdateDeltaTime();

	private:
		GameLoop m_GameLoop = {};
		RenderLoop m_RenderLoop = {};
	};

	inline EngineLoop* CreateEngineLoop()
	{
		if (GEngineLoop) return GEngineLoop;
		return (GEngineLoop = new EngineLoop());
	}

	inline bool DestroyEngineLoop()
	{
		if (!GEngineLoop) return false;
		GEngineLoop->Destroy();
		delete GEngineLoop;
		GEngineLoop = nullptr;
		return true;
	}
}