#include "RenderSystem.h"
#include "Coordinator.h"
#include "Game/Camera.h"
#include "Renderer/Renderer.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"

struct ScopeFrameControl
{
	ScopeFrameControl(vge::Renderer* renderer) : Renderer(renderer)
	{
		Cmd = Renderer->BeginFrame();
		Cmd->BeginRecord();
		Cmd->BeginRenderPass(Renderer->GetRenderPass(), Renderer->GetCurrentFrameBuffer());
	}

	~ScopeFrameControl()
	{
		Cmd->EndRenderPass();
		Cmd->EndRecord();
		Renderer->EndFrame();
	}

	vge::Renderer* Renderer = nullptr;
	vge::CommandBuffer* Cmd = {};
};

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
		ScopeFrameControl scopeFrame(m_Renderer);
		RecordCommandBuffer(scopeFrame.Cmd);
		m_Renderer->UpdateUniformBuffers();
	}
}

void vge::RenderSystem::RecordCommandBuffer(CommandBuffer* cmd)
{
	{
		glm::vec2 size;
		size.x = static_cast<float>(m_Renderer->GetSwapchainExtent().width);
		size.y = static_cast<float>(m_Renderer->GetSwapchainExtent().height);
		cmd->SetViewport(size);
	}

	cmd->SetScissor(m_Renderer->GetSwapchainExtent());

	int32 pipelineIndex = 0;
	const Pipeline* currentPipeline = m_Renderer->FindPipeline(pipelineIndex++);
	if (!currentPipeline)
	{
		return;
	}

	// 1st subpass.
	{
		cmd->Bind(currentPipeline);

		for (const Entity& entity : m_Entities)
		{
			const auto& renderComponent = GCoordinator->GetComponent<RenderComponent>(entity);
			const Model* model = m_Renderer->FindModel(renderComponent.ModelId);
			
			if (!model)
			{
				continue;
			}

			const ModelData& modelData = model->GetModelData();
			cmd->PushConstants(currentPipeline, currentPipeline->GetShader(ShaderStage::Vertex), sizeof(ModelData), &modelData);

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

				std::vector<const VertexBuffer*> vertBuffers = { mesh->GetVertexBuffer() };
				cmd->Bind(static_cast<uint32>(vertBuffers.size()), vertBuffers.data(), currentPipeline->GetShader(ShaderStage::Vertex));
				cmd->Bind(mesh->GetIndexBuffer());

				std::vector<VkDescriptorSet> descriptorSets = { m_Renderer->GetCurrentUniformDescriptorSet(), texture->GetDescriptor() };
				cmd->Bind(currentPipeline, static_cast<uint32>(descriptorSets.size()), descriptorSets.data());
				cmd->DrawIndexed(static_cast<uint32>(mesh->GetIndexCount()));
			}
		}
	}

	currentPipeline = m_Renderer->FindPipeline(pipelineIndex++);
	if (!currentPipeline)
	{
		return;
	}

	// 2nd subpass.
	{
		cmd->NextSubpass();
		cmd->Bind(currentPipeline);

		std::vector<VkDescriptorSet> descriptorSets = { m_Renderer->GetCurrentInputDescriptorSet() };
		cmd->Bind(currentPipeline, static_cast<uint32>(descriptorSets.size()), descriptorSets.data());
		cmd->Draw(3u); // fill screen with one big triangle to draw on
	}
}
