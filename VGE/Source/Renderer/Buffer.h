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
		static IndexBuffer Create(const Device* device, const std::vector<uint32>& indices);
		static IndexBuffer Create(const Device* device, size_t indexCount, const uint32* indices);

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
		static VertexBuffer Create(const Device* device, size_t vertexCount, const Vertex* vertices);

	public:
		VertexBuffer() = default;
		inline void Destroy() { m_AllocatedBuffer.Destroy(); }
		inline Buffer Get() const { return m_AllocatedBuffer; }

	private:
		Buffer m_AllocatedBuffer = {};
	};
}
