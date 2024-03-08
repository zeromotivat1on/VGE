#include "Device.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

vge::Device::Device(PhysicalDevice& gpu, VkSurfaceKHR surface, /*std::unique_ptr<DebugUtils>&& debug_utils,*/ std::unordered_map<const char*, bool> requestedExtensions /*= {}*/)
	: VulkanResource(VK_NULL_HANDLE, this), _Gpu(gpu), _ResourceCache(*this)
{
	LOG(Log, "Selected GPU: %s", gpu.GetProperties().deviceName);

	// Prepare the device queues.
	u32 queueFamilyPropertiesCount = ToU32(gpu.GetQueueFamilyProperties().size());
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(queueFamilyPropertiesCount, { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO });
	std::vector<std::vector<float>> queuePriorities(queueFamilyPropertiesCount);

	for (u32 queueFamilyIndex = 0U; queueFamilyIndex < queueFamilyPropertiesCount; ++queueFamilyIndex)
	{
		const VkQueueFamilyProperties& queueFamilyProperty = gpu.GetQueueFamilyProperties()[queueFamilyIndex];

		//if (gpu.has_high_priority_graphics_queue())
		//{
		//	u32 graphics_queue_family = get_queue_family_index(VK_QUEUE_GRAPHICS_BIT);
		//	if (graphics_queue_family == queueFamilyIndex)
		//	{
		//		queuePriorities[queueFamilyIndex].reserve(queueFamilyProperty.queueCount);
		//		queuePriorities[queueFamilyIndex].push_back(1.0f);
		//		for (u32 i = 1; i < queueFamilyProperty.queueCount; i++)
		//		{
		//			queuePriorities[queueFamilyIndex].push_back(0.5f);
		//		}
		//	}
		//	else
		//	{
		//		queuePriorities[queueFamilyIndex].resize(queueFamilyProperty.queueCount, 0.5f);
		//	}
		//}
		//else
		{
			queuePriorities[queueFamilyIndex].resize(queueFamilyProperty.queueCount, 0.5f);
		}

		VkDeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos[queueFamilyIndex];
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = queueFamilyProperty.queueCount;
		queueCreateInfo.pQueuePriorities = queuePriorities[queueFamilyIndex].data();
	}

	// Check extensions to enable Vma Dedicated Allocation.
	u32 deviceExtensionCount;
	VK_ENSURE(vkEnumerateDeviceExtensionProperties(gpu.GetHandle(), nullptr, &deviceExtensionCount, nullptr));
	_DeviceExtensions = std::vector<VkExtensionProperties>(deviceExtensionCount);
	VK_ENSURE(vkEnumerateDeviceExtensionProperties(gpu.GetHandle(), nullptr, &deviceExtensionCount, _DeviceExtensions.data()));

	// Display supported extensions.
	if (_DeviceExtensions.size() > 0)
	{
		LOG(Log, "Device supports the following extensions:");
		for (auto& extension : _DeviceExtensions)
		{
			LOG(Log, "  \t%s", extension.extensionName);
		}
	}

	const bool canGetMemoryRequirements = IsExtensionSupported("VK_KHR_get_memory_requirements2");
	const bool hasDedicatedAllocation = IsExtensionSupported("VK_KHR_dedicated_allocation");

	if (canGetMemoryRequirements && hasDedicatedAllocation)
	{
		_EnabledExtensions.push_back("VK_KHR_get_memory_requirements2");
		_EnabledExtensions.push_back("VK_KHR_dedicated_allocation");

		LOG(Log, "Dedicated Allocation enabled.");
	}

	// For performance queries, we also use host query reset since queryPool resets cannot
	// live in the same command buffer as beginQuery.
	if (IsExtensionSupported("VK_KHR_performance_query") && IsExtensionSupported("VK_EXT_host_query_reset"))
	{
		const auto perfCounterFeatures = gpu.RequestExtensionFeatures<VkPhysicalDevicePerformanceQueryFeaturesKHR>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR);
		const auto hostQueryResetFeatures = gpu.RequestExtensionFeatures<VkPhysicalDeviceHostQueryResetFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES);

		if (perfCounterFeatures.performanceCounterQueryPools && hostQueryResetFeatures.hostQueryReset)
		{
			_EnabledExtensions.push_back("VK_KHR_performance_query");
			_EnabledExtensions.push_back("VK_EXT_host_query_reset");
			LOG(Log, "Performance query enabled.");
		}
	}

	// Check that extensions are supported before trying to create the device.
	std::vector<const char*> unsupportedExtensions = {};
	for (auto& extension : requestedExtensions)
	{
		if (IsExtensionSupported(extension.first))
		{
			_EnabledExtensions.emplace_back(extension.first);
		}
		else
		{
			unsupportedExtensions.emplace_back(extension.first);
		}
	}

	if (_EnabledExtensions.size() > 0)
	{
		LOG(Log, "Device supports the following requested extensions:");
		for (auto& extension : _EnabledExtensions)
		{
			LOG(Log, "  \t%s", extension);
		}
	}

	if (unsupportedExtensions.size() > 0)
	{
		bool error = false;
		for (auto& extension : unsupportedExtensions)
		{
			const bool extensionIsOptional = requestedExtensions[extension];
			if (extensionIsOptional)
			{
				LOG(Warning, "Optional device extension %s not available, some features may be disabled.", extension);
			}
			else
			{
				LOG(Error, "Required device extension %s not available, cannot run.", extension);
				error = true;
			}
		}

		ENSURE_MSG(error, "Some of required extensions is not present.");
	}

	VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

	// Latest requested feature will have the pNext's all set up for device creation.
	createInfo.pNext = gpu.GetExtensionFeatureChain();

	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = ToU32(queueCreateInfos.size());
	createInfo.enabledExtensionCount = ToU32(_EnabledExtensions.size());
	createInfo.ppEnabledExtensionNames = _EnabledExtensions.data();

	const VkPhysicalDeviceFeatures& requestedGpuFeatures = gpu.GetRequestedFeatures();
	createInfo.pEnabledFeatures = &requestedGpuFeatures;

	VK_ENSURE(vkCreateDevice(gpu.GetHandle(), &createInfo, nullptr, &_Handle));

	_Queues.resize(queueFamilyPropertiesCount);

	for (u32 queueFamilyIndex = 0U; queueFamilyIndex < queueFamilyPropertiesCount; ++queueFamilyIndex)
	{
		const VkQueueFamilyProperties& queueFamilyProperty = gpu.GetQueueFamilyProperties()[queueFamilyIndex];

		VkBool32 presentSupported = gpu.IsPresentSupported(surface, queueFamilyIndex);

		for (u32 queue_index = 0U; queue_index < queueFamilyProperty.queueCount; ++queue_index)
		{
			_Queues[queueFamilyIndex].emplace_back(*this, queueFamilyIndex, queueFamilyProperty, presentSupported, queue_index);
		}
	}

	VmaVulkanFunctions vmaVulkanFuncs = {};
	vmaVulkanFuncs.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	vmaVulkanFuncs.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vmaVulkanFuncs.vkAllocateMemory = vkAllocateMemory;
	vmaVulkanFuncs.vkBindBufferMemory = vkBindBufferMemory;
	vmaVulkanFuncs.vkBindImageMemory = vkBindImageMemory;
	vmaVulkanFuncs.vkCreateBuffer = vkCreateBuffer;
	vmaVulkanFuncs.vkCreateImage = vkCreateImage;
	vmaVulkanFuncs.vkDestroyBuffer = vkDestroyBuffer;
	vmaVulkanFuncs.vkDestroyImage = vkDestroyImage;
	vmaVulkanFuncs.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vmaVulkanFuncs.vkFreeMemory = vkFreeMemory;
	vmaVulkanFuncs.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vmaVulkanFuncs.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vmaVulkanFuncs.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vmaVulkanFuncs.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vmaVulkanFuncs.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vmaVulkanFuncs.vkMapMemory = vkMapMemory;
	vmaVulkanFuncs.vkUnmapMemory = vkUnmapMemory;
	vmaVulkanFuncs.vkCmdCopyBuffer = vkCmdCopyBuffer;

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = gpu.GetHandle();
	allocatorInfo.device = _Handle;
	allocatorInfo.instance = gpu.GetInstance().GetHandle();

	if (canGetMemoryRequirements && hasDedicatedAllocation)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
		vmaVulkanFuncs.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
		vmaVulkanFuncs.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
	}

	if (IsExtensionSupported(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) && IsExtensionEnabled(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	}

	allocatorInfo.pVulkanFunctions = &vmaVulkanFuncs;

	VK_ENSURE(vmaCreateAllocator(&allocatorInfo, &_MemoryAllocator));

	_CommandPool = std::make_unique<CommandPool>(*this, GetQueueByFlags(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0).GetFamilyIndex());
	_FencePool = std::make_unique<FencePool>(*this);
}

vge::Device::Device(PhysicalDevice& gpu, VkDevice& vulkanDevice, VkSurfaceKHR surface) 
	: _Gpu(gpu), _ResourceCache(*this)
{
	_Handle = vulkanDevice;
}

vge::Device::~Device()
{
	_ResourceCache.Clear();

	_CommandPool.reset();
	_FencePool.reset();

	if (_MemoryAllocator)
	{
		//VmaStats stats;
		//vmaCalculateStats(_MemoryAllocator, &stats);
		//
		//LOG(Log, "Total device memory leaked: %d bytes.", stats.total.usedBytes);

		vmaDestroyAllocator(_MemoryAllocator);
	}

	if (_Handle)
	{
		vkDestroyDevice(_Handle, nullptr);
	}
}

vge::DriverVersion vge::Device::GetDriverVersion() const
{
	DriverVersion version;

	switch (_Gpu.GetProperties().vendorID)
	{
		case 0x10DE: // Nvidia
		{
			version.Major = (_Gpu.GetProperties().driverVersion >> 22) & 0x3ff;
			version.Minor = (_Gpu.GetProperties().driverVersion >> 14) & 0x0ff;
			version.Patch = (_Gpu.GetProperties().driverVersion >> 6) & 0x0ff;
			// Ignoring optional tertiary info in lower 6 bits.
			break;
		}
		default:
		{
			version.Major = VK_VERSION_MAJOR(_Gpu.GetProperties().driverVersion);
			version.Minor = VK_VERSION_MINOR(_Gpu.GetProperties().driverVersion);
			version.Patch = VK_VERSION_PATCH(_Gpu.GetProperties().driverVersion);
		}
	}

	return version;
}

bool vge::Device::IsExtensionSupported(const char* extension) const
{
	return std::find_if(_DeviceExtensions.begin(), _DeviceExtensions.end(),
		[extension](const VkExtensionProperties& deviceExtension) 
		{
			return std::strcmp(deviceExtension.extensionName, extension) == 0;
		}) != _DeviceExtensions.end();
}

bool vge::Device::IsExtensionEnabled(const char* extension) const
{
	return std::find_if(_EnabledExtensions.begin(), _EnabledExtensions.end(), 
		[extension](const char* enabled_extension) 
		{ 
			return strcmp(extension, enabled_extension) == 0; 
		}) != _EnabledExtensions.end();
}

bool vge::Device::IsImageFormatSupported(VkFormat format) const
{
	VkImageFormatProperties formatProperties;

	VkResult result = vkGetPhysicalDeviceImageFormatProperties(_Gpu.GetHandle(),
		format,
		VK_IMAGE_TYPE_2D,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_SAMPLED_BIT,
		0,	// no create flags
		&formatProperties);

	return result != VK_ERROR_FORMAT_NOT_SUPPORTED;
}

const vge::Queue& vge::Device::GetQueueByFlags(VkQueueFlags requiredQueueFlags, u32 queueIndex) const
{
	for (u32 queueFamilyIndex = 0U; queueFamilyIndex < _Queues.size(); ++queueFamilyIndex)
	{
		const Queue& firstQueue = _Queues[queueFamilyIndex][0];
		const VkQueueFlags queueFlags = firstQueue.GetProperties().queueFlags;
		const u32 queueCount = firstQueue.GetProperties().queueCount;

		if (((queueFlags & requiredQueueFlags) == requiredQueueFlags) && queueIndex < queueCount)
		{
			return _Queues[queueFamilyIndex][queueIndex];
		}
	}

	ENSURE_MSG(false, "Queue not found.")
}

const vge::Queue& vge::Device::GetQueueByPresent(u32 queueIndex) const
{
	for (u32 queueFamilyIndex = 0U; queueFamilyIndex < _Queues.size(); ++queueFamilyIndex)
	{
		const Queue& firstQueue = _Queues[queueFamilyIndex][0];
		const u32 queueCount = firstQueue.GetProperties().queueCount;

		if (firstQueue.SupportPresent() && queueIndex < queueCount)
		{
			return _Queues[queueFamilyIndex][queueIndex];
		}
	}

	ENSURE_MSG(false, "Queue not found.")
}

const vge::Queue& vge::Device::GetSuitableGraphicsQueue() const
{
	for (u32 queueFamilyIndex = 0U; queueFamilyIndex < _Queues.size(); ++queueFamilyIndex)
	{
		const Queue& firstQueue = _Queues[queueFamilyIndex][0];
		const u32 queueCount = firstQueue.GetProperties().queueCount;

		if (firstQueue.SupportPresent() && 0 < queueCount)
		{
			return _Queues[queueFamilyIndex][0];
		}
	}

	return GetQueueByFlags(VK_QUEUE_GRAPHICS_BIT, 0);
}

vge::u32 vge::Device::GetQueueFamilyIndex(VkQueueFlagBits queueFlag) const
{
	const auto& queueFamilyProperties = _Gpu.GetQueueFamilyProperties();

	// Dedicated queue for compute.
	// Try to find a queue family index that supports compute but not graphics.
	if (queueFlag & VK_QUEUE_COMPUTE_BIT)
	{
		for (u32 i = 0; i < ToU32(queueFamilyProperties.size()); i++)
		{
			if ((queueFamilyProperties[i].queueFlags & queueFlag) 
				&& !(queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				return i;
			}
		}
	}

	// Dedicated queue for transfer.
	// Try to find a queue family index that supports transfer but not graphics and compute.
	if (queueFlag & VK_QUEUE_TRANSFER_BIT)
	{
		for (u32 i = 0; i < ToU32(queueFamilyProperties.size()); i++)
		{
			if ((queueFamilyProperties[i].queueFlags & queueFlag) 
				&& !(queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
				&& !(queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
			{
				return i;
			}
		}
	}

	// For other queue types or if no separate compute queue is present, return the first one to support the requested flags.
	for (u32 i = 0; i < ToU32(queueFamilyProperties.size()); i++)
	{
		if (queueFamilyProperties[i].queueFlags & queueFlag)
		{
			return i;
		}
	}

	ENSURE_MSG(false, "Could not find a matching queue family index.");
}

vge::u32 vge::Device::GetNumQueuesForQueueFamily(u32 queueFamilyIndex) const
{
	const auto& queueFamilyProperties = _Gpu.GetQueueFamilyProperties();
	return queueFamilyProperties[queueFamilyIndex].queueCount;
}

void vge::Device::AddQueue(size_t globalIndex, u32 familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent)
{
	if (_Queues.size() < globalIndex + 1)
	{
		_Queues.resize(globalIndex + 1);
	}
	_Queues[globalIndex].emplace_back(*this, familyIndex, properties, canPresent, 0);
}

vge::u32 vge::Device::GetMemoryType(u32 bits, VkMemoryPropertyFlags properties, VkBool32* memoryTypeFound /*= nullptr*/) const
{
	for (u32 i = 0; i < _Gpu.GetMemoryProperties().memoryTypeCount; i++)
	{
		if ((bits & 1) == 1)
		{
			if ((_Gpu.GetMemoryProperties().memoryTypes[i].propertyFlags & properties) == properties)
			{
				if (memoryTypeFound)
				{
					*memoryTypeFound = true;
				}
				return i;
			}
		}
		bits >>= 1;
	}

	if (memoryTypeFound)
	{
		*memoryTypeFound = false;
		return 0;
	}
	else
	{
		ENSURE_MSG(false, "Could not find a matching memory type");
	}
}

VkCommandPool vge::Device::CreateCommandPool(u32 queueIndex, VkCommandPoolCreateFlags flags /*= 0*/) const
{
	VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	commandPoolInfo.queueFamilyIndex = queueIndex;
	commandPoolInfo.flags = flags;

	VkCommandPool commandPool;
	VK_ENSURE(vkCreateCommandPool(_Handle, &commandPoolInfo, nullptr, &commandPool));
	return commandPool;
}

VkCommandBuffer vge::Device::CreateCommandBuffer(VkCommandBufferLevel level, bool begin /*= false*/) const
{
	ASSERT(_CommandPool);

	VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
	cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.commandPool = _CommandPool->GetHandle();
	cmdBufAllocateInfo.level = level;
	cmdBufAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VK_ENSURE(vkAllocateCommandBuffers(_Handle, &cmdBufAllocateInfo, &commandBuffer));

	// If requested, also start recording for the new command buffer.
	if (begin)
	{
		VkCommandBufferBeginInfo commandBufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		VK_ENSURE(vkBeginCommandBuffer(commandBuffer, &commandBufferInfo));
	}

	return commandBuffer;
}

void vge::Device::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free /*= true*/, VkSemaphore signalSemaphore /*= VK_NULL_HANDLE*/) const
{
	if (!commandBuffer)
	{
		return;
	}

	VK_ENSURE(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	if (signalSemaphore)
	{
		submitInfo.pSignalSemaphores = &signalSemaphore;
		submitInfo.signalSemaphoreCount = 1;
	}

	// Create fence to ensure that the command buffer has finished executing.
	VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceInfo.flags = VK_FLAGS_NONE;

	VkFence fence;
	VK_ENSURE(vkCreateFence(_Handle, &fenceInfo, nullptr, &fence));

	// Submit to the queue.
	VkResult result = vkQueueSubmit(queue, 1, &submitInfo, fence);
	// Wait for the fence to signal that command buffer has finished executing.
	VK_ENSURE(vkWaitForFences(_Handle, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

	vkDestroyFence(_Handle, fence, nullptr);

	if (_CommandPool && free)
	{
		vkFreeCommandBuffers(_Handle, _CommandPool->GetHandle(), 1, &commandBuffer);
	}
}

void vge::Device::CopyBuffer(Buffer& src, Buffer& dst, VkQueue queue, VkBufferCopy* copyRegion /*= nullptr*/) const
{
	ASSERT(dst.GetSize() <= src.GetSize());
	ASSERT(src.GetHandle());

	VkCommandBuffer commandBuffer = CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkBufferCopy bufferCopy = {};
	if (copyRegion)
	{
		bufferCopy = *copyRegion;
	}
	else
	{
		bufferCopy.size = src.GetSize();
	}

	vkCmdCopyBuffer(commandBuffer, src.GetHandle(), dst.GetHandle(), 1, &bufferCopy);

	FlushCommandBuffer(commandBuffer, queue);
}
