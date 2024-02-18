#include "SemaphorePool.h"
#include "Device.h"

vge::SemaphorePool::SemaphorePool(Device& device) 
	: device(device)
{
}

vge::SemaphorePool::~SemaphorePool()
{
	Reset();

	// Destroy all semaphores
	for (VkSemaphore semaphore : _Semaphores)
	{
		vkDestroySemaphore(_Device.GetHandle(), semaphore, nullptr);
	}

	_Semaphores.clear();
}

VkSemaphore vge::SemaphorePool::RequestSemaphoreWithOwnership()
{
	// Check if there is an available semaphore, if so, just pilfer one.
	if (_ActiveSemaphoreCount < _Semaphores.size())
	{
		VkSemaphore semaphore = _Semaphores.back();
		_Semaphores.pop_back();
		return semaphore;
	}

	// Otherwise, we need to create one, and don't keep track of it, app will release.
	VkSemaphore semaphore = VK_NULL_HANDLE;
	VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

	VK_ENSURE(vkCreateSemaphore(_Device.GetHandle(), &createInfo, nullptr, &semaphore));

	return semaphore;
}

void vge::SemaphorePool::ReleaseOwnedSemaphore(VkSemaphore semaphore)
{
	// We cannot reuse this semaphore until ::Reset().
	_ReleasedSemaphores.push_back(semaphore);
}

VkSemaphore vge::SemaphorePool::RequestSemaphore()
{
	// Check if there is an available semaphore.
	if (_ActiveSemaphoreCount < _Semaphores.size())
	{
		return _Semaphores[_ActiveSemaphoreCount++];
	}

	VkSemaphore semaphore = VK_NULL_HANDLE;
	VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	
	VK_ENSURE(vkCreateSemaphore(_Device.GetHandle(), &createInfo, nullptr, &semaphore));

	_Semaphores.push_back(semaphore);

	_ActiveSemaphoreCount++;

	return semaphore;
}

void vge::SemaphorePool::Reset()
{
	_ActiveSemaphoreCount = 0;

	// Now we can safely recycle the released semaphores.
	for (auto& semaphore : _ReleasedSemaphores)
	{
		_Semaphores.push_back(semaphore);
	}

	_ReleasedSemaphores.clear();
}