#pragma once

#include "Types.h"

namespace vge
{
#ifdef DEBUG
	inline constexpr bool GEnableValidationLayers = false;
#else
	inline constexpr bool GEnableValidationLayers = false;
#endif

	inline const char* GValidationLayers[] = { "VK_LAYER_KHRONOS_validation" };
	inline const char* GDeviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}
