#pragma once

#include "Common.h"

namespace vge
{
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
		VkImage Image = VK_NULL_HANDLE;
		VkImageView View = VK_NULL_HANDLE;
	};

	struct Texture
	{
		VkImage Image = VK_NULL_HANDLE;
		VkImageView View = VK_NULL_HANDLE;
		// TODO: Have 1 VkDeviceMemory and VkImage's just reference it with offsets.
		VkDeviceMemory Memory = VK_NULL_HANDLE;
		VkDescriptorSet Descriptor = VK_NULL_HANDLE;

		void Destroy(VkDevice device)
		{
			vkDestroyImageView(device, View, nullptr);
			vkDestroyImage(device, Image, nullptr);
			vkFreeMemory(device, Memory, nullptr);
		}
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
	VkFormat GetBestImageFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features);

	void CreateImage(VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps, VkImage& outImage, VkDeviceMemory& outImageMemory);
	void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags, VkImageView& outImageView);
	void CreateTextureImage(VkQueue transferQueue, VkCommandPool transferCmdPool, const char* filename, VkImage& outImage, VkDeviceMemory& outImageMemory);
	void CreateTextureDescriptorSet(VkSampler sampler, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descrptorSetLayout, VkImageView textureImageView, VkDescriptorSet& outTextureDescriptorSet);
	void TransitionImageLayout(VkQueue queue, VkCommandPool cmdPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
}
