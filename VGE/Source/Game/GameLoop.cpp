#include "GameLoop.h"
#include "Application.h"
#include "ECS/Coordinator.h"
#include "Game/Camera.h"
#include "Renderer/Window.h"
#include "Components/InputComponent.h"
#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"

void vge::GameLoop::Initialize()
{
	const ApplicationSpecs& appSpecs = GApplication->Specs;
	CreateWindow(appSpecs.Window.Name, appSpecs.Window.Width, appSpecs.Window.Height);
	ENSURE(GWindow);
	GWindow->Initialize();

	CreateCoordinator();
	ENSURE(GCoordinator);
	GCoordinator->Initialize();

	RegisterDefaultComponents();
	RegisterSystems();
}

void vge::GameLoop::Tick(f32 deltaTime)
{
	GWindow->PollEvents();

	m_InputSystem->Tick(deltaTime);
	m_CameraSystem->Tick(deltaTime);
}

void vge::GameLoop::Destroy()
{
	DestroyCoordinator();
	DestroyWindow();
}

void vge::GameLoop::RegisterDefaultComponents() const
{
	GCoordinator->RegisterComponent<TransformComponent>();
	GCoordinator->RegisterComponent<CameraComponent>();
	GCoordinator->RegisterComponent<InputComponent>();
}

void vge::GameLoop::RegisterSystems()
{
	// Input system.
	{
		m_InputSystem = RegisterSystem<InputSystem>();
		ENSURE(m_InputSystem);

		Signature signature;
		signature.set(GCoordinator->GetComponentType<TransformComponent>());
		signature.set(GCoordinator->GetComponentType<InputComponent>());
		GCoordinator->SetSystemSignature<InputSystem>(signature);

		m_InputSystem->Initialize();
	}

	// Camera system.
	{
		m_CameraSystem = RegisterSystem<CameraSystem>();
		ENSURE(m_CameraSystem);

		Signature signature;
		signature.set(GCoordinator->GetComponentType<TransformComponent>());
		signature.set(GCoordinator->GetComponentType<CameraComponent>());
		GCoordinator->SetSystemSignature<CameraSystem>(signature);

		m_CameraSystem->Initialize(GCamera);
	}
	
	// Add 1 camera viewer entity.
	{
		const Entity entity = GCoordinator->CreateEntity();
		AddComponent(entity, TransformComponent());
		AddComponent(entity, InputComponent(true));
		AddComponent(entity, CameraComponent(true));

		//m_GameSystem->Add(entity);
		m_InputSystem->Add(entity);
		m_CameraSystem->Add(entity);
	}

}
