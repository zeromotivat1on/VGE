#include "VulkanUtils.h"
#include "VulkanContext.h"
#include "Renderer.h"
#include "Window.h"
#include "Buffer.h"
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

	VkPhysicalDeviceFeatures gpuFeatures;
	vkGetPhysicalDeviceFeatures(gpu, &gpuFeatures);

	QueueFamilyIndices indices = GetQueueFamilies(gpu, surface);

	std::vector<const char*> deviceExtensions;
	deviceExtensions.assign(GDeviceExtensions, GDeviceExtensions + C_ARRAY_NUM(GDeviceExtensions));

	SwapchainDetails swapchainDetails = GetSwapchainDetails(gpu, surface);

	return indices.IsValid() && SupportDeviceExtensions(gpu, deviceExtensions) && swapchainDetails.IsValid() && gpuFeatures.samplerAnisotropy;
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

	LOG(Warning, "Failed to find memory type index. Returning 0.");
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

VkFormat vge::GetBestImageFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (const VkFormat& format : formats)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(VulkanContext::Gpu, format, &formatProps);

		if (tiling == VK_IMAGE_TILING_LINEAR && (formatProps.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProps.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	LOG(Warning, "Failed to find matching format, returning VK_FORMAT_UNDEFINED.");
	return VK_FORMAT_UNDEFINED;
}

void vge::CreateImage(VmaAllocator allocator, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memAllocUsage, VmaImage& outImage)
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

	VmaAllocationCreateInfo vmaAllocCreateInfo = {};
	vmaAllocCreateInfo.usage = memAllocUsage;

	VK_ENSURE(vmaCreateImage(allocator, &imageCreateInfo, &vmaAllocCreateInfo, &outImage.Handle, &outImage.Allocation, &outImage.AllocInfo));
}

void vge::CreateImage(VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps, VkImage& outImage, VkDeviceMemory& outImageMemory)
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

	VK_ENSURE_MSG(vkCreateImage(VulkanContext::Device, &imageCreateInfo, nullptr, &outImage), "Failed to create image.");

	VkMemoryRequirements memRequriements = {};
	vkGetImageMemoryRequirements(VulkanContext::Device, outImage, &memRequriements);

	VkMemoryAllocateInfo imageMemoryAllocInfo = {};
	imageMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageMemoryAllocInfo.allocationSize = memRequriements.size;
	imageMemoryAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(VulkanContext::Gpu, memRequriements.memoryTypeBits, memProps);

	VK_ENSURE_MSG(vkAllocateMemory(VulkanContext::Device, &imageMemoryAllocInfo, nullptr, &outImageMemory), "Failed to allocate image memory.");

	vkBindImageMemory(VulkanContext::Device, outImage, outImageMemory, 0);
}

void vge::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags, VkImageView& outImageView)
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

	VK_ENSURE_MSG(vkCreateImageView(VulkanContext::Device, &createInfo, nullptr, &outImageView), "Failed to create image view.");
}

void vge::CreateTextureImage(VmaAllocator allocator, VkQueue transferQueue, VkCommandPool transferCmdPool, const char* filename, VmaImage& outImage)
{
	int32 width = 0, height = 0;
	VkDeviceSize textureSize = 0;
	stbi_uc* textureData = file::LoadTexture(filename, width, height, textureSize);

	ScopeStageBuffer textureStageBuffer(textureSize);

	void* data;
	vmaMapMemory(VulkanContext::Allocator, textureStageBuffer.Get().Allocation, &data);
	memcpy(data, textureData, static_cast<size_t>(textureSize));
	vmaUnmapMemory(VulkanContext::Allocator, textureStageBuffer.Get().Allocation);

	file::FreeTexture(textureData);

	const VkExtent2D textureExtent = { static_cast<uint32>(width), static_cast<uint32>(height) };
	const VkFormat textureFormat = VK_FORMAT_R8G8B8A8_UNORM;
	const VkImageTiling textureTiling = VK_IMAGE_TILING_OPTIMAL;
	const VkImageUsageFlags textureUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	const VmaMemoryUsage textureMemUsage = VMA_MEMORY_USAGE_GPU_ONLY;
	CreateImage(allocator, textureExtent, textureFormat, textureTiling, textureUsage, textureMemUsage, outImage);
	
	// Transition image to be destination for copy operation.
	{
		const VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		const VkImageLayout newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		TransitionImageLayout(transferQueue, transferCmdPool, outImage.Handle, oldLayout, newLayout);
	}

	CopyImageBuffer(transferQueue, transferCmdPool, textureStageBuffer.Get().Handle, outImage.Handle, textureExtent);

	// Transition image to be shader readable for shade usage.
	{
		const VkImageLayout oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		const VkImageLayout newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		TransitionImageLayout(transferQueue, transferCmdPool, outImage.Handle, oldLayout, newLayout);
	}
}

void vge::CreateTextureDescriptorSet(VkSampler sampler, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descrptorSetLayout, VkImageView textureImageView, VkDescriptorSet& outTextureDescriptorSet)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descrptorSetLayout;

	VK_ENSURE_MSG(vkAllocateDescriptorSets(VulkanContext::Device, &descriptorSetAllocInfo, &outTextureDescriptorSet), "Failed to allocate texture descriptor set.");

	VkDescriptorImageInfo descriptorImageInfo = {};
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImageInfo.imageView = textureImageView;
	descriptorImageInfo.sampler = sampler;

	VkWriteDescriptorSet descriptorSetWrite = {};
	descriptorSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorSetWrite.dstSet = outTextureDescriptorSet;
	descriptorSetWrite.dstBinding = 0;
	descriptorSetWrite.dstArrayElement = 0;
	descriptorSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorSetWrite.descriptorCount = 1;
	descriptorSetWrite.pImageInfo = &descriptorImageInfo;

	vkUpdateDescriptorSets(VulkanContext::Device, 1, &descriptorSetWrite, 0, nullptr);
}

void vge::TransitionImageLayout(VkQueue queue, VkCommandPool cmdPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	ScopeCmdBuffer cmdBuffer(cmdPool, queue);

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

	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_NONE;
	VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_NONE;

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

void vge::GetTexturesFromMaterials(const aiScene* scene, std::vector<const char*>& outTextures)
{
	outTextures.resize(scene->mNumMaterials, "");

	for (uint32 i = 0; i < scene->mNumMaterials; ++i)
	{
		const aiMaterial* material = scene->mMaterials[i];

		if (!material)
		{
			continue;
		}

		// TODO: add possibility to load different textures.
		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;
			// TODO: retreive all textures from material.
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				const int32 LastSlashIndex = static_cast<int32>(std::string(path.data).rfind("\\"));
				const std::string filename = std::string(path.data).substr(LastSlashIndex + 1);
				outTextures[i] = filename.c_str();
			}
		}
	}
}

void vge::ResolveTexturesForDescriptors(Renderer* renderer, const std::vector<const char*>& texturePaths, std::vector<int32>& outTextureToDescriptorSet)
{
	if (!renderer)
	{
		return;
	}

	outTextureToDescriptorSet.resize(texturePaths.size());

	for (size_t i = 0; i < texturePaths.size(); ++i)
	{
		if (texturePaths[i] == "")
		{
			outTextureToDescriptorSet[i] = 0;
		}
		else
		{
			outTextureToDescriptorSet[i] = renderer->CreateTexture(texturePaths[i]);
		}
	}
}
