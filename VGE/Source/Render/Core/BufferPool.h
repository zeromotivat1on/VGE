#pragma once

#include "Core/Buffer.h"
#include "Core/Error.h"

namespace vge
{
class Device;

// An allocation of vulkan memory; different buffer allocations, with different offset and size, may come from the same Vulkan buffer
class BufferAllocation
{
public:
	BufferAllocation() = default;
	BufferAllocation(Buffer& buffer, VkDeviceSize size, VkDeviceSize offset);

	COPY_CTOR_DEL(BufferAllocation);
	MOVE_CTOR_DEF(BufferAllocation);
	
	COPY_OP_DEL(BufferAllocation);
	MOVE_OP_DEF(BufferAllocation);

public:
	inline bool IsEmpty() const { return _Size == 0 || _Buffer == nullptr; }
	inline VkDeviceSize GetSize() const { return _Size; }
	inline VkDeviceSize GetOffset() const { return _BaseOffset; }
	inline Buffer& GetBuffer() { ASSERT(_Buffer); return *_Buffer; }

	void Update(const std::vector<uint8_t>& data, u32 offset = 0);

	template <class T>
	void Update(const T& value, u32 offset = 0)
	{
		Update(ToBytes(value), offset);
	}

private:
	Buffer* _Buffer = nullptr;
	VkDeviceSize _BaseOffset = 0;
	VkDeviceSize _Size = 0;
};

/**
	* @brief Helper class which handles multiple allocation from the same underlying Vulkan buffer.
	*/
class BufferBlock
{
public:
	BufferBlock(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

public:
	inline VkDeviceSize GetSize() const { return _Buffer.GetSize(); }
	inline bool CanAllocate(VkDeviceSize size) const { ASSERT(size > 0); return (AlignedOffset() + size <= _Buffer.GetSize()); }
	
	inline void Reset() { _Offset = 0; }

	BufferAllocation Allocate(VkDeviceSize size); //allocate an usable view on a portion of the underlying buffer

private:
	inline VkDeviceSize AlignedOffset() const { return (_Offset + _Alignment - 1) & ~(_Alignment - 1); }

private:
	Buffer _Buffer;
	VkDeviceSize _Alignment = 0; // memory alignment, may change according to the usage
	VkDeviceSize _Offset = 0; // current offset, increases on every allocation
};

/**
* A pool of buffer blocks for a specific usage. It may contain inactive blocks that can be recycled.
*
* BufferPool is a linear allocator for buffer chunks, it gives you a view of the size you want.
* A BufferBlock is the corresponding VkBuffer and you can get smaller offsets inside it.
* Since a shader cannot specify dynamic UBOs, it has to be done from the code (set_resource_dynamic).
*
* When a new frame starts, buffer blocks are returned, the offset is reset and contents are overwritten. 
* The minimum allocation size is 256 kb, if you ask for more you get a dedicated buffer allocation.
*
* We re-use descriptor sets: we only need one for the corresponding buffer infos (and we only
* have one VkBuffer per BufferBlock), then it is bound and we use dynamic offsets.
*/
class BufferPool
{
public:
	BufferPool(Device& device, VkDeviceSize blockSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU);

public:
	BufferBlock& RequestBufferBlock(VkDeviceSize minimumSize, bool minimal = false);
	void Reset();

private:
	Device& _Device;
	std::vector<std::unique_ptr<BufferBlock>> _BufferBlocks;
	VkDeviceSize _BlockSize = 0;
	VkBufferUsageFlags _Usage;
	VmaMemoryUsage _MemoryUsage;
};
}	// namespace vge
