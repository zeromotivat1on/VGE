#include "CmdBuffer.h"
#include "Device.h"

VkCommandBuffer vge::BeginOneTimeCmdBuffer(const Device* device)
{
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandPool = device->GetCommandPool();
	cmdBufferAllocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device->GetHandle(), &cmdBufferAllocInfo, &cmdBuffer) != VK_SUCCESS)
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

void vge::EndOneTimeCmdBuffer(const Device* device, VkCommandBuffer cmdBuffer)
{
	vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit(device->GetGfxQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device->GetGfxQueue()); // not good if we have a lot of calls to this

	vkFreeCommandBuffers(device->GetHandle(), device->GetCommandPool(), 1, &cmdBuffer);
}

vge::ScopeCmdBuffer::ScopeCmdBuffer(const Device* device)
	: m_Device(device)
{
	m_CmdBuffer = BeginOneTimeCmdBuffer(device);
}

vge::ScopeCmdBuffer::~ScopeCmdBuffer()
{
	EndOneTimeCmdBuffer(m_Device, m_CmdBuffer);
}
