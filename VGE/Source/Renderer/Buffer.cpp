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

//void vge::CreateBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memAllocUsage, Buffer& outBuffer)
//{
//	VkBufferCreateInfo bufferCreateInfo = {};
//	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	bufferCreateInfo.size = size;
//	bufferCreateInfo.usage = usage;
//	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//	VmaAllocationCreateInfo vmaAllocCreateInfo = {};
//	vmaAllocCreateInfo.usage = memAllocUsage;
//
//	VK_ENSURE(vmaCreateBuffer(allocator, &bufferCreateInfo, &vmaAllocCreateInfo, &outBuffer.Handle, &outBuffer.Allocation, &outBuffer.AllocInfo));
//}

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

//void vge::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps, VkBuffer& outBuffer, VkDeviceMemory& outMemory)
//{
//	VkBufferCreateInfo bufferCreateInfo = {};
//	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	bufferCreateInfo.size = size;
//	bufferCreateInfo.usage = usage;
//	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//	if (vkCreateBuffer(VulkanContext::Device, &bufferCreateInfo, nullptr, &outBuffer) != VK_SUCCESS)
//	{
//		LOG(Error, "Failed to create buffer.");
//		return;
//	}
//
//	VkMemoryRequirements memoryRequirements = {};
//	vkGetBufferMemoryRequirements(VulkanContext::Device, outBuffer, &memoryRequirements);
//
//	VkMemoryAllocateInfo memoryAllocateInfo = {};
//	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	memoryAllocateInfo.allocationSize = memoryRequirements.size;
//	memoryAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(VulkanContext::Gpu, memoryRequirements.memoryTypeBits, memProps);
//
//	if (vkAllocateMemory(VulkanContext::Device, &memoryAllocateInfo, nullptr, &outMemory) != VK_SUCCESS)
//	{
//		LOG(Error, "Failed to allocate memory for buffer.");
//		return;
//	}
//
//	vkBindBufferMemory(VulkanContext::Device, outBuffer, outMemory, 0);
//}
//
//void vge::CopyBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
//{
//	ScopeCmdBuffer transferCmdBuffer(transferCmdPool, transferQueue);
//
//	VkBufferCopy bufferCopyRegion = {};
//	bufferCopyRegion.srcOffset = 0;
//	bufferCopyRegion.dstOffset = 0;
//	bufferCopyRegion.size = size;
//
//	vkCmdCopyBuffer(transferCmdBuffer.GetHandle(), srcBuffer, dstBuffer, 1, &bufferCopyRegion);
//}

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
	const VkDeviceSize bufferSize = STD_VECTOR_ALLOC_SIZE(indices);

	ScopeStageBuffer stageBuffer(device, bufferSize);
	stageBuffer.Get().TransferToGpuMemory(indices.data(), static_cast<size_t>(bufferSize));

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

//vge::IndexBuffer::IndexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<uint32>& indices)
//{
//	const VkDeviceSize bufferSize = STD_VECTOR_ALLOC_SIZE(indices);
//
//	ScopeStageBuffer stageBuffer(bufferSize);
//
//	void* data;
//	vmaMapMemory(VulkanContext::Allocator, stageBuffer.Get().Allocation, &data);
//	memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
//	vmaUnmapMemory(VulkanContext::Allocator, stageBuffer.Get().Allocation);
//
//	CreateBuffer(VulkanContext::Allocator, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_AllocatedBuffer);
//
//	CopyBuffer(transferQueue, transferCmdPool, stageBuffer.Get().Handle, m_AllocatedBuffer.Handle, bufferSize);
//}

vge::VertexBuffer vge::VertexBuffer::Create(const Device* device, const std::vector<Vertex>& vertices)
{
	const VkDeviceSize bufferSize = STD_VECTOR_ALLOC_SIZE(vertices);

	ScopeStageBuffer stageBuffer(device, bufferSize);
	stageBuffer.Get().TransferToGpuMemory(vertices.data(), static_cast<size_t>(bufferSize));

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

//vge::VertexBuffer::VertexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices)
//{
//	const VkDeviceSize bufferSize = STD_VECTOR_ALLOC_SIZE(vertices);
//
//	ScopeStageBuffer stageBuffer(bufferSize);
//
//	void* data;
//	vmaMapMemory(VulkanContext::Allocator, stageBuffer.Get().Allocation, &data);
//	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
//	vmaUnmapMemory(VulkanContext::Allocator, stageBuffer.Get().Allocation);
//
//	CreateBuffer(VulkanContext::Allocator, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_AllocatedBuffer);
//
//	CopyBuffer(transferQueue, transferCmdPool, stageBuffer.Get().Handle, m_AllocatedBuffer.Handle, bufferSize);
//}
