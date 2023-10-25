#pragma once

#include "Common.h"
#include "RenderCommon.h"

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

	struct BufferImageCopyInfo
	{
		const Device* Device = nullptr;
		VkBuffer SrcBuffer = VK_NULL_HANDLE;
		VkImage DstImage = VK_NULL_HANDLE;
		VkExtent2D Extent = {};
	};

	struct Buffer
	{
	public:
		static Buffer Create(const BufferCreateInfo& data);
		static void Copy(const BufferCopyInfo& data);
		static void CopyToImage(const BufferImageCopyInfo& data);

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
		static IndexBuffer Create(const Device* device, const std::vector<u32>& indices);
		static IndexBuffer Create(const Device* device, size_t indexCount, const u32* indices);

	public:
		IndexBuffer() = default;
		inline void Destroy() { m_AllocatedBuffer.Destroy(); m_IndexCount = 0; }
		inline Buffer Get() const { return m_AllocatedBuffer; }
		inline size_t GetIndexCount() const { return m_IndexCount; }
		inline VkIndexType GetIndexType() const { return m_IndexType; }

	private:
		Buffer m_AllocatedBuffer = {};
		size_t m_IndexCount = 0;
		VkIndexType m_IndexType = VK_INDEX_TYPE_UINT32;
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
		static VertexBuffer Create(const Device* device, size_t vertexCount, const Vertex* vertices);

	public:
		VertexBuffer() = default;
		inline void Destroy() { m_AllocatedBuffer.Destroy(); m_VertexCount = 0; }
		inline Buffer Get() const { return m_AllocatedBuffer; }
		inline size_t GetVertexCount() const { return m_VertexCount; }

	private:
		Buffer m_AllocatedBuffer = {};
		size_t m_VertexCount = 0;
	};

	struct FrameBufferCreateInfo
	{
		const Device* Device = nullptr;
		VkRenderPass RenderPass = VK_NULL_HANDLE;
		u32 AttachmentCount = 0; 
		const VkImageView* Attachments = nullptr;
		VkExtent2D Extent = { 0, 0 };
	};

	class FrameBuffer
	{
	public:
		static FrameBuffer Create(const FrameBufferCreateInfo& data);

	public:
		FrameBuffer() = default;
		void Destroy();

		inline VkFramebuffer GetHandle() const { return m_Handle; }
		inline VkExtent2D GetExtent() const { return m_Extent; }

	private:
		const Device* m_Device = nullptr;
		VkFramebuffer m_Handle = VK_NULL_HANDLE;
		VkExtent2D m_Extent = { 0, 0 };
	};
}
