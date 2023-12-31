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
		inline f32 GetDeltaTime() const { return m_DeltaTime; }

	private:
		void Tick();
		void UpdateDeltaTime();

	private:
		GameLoop m_GameLoop = {};
		RenderLoop m_RenderLoop = {};

		f32 m_StartTime = 0.0f;
		f32 m_LastTime = 0.0f;
		f32 m_DeltaTime = 0.0f;
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