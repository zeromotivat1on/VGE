#include "RenderSystem.h"
#include "Coordinator.h"
#include "Game/Camera.h"
#include "Renderer/Renderer.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"

namespace vge
{
	struct ScopeFrameControl
	{
	public:
		vge::CommandBuffer* Cmd = {};

		ScopeFrameControl(Renderer* renderer) : m_Renderer(renderer)
		{
			Cmd = m_Renderer->BeginFrame();
			Cmd->BeginRecord();
			Cmd->BeginRenderPass(m_Renderer->GetRenderPass(), m_Renderer->GetCurrentFrameBuffer());
		}

		~ScopeFrameControl()
		{
			Cmd->EndRenderPass();
			Cmd->EndRecord();
			m_Renderer->EndFrame();
		}

		void RecordCmd(const std::unordered_set<Entity>& entities)
		{
			RecordCmdPreSubpass();

			int32 pipelineIdx = 0;
			RecordCmdFirstSubpass(pipelineIdx++, entities);
			RecordCmdSecondSubpass(pipelineIdx++);
		}

		inline void UpdateUniforms() { m_Renderer->UpdateUniformBuffers(); }

	private:
		vge::Renderer* m_Renderer = nullptr;

	private:
		void RecordCmdPreSubpass()
		{
			glm::vec2 viewportSize;
			viewportSize.x = static_cast<float>(m_Renderer->GetSwapchainExtent().width);
			viewportSize.y = static_cast<float>(m_Renderer->GetSwapchainExtent().height);
			Cmd->SetViewport(viewportSize);
			Cmd->SetScissor(m_Renderer->GetSwapchainExtent());
		}

		void RecordCmdFirstSubpass(int32 pipelineIdx, const std::unordered_set<Entity>& entities)
		{
			Pipeline* pipeline = m_Renderer->FindPipeline(pipelineIdx);
			if (!pipeline)
			{
				return;
			}

			Cmd->Bind(pipeline);

			for (const Entity& entity : entities)
			{
				const auto* renderComponent = RenderComponent::GetFrom(entity);
				if (!renderComponent)
				{
					continue;
				}

				const Model* model = m_Renderer->FindModel(renderComponent->ModelId);
				if (!model)
				{
					continue;
				}

				const ModelData& modelData = model->GetModelData();
				Cmd->PushConstants(pipeline, pipeline->GetShader(ShaderStage::Vertex), sizeof(ModelData), &modelData);

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
					Cmd->Bind(static_cast<uint32>(vertBuffers.size()), vertBuffers.data(), pipeline->GetShader(ShaderStage::Vertex));
					Cmd->Bind(mesh->GetIndexBuffer());

					std::vector<VkDescriptorSet> descriptorSets = { m_Renderer->GetCurrentUniformDescriptorSet(), texture->GetDescriptor() };
					Cmd->Bind(pipeline, static_cast<uint32>(descriptorSets.size()), descriptorSets.data());
					Cmd->DrawIndexed(static_cast<uint32>(mesh->GetIndexCount()));
				}
			}
		}

		void RecordCmdSecondSubpass(int32 pipelineIdx)
		{
			Pipeline* pipeline = m_Renderer->FindPipeline(pipelineIdx);
			if (!pipeline)
			{
				return;
			}

			Cmd->NextSubpass();
			Cmd->Bind(pipeline);

			std::vector<VkDescriptorSet> descriptorSets = { m_Renderer->GetCurrentInputDescriptorSet() };
			Cmd->Bind(pipeline, static_cast<uint32>(descriptorSets.size()), descriptorSets.data());
			Cmd->Draw(3); // fill screen with one big triangle to draw on
		}
	};
}

void vge::RenderSystem::Initialize(Renderer* renderer, Camera* camera)
{
	m_Renderer = renderer;
	m_Camera = camera;
}

void vge::RenderSystem::Tick(float deltaTime)
{
	m_Renderer->SetView(m_Camera->GetViewMatrix());
	m_Renderer->SetProjection(m_Camera->GetProjectionMatrix());

	// TODO: transfer model data update to separate system etc.
	for (const Entity& entity : m_Entities)
	{
		const auto* renderComponent = RenderComponent::GetFrom(entity);
		if (!renderComponent)
		{
			continue;
		}

		const auto* transformComponent = TransformComponent::GetFrom(entity);
		if (!transformComponent)
		{
			continue;
		}

		m_Renderer->UpdateModelMatrix(renderComponent->ModelId, transformComponent->GetMat4());
	}

	ScopeFrameControl scopeFrame(m_Renderer);
	scopeFrame.RecordCmd(m_Entities);
	scopeFrame.UpdateUniforms();
}
