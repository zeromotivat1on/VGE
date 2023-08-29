#pragma once

#include "Common.h"

namespace vge
{
	struct VmaImage;
	
	const char* GpuTypeToString(VkPhysicalDeviceType gpuType);

	uint32 FindMemoryTypeIndex(VkPhysicalDevice gpu, uint32 allowedTypes, VkMemoryPropertyFlags flags);

	VkFormat GetBestImageFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features);

	void CreateImage(VmaAllocator allocator, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memAllocUsage, VmaImage* outImage);
	void CreateImage(VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps, VkImage& outImage, VkDeviceMemory& outImageMemory);
	void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags, VkImageView& outImageView);
	void CreateTextureImage(VmaAllocator allocator, VkQueue transferQueue, VkCommandPool transferCmdPool, const char* filename, VmaImage* outImage);
	void CreateTextureDescriptorSet(VkSampler sampler, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descrptorSetLayout, VkImageView textureImageView, VkDescriptorSet& outTextureDescriptorSet);
	void TransitionImageLayout(VkQueue queue, VkCommandPool cmdPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

	// Get texture names from a given scene, preserves 1 to 1 relationship.
	// If failed to get a texture from material, its name will be empty in out array.
	void GetTexturesFromMaterials(const aiScene* scene, std::vector<const char*>& outTextures);

	// Resolve given textures to be mapped with descriptor sets.
	void ResolveTexturesForDescriptors(class Renderer* renderer, const std::vector<const char*>& texturePaths, std::vector<int32>& outTextureToDescriptorSet);
}
