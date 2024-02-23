#include "RenderLoop.h"
#include "Application.h"
#include "Renderer.h"
#include "Window.h"
#include "Game/Camera.h"
#include "ECS/Coordinator.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"

void vge::RenderLoop::Initialize()
{
	ENSURE(GWindow);

	CreateDevice(GWindow);
	ENSURE(GDevice);
	GDevice->Initialize();

	CreateRenderer(GDevice);
	ENSURE(GRenderer);
	GRenderer->Initialize();

	RegisterDefaultComponents();
	RegisterSystems();
}

void vge::RenderLoop::Tick(f32 deltaTime)
{
	m_RenderSystem->Tick(deltaTime);
}

void vge::RenderLoop::Destroy()
{
	DestroyRenderer();
	DestroyDevice();
}

void vge::RenderLoop::RegisterDefaultComponents() const
{
	ecs::RegisterComponent<RenderComponent>();
}

void vge::RenderLoop::RegisterSystems()
{
	m_RenderSystem = ecs::RegisterSystem<RenderSystem>();
	ENSURE(m_RenderSystem);

	// Initialize default camera.
	{
		GCamera->ShouldInvertProjectionYAxis(true);
		GCamera->SetViewDirection(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		GCamera->SetPerspectiveProjection(glm::radians(45.0f), GRenderer->GetSwapchainAspectRatio(), 0.001f, 100000.0f);
	}

	m_RenderSystem->Initialize(GRenderer, GCamera);

	Signature signature;
	signature.set(ecs::GetComponentType<RenderComponent>());
	signature.set(ecs::GetComponentType<TransformComponent>());
	ecs::SetSystemSignature<RenderSystem>(signature);

	// Add 1 test renderable entity.
	{
		const Entity entity = ecs::CreateEntity();
		ecs::AddComponent(entity, TransformComponent());
		ecs::AddComponent(entity, RenderComponent(GRenderer->CreateModel("Models/male.obj")));
		m_RenderSystem->Add(entity);
	}
}
