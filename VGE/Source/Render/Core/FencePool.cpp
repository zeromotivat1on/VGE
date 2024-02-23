#include "FencePool.h"
#include "Device.h"

vge::FencePool::FencePool(Device& device) 
	: _Device{ device }
{
}

vge::FencePool::~FencePool()
{
	Wait();
	Reset();

	for (VkFence fence : _Fences)
	{
		vkDestroyFence(_Device.GetHandle(), fence, nullptr);
	}

	_Fences.clear();
}

VkFence vge::FencePool::RequestFence()
{
	// Check if there is an available fence
	if (_ActiveFenceCount < _Fences.size())
	{
		return _Fences[_ActiveFenceCount++];
	}

	VkFence fence = VK_NULL_HANDLE;
	VkFenceCreateInfo createInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	
	VK_ENSURE(vkCreateFence(_Device.GetHandle(), &createInfo, nullptr, &fence));

	_Fences.push_back(fence);

	_ActiveFenceCount++;

	return _Fences.back();
}

VkResult vge::FencePool::Wait(u64 timeout /*= DEFAULT_FENCE_TIMEOUT*/) const
{
	if (_ActiveFenceCount < 1 || _Fences.empty())
	{
		return VK_SUCCESS;
	}

	return vkWaitForFences(_Device.GetHandle(), _ActiveFenceCount, _Fences.data(), true, timeout);
}

VkResult vge::FencePool::Reset()
{
	if (_ActiveFenceCount < 1 || _Fences.empty())
	{
		return VK_SUCCESS;
	}

	VkResult result = vkResetFences(_Device.GetHandle(), _ActiveFenceCount, _Fences.data());

	if (result != VK_SUCCESS)
	{
		return result;
	}

	_ActiveFenceCount = 0;

	return VK_SUCCESS;
}
