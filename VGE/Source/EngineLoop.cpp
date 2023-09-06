#include "EngineLoop.h"
#include "Application.h"
#include "Profiling.h"
#include "Renderer/RenderCommon.h"

static inline void IncrementAppFrame() { ++vge::GAppFrame; }

void vge::EngineLoop::Initialize()
{
	m_GameLoop.Initialize();
	m_RenderLoop.Initialize();
}

void vge::EngineLoop::Start()
{
	StartTime = static_cast<float>(glfwGetTime());

	while (!GApplication->ShouldClose())
	{
		SCOPE_TIMER("Tick");
		UpdateTime();
		Tick();
		IncrementAppFrame();
	}

	const float loopDurationTime = LastTime - StartTime;
	LOG(Log, "Engine loop stats:");
	LOG(Log, " Duration: %.2fs", loopDurationTime);
	LOG(Log, " Iterations: %d", GAppFrame);
	LOG(Log, " Average FPS: %.2f", static_cast<float>(GAppFrame) / loopDurationTime);
}

void vge::EngineLoop::Tick()
{
	// TODO: make separate threads for game and render.
	m_GameLoop.Tick(DeltaTime);
	m_RenderLoop.Tick(DeltaTime);
	// TODO: when separate threads are done - implement their sync.
}

void vge::EngineLoop::Destroy()
{
	m_RenderLoop.Destroy();
	m_GameLoop.Destroy();
}

void vge::EngineLoop::UpdateTime()
{
	const float nowTime = static_cast<float>(glfwGetTime());
	DeltaTime = nowTime - LastTime;
	LastTime = nowTime;
}
