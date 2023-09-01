#include "Utils.h"
#include "Renderer.h"
#include "Texture.h"
#include "Window.h"
#include "Buffer.h"
#include "File.h"

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

	LOG(Warning, "Failed to find matching format, returning VK_FORMAT_UNDEFINED.");
	return VK_FORMAT_UNDEFINED;
}

//void vge::CreateImage(VmaAllocator allocator, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memAllocUsage, Image* outImage)
//{
//	if (!outImage)
//	{
//		LOG(Warning, "Given out image parameter is null.");
//		return;
//	}
//
//	VkImageCreateInfo imageCreateInfo = {};
//	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
//	imageCreateInfo.extent.width = extent.width;
//	imageCreateInfo.extent.height = extent.height;
//	imageCreateInfo.extent.depth = 1;
//	imageCreateInfo.mipLevels = 1;
//	imageCreateInfo.arrayLayers = 1;
//	imageCreateInfo.format = format;
//	imageCreateInfo.tiling = tiling;
//	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // dont care
//	imageCreateInfo.usage = usage;
//	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // whether can be shared between queues
//
//	VmaAllocationCreateInfo vmaAllocCreateInfo = {};
//	vmaAllocCreateInfo.usage = memAllocUsage;
//
//	VK_ENSURE(vmaCreateImage(allocator, &imageCreateInfo, &vmaAllocCreateInfo, &outImage->Handle, &outImage->Allocation, &outImage->AllocInfo));
//}

//void vge::CreateImage(VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps, VkImage& outImage, VkDeviceMemory& outImageMemory)
//{
//	VkImageCreateInfo imageCreateInfo = {};
//	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
//	imageCreateInfo.extent.width = extent.width;
//	imageCreateInfo.extent.height = extent.height;
//	imageCreateInfo.extent.depth = 1;
//	imageCreateInfo.mipLevels = 1;
//	imageCreateInfo.arrayLayers = 1;
//	imageCreateInfo.format = format;
//	imageCreateInfo.tiling = tiling;
//	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // dont care
//	imageCreateInfo.usage = usage;
//	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // whether can be shared between queues
//
//	VK_ENSURE_MSG(vkCreateImage(VulkanContext::Device, &imageCreateInfo, nullptr, &outImage), "Failed to create image.");
//
//	VkMemoryRequirements memRequriements = {};
//	vkGetImageMemoryRequirements(VulkanContext::Device, outImage, &memRequriements);
//
//	VkMemoryAllocateInfo imageMemoryAllocInfo = {};
//	imageMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	imageMemoryAllocInfo.allocationSize = memRequriements.size;
//	imageMemoryAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(VulkanContext::Gpu, memRequriements.memoryTypeBits, memProps);
//
//	VK_ENSURE_MSG(vkAllocateMemory(VulkanContext::Device, &imageMemoryAllocInfo, nullptr, &outImageMemory), "Failed to allocate image memory.");
//
//	vkBindImageMemory(VulkanContext::Device, outImage, outImageMemory, 0);
//}

void vge::CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags, VkImageView& outImageView)
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

	VK_ENSURE_MSG(vkCreateImageView(device, &createInfo, nullptr, &outImageView), "Failed to create image view.");
}

//void vge::CreateTextureImage(VmaAllocator allocator, VkQueue transferQueue, VkCommandPool transferCmdPool, const char* filename, Image* outImage)
//{
//	if (!outImage)
//	{
//		LOG(Warning, "Given out image parameter is null.");
//		return;
//	}
//
//	int32 width = 0, height = 0;
//	VkDeviceSize textureSize = 0;
//	stbi_uc* textureData = file::LoadTexture(filename, width, height, textureSize);
//
//	ScopeStageBuffer textureStageBuffer(textureSize);
//
//	void* data;
//	vmaMapMemory(allocator, textureStageBuffer.Get().Allocation, &data);
//	memcpy(data, textureData, static_cast<size_t>(textureSize));
//	vmaUnmapMemory(allocator, textureStageBuffer.Get().Allocation);
//
//	file::FreeTexture(textureData);
//
//	const VkExtent2D textureExtent = { static_cast<uint32>(width), static_cast<uint32>(height) };
//	const VkFormat textureFormat = VK_FORMAT_R8G8B8A8_UNORM;
//	const VkImageTiling textureTiling = VK_IMAGE_TILING_OPTIMAL;
//	const VkImageUsageFlags textureUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//	const VmaMemoryUsage textureMemUsage = VMA_MEMORY_USAGE_GPU_ONLY;
//	CreateImage(allocator, textureExtent, textureFormat, textureTiling, textureUsage, textureMemUsage, outImage);
//	
//	// Transition image to be destination for copy operation.
//	{
//		const VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//		const VkImageLayout newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//		TransitionImageLayout(transferQueue, transferCmdPool, outImage->Handle, oldLayout, newLayout);
//	}
//
//	CopyImageBuffer(transferQueue, transferCmdPool, textureStageBuffer.Get().Handle, outImage->Handle, textureExtent);
//
//	// Transition image to be shader readable for shade usage.
//	{
//		const VkImageLayout oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//		const VkImageLayout newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//		TransitionImageLayout(transferQueue, transferCmdPool, outImage->Handle, oldLayout, newLayout);
//	}
//}

void vge::CreateTextureDescriptorSet(VkDevice device, VkSampler sampler, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descrptorSetLayout, VkImageView textureImageView, VkDescriptorSet& outTextureDescriptorSet)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descrptorSetLayout;

	VK_ENSURE_MSG(vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &outTextureDescriptorSet), "Failed to allocate texture descriptor set.");

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

	vkUpdateDescriptorSets(device, 1, &descriptorSetWrite, 0, nullptr);
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
