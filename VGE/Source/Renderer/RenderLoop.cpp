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
	RegisterRenderSystem();
}

void vge::RenderLoop::Tick(float deltaTime)
{
	GCamera->SetPerspectiveProjection(glm::radians(45.0f), GRenderer->GetSwapchainAspectRatio(), 0.1f, 1000.0f);
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

	// Initialize default camera.
	{
		GCamera->ShouldInvertProjectionYAxis(true);
		GCamera->SetViewDirection(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//GCamera->SetViewTarget(glm::vec3(-30.0f, -30.0f, 30.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		//GCamera->SetOrthographicProjection(-20.0f, 20.0f, -20.0f, 20.0f, -10000.0f, 10000.0f);
	}

	m_RenderSystem->Initialize(GRenderer, GCamera);

	Signature signature;
	signature.set(GCoordinator->GetComponentType<RenderComponent>());
	signature.set(GCoordinator->GetComponentType<TransformComponent>());
	GCoordinator->SetSystemSignature<RenderSystem>(signature);
}
