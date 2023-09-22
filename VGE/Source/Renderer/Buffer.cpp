#include "Buffer.h"
#include "Device.h"
#include "Logging.h"
#include "CmdBuffer.h"

vge::Buffer vge::Buffer::Create(const BufferCreateInfo& data)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = data.Size;
	bufferCreateInfo.usage = data.Usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo vmaAllocCreateInfo = {};
	vmaAllocCreateInfo.usage = data.MemAllocUsage;

	Buffer buffer = {};
	buffer.m_Allocator = data.Device->GetAllocator();
	VK_ENSURE(vmaCreateBuffer(buffer.m_Allocator, &bufferCreateInfo, &vmaAllocCreateInfo, &buffer.Handle, &buffer.Allocation, &buffer.AllocInfo));

	return buffer;
}

void vge::Buffer::Copy(const BufferCopyInfo& data)
{
	ScopeCmdBuffer transferCmdBuffer(data.Device);

	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = data.Size;

	vkCmdCopyBuffer(transferCmdBuffer.Get(), data.Source, data.Destination, 1, &bufferCopyRegion);
}

void vge::Buffer::CopyToImage(const BufferImageCopyInfo& data)
{
	ScopeCmdBuffer transferCmdBuffer(data.Device);

	VkBufferImageCopy bufferImageCopyRegion = {};
	bufferImageCopyRegion.bufferOffset = 0;
	bufferImageCopyRegion.bufferRowLength = 0;										// for data spacing
	bufferImageCopyRegion.bufferImageHeight = 0;									// for data spacing
	bufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// which image aspect to copy
	bufferImageCopyRegion.imageSubresource.mipLevel = 0;
	bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
	bufferImageCopyRegion.imageSubresource.layerCount = 1;
	bufferImageCopyRegion.imageOffset = { 0, 0, 0 };
	bufferImageCopyRegion.imageExtent = { data.Extent.width, data.Extent.height, 1 };

	const VkImageLayout imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	vkCmdCopyBufferToImage(transferCmdBuffer.Get(), data.SrcBuffer, data.DstImage, imageLayout, 1, &bufferImageCopyRegion);
}

void vge::Buffer::Destroy()
{
	vmaDestroyBuffer(m_Allocator, Handle, Allocation);
	Handle = VK_NULL_HANDLE;
	Allocation = VK_NULL_HANDLE;
	memory::memzero(&AllocInfo, sizeof(VmaAllocationInfo));
	m_Allocator = VK_NULL_HANDLE;
}

void vge::Buffer::TransferToGpuMemory(const void* src, size_t size) const
{
	void* data;
	vmaMapMemory(m_Allocator, Allocation, &data);
	memcpy(data, src, size);
	vmaUnmapMemory(m_Allocator, Allocation);
}

vge::ScopeStageBuffer::ScopeStageBuffer(const Device* device, VkDeviceSize size)
{
	BufferCreateInfo buffCreateInfo = {};
	buffCreateInfo.Device = device;
	buffCreateInfo.Size = size;
	buffCreateInfo.Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffCreateInfo.MemAllocUsage = VMA_MEMORY_USAGE_CPU_ONLY;

	m_AllocatedBuffer = Buffer::Create(buffCreateInfo);
}

vge::VertexInputDescription vge::Vertex::GetDescription()
{
	VertexInputDescription description = {};

	VkVertexInputBindingDescription mainDescription = {};
	mainDescription.binding = 0;
	mainDescription.stride = sizeof(Vertex);
	mainDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.Bindings.push_back(mainDescription);

	VkVertexInputAttributeDescription positionAttribute = {};
	positionAttribute.binding = 0;
	positionAttribute.location = 0;
	positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute.offset = offsetof(Vertex, Position);

	VkVertexInputAttributeDescription colorAttribute = {};
	colorAttribute.binding = 0;
	colorAttribute.location = 1;
	colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	colorAttribute.offset = offsetof(Vertex, Color);

	VkVertexInputAttributeDescription textureAttribute = {};
	textureAttribute.binding = 0;
	textureAttribute.location = 2;
	textureAttribute.format = VK_FORMAT_R32G32_SFLOAT;
	textureAttribute.offset = offsetof(Vertex, TexCoords);

	description.Attributes.push_back(positionAttribute);
	description.Attributes.push_back(colorAttribute);
	description.Attributes.push_back(textureAttribute);

	return description;
}

vge::IndexBuffer vge::IndexBuffer::Create(const Device* device, const std::vector<uint32>& indices)
{
	return Create(device, indices.size(), indices.data());
}

vge::IndexBuffer vge::IndexBuffer::Create(const Device* device, size_t indexCount, const uint32* indices)
{
	const VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

	ScopeStageBuffer stageBuffer(device, bufferSize);
	stageBuffer.Get().TransferToGpuMemory(indices, static_cast<size_t>(bufferSize));

	BufferCreateInfo buffCreateInfo = {};
	buffCreateInfo.Device = device;
	buffCreateInfo.Size = bufferSize;
	buffCreateInfo.Usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buffCreateInfo.MemAllocUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	IndexBuffer idxBuffer = {};
	idxBuffer.m_AllocatedBuffer = Buffer::Create(buffCreateInfo);

	BufferCopyInfo buffCopyInfo = {};
	buffCopyInfo.Device = device;
	buffCopyInfo.Size = bufferSize;
	buffCopyInfo.Source = stageBuffer.Get().Handle;
	buffCopyInfo.Destination = idxBuffer.m_AllocatedBuffer.Handle;

	Buffer::Copy(buffCopyInfo);

	return idxBuffer;
}

vge::VertexBuffer vge::VertexBuffer::Create(const Device* device, const std::vector<Vertex>& vertices)
{
	return Create(device, vertices.size(), vertices.data());
}

vge::VertexBuffer vge::VertexBuffer::Create(const Device* device, size_t vertexCount, const Vertex* vertices)
{
	const VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

	ScopeStageBuffer stageBuffer(device, bufferSize);
	stageBuffer.Get().TransferToGpuMemory(vertices, static_cast<size_t>(bufferSize));

	BufferCreateInfo buffCreateInfo = {};
	buffCreateInfo.Device = device;
	buffCreateInfo.Size = bufferSize;
	buffCreateInfo.Usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buffCreateInfo.MemAllocUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	VertexBuffer vertBuffer = {};
	vertBuffer.m_AllocatedBuffer = Buffer::Create(buffCreateInfo);

	BufferCopyInfo buffCopyInfo = {};
	buffCopyInfo.Device = device;
	buffCopyInfo.Size = bufferSize;
	buffCopyInfo.Source = stageBuffer.Get().Handle;
	buffCopyInfo.Destination = vertBuffer.m_AllocatedBuffer.Handle;

	Buffer::Copy(buffCopyInfo);

	return vertBuffer;
}
