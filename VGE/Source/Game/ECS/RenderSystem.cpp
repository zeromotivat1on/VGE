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
	for (const Entity& entity : m_Entities)
	{
		const auto& renderComponent = GCoordinator->GetComponent<RenderComponent>(entity);
		const auto& transformComponent = GCoordinator->GetComponent<TransformComponent>(entity);
		
		// TODO: transfer model data update to separate system etc.
		m_Renderer->UpdateModelMatrix(renderComponent.ModelId, GetMat4(transformComponent));
	}

	//VK_ENSURE(m_Renderer->BeginFrame());
	//VkCommandBuffer cmd = m_Renderer->BeginCmdBufferRecord();

	//{
	//	VkViewport viewport = {};
	//	viewport.x = 0.0f;
	//	viewport.y = 0.0f;
	//	viewport.width = static_cast<float>(m_Renderer->GetSwapchainExtent().width);
	//	viewport.height = static_cast<float>(m_Renderer->GetSwapchainExtent().height);
	//	viewport.minDepth = 0.0f;
	//	viewport.maxDepth = 1.0f;
	//	vkCmdSetViewport(cmd, 0, 1, &viewport);

	//	VkRect2D scissor = {};
	//	scissor.offset = { 0, 0 };
	//	scissor.extent = m_Renderer->GetSwapchainExtent();
	//	vkCmdSetScissor(cmd, 0, 1, &scissor);


	//}

	//m_Renderer->EndCmdBufferRecord(cmd);
	//m_Renderer->EndFrame();

	m_Renderer->Draw();
}
