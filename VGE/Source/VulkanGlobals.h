#pragma once

namespace vge
{
	inline const char* GValidationLayers[] = {
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	inline constexpr bool GEnableValidationLayers = false;
#else
	inline constexpr bool GEnableValidationLayers = true;
#endif
}
