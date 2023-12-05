#include "GameSystem.h"
#include "Coordinator.h"
#include "Game/Camera.h"
#include "Renderer/Window.h"
#include "Components/TransformComponent.h"

void vge::GameSystem::Initialize()
{
	m_InputController = InputController(GWindow);
	m_InputController.SetMoveSpeed(80.0f);
	m_InputController.SetRotateSpeed(100.0f);
	m_InputController.ShouldInvertVerticalAxis(true);
}

void vge::GameSystem::Tick(f32 deltaTime)
{
	const auto& firstEntity = *m_Entities.begin();
	auto* transform = GCoordinator->GetComponent<TransformComponent>(firstEntity);
	
	m_InputController.MoveInPlaneXZ(deltaTime, firstEntity);
	//GCamera->SetViewYXZ(transform->Translation, transform->Rotation);
}
