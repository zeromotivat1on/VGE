#pragma once

#include "Common.h"
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
		Mesh(VkPhysicalDevice gpu, VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, 
			const std::vector<Vertex>& vertices, const std::vector<uint32>& indices, int32 TextureId);

		size_t GetVertexCount() const { return m_VertexCount; }
		size_t GetIndexCount() const { return m_IndexCount; }
		VkBuffer GetVertexBuffer() const { return m_VertexBuffer; }
		VkBuffer GetIndexBuffer() const { return m_IndexBuffer; }

		int32 GetTextureId() const { return m_TextureId; }
		
		const ModelData& GetModelDataRef() const { return m_ModelData; }
			  ModelData  GetModelData()    const { return m_ModelData; }

		void SetModelMatrix(glm::mat4 model) { m_ModelData.ModelMatrix = model; }

		void Destroy();

	private:
		VkPhysicalDevice m_Gpu = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;

		ModelData m_ModelData = {};
		int32 m_TextureId = -1;

		size_t m_VertexCount = 0;
		VkBuffer m_VertexBuffer = {};
		VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;

		size_t m_IndexCount = 0;
		VkBuffer m_IndexBuffer = {};
		VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;

	private:
		void CreateVertexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices);
		void CreateIndexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<uint32>& indices);
	};
}
