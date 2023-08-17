#pragma once

#include "Common.h"

namespace vge
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Color;
		glm::vec2 TexCoords;
	};

	struct QueueFamilyIndices
	{
		int32 GraphicsFamily = -1;
		int32 PresentFamily = -1;

		inline bool IsValid()
		{
			return GraphicsFamily >= 0 && PresentFamily >= 0;
		}
	};

	struct SwapchainDetails
	{
		VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		std::vector<VkSurfaceFormatKHR> SurfaceFormats;
		std::vector<VkPresentModeKHR> PresentModes;

		inline bool IsValid()
		{
			return !SurfaceFormats.empty() && !PresentModes.empty();
		}
	};

	struct SwapchainImage
	{
		VkImage Handle = VK_NULL_HANDLE;
		VkImageView View = VK_NULL_HANDLE;
	};

	struct Texture
	{
		VkImage Image = VK_NULL_HANDLE;
		VkImageView View = VK_NULL_HANDLE;
		// TODO: Have 1 VkDeviceMemory and VkImage's just reference it with offsets.
		VkDeviceMemory Memory = VK_NULL_HANDLE;
		VkDescriptorSet Descriptor = VK_NULL_HANDLE;
	};

	bool SupportValidationLayers();
	void GetRequriedInstanceExtensions(std::vector<const char*>& outExtensions);
	bool SupportInstanceExtensions(const std::vector<const char*>& checkExtensions);
	bool SupportDeviceExtensions(VkPhysicalDevice gpu, const std::vector<const char*>& checkExtensions);
	bool SuitableGpu(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	
	const char* GpuTypeToString(VkPhysicalDeviceType gpuType);

	QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	SwapchainDetails GetSwapchainDetails(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	uint32 FindMemoryTypeIndex(VkPhysicalDevice gpu, uint32 allowedTypes, VkMemoryPropertyFlags flags);

	VkSurfaceFormatKHR GetBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR GetBestPresentMode(const std::vector<VkPresentModeKHR>& modes);
	VkExtent2D GetBestSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities);
	VkFormat GetBestImageFormat(VkPhysicalDevice gpu, const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features);

	void CreateImage(VkPhysicalDevice gpu, VkDevice device, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps, VkImage& outImage, VkDeviceMemory& outImageMemory);
	void CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags, VkImageView& outImageView);
	void CreateTextureImage(VkPhysicalDevice gpu, VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, const char* filename, VkImage& outImage, VkDeviceMemory& outImageMemory);
	void CreateTextureDescriptorSet(VkDevice device, VkSampler sampler, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descrptorSetLayout, VkImageView textureImageView, VkDescriptorSet& outTextureDescriptorSet);
	void TransitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool cmdPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
}
