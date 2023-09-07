#include "GameSystem.h"
#include "Coordinator.h"
#include "Components/TransformComponent.h"

void vge::GameSystem::Initialize()
{
}

void vge::GameSystem::Tick(float deltaTime)
{
	for (const auto& entity : m_Entities)
	{
		auto& transformComponent = GCoordinator->GetComponent<TransformComponent>(entity);
		transformComponent.Rotation.x += (30.0f * deltaTime);
		transformComponent.Rotation.y += (30.0f * deltaTime);
	}
}
