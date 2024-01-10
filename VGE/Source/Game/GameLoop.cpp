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

	ecs::CreateCoordinator();
	ENSURE(ecs::GCoordinator);
	ecs::GCoordinator->Initialize();

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
	ecs::DestroyCoordinator();
	DestroyWindow();
}

void vge::GameLoop::RegisterDefaultComponents() const
{
	ecs::RegisterComponent<TransformComponent>();
	ecs::RegisterComponent<CameraComponent>();
	ecs::RegisterComponent<InputComponent>();
}

void vge::GameLoop::RegisterSystems()
{
	// Input system.
	{
		m_InputSystem = ecs::RegisterSystem<InputSystem>();
		ENSURE(m_InputSystem);

		Signature signature;
		signature.set(ecs::GetComponentType<TransformComponent>());
		signature.set(ecs::GetComponentType<InputComponent>());
		ecs::SetSystemSignature<InputSystem>(signature);

		m_InputSystem->Initialize();
	}

	// Camera system.
	{
		m_CameraSystem = ecs::RegisterSystem<CameraSystem>();
		ENSURE(m_CameraSystem);

		Signature signature;
		signature.set(ecs::GetComponentType<TransformComponent>());
		signature.set(ecs::GetComponentType<CameraComponent>());
		ecs::SetSystemSignature<CameraSystem>(signature);

		m_CameraSystem->Initialize(GCamera);
	}
	
	// Add 1 camera viewer entity.
	{
		const Entity entity = ecs::CreateEntity();
		ecs::AddComponent(entity, TransformComponent());
		ecs::AddComponent(entity, InputComponent(true));
		ecs::AddComponent(entity, CameraComponent(true));
	}
}
