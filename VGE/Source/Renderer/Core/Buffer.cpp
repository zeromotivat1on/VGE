#include "Buffer.h"
#include "Device.h"

vge::Buffer::Buffer(
	const Device& device, 
	VkDeviceSize size, 
	VkBufferUsageFlags bufferUsage, 
	VmaMemoryUsage memoryUsage, 
	VmaAllocationCreateFlags flags, /*= VMA_ALLOCATION_CREATE_MAPPED_BIT*/
	const std::vector<u32>& queueFamilyIndices /*= {}*/) 
	: VulkanResource(VK_NULL_HANDLE, &device), _Size(size)
{
	_Persistent = (flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0;

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.usage = bufferUsage;
	bufferInfo.size = size;
	if (queueFamilyIndices.size() >= 2)
	{
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
		bufferInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}

	VmaAllocationCreateInfo memory_info{};
	memory_info.flags = flags;
	memory_info.usage = memoryUsage;

	VmaAllocationInfo allocation_info{};
	VK_ENSURE(vmaCreateBuffer(device.GetMemoryAllocator(), &bufferInfo, &memory_info, &_Handle, &_Allocation, &allocation_info));

	_Memory = allocation_info.deviceMemory;

	if (_Persistent)
	{
		_MappedData = static_cast<uint8_t*>(allocation_info.pMappedData);
	}
}

vge::Buffer::Buffer(Buffer&& other)
{
	// Reset other handles to avoid releasing on destruction
	other._Allocation = VK_NULL_HANDLE;
	other._Memory = VK_NULL_HANDLE;
	other._MappedData = nullptr;
	other._Mapped = false;
}

vge::Buffer::~Buffer()
{
	if (_Handle != VK_NULL_HANDLE && _Allocation != VK_NULL_HANDLE)
	{
		Unmap();
		vmaDestroyBuffer(_Device->GetMemoryAllocator(), _Handle, _Allocation);
	}
}

void vge::Buffer::Flush() const
{
	vmaFlushAllocation(_Device->GetMemoryAllocator(), _Allocation, 0, _Size);
}

uint8_t* vge::Buffer::Map()
{
	if (!_Mapped && !_MappedData)
	{
		VK_ENSURE(vmaMapMemory(_Device->GetMemoryAllocator(), _Allocation, reinterpret_cast<void**>(&_MappedData)));
		_Mapped = 1;
	}
	return _MappedData;
}

void vge::Buffer::Unmap()
{
	if (_Mapped)
	{
		vmaUnmapMemory(_Device->GetMemoryAllocator(), _Allocation);
		_MappedData = nullptr;
		_Mapped = 0;
	}
}

void vge::Buffer::Update(const u8* data, size_t size, size_t offset /*= 0*/)
{
	if (_Persistent)
	{
		std::copy(data, data + size, _MappedData + offset);
		Flush();
	}
	else
	{
		Map();
		std::copy(data, data + size, _MappedData + offset);
		Flush();
		Unmap();
	}
}

vge::u64 vge::Buffer::GetDeviceAddress()
{
	VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
	bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferDeviceAddressInfo.buffer = _Handle;
	return vkGetBufferDeviceAddressKHR(_Device->GetHandle(), &bufferDeviceAddressInfo);
}
