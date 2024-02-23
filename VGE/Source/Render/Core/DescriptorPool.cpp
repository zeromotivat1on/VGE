#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "Device.h"

vge::DescriptorPool::DescriptorPool(Device& device, const DescriptorSetLayout& descriptorSetLayout, u32 poolSize) 
	: _Device(device), _DescriptorSetLayout(&descriptorSetLayout)
{
	const auto& bindings = descriptorSetLayout.GetBindings();
	std::map<VkDescriptorType, u32> descriptorTypeCounts;

	// Count each type of descriptor set.
	for (auto& binding : bindings)
	{
		descriptorTypeCounts[binding.descriptorType] += binding.descriptorCount;
	}

	// Allocate pool sizes array.
	_PoolSizes.resize(descriptorTypeCounts.size());

	auto poolSizeIt = _PoolSizes.begin();

	// Fill pool size for each descriptor type count multiplied by the pool size.
	for (auto& it : descriptorTypeCounts)
	{
		poolSizeIt->type = it.first;
		poolSizeIt->descriptorCount = it.second * poolSize;

		++poolSizeIt;
	}

	_PoolMaxSets = poolSize;
}

vge::DescriptorPool::~DescriptorPool()
{
	// Destroy all descriptor pools
	for (auto pool : _Pools)
	{
		vkDestroyDescriptorPool(_Device.GetHandle(), pool, nullptr);
	}
}

void vge::DescriptorPool::Reset()
{
	// Reset all descriptor pools.
	for (auto pool : _Pools)
	{
		vkResetDescriptorPool(_Device.GetHandle(), pool, 0);
	}

	// Clear internal tracking of descriptor set allocations.
	std::fill(_PoolSetsCount.begin(), _PoolSetsCount.end(), 0);
	_SetPoolMapping.clear();

	// Reset the pool index from which descriptor sets are allocated.
	_PoolIndex = 0;
}

VkDescriptorSet vge::DescriptorPool::Allocate()
{
	_PoolIndex = FindAvailablePool(_PoolIndex);

	// Increment allocated set count for the current pool.
	++_PoolSetsCount[_PoolIndex];

	VkDescriptorSetLayout setLayout = GetDescriptorSetLayout().GetHandle();

	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = _Pools[_PoolIndex];
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &setLayout;

	VkDescriptorSet handle = VK_NULL_HANDLE;

	// Allocate a new descriptor set from the current pool.
	auto result = vkAllocateDescriptorSets(_Device.GetHandle(), &allocInfo, &handle);

	if (result != VK_SUCCESS)
	{
		// Decrement allocated set count for the current pool.
		--_PoolSetsCount[_PoolIndex];

		return VK_NULL_HANDLE;
	}

	// Store mapping between the descriptor set and the pool.
	_SetPoolMapping.emplace(handle, _PoolIndex);

	return handle;
}

VkResult vge::DescriptorPool::Free(VkDescriptorSet descriptor_set)
{
	// Get the pool index of the descriptor set.
	auto it = _SetPoolMapping.find(descriptor_set);

	if (it == _SetPoolMapping.end())
	{
		return VK_INCOMPLETE;
	}

	auto descPoolIndex = it->second;

	// Free descriptor set from the pool.
	vkFreeDescriptorSets(_Device.GetHandle(), _Pools[descPoolIndex], 1, &descriptor_set);

	// Remove descriptor set mapping to the pool.
	_SetPoolMapping.erase(it);

	// Decrement allocated set count for the pool.
	--_PoolSetsCount[descPoolIndex];

	// Change the current pool index to use the available pool.
	_PoolIndex = descPoolIndex;

	return VK_SUCCESS;
}

vge::u32 vge::DescriptorPool::FindAvailablePool(u32 searchIndex)
{
	// Create a new pool
	if (_Pools.size() <= searchIndex)
	{
		VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		createInfo.poolSizeCount = ToU32(_PoolSizes.size());
		createInfo.pPoolSizes = _PoolSizes.data();
		createInfo.maxSets = _PoolMaxSets;
		createInfo.flags = 0; // we do not set FREE_DESCRIPTOR_SET_BIT as we do not need to free individual descriptor sets

		// Check descriptor set layout and enable the required flags.
		auto& bindingFlags = _DescriptorSetLayout->GetBindingFlags();
		for (auto bindingFlag : bindingFlags)
		{
			if (bindingFlag & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT)
			{
				createInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
			}
		}

		VkDescriptorPool handle = VK_NULL_HANDLE;

		// Create the Vulkan descriptor pool.
		auto result = vkCreateDescriptorPool(_Device.GetHandle(), &createInfo, nullptr, &handle);

		if (result != VK_SUCCESS)
		{
			return 0;
		}

		// Store internally the Vulkan handle.
		_Pools.push_back(handle);

		// Add set count for the descriptor pool.
		_PoolSetsCount.push_back(0);

		return searchIndex;
	}
	else if (_PoolSetsCount[searchIndex] < _PoolMaxSets)
	{
		return searchIndex;
	}

	// Increment pool index.
	return FindAvailablePool(++searchIndex);
}
