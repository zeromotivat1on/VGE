#include "Queue.h"
#include "Device.h"
#include "CommandBuffer.h"

vge::Queue::Queue(Device& device, u32 familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent, u32 index)
	: _Device(device), _FamilyIndex(familyIndex), _Index(index), _CanPresent(canPresent), _Properties(properties)
{
	vkGetDeviceQueue(_Device.GetHandle(), familyIndex, index, &_Handle);
}

vge::Queue::Queue(Queue&& other) 
	: _Device(other._Device), _FamilyIndex(other._FamilyIndex), _Index(other._Index), _CanPresent(other._CanPresent), _Properties(other._Properties)
{
	other._Handle = VK_NULL_HANDLE;
	other._FamilyIndex = {};
	other._Index = 0;
	other._CanPresent = VK_FALSE;
	other._Properties = {};
}

VkResult vge::Queue::Submit(const std::vector<VkSubmitInfo>& submitInfos, VkFence fence) const
{
	return vkQueueSubmit(_Handle, ToU32(submitInfos.size()), submitInfos.data(), fence);
}

VkResult vge::Queue::Submit(const CommandBuffer& commandBuffer, VkFence fence) const
{
	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer.GetHandle();

	return Submit({ submitInfo }, fence);
}

VkResult vge::Queue::Present(const VkPresentInfoKHR& presentInfos) const
{
	if (!_CanPresent)
	{
		return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
	}

	return vkQueuePresentKHR(_Handle, &presentInfos);
}
