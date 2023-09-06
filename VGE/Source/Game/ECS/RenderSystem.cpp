#include "RenderSystem.h"
#include "Coordinator.h"
#include "Renderer/Renderer.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"

void vge::RenderSystem::Initialize(Renderer* renderer)
{
	m_Renderer = renderer;
}

void vge::RenderSystem::Tick(float deltaTime)
{
	for (const auto& entity : m_Entities)
	{
		auto& renderComponent = GCoordinator->GetComponent<RenderComponent>(entity);
		auto& transformComponent = GCoordinator->GetComponent<TransformComponent>(entity);
		transformComponent.RotationAngle += (30.0f * deltaTime);
		
		// TODO: transfer model data update to separate system etc.
		m_Renderer->UpdateModelMatrix(renderComponent.ModelId, GetTransformMatrix(transformComponent));
	}

	m_Renderer->Draw();
}
