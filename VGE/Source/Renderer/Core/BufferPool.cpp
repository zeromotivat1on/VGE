#include "BufferPool.h"
#include "Device.h"

vge::BufferAllocation::BufferAllocation(Buffer& buffer, VkDeviceSize size, VkDeviceSize offset) 
	: _Buffer(&buffer), _Size(size), _BaseOffset(offset)
{
}

void vge::BufferAllocation::Update(const std::vector<uint8_t>& data, uint32_t offset)
{
	ASSERT(_Buffer);

	if (offset + data.size() <= _Size)
	{
		_Buffer->Update(data, ToU32(_BaseOffset) + offset);
	}
	else
	{
		LOG(Error, "Ignore buffer allocation update.");
	}
}

vge::BufferBlock::BufferBlock(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) 
	: _Buffer(device, size, usage, memoryUsage)
{
	if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
	{
		_Alignment = device.GetGpu().GetProperties().limits.minUniformBufferOffsetAlignment;
	}
	else if (usage == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
	{
		_Alignment = device.GetGpu().GetProperties().limits.minStorageBufferOffsetAlignment;
	}
	else if (usage == VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)
	{
		_Alignment = device.GetGpu().GetProperties().limits.minTexelBufferOffsetAlignment;
	}
	else if (usage == VK_BUFFER_USAGE_INDEX_BUFFER_BIT || usage == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT || usage == VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
	{
		// Used to calculate the offset, required when allocating memory (its value should be power of 2).
		_Alignment = 16;
	}
	else
	{
		ENSURE_MSG(false, "Buffer memory usage not recognized.");
	}
}

vge::BufferAllocation vge::BufferBlock::Allocate(VkDeviceSize size)
{
	if (CanAllocate(size))
	{
		// Move the current offset and return an allocation.
		const VkDeviceSize aligned = AlignedOffset();
		_Offset = aligned + size;
		return BufferAllocation(_Buffer, size, aligned);
	}

	// No more space available from the underlying buffer, return empty allocation.
	return BufferAllocation();
}

vge::BufferPool::BufferPool(Device& device, VkDeviceSize blockSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) 
	: _Device(device), _BlockSize(blockSize), _Usage(usage), _MemoryUsage(memoryUsage)
{
}

vge::BufferBlock& vge::BufferPool::RequestBufferBlock(const VkDeviceSize minimumSize, bool minimal)
{
	// Find a block in the range of the blocks which can fit the minimum size.
	auto it = minimal ? 
		std::find_if(_BufferBlocks.begin(), _BufferBlocks.end(),
			[&minimumSize](const std::unique_ptr<BufferBlock>& bufferBlock) { return (bufferBlock->GetSize() == minimumSize) && bufferBlock->CanAllocate(minimumSize); }) 
		:
		std::find_if(_BufferBlocks.begin(), _BufferBlocks.end(),
			[&minimumSize](const std::unique_ptr<BufferBlock>& bufferBlock) { return bufferBlock->CanAllocate(minimumSize); });

	if (it == _BufferBlocks.end())
	{
		LOG(Log, "Building %d buffer block (%s).", _BufferBlocks.size(), ToString(_Usage));

		VkDeviceSize newBlockSize = minimal ? minimumSize : std::max(_BlockSize, minimumSize);
		it = _BufferBlocks.emplace(_BufferBlocks.end(), std::make_unique<BufferBlock>(_Device, newBlockSize, _Usage, _MemoryUsage));
	}

	return *it->get();
}

void vge::BufferPool::Reset()
{
	for (auto& bufferBlock : _BufferBlocks)
	{
		bufferBlock->Reset();
	}
}
