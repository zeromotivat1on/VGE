#include "RenderLoop.h"
#include "Application.h"
#include "Renderer.h"
#include "Window.h"
#include "ECS/Coordinator.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"

void vge::RenderLoop::Initialize()
{
	ENSURE(GWindow);

	CreateDevice(GWindow);
	ENSURE(GDevice);
	GDevice->Initialize();

	CreateRenderer(*GDevice);
	ENSURE(GRenderer);
	GRenderer->Initialize();

	RegisterDefaultComponents();
	RegisterRenderSystem();
}

void vge::RenderLoop::Tick(float deltaTime)
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
	GCoordinator->RegisterComponent<RenderComponent>();
}

void vge::RenderLoop::RegisterRenderSystem()
{
	m_RenderSystem = GCoordinator->RegisterSystem<RenderSystem>();
	ENSURE(m_RenderSystem);
	m_RenderSystem->Initialize(GRenderer);

	Signature signature;
	signature.set(GCoordinator->GetComponentType<RenderComponent>());
	signature.set(GCoordinator->GetComponentType<TransformComponent>());
	GCoordinator->SetSystemSignature<RenderSystem>(signature);
}
