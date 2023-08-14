#include "VulkanUtils.h"
#include "Window.h"

bool vge::SupportValidationLayers()
{
	uint32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : GValidationLayers)
	{
		bool hasLayer = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				hasLayer = true;
				break;
			}
		}

		if (!hasLayer)
		{
			return false;
		}
	}

	return true;
}

void vge::GetRequriedInstanceExtensions(std::vector<const char*>& outExtensions)
{
	GetGlfwInstanceExtensions(outExtensions);
	outExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}

bool vge::SupportInstanceExtensions(const std::vector<const char*>& checkExtensions)
{
	uint32 extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	if (extensionCount == 0) return false;

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	for (const auto& checkExtension : checkExtensions)
	{
		bool hasExtension = false;
		for (const auto& availableExtension : availableExtensions)
		{
			if (strcmp(checkExtension, availableExtension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			return false;
		}
	}

	return true;
}

bool vge::SupportDeviceExtensions(VkPhysicalDevice gpu, const std::vector<const char*>& checkExtensions)
{
	uint32 extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);

	if (extensionCount == 0) return false;

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, availableExtensions.data());

	for (const auto& checkExtension : checkExtensions)
	{
		bool hasExtension = false;
		for (const auto& availableExtension : availableExtensions)
		{
			if (strcmp(checkExtension, availableExtension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			return false;
		}
	}

	return true;
}

bool vge::SuitableGpu(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	//VkPhysicalDeviceProperties gpuProps;
	//vkGetPhysicalDeviceProperties(gpu, &gpuProps);

	//VkPhysicalDeviceFeatures gpuFeatures;
	//vkGetPhysicalDeviceFeatures(gpu, &gpuFeatures);

	QueueFamilyIndices indices = GetQueueFamilies(gpu, surface);

	std::vector<const char*> deviceExtensions;
	deviceExtensions.assign(GDeviceExtensions, GDeviceExtensions + C_ARRAY_NUM(GDeviceExtensions));

	SwapchainDetails swapchainDetails = GetSwapchainDetails(gpu, surface);

	return indices.IsValid() && SupportDeviceExtensions(gpu, deviceExtensions) && swapchainDetails.IsValid();
}

const char* vge::GpuTypeToString(VkPhysicalDeviceType gpuType)
{
	switch (gpuType)
	{
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		return "Integrated";
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		return "Discrete";
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		return "Virtual";
	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		return "CPU";
	default:
		return "Other";
	}
}

vge::QueueFamilyIndices vge::GetQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	uint32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilies.data());

	QueueFamilyIndices indices = {};
	int32 queueFamilyIndex = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.GraphicsFamily = queueFamilyIndex;
		}

		if (surface)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(gpu, queueFamilyIndex, surface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport)
			{
				indices.PresentFamily = queueFamilyIndex;
			}
		}

		if (indices.IsValid())
		{
			break;
		}

		queueFamilyIndex++;
	}

	return indices;
}

vge::SwapchainDetails vge::GetSwapchainDetails(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	SwapchainDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &details.SurfaceCapabilities);

	uint32 formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.SurfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, details.SurfaceFormats.data());
	}

	uint32 presentCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentCount, nullptr);

	if (presentCount != 0)
	{
		details.PresentModes.resize(presentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentCount, details.PresentModes.data());
	}

	return details;
}

uint32 vge::FindMemoryTypeIndex(VkPhysicalDevice gpu, uint32 allowedTypes, VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(gpu, &memoryProperties);

	for (uint32 i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		const bool typeIsAllowed = allowedTypes & (1 << i);
		const bool propsHaveGivenFlags = (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags;

		if (typeIsAllowed && propsHaveGivenFlags)
		{
			return i;
		}
	}

	LOG(Error, "Failed to find memory type index. Returning 0.");
	return 0;
}

VkSurfaceFormatKHR vge::GetBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	static constexpr VkSurfaceFormatKHR defaultFormat = { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		LOG(Warning, "Given format is undefined, returning default surface format.");
		return defaultFormat;
	}

	for (const VkSurfaceFormatKHR& format : formats)
	{
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	return formats[0];
}

VkPresentModeKHR vge::GetBestPresentMode(const std::vector<VkPresentModeKHR>& modes)
{
	static constexpr VkPresentModeKHR desiredMode = VK_PRESENT_MODE_MAILBOX_KHR;
	static constexpr VkPresentModeKHR defaultMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const VkPresentModeKHR& mode : modes)
	{
		if (mode == desiredMode)
		{
			return desiredMode;
		}
	}

	return defaultMode;
}

VkExtent2D vge::GetBestSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities)
{
	if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
	{
		return surfaceCapabilities.currentExtent;
	}

	int width, height;
	glfwGetFramebufferSize(GWindow, &width, &height);

	VkExtent2D newExtent = {};
	newExtent.width = std::clamp(static_cast<uint32>(width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
	newExtent.height = std::clamp(static_cast<uint32>(height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

	return newExtent;
}

VkFormat vge::GetBestImageFormat(VkPhysicalDevice gpu, const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (const VkFormat& format : formats)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(gpu, format, &formatProps);

		if (tiling == VK_IMAGE_TILING_LINEAR && (formatProps.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProps.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	LOG(Error, "Failed to find matching format, returning VK_FORMAT_UNDEFINED.");
	return VK_FORMAT_UNDEFINED;

}

VkImage vge::CreateImage2D(VkPhysicalDevice gpu, VkDevice device, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps, VkDeviceMemory& outImageMemory)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = extent.width;
	imageCreateInfo.extent.height = extent.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // dont care
	imageCreateInfo.usage = usage;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // whether can be shared between queues

	VkImage image = VK_NULL_HANDLE;
	if (vkCreateImage(device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
	{
		LOG(Error, "Failed to create image.");
		return VK_NULL_HANDLE;
	}

	VkMemoryRequirements memRequriements = {};
	vkGetImageMemoryRequirements(device, image, &memRequriements);

	VkMemoryAllocateInfo imageMemoryAllocInfo = {};
	imageMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageMemoryAllocInfo.allocationSize = memRequriements.size;
	imageMemoryAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(gpu, memRequriements.memoryTypeBits, memProps);

	if (vkAllocateMemory(device, &imageMemoryAllocInfo, nullptr, &outImageMemory) != VK_SUCCESS)
	{
		LOG(Error, "Failed to allocate image memory.");
		return VK_NULL_HANDLE;
	}

	vkBindImageMemory(device, image, outImageMemory, 0);

	return image;
}

VkImageView vge::CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags)
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	VkImageView imageView = VK_NULL_HANDLE;
	if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		LOG(Error, "Failed to create image view");
	}

	return imageView;
}

void vge::CreateBuffer(VkPhysicalDevice gpu, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& outBuffer, VkDeviceMemory& outMemory)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, &outBuffer) != VK_SUCCESS)
	{
		LOG(Error, "Failed to create buffer.");
		return;
	}

	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements(device, outBuffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(gpu, memoryRequirements.memoryTypeBits, props);

	if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &outMemory) != VK_SUCCESS)
	{
		LOG(Error, "Failed to allocate memory for buffer.");
		return;
	}

	vkBindBufferMemory(device, outBuffer, outMemory, 0);
}

void vge::CopyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer transferCmdBuffer = VK_NULL_HANDLE;

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandPool = transferCmdPool;
	cmdBufferAllocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, &transferCmdBuffer) != VK_SUCCESS)
	{
		LOG(Error, "Failed to allcoate transfer command buffer.");
		return;
	}

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(transferCmdBuffer, &cmdBufferBeginInfo);

	{
		VkBufferCopy bufferCopyRegion = {};
		bufferCopyRegion.srcOffset = 0;
		bufferCopyRegion.dstOffset = 0;
		bufferCopyRegion.size = size;

		vkCmdCopyBuffer(transferCmdBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);
	}

	vkEndCommandBuffer(transferCmdBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &transferCmdBuffer;

	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue); // not good if we have a lot of calls to this

	vkFreeCommandBuffers(device, transferCmdPool, 1, &transferCmdBuffer);
}
