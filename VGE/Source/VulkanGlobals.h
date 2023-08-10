#pragma once

namespace vge
{
#ifdef NDEBUG
	inline constexpr bool GEnableValidationLayers = false;
#else
	inline constexpr bool GEnableValidationLayers = true;
#endif

	inline const char* GValidationLayers[] = { "VK_LAYER_KHRONOS_validation" };
	inline const char* GDeviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}
