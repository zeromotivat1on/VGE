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

	GCoordinator->RegisterComponent<RenderComponent>();
	GCoordinator->RegisterComponent<TransformComponent>();
}

void vge::GameLoop::Tick(float deltaTime)
{
	GWindow->PollEvents();
}

void vge::GameLoop::Destroy()
{
	DestroyCoordinator();
	DestroyWindow();
}
