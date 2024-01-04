#include "EngineLoop.h"
#include "Application.h"
#include "Profiling.h"
#include "Game/Camera.h"
#include "ECS/Coordinator.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderCommon.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"

static inline void IncrementAppFrame() 
{ 
	++vge::GAppFrame; 
}

void vge::EngineLoop::Initialize()
{
	GCamera = new Camera();

	m_GameLoop.Initialize();
	m_RenderLoop.Initialize();
}

void vge::EngineLoop::Start()
{
	m_StartTime = static_cast<f32>(glfwGetTime());

	while (!GApplication->ShouldClose())
	{
		//SCOPE_TIMER("Tick");
		UpdateDeltaTime();
		Tick();
		IncrementAppFrame();
	}

	const f32 loopDurationTime = m_LastTime - m_StartTime;
	LOG(Log, "Engine loop stats:");
	LOG(Log, " Duration: %.2fs", loopDurationTime);
	LOG(Log, " Frames: %d", GAppFrame);
	LOG(Log, " Average FPS: %.2f", static_cast<f32>(GAppFrame) / loopDurationTime);
}

void vge::EngineLoop::Tick()
{
	// TODO: make separate threads for game and render.
	m_GameLoop.Tick(m_DeltaTime);
	m_RenderLoop.Tick(m_DeltaTime);
	// TODO: when separate threads are done - implement their sync.
}

void vge::EngineLoop::Destroy()
{
	m_RenderLoop.Destroy();
	m_GameLoop.Destroy();
	
	if (GCamera)
	{
		delete GCamera;
	}
}

void vge::EngineLoop::UpdateDeltaTime()
{
	const f32 nowTime = static_cast<f32>(glfwGetTime());
	m_DeltaTime = nowTime - m_LastTime;
	m_LastTime = nowTime;
}
