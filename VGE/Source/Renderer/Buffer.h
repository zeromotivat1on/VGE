#pragma once

#include "VulkanUtils.h"
#include "VulkanContext.h"

namespace vge
{
	class Device;

	struct BufferCreateInfo
	{
		const Device* Device = nullptr;
		VkDeviceSize Size = 0;
		VkBufferUsageFlags Usage = 0;
		VmaMemoryUsage MemAllocUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN;
	};

	struct BufferCopyInfo
	{
		const Device* Device = nullptr;
		VkBuffer Source = VK_NULL_HANDLE;
		VkBuffer Destination = VK_NULL_HANDLE;
		VkDeviceSize Size = 0;
	};

	struct Buffer
	{
	public:
		static Buffer Create(const BufferCreateInfo& data);
		static void Copy(const BufferCopyInfo& data);

	public:
		Buffer() = default;
		void Destroy();

		void TransferToGpuMemory(const void* src, size_t size) const;

	public:
		VkBuffer Handle = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo AllocInfo = {};

	private:
		VmaAllocator m_Allocator = VK_NULL_HANDLE;
	};

	//void CreateBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memAllocUsage, Buffer& outBuffer);

	VkCommandBuffer BeginOneTimeCmdBuffer(VkCommandPool cmdPool);
	void			EndOneTimeCmdBuffer(VkCommandPool cmdPool, VkQueue queue, VkCommandBuffer cmdBuffer);

	//void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps, VkBuffer& outBuffer, VkDeviceMemory& outMemory);

	//void CopyBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
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
		ScopeStageBuffer(const Device* device, VkDeviceSize size);
		~ScopeStageBuffer() { m_AllocatedBuffer.Destroy(); }

		inline Buffer Get() const { return m_AllocatedBuffer; }

	private:
		Buffer m_AllocatedBuffer = {};
	};

	class IndexBuffer
	{
	public:
		static IndexBuffer Create(const Device* device, const std::vector<uint32>& indices);

	public:
		IndexBuffer() = default;
		inline void Destroy() { m_AllocatedBuffer.Destroy(); }
		inline Buffer Get() const { return m_AllocatedBuffer; }

	private:
		Buffer m_AllocatedBuffer = {};
	};

	struct VertexInputDescription
	{
		std::vector<VkVertexInputBindingDescription> Bindings = {};
		std::vector<VkVertexInputAttributeDescription> Attributes = {};

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Color;
		glm::vec2 TexCoords;

		static VertexInputDescription GetDescription();
	};

	class VertexBuffer
	{
	public:
		static VertexBuffer Create(const Device* device, const std::vector<Vertex>& vertices);

	public:
		VertexBuffer() = default;
		inline void Destroy() { m_AllocatedBuffer.Destroy(); }
		inline Buffer Get() const { return m_AllocatedBuffer; }

	private:
		Buffer m_AllocatedBuffer = {};
	};
}
