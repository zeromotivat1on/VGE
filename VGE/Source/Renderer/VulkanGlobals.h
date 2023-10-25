#pragma once

#include "Types.h"

namespace vge
{
#ifdef NDEBUG
	inline constexpr bool GEnableValidationLayers = false;
#else
	inline constexpr bool GEnableValidationLayers = true;
#endif

	inline const c8* GValidationLayers[] = { "VK_LAYER_KHRONOS_validation" };
	inline const c8* GDeviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}
