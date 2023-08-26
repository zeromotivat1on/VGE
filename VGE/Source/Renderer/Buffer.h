#pragma once

#include "VulkanUtils.h"
#include "VulkanContext.h"

namespace vge
{
	struct VmaBuffer
	{
		VkBuffer Handle = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo AllocInfo = {};
	};

	void CreateBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memAllocUsage, VmaBuffer& outBuffer);

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
			CreateBuffer(VulkanContext::Allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_AllocatedBuffer);
		}

		~ScopeStageBuffer()
		{
			vmaDestroyBuffer(VulkanContext::Allocator, m_AllocatedBuffer.Handle, m_AllocatedBuffer.Allocation);
		}

		VmaBuffer Get() const { return m_AllocatedBuffer; }

	private:
		VmaBuffer m_AllocatedBuffer = {};
	};

	class IndexBuffer
	{
	public:
		IndexBuffer() = default;
		IndexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<uint32>& indices);

		VmaBuffer Get() const { return m_AllocatedBuffer; }

		void Destroy();

	private:
		VmaBuffer m_AllocatedBuffer = {};
	};

	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		VertexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices);

		VmaBuffer Get() const { return m_AllocatedBuffer; }

		void Destroy();

	private:
		VmaBuffer m_AllocatedBuffer = {};
	};
}
