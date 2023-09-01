#pragma once

#include "Common.h"

namespace vge
{	
	const char* GpuTypeToString(VkPhysicalDeviceType gpuType);

	uint32 FindMemoryTypeIndex(VkPhysicalDevice gpu, uint32 allowedTypes, VkMemoryPropertyFlags flags);

	VkFormat GetBestImageFormat(VkPhysicalDevice gpu, const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features);

	void CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags, VkImageView& outImageView);
	void CreateTextureDescriptorSet(VkDevice device, VkSampler sampler, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descrptorSetLayout, VkImageView textureImageView, VkDescriptorSet& outTextureDescriptorSet);

	// Get texture names from a given scene, preserves 1 to 1 relationship.
	// If failed to get a texture from material, its name will be empty in out array.
	void GetTexturesFromMaterials(const aiScene* scene, std::vector<const char*>& outTextures);

	// Resolve given textures to be mapped with descriptor sets.
	void ResolveTexturesForDescriptors(class Renderer* renderer, const std::vector<const char*>& texturePaths, std::vector<int32>& outTextureToDescriptorSet);
}
