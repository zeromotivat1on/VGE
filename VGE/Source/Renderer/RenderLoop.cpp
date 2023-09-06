#include "RenderLoop.h"
#include "Renderer.h"
#include "Window.h"
#include "ECS/Coordinator.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"

void vge::RenderLoop::Initialize()
{
	ENSURE(GWindow);

	CreateDevice(GWindow);
	ENSURE(GDevice);
	GDevice->Initialize();

	CreateRenderer(*GDevice);
	ENSURE(GRenderer);
	GRenderer->Initialize();

	{
		m_RenderSystem = GCoordinator->RegisterSystem<RenderSystem>();
		ENSURE(m_RenderSystem);
		m_RenderSystem->Initialize(GRenderer);

		Signature signature;
		signature.set(GCoordinator->GetComponentType<RenderComponent>());
		signature.set(GCoordinator->GetComponentType<TransformComponent>());
		GCoordinator->SetSystemSignature<RenderSystem>(signature);

		std::vector<Entity> renderSystemEntities;
		renderSystemEntities.push_back(GCoordinator->CreateEntity());

		TransformComponent transformComponent = {};
		transformComponent.Location = glm::vec3(0.0f, 0.0f, -25.0f);
		transformComponent.Rotation = glm::vec3(0.0f, 1.0f, 0.0f);
		transformComponent.Scale = glm::vec3(1.0f, 1.0f, 1.0f);
		transformComponent.RotationAngle = 0.0f;
		GCoordinator->AddComponent(renderSystemEntities[0], transformComponent);

		RenderComponent renderComponent = {};
		renderComponent.ModelId = GRenderer->CreateModel("Models/cottage/Cottage_FREE.obj");
		GCoordinator->AddComponent(renderSystemEntities[0], renderComponent);

		for (auto& entity : renderSystemEntities)
		{
			m_RenderSystem->Add(entity);
		}
	}
}

void vge::RenderLoop::Tick(float deltaTime)
{
	m_RenderSystem->Tick(deltaTime);
}

void vge::RenderLoop::Destroy()
{
	DestroyRenderer();
	DestroyDevice();
}
