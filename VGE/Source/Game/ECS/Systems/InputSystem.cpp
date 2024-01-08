#include "InputSystem.h"
#include "Renderer/Window.h"
#include "ECS/Coordinator.h"
#include "Components/InputComponent.h"

void vge::InputSystem::Initialize()
{
	m_InputController = InputController(GWindow);
	m_InputController.SetMoveSpeed(80.0f);
	m_InputController.SetRotateSpeed(100.0f);
	m_InputController.ShouldInvertVerticalAxis(true);
}

void vge::InputSystem::Tick(f32 deltaTime)
{
	ForEachEntity([this, deltaTime](Entity entity) 
		{
			const auto& input = ecs::GetComponent<InputComponent>(entity);
			if (input.IsControllable())
			{
				m_InputController.MoveInPlaneXZ(deltaTime, entity);
			}
		});
}
