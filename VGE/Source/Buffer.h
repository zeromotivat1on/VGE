#pragma once

#include "VulkanUtils.h"
#include "VulkanContext.h"

namespace vge
{
	struct VmaBuffer
	{
		VkBuffer Handle = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
	};

	void CreateBuffer(VmaAllocator vmaAllocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memAllocUsage, VmaBuffer& outBuffer);

	VkCommandBuffer BeginOneTimeCmdBuffer(VkCommandPool cmdPool);
	void			EndOneTimeCmdBuffer(VkCommandPool cmdPool, VkQueue queue, VkCommandBuffer cmdBuffer);

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps, VkBuffer& outBuffer, VkDeviceMemory& outMemory);
	void CopyBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CopyImageBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, VkBuffer srcBuffer, VkImage dstImage, VkExtent2D extent);

	// Simple wrapper for BeginOneTimeCmdBuffer (ctor) and EndOneTimeCmdBuffer (dtor) functions.
	struct ScopeCmdBuffer
	{
	public:
		ScopeCmdBuffer(VkCommandPool cmdPool, VkQueue queue)
			: m_CmdPool(cmdPool), m_Queue(queue)
		{
			m_CmdBuffer = BeginOneTimeCmdBuffer(m_CmdPool);
		}

		~ScopeCmdBuffer()
		{
			EndOneTimeCmdBuffer(m_CmdPool, m_Queue, m_CmdBuffer);
		}

		VkCommandBuffer GetHandle() const { return m_CmdBuffer; }

	private:
		VkCommandPool m_CmdPool = VK_NULL_HANDLE;
		VkQueue m_Queue = VK_NULL_HANDLE;
		VkCommandBuffer m_CmdBuffer = VK_NULL_HANDLE;
	};

	// Simple wrapper for scoped stage buffer.
	struct ScopeStageBuffer
	{
	public:
		ScopeStageBuffer(VkDeviceSize size)
		{
			CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Handle, m_Memory);
		}

		~ScopeStageBuffer()
		{
			vkDestroyBuffer(VulkanContext::Device, m_Handle, nullptr);
			vkFreeMemory(VulkanContext::Device, m_Memory, nullptr);
		}

		VkBuffer GetHandle() const { return m_Handle; }
		VkDeviceMemory GetMemory() const { return m_Memory; }

	private:
		VkBuffer m_Handle = VK_NULL_HANDLE;
		VkDeviceMemory m_Memory = VK_NULL_HANDLE;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer() = default;
		IndexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<uint32>& indices);

		VkBuffer GetHandle() const { return m_Handle; }
		VkDeviceMemory GetMemory() const { return m_Memory; }

		void Destroy();

	private:
		VkBuffer m_Handle = VK_NULL_HANDLE;
		VkDeviceMemory m_Memory = VK_NULL_HANDLE;
	};

	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		VertexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices);

		VkBuffer GetHandle() const { return m_Handle; }
		VkDeviceMemory GetMemory() const { return m_Memory; }

		void Destroy();

	private:
		VkBuffer m_Handle = VK_NULL_HANDLE;
		VkDeviceMemory m_Memory = VK_NULL_HANDLE;
	};
}
