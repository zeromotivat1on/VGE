#include "GameLoop.h"
#include "Application.h"
#include "ECS/Coordinator.h"
#include "Renderer/Window.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"

void vge::GameLoop::Initialize()
{
	const ApplicationSpecs appSpecs = GApplication->Specs;
	CreateWindow(appSpecs.Window.Name, appSpecs.Window.Width, appSpecs.Window.Height);
	ENSURE(GWindow);
	GWindow->Initialize();

	CreateCoordinator();
	ENSURE(GCoordinator);
	GCoordinator->Initialize();

	RegisterDefaultComponents();
	RegisterGameSystem();
}

void vge::GameLoop::Tick(float deltaTime)
{
	GWindow->PollEvents();
	m_GameSystem->Tick(deltaTime);
}

void vge::GameLoop::Destroy()
{
	DestroyCoordinator();
	DestroyWindow();
}

void vge::GameLoop::RegisterDefaultComponents() const
{
	GCoordinator->RegisterComponent<TransformComponent>();
}

void vge::GameLoop::RegisterGameSystem()
{
	m_GameSystem = GCoordinator->RegisterSystem<GameSystem>();
	ENSURE(m_GameSystem);
	m_GameSystem->Initialize();

	Signature signature;
	signature.set(GCoordinator->GetComponentType<TransformComponent>());
	GCoordinator->SetSystemSignature<GameSystem>(signature);
}
