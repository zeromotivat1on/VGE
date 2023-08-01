#include "VulkanUtils.h"
#include "Common.h"

bool vge::SupportValidationLayers()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : GValidationLayers)
	{
		bool hasLayer = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				hasLayer = true;
				break;
			}
		}

		if (!hasLayer)
		{
			return false;
		}
	}

	return true;
}

const char* vge::GpuTypeToString(VkPhysicalDeviceType gpuType)
{
	switch (gpuType)
	{
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		return "Integrated";
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		return "Discrete";
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		return "Virtual";
	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		return "CPU";
	default:
		return "Other";
	}
}
