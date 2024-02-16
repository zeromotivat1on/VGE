#include "PhysicalDevice.h"

vge::PhysicalDevice::PhysicalDevice(Instance& instance, VkPhysicalDevice physicalDevice) : _Instance(instance), _Handle(physicalDevice)
{
	vkGetPhysicalDeviceFeatures(physicalDevice, &_Features);
	vkGetPhysicalDeviceProperties(physicalDevice, &_Properties);
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &_MemoryProperties);

	LOG(Log, "Found GPU: %s.", _Properties.deviceName);

	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);

	_QueueFamilyProperties = std::vector<VkQueueFamilyProperties>(queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, _QueueFamilyProperties.data());
}

const VkFormatProperties vge::PhysicalDevice::GetFormatProperties(VkFormat format) const
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(_Handle, format, &formatProperties);

	return formatProperties;
}

VkBool32 vge::PhysicalDevice::IsPresentSupported(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const
{
	VkBool32 presentSupported = VK_FALSE;

	if (surface)
	{
		VK_ENSURE(vkGetPhysicalDeviceSurfaceSupportKHR(_Handle, queueFamilyIndex, surface, &presentSupported));
	}

	return presentSupported;
}
