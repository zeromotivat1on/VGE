#pragma once

#include "Common.h"

namespace vge
{
	struct QueueFamilyIndices
	{
		int32_t GraphicsFamily = -1;
		int32_t PresentFamily = -1;

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
	
	QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	SwapchainDetails GetSwapchainDetails(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	
	VkSurfaceFormatKHR GetBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR GetBestPresentMode(const std::vector<VkPresentModeKHR>& modes);
	VkExtent2D GetBestSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities);

	const char* GpuTypeToString(VkPhysicalDeviceType gpuType);
}
