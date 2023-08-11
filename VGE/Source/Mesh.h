#pragma once

#include "Common.h"
#include "VulkanUtils.h"

namespace vge
{
	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(VkPhysicalDevice gpu, VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices);

		size_t GetVertexCount() const { return m_VertexCount; }
		VkBuffer GetVertexBuffer() const { return m_VertexBuffer; }
			
		void DestroyVertexBuffer();

	private:
		size_t m_VertexCount = 0;

		VkPhysicalDevice m_Gpu = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;

		VkBuffer m_VertexBuffer = {};
		VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;

	private:
		void CreateVertexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices);
	};
}
