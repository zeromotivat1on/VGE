#pragma once

#include "Common.h"

namespace vge
{
	static constexpr size_t GPtrSize = sizeof(void*);

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Color;
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
		VkImageView ImageView = VK_NULL_HANDLE;
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

	void CreateBuffer(VkPhysicalDevice gpu, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& outBuffer, VkDeviceMemory& outMemory);
	void CopyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
}
