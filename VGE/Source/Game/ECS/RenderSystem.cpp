#include "RenderSystem.h"
#include "Coordinator.h"
#include "Game/Camera.h"
#include "Renderer/Renderer.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"

void vge::RenderSystem::Initialize(Renderer* renderer, Camera* camera)
{
	m_Renderer = renderer;
	m_Camera = camera;

	m_Renderer->SetView(m_Camera->GetViewMatrix());
	m_Renderer->SetProjection(m_Camera->GetProjectionMatrix());
}

void vge::RenderSystem::Tick(float deltaTime)
{
	// TODO: transfer model data update to separate system etc.
	for (const Entity& entity : m_Entities)
	{
		const auto& renderComponent = GCoordinator->GetComponent<RenderComponent>(entity);
		const auto& transformComponent = GCoordinator->GetComponent<TransformComponent>(entity);
		m_Renderer->UpdateModelMatrix(renderComponent.ModelId, GetMat4(transformComponent));
	}

	{
		VK_ENSURE(m_Renderer->BeginFrame());
		VkCommandBuffer cmd = m_Renderer->BeginCmdBufferRecord();

		RecordCommandBuffer(cmd);
		m_Renderer->UpdateUniformBuffers();

		m_Renderer->EndCmdBufferRecord(cmd);
		m_Renderer->EndFrame();
	}
}

void vge::RenderSystem::RecordCommandBuffer(VkCommandBuffer cmd)
{
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_Renderer->GetSwapchainExtent().width);
	viewport.height = static_cast<float>(m_Renderer->GetSwapchainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = m_Renderer->GetSwapchainExtent();
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	int32 pipelineIndex = 0;
	Pipeline* currentPipeline = m_Renderer->FindPipeline(pipelineIndex++);
	if (!currentPipeline)
	{
		return;
	}

	// 1st subpass.
	{
		currentPipeline->Bind(cmd);

		for (const Entity& entity : m_Entities)
		{
			const auto& renderComponent = GCoordinator->GetComponent<RenderComponent>(entity);
			const Model* model = m_Renderer->FindModel(renderComponent.ModelId);
			
			if (!model)
			{
				continue;
			}

			const ModelData& modelData = model->GetModelData();
			vkCmdPushConstants(cmd, currentPipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelData), &modelData);

			for (size_t MeshIndex = 0; MeshIndex < model->GetMeshCount(); ++MeshIndex)
			{
				const Mesh* mesh = model->GetMesh(MeshIndex);
				if (!mesh)
				{
					continue;
				}

				const Texture* texture = m_Renderer->FindTexture(mesh->GetTextureId());
				if (!texture)
				{
					continue;
				}


				VkBuffer vertexBuffers[] = { mesh->GetVertexBuffer().Get().Handle };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(cmd, mesh->GetIndexBuffer().Get().Handle, 0, VK_INDEX_TYPE_UINT32);

				//const uint32 dynamicOffset = static_cast<uint32>(m_ModelUniformAlignment * MeshIndex);
				const std::array<VkDescriptorSet, 2> currentDescriptorSets = { m_Renderer->GetCurrentUniformDescriptorSet(), texture->GetDescriptor()};
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->GetLayout(),
					0, static_cast<uint32>(currentDescriptorSets.size()), currentDescriptorSets.data(), 0, nullptr);

				vkCmdDrawIndexed(cmd, static_cast<uint32>(mesh->GetIndexCount()), 1, 0, 0, 0);
			}
		}
	}

	currentPipeline = m_Renderer->FindPipeline(pipelineIndex++);
	if (!currentPipeline)
	{
		return;
	}

	// 2 subpass.
	{
		vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
		currentPipeline->Bind(cmd);

		const std::array<VkDescriptorSet, 1> currentDescriptorSets = { m_Renderer->GetCurrentInputDescriptorSet() };
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->GetLayout(),
			0, static_cast<uint32>(currentDescriptorSets.size()), currentDescriptorSets.data(), 0, nullptr);

		vkCmdDraw(cmd, 3, 1, 0, 0); // fill screen with one big triangle to draw on
	}
}
