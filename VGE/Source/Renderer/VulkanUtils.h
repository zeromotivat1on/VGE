#pragma once

#include "Common.h"

namespace vge
{
	struct VmaImage;

	struct VertexInputDescription
	{
		std::vector<VkVertexInputBindingDescription> Bindings = {};
		std::vector<VkVertexInputAttributeDescription> Attributes = {};

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Color;
		glm::vec2 TexCoords;

		static VertexInputDescription GetDescription()
		{
			VertexInputDescription description = {};

			VkVertexInputBindingDescription mainDescription = {};
			mainDescription.binding = 0;
			mainDescription.stride = sizeof(Vertex);
			mainDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			description.Bindings.push_back(mainDescription);

			VkVertexInputAttributeDescription positionAttribute = {};
			positionAttribute.binding = 0;
			positionAttribute.location = 0;
			positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
			positionAttribute.offset = offsetof(Vertex, Position);

			VkVertexInputAttributeDescription colorAttribute = {};
			colorAttribute.binding = 0;
			colorAttribute.location = 1;
			colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
			colorAttribute.offset = offsetof(Vertex, Color);

			VkVertexInputAttributeDescription textureAttribute = {};
			textureAttribute.binding = 0;
			textureAttribute.location = 2;
			textureAttribute.format = VK_FORMAT_R32G32_SFLOAT;
			textureAttribute.offset = offsetof(Vertex, TexCoords);

			description.Attributes.push_back(positionAttribute);
			description.Attributes.push_back(colorAttribute);
			description.Attributes.push_back(textureAttribute);

			return description;
		}
	};

	struct SwapchainImage
	{
		VkImage Image = VK_NULL_HANDLE;
		VkImageView View = VK_NULL_HANDLE;
	};
	
	const char* GpuTypeToString(VkPhysicalDeviceType gpuType);

	uint32 FindMemoryTypeIndex(VkPhysicalDevice gpu, uint32 allowedTypes, VkMemoryPropertyFlags flags);

	VkSurfaceFormatKHR GetBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR GetBestPresentMode(const std::vector<VkPresentModeKHR>& modes);
	VkExtent2D GetBestSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities);
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
