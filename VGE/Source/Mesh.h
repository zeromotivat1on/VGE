#pragma once

#include "Common.h"
#include "VulkanUtils.h"

namespace vge
{
	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(VkPhysicalDevice gpu, VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		size_t GetVertexCount() const { return m_VertexCount; }
		size_t GetIndexCount() const { return m_IndexCount; }
		VkBuffer GetVertexBuffer() const { return m_VertexBuffer; }
		VkBuffer GetIndexBuffer() const { return m_IndexBuffer; }

		void DestroyVertexBuffer() const;
		void DestroyIndexBuffer() const;

	private:
		VkPhysicalDevice m_Gpu = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;

		size_t m_VertexCount = 0;
		VkBuffer m_VertexBuffer = {};
		VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;

		size_t m_IndexCount = 0;
		VkBuffer m_IndexBuffer = {};
		VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;

	private:
		void CreateVertexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices);
		void CreateIndexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<uint32_t>& indices);
	};
}
