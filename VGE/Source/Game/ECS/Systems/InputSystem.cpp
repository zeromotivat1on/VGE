#include "InputSystem.h"
#include "Render/Window.h"
#include "ECS/Coordinator.h"
#include "ECS/Components/InputComponent.h"

void vge::InputSystem::Initialize()
{
	_InputController = InputController(GWindow);
	_InputController.SetMoveSpeed(80.0f);
	_InputController.SetRotateSpeed(100.0f);
	_InputController.ShouldInvertVerticalAxis(true);
}

void vge::InputSystem::Tick(f32 deltaTime)
{
	ForEachEntity([this, deltaTime](Entity entity) 
	{
		const auto& input = ecs::GetComponent<InputComponent>(entity);
		if (input.IsControllable())
		{
			_InputController.MoveInPlaneXZ(deltaTime, entity);
		}
	});
}
