#pragma once

#include "Core/VulkanResource.h"
#include "Core/PhysicalDevice.h"
#include "Core/ResourceCache.h"
#include "Core/CommandPool.h"
#include "Core/FencePool.h"
#include "Core/Instance.h"
#include "Core/Buffer.h"
#include "Core/Queue.h"

namespace vge
{
struct DriverVersion
{
	u16 Major;
	u16 Minor;
	u16 Patch;
};

class Device : public VulkanResource<VkDevice, VK_OBJECT_TYPE_DEVICE>
{
public:
	Device(PhysicalDevice& gpu, VkSurfaceKHR surface, /*std::unique_ptr<DebugUtils>&& debug_utils,*/ std::unordered_map<const char*, bool> requestedExtensions = {});
	Device(PhysicalDevice& gpu, VkDevice& vulkanDevice, VkSurfaceKHR surface);

	COPY_CTOR_DEL(Device);
	MOVE_CTOR_DEL(Device);

	~Device();

	COPY_OP_DEL(Device);
	MOVE_OP_DEL(Device);

public:
	inline const PhysicalDevice& GetGpu() const { return _Gpu; }
	inline VmaAllocator GetMemoryAllocator() const { return _MemoryAllocator; }
	inline CommandPool& GetCommandPool() const { return *_CommandPool; }
	inline FencePool& GetFencePool() const { return *_FencePool; }
	inline const Queue& GetQueue(u32 queueFamilyIndex, u32 queueIndex) const { return _Queues[queueFamilyIndex][queueIndex]; }
	inline ResourceCache& GetResourceCache() { return _ResourceCache; }
	inline CommandBuffer& RequestCommandBuffer() const { return _CommandPool->RequestCommandBuffer(); }
	inline VkFence RequestFence() const { return _FencePool->RequestFence(); }
	//inline void CreateInternalFencePool() { _FencePool = std::make_unique<FencePool>(*this); }
	//inline void CreateInternalCommandPool() { _CommandPool = std::make_unique<CommandPool>(*this, GetQueueByFlags(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0).GetFamilyIndex()); }
	inline void WaitIdle() const { vkDeviceWaitIdle(_Handle); }

	DriverVersion GetDriverVersion() const;

	bool IsExtensionSupported(const char* extension) const;
	bool IsExtensionEnabled(const char* extension) const;

	bool IsImageFormatSupported(VkFormat format) const;

	const Queue& GetQueueByFlags(VkQueueFlags requiredQueueFlags, u32 queueIndex) const;
	const Queue& GetQueueByPresent(u32 queueIndex) const;
	const Queue& GetSuitableGraphicsQueue() const;
	u32 GetQueueFamilyIndex(VkQueueFlagBits queueFlag) const;
	u32 GetNumQueuesForQueueFamily(u32 queueFamilyIndex) const;
	void AddQueue(size_t globalIndex, u32 familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent);

	u32 GetMemoryType(u32 bits, VkMemoryPropertyFlags properties, VkBool32* memoryTypeFound = nullptr) const;

	VkCommandPool CreateCommandPool(u32 queueIndex, VkCommandPoolCreateFlags flags = 0) const;
	VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false) const;
	void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true, VkSemaphore signalSemaphore = VK_NULL_HANDLE) const;

	void CopyBuffer(Buffer& src, Buffer& dst, VkQueue queue, VkBufferCopy* copyRegion = nullptr) const;

	//void CreateMemoryAllocator();

private:
	const PhysicalDevice& _Gpu;
	VkSurfaceKHR _Surface = VK_NULL_HANDLE;
	VmaAllocator _MemoryAllocator = VK_NULL_HANDLE;
	//std::unique_ptr<DebugUtils> _DebugUtils;
	std::unique_ptr<CommandPool> _CommandPool; // command pool associated to the primary queue
	std::unique_ptr<FencePool> _FencePool; // fence pool associated to the primary queue
	std::vector<VkExtensionProperties> _DeviceExtensions;
	std::vector<const char*> _EnabledExtensions;
	std::vector<std::vector<Queue>> _Queues;
	ResourceCache _ResourceCache;
};
}	// namepsace vge

