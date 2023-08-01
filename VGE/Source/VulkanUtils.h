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

	const char* GpuTypeToString(VkPhysicalDeviceType gpuType);
}
