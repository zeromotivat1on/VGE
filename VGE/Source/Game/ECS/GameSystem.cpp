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
		transformComponent.Translation.z = 30.0f;
		transformComponent.Translation.x = 15.0f;
		transformComponent.Rotation.y += (30.0f * deltaTime);
	}
}