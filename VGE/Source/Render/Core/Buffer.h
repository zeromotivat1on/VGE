#pragma once

#include "Core/Helpers.h"
#include "Core/VkCommon.h"
#include "Core/VulkanResource.h"

namespace vge
{
class Device;

class Buffer : public VulkanResource<VkBuffer, VK_OBJECT_TYPE_BUFFER, const Device>
{
public:
	Buffer( // create buffer using vma
		const Device& device, 
		VkDeviceSize size, 
		VkBufferUsageFlags bufferUsage, 
		VmaMemoryUsage memoryUsage, 
		VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		const std::vector<u32>& queueFamilyIndices = {});

	COPY_CTOR_DEL(Buffer);

	Buffer(Buffer&& other);

	COPY_OP_DEL(Buffer);
	
	MOVE_OP_DEL(Buffer);

	~Buffer();

public:
	inline const VkBuffer* Get() const { return &_Handle; };
	inline const u8* GetData() const { return _MappedData; };
	inline VmaAllocation GetAllocation() const { return _Allocation; }
	inline VkDeviceMemory GetMemory() const { return _Memory; }
	inline VkDeviceSize GetSize() const { return _Size; }

	void Flush() const; // flushes memory if it is HOST_VISIBLE and not HOST_COHERENT
	
	uint8_t* Map(); // maps vulkan memory if it hasn't been already mapped to a host visible address
	void Unmap(); // unmaps vulkan memory from the host visible address
	
	void Update(const u8* data, size_t size, size_t offset = 0); // copies byte data into the buffer
	inline void Update(const void* data, size_t size, size_t offset = 0) { Update(reinterpret_cast<const uint8_t*>(data), size, offset); }
	inline void Update(const std::vector<uint8_t>& data, size_t offset = 0) { Update(data.data(), data.size(), offset); }
	
	template <typename T, size_t N>
	inline void Update(std::array<T, N> const& data, size_t offset = 0) { Update(data.data(), data.size() * sizeof(T), offset); }

	template <typename T>
	inline void Update(std::vector<T> const& data, size_t offset = 0) { Update(data.data(), data.size() * sizeof(T), offset); }

	template <class T>
	inline void Update(const T& object, size_t offset = 0) { Update(reinterpret_cast<const uint8_t*>(&object), sizeof(T), offset); }

	u64 GetDeviceAddress(); // return the buffer's device address (requires that the buffer has been created with the VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT usage flag)

private:
	VmaAllocation _Allocation = VK_NULL_HANDLE;
	VkDeviceMemory _Memory = VK_NULL_HANDLE;
	VkDeviceSize _Size = 0;
	u8* _MappedData = nullptr;
	u8 _Persistent:1; // whether the buffer is persistently mapped or not
	u8 _Mapped:1; // whether the buffer has been mapped with vmaMapMemory
};
}	// namespace vge

