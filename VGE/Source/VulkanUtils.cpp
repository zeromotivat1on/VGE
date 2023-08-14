#include "VulkanUtils.h"
#include "ScopeBuffers.h"
#include "Window.h"
#include "File.h"

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

VkImage vge::CreateImage(VkPhysicalDevice gpu, VkDevice device, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps, VkDeviceMemory& outImageMemory)
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

void vge::TransitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool cmdPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	ScopeCmdBuffer cmdBuffer(device, cmdPool, queue);

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// do not transfer to other queue
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// do not transfer to other queue
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags srcStageFlags;
	VkPipelineStageFlags dstStageFlags;

	// If transtion from new image to image ready to receive data ...
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		// Memory access stage must start after ...
		imageMemoryBarrier.srcAccessMask = 0;										// basically any memory access stage
		// ... but must end before ...
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	// If transition from transfer destination to shader readable ...
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	vkCmdPipelineBarrier(
		cmdBuffer.GetHandle(),			// command buffer
		srcStageFlags, dstStageFlags,	// pipeline stages (match to src/dst Access Masks from barrier)
		0,								// dependency flags
		0, nullptr,						// memory barrier
		0, nullptr,						// buffer memory barrier
		1, &imageMemoryBarrier			// image memory barrier
	);
}

VkCommandBuffer vge::BeginOneTimeCmdBuffer(VkDevice device, VkCommandPool cmdPool)
{
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandPool = cmdPool;
	cmdBufferAllocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, &cmdBuffer) != VK_SUCCESS)
	{
		LOG(Error, "Failed to allcoate transfer command buffer.");
		return VK_NULL_HANDLE;
	}

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo);

	return cmdBuffer;
}

void vge::EndOneTimeCmdBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkCommandBuffer cmdBuffer)
{
	vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue); // not good if we have a lot of calls to this

	vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuffer);
}

void vge::CreateBuffer(VkPhysicalDevice gpu, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps, VkBuffer& outBuffer, VkDeviceMemory& outMemory)
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
	memoryAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(gpu, memoryRequirements.memoryTypeBits, memProps);

	if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &outMemory) != VK_SUCCESS)
	{
		LOG(Error, "Failed to allocate memory for buffer.");
		return;
	}

	vkBindBufferMemory(device, outBuffer, outMemory, 0);
}

void vge::CopyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	ScopeCmdBuffer transferCmdBuffer(device, transferCmdPool, transferQueue);

	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = size;

	vkCmdCopyBuffer(transferCmdBuffer.GetHandle(), srcBuffer, dstBuffer, 1, &bufferCopyRegion);
}

void vge::CopyImageBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, VkBuffer srcBuffer, VkImage dstImage, VkExtent2D extent)
{
	ScopeCmdBuffer transferCmdBuffer(device, transferCmdPool, transferQueue);

	VkBufferImageCopy bufferImageCopyRegion = {};
	bufferImageCopyRegion.bufferOffset = 0;
	bufferImageCopyRegion.bufferRowLength = 0;										// for data spacing
	bufferImageCopyRegion.bufferImageHeight = 0;									// for data spacing
	bufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// which image aspect to copy
	bufferImageCopyRegion.imageSubresource.mipLevel = 0;
	bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
	bufferImageCopyRegion.imageSubresource.layerCount = 1;
	bufferImageCopyRegion.imageOffset = { 0, 0, 0 };
	bufferImageCopyRegion.imageExtent = { extent.width, extent.height, 1 };

	const VkImageLayout imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	vkCmdCopyBufferToImage(transferCmdBuffer.GetHandle(), srcBuffer, dstImage, imageLayout, 1, &bufferImageCopyRegion);
}

void vge::CreateTexture(VkPhysicalDevice gpu, VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, const char* filename, VkImage& outTextureImage, VkDeviceMemory& outImageTextureMemory)
{
	int32 width = 0, height = 0;
	VkDeviceSize textureSize = 0;
	stbi_uc* textureData = file::LoadTexture(filename, width, height, textureSize);

	ScopeStageBuffer textureStageBuffer(gpu, device, textureSize);

	void* data;
	vkMapMemory(device, textureStageBuffer.GetMemory(), 0, textureSize, 0, &data);
	memcpy(data, textureData, static_cast<size_t>(textureSize));
	vkUnmapMemory(device, textureStageBuffer.GetMemory());

	stbi_image_free(textureData);

	const VkExtent2D textureExtent = { static_cast<uint32>(width), static_cast<uint32>(height) };
	const VkFormat textureFormat = VK_FORMAT_R8G8B8A8_UNORM;
	const VkImageTiling textureTiling = VK_IMAGE_TILING_OPTIMAL;
	const VkImageUsageFlags textureUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	const VkMemoryPropertyFlags textureMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	outTextureImage = CreateImage(gpu, device, textureExtent, textureFormat, textureTiling, textureUsage, textureMemProps, outImageTextureMemory);

	// Transition image to be destination for copy operation.
	{
		const VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		const VkImageLayout newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		TransitionImageLayout(device, transferQueue, transferCmdPool, outTextureImage, oldLayout, newLayout);
	}

	CopyImageBuffer(device, transferQueue, transferCmdPool, textureStageBuffer.GetHandle(), outTextureImage, textureExtent);

	// Transition image to be shader readable for shade usage.
	{
		const VkImageLayout oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		const VkImageLayout newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		TransitionImageLayout(device, transferQueue, transferCmdPool, outTextureImage, oldLayout, newLayout);
	}
}
