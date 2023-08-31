#include "Buffer.h"
#include "Device.h"
#include "Logging.h"

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
	ScopeCmdBuffer transferCmdBuffer(data.Device->GetCommandPool(), data.Device->GetGfxQueue());

	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = data.Size;

	vkCmdCopyBuffer(transferCmdBuffer.GetHandle(), data.Source, data.Destination, 1, &bufferCopyRegion);
}

void vge::Buffer::Destroy()
{
	vmaDestroyBuffer(VulkanContext::Allocator, Handle, Allocation);
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

VkCommandBuffer vge::BeginOneTimeCmdBuffer(VkCommandPool cmdPool)
{
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandPool = cmdPool;
	cmdBufferAllocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(VulkanContext::Device, &cmdBufferAllocInfo, &cmdBuffer) != VK_SUCCESS)
	{
		LOG(Error, "Failed to allcoate transfer command buffer.");
		return VK_NULL_HANDLE;
	}

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo);

	return cmdBuffer;
}

void vge::EndOneTimeCmdBuffer(VkCommandPool cmdPool, VkQueue queue, VkCommandBuffer cmdBuffer)
{
	vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue); // not good if we have a lot of calls to this

	vkFreeCommandBuffers(VulkanContext::Device, cmdPool, 1, &cmdBuffer);
}

void vge::CopyImageBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, VkBuffer srcBuffer, VkImage dstImage, VkExtent2D extent)
{
	ScopeCmdBuffer transferCmdBuffer(transferCmdPool, transferQueue);

	VkBufferImageCopy bufferImageCopyRegion = {};
	bufferImageCopyRegion.bufferOffset = 0;
	bufferImageCopyRegion.bufferRowLength = 0;										// for data spacing
	bufferImageCopyRegion.bufferImageHeight = 0;									// for data spacing
	bufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// which image aspect to copy
	bufferImageCopyRegion.imageSubresource.mipLevel = 0;
	bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
	bufferImageCopyRegion.imageSubresource.layerCount = 1;
	bufferImageCopyRegion.imageOffset = { 0, 0, 0 };
	bufferImageCopyRegion.imageExtent = { extent.width, extent.height, 1 };

	const VkImageLayout imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	vkCmdCopyBufferToImage(transferCmdBuffer.GetHandle(), srcBuffer, dstImage, imageLayout, 1, &bufferImageCopyRegion);
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

vge::IndexBuffer vge::IndexBuffer::Create(const Device* device, size_t indexCount, const uint32* pIndices)
{
	const VkDeviceSize bufferSize = sizeof(pIndices[0]) * indexCount;

	ScopeStageBuffer stageBuffer(device, bufferSize);
	stageBuffer.Get().TransferToGpuMemory(pIndices, static_cast<size_t>(bufferSize));

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

vge::VertexBuffer vge::VertexBuffer::Create(const Device* device, size_t vertexCount, const Vertex* pVertices)
{
	const VkDeviceSize bufferSize = sizeof(pVertices[0]) * vertexCount;

	ScopeStageBuffer stageBuffer(device, bufferSize);
	stageBuffer.Get().TransferToGpuMemory(pVertices, static_cast<size_t>(bufferSize));

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
