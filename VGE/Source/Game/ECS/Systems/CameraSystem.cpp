#include "CameraSystem.h"
#include "ECS/Coordinator.h"
#include "Game/Camera.h"
#include "Renderer/Window.h"
#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"

void vge::CameraSystem::Initialize(Camera* camera)
{
	ENSURE(camera);
	m_Camera = camera;
}

void vge::CameraSystem::Tick(f32 deltaTime)
{
	ForEachEntity([this, deltaTime](Entity entity)
		{
			const auto& camera = GetComponent<CameraComponent>(entity);
			const auto& transform = GetComponent<TransformComponent>(entity);

			if (camera.ShouldObserve())
			{
				m_Camera->SetViewYXZ(transform.Translation, transform.Rotation);
			}
		});
}