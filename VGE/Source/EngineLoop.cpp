#include "EngineLoop.h"
#include "Application.h"
#include "ECS/Coordinator.h"
#include "Profiling.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderCommon.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"

static inline void IncrementAppFrame() { ++vge::GAppFrame; }

void vge::EngineLoop::Initialize()
{
	m_GameLoop.Initialize();
	m_RenderLoop.Initialize();

	// Add 1 test entity.
	const Entity entity = GCoordinator->CreateEntity();

	{
		m_GameLoop.GetGameSystem()->Add(entity);
		GCoordinator->AddComponent(entity, TransformComponent());
	}

	{
		m_RenderLoop.GetRenderSystem()->Add(entity);

		RenderComponent renderComponent = {};
		renderComponent.ModelId = GRenderer->CreateModel("Models/cottage/Cottage_FREE.obj");
		GCoordinator->AddComponent(entity, renderComponent);
	}
}

void vge::EngineLoop::Start()
{
	StartTime = static_cast<float>(glfwGetTime());

	while (!GApplication->ShouldClose())
	{
		SCOPE_TIMER("Tick");
		UpdateDeltaTime();
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

void vge::EngineLoop::UpdateDeltaTime()
{
	const float nowTime = static_cast<float>(glfwGetTime());
	DeltaTime = nowTime - LastTime;
	LastTime = nowTime;
}
