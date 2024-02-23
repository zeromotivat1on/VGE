#include "RenderFrame.h"
#include "Core/Device.h"
#include "Core/PipelineState.h"
#include "Core/RenderContext.h"

vge::RenderFrame::RenderFrame(Device& device, std::unique_ptr<RenderTarget>&& renderTarget, size_t threadCount) 
	: _Device(device), _FencePool(device), _SemaphorePool(device), _SwapchainRenderTarget(std::move(renderTarget)), _ThreadCount(threadCount)
{
	for (auto& usageIt : SupportedUsageMap)
	{
		std::vector<std::pair<BufferPool, BufferBlock*>> usageBufferPools;
		for (size_t i = 0; i < threadCount; ++i)
		{
			usageBufferPools.push_back(std::make_pair(BufferPool(device, BufferPoolBlockSize * 1024 * usageIt.second, usageIt.first), nullptr));
		}

		auto insertedIt = _BufferPools.emplace(usageIt.first, std::move(usageBufferPools));
		ENSURE_MSG(insertedIt.second, "Failed to insert buffer pool.");
	}

	for (size_t i = 0; i < threadCount; ++i)
	{
		_DescriptorPools.push_back(std::make_unique<std::unordered_map<size_t, DescriptorPool>>());
		_DescriptorSets.push_back(std::make_unique<std::unordered_map<size_t, DescriptorSet>>());
	}
}

void vge::RenderFrame::Reset()
{
	VK_ENSURE(_FencePool.Wait());

	_FencePool.Reset();

	for (auto& commandPoolsPerQueue : _CommandPools)
	{
		for (auto& commandPool : commandPoolsPerQueue.second)
		{
			commandPool->ResetPool();
		}
	}

	for (auto& bufferPoolsPerUsage : _BufferPools)
	{
		for (auto& bufferPool : bufferPoolsPerUsage.second)
		{
			bufferPool.first.Reset();
			bufferPool.second = nullptr;
		}
	}

	_SemaphorePool.Reset();

	if (_DescriptorManagementStrategy == DescriptorManagementStrategy::CreateDirectly)
	{
		ClearDescriptors();
	}
}

std::vector<std::unique_ptr<vge::CommandPool>>& vge::RenderFrame::GetCommandPools(const Queue& queue, CommandBuffer::ResetMode resetMode)
{
	auto commandPoolIt = _CommandPools.find(queue.GetFamilyIndex());

	if (commandPoolIt != _CommandPools.end())
	{
		ASSERT(!commandPoolIt->second.empty());
		if (commandPoolIt->second[0]->GetResetMode() != resetMode)
		{
			_Device.WaitIdle();
			_CommandPools.erase(commandPoolIt);
		}
		else
		{
			return commandPoolIt->second;
		}
	}

	std::vector<std::unique_ptr<CommandPool>> queueCommandPools;
	for (size_t i = 0; i < _ThreadCount; i++)
	{
		queueCommandPools.push_back(std::make_unique<CommandPool>(_Device, queue.GetFamilyIndex(), this, i, resetMode));
	}

	auto insertedIt = _CommandPools.emplace(queue.GetFamilyIndex(), std::move(queueCommandPools));
	ENSURE_MSG(insertedIt.second, "Failed to insert command pool.")

	commandPoolIt = insertedIt.first;

	return commandPoolIt->second;
}

std::vector<vge::u32> vge::RenderFrame::CollectBindingsToUpdate(const DescriptorSetLayout& descriptorSetLayout, const BindingMap<VkDescriptorBufferInfo>& bufferInfos, const BindingMap<VkDescriptorImageInfo>& imageInfos)
{
	std::vector<u32> bindingsToUpdate;

	bindingsToUpdate.reserve(bufferInfos.size() + imageInfos.size());
	auto aggregateBindingToUpdate = [&bindingsToUpdate, &descriptorSetLayout](const auto& infosMap) 
	{
		for (const auto& pair : infosMap)
		{
			const u32 bindingIndex = pair.first;
			if (!(descriptorSetLayout.GetLayoutBindingFlag(bindingIndex) & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT) &&
				std::find(bindingsToUpdate.begin(), bindingsToUpdate.end(), bindingIndex) == bindingsToUpdate.end())
			{
				bindingsToUpdate.push_back(bindingIndex);
			}
		}
	};

	aggregateBindingToUpdate(bufferInfos);
	aggregateBindingToUpdate(imageInfos);

	return bindingsToUpdate;
}


vge::CommandBuffer& vge::RenderFrame::RequestCommandBuffer(const Queue& queue, CommandBuffer::ResetMode resetMode, VkCommandBufferLevel level, size_t threadIndex)
{
	ASSERT_MSG(threadIndex < _ThreadCount, "Thread index is out of bounds.");

	auto& commandPools = GetCommandPools(queue, resetMode);
	auto commandPoolIt = std::find_if(commandPools.begin(), commandPools.end(), [&threadIndex](std::unique_ptr<CommandPool>& cmdPool) { return cmdPool->GetThreadIndex() == threadIndex; });

	return (*commandPoolIt)->RequestCommandBuffer(level);
}

VkDescriptorSet vge::RenderFrame::RequestDescriptorSet(const DescriptorSetLayout& descriptorSetLayout, const BindingMap<VkDescriptorBufferInfo>& bufferInfos, const BindingMap<VkDescriptorImageInfo>& imageInfos, bool updateAfterBind, size_t threadIndex)
{
	ASSERT_MSG(threadIndex < _ThreadCount, "Thread index is out of bounds.");
	ASSERT(threadIndex < _DescriptorPools.size());

	auto& descriptorPool = RequestResource(_Device, nullptr, *_DescriptorPools[threadIndex], descriptorSetLayout);
	if (_DescriptorManagementStrategy == DescriptorManagementStrategy::StoreInCache)
	{
		// The bindings we want to update before binding, if empty we update all bindings.
		std::vector<u32> bindingsToUpdate;

		// If update after bind is enabled, we store the binding index of each binding that need to be updated before being bound.
		if (updateAfterBind)
		{
			bindingsToUpdate = CollectBindingsToUpdate(descriptorSetLayout, bufferInfos, imageInfos);
		}

		// Request a descriptor set from the render frame, and write the buffer infos and image infos of all the specified bindings
		ASSERT(threadIndex < _DescriptorSets.size());

		auto& descriptorSet = RequestResource(_Device, nullptr, *_DescriptorSets[threadIndex], descriptorSetLayout, descriptorPool, bufferInfos, imageInfos);
		descriptorSet.Update(bindingsToUpdate);

		return descriptorSet.GetHandle();
	}
	else
	{
		// Request a descriptor pool, allocate a descriptor set, write buffer and image data to it.
		DescriptorSet descriptorSet = DescriptorSet(_Device, descriptorSetLayout, descriptorPool, bufferInfos, imageInfos);
		descriptorSet.ApplyWrites();

		return descriptorSet.GetHandle();
	}
}

void vge::RenderFrame::UpdateDescriptorSets(size_t threadIndex)
{
	ASSERT(threadIndex < _DescriptorSets.size());

	auto& threadDescriptorSets = *_DescriptorSets[threadIndex];
	for (auto& descriptorSetIt : threadDescriptorSets)
	{
		descriptorSetIt.second.Update();
	}
}

void vge::RenderFrame::ClearDescriptors()
{
	for (auto& descSetsPerThread : _DescriptorSets)
	{
		descSetsPerThread->clear();
	}

	for (auto& descPoolsPerThread : _DescriptorPools)
	{
		for (auto& descPool : *descPoolsPerThread)
		{
			descPool.second.Reset();
		}
	}
}

vge::BufferAllocation vge::RenderFrame::AllocateBuffer(const VkBufferUsageFlags usage, const VkDeviceSize size, size_t threadIndex)
{
	ASSERT_MSG(threadIndex < _ThreadCount, "Thread index is out of bounds.");

	// Find a pool for this usage
	auto bufferPoolIt = _BufferPools.find(usage);
	if (bufferPoolIt == _BufferPools.end())
	{
		LOG(Error, "No buffer pool for buffer usage %s.", ToString(usage));
		return BufferAllocation{};
	}

	ASSERT(threadIndex < bufferPoolIt->second.size());

	auto& bufferPool = bufferPoolIt->second[threadIndex].first;
	auto& bufferBlock = bufferPoolIt->second[threadIndex].second;

	bool wantMinimalBlock = _BufferAllocationStrategy == BufferAllocationStrategy::OneAllocationPerBuffer;

	if (wantMinimalBlock || !bufferBlock || !bufferBlock->CanAllocate(size))
	{
		// If we are creating a buffer for each allocation of there is no block associated with the pool
		// or the current block is too small for this allocation, request a new buffer block.
		bufferBlock = &bufferPool.RequestBufferBlock(size, wantMinimalBlock);
	}

	return bufferBlock->Allocate(ToU32(size));
}
