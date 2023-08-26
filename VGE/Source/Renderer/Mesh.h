#pragma once

#include "Common.h"
#include "Buffer.h"
#include "VulkanUtils.h"

namespace vge
{
	struct ModelData
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0f);
	};

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices, const std::vector<uint32>& indices, int32 TextureId);

		size_t GetVertexCount() const { return m_VertexCount; }
		size_t GetIndexCount() const { return m_IndexCount; }
		int32 GetTextureId() const { return m_TextureId; }
		
		const VertexBuffer& GetVertexBuffer() const { return m_VertexBuffer; }
			  VertexBuffer& GetVertexBuffer()		{ return m_VertexBuffer; }

		const IndexBuffer& GetIndexBuffer() const { return m_IndexBuffer; }
			  IndexBuffer& GetIndexBuffer()		  { return m_IndexBuffer; }

		const ModelData& GetModelData() const { return m_ModelData; }
			  ModelData& GetModelData()		  { return m_ModelData; }

		void SetModelMatrix(glm::mat4 model) { m_ModelData.ModelMatrix = model; }

		void Destroy();

	private:
		size_t m_VertexCount = 0;
		size_t m_IndexCount = 0;

		ModelData m_ModelData = {};
		int32 m_TextureId = -1;

		VertexBuffer m_VertexBuffer = {};
		IndexBuffer m_IndexBuffer = {};
	};
}
