#include "Buffer.h"
#include "Logging.h"

void vge::CreateBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memAllocUsage, VmaBuffer& outBuffer)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo vmaAllocCreateInfo = {};
	vmaAllocCreateInfo.usage = memAllocUsage;

	VK_ENSURE(vmaCreateBuffer(allocator, &bufferCreateInfo, &vmaAllocCreateInfo, &outBuffer.Handle, &outBuffer.Allocation, &outBuffer.AllocInfo));
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

void vge::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps, VkBuffer& outBuffer, VkDeviceMemory& outMemory)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(VulkanContext::Device, &bufferCreateInfo, nullptr, &outBuffer) != VK_SUCCESS)
	{
		LOG(Error, "Failed to create buffer.");
		return;
	}

	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements(VulkanContext::Device, outBuffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(VulkanContext::Gpu, memoryRequirements.memoryTypeBits, memProps);

	if (vkAllocateMemory(VulkanContext::Device, &memoryAllocateInfo, nullptr, &outMemory) != VK_SUCCESS)
	{
		LOG(Error, "Failed to allocate memory for buffer.");
		return;
	}

	vkBindBufferMemory(VulkanContext::Device, outBuffer, outMemory, 0);
}

void vge::CopyBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	ScopeCmdBuffer transferCmdBuffer(transferCmdPool, transferQueue);

	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = size;

	vkCmdCopyBuffer(transferCmdBuffer.GetHandle(), srcBuffer, dstBuffer, 1, &bufferCopyRegion);
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

vge::IndexBuffer::IndexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<uint32>& indices)
{
	const VkDeviceSize bufferSize = STD_VECTOR_ALLOC_SIZE(indices);

	ScopeStageBuffer stageBuffer(bufferSize);

	void* data;
	vmaMapMemory(VulkanContext::Allocator, stageBuffer.Get().Allocation, &data);
	memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
	vmaUnmapMemory(VulkanContext::Allocator, stageBuffer.Get().Allocation);

	CreateBuffer(VulkanContext::Allocator, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_AllocatedBuffer);

	CopyBuffer(transferQueue, transferCmdPool, stageBuffer.Get().Handle, m_AllocatedBuffer.Handle, bufferSize);
}

vge::VertexBuffer::VertexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices)
{
	const VkDeviceSize bufferSize = STD_VECTOR_ALLOC_SIZE(vertices);

	ScopeStageBuffer stageBuffer(bufferSize);

	void* data;
	vmaMapMemory(VulkanContext::Allocator, stageBuffer.Get().Allocation, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vmaUnmapMemory(VulkanContext::Allocator, stageBuffer.Get().Allocation);

	CreateBuffer(VulkanContext::Allocator, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_AllocatedBuffer);

	CopyBuffer(transferQueue, transferCmdPool, stageBuffer.Get().Handle, m_AllocatedBuffer.Handle, bufferSize);
}