#pragma once

#include "Core/Common.h"

namespace vge
{
class Instance;

class PhysicalDevice
{
public:
	PhysicalDevice(Instance &instance, VkPhysicalDevice physicalDevice);
	
	COPY_CTOR_DEL(PhysicalDevice);
	MOVE_CTOR_DEL(PhysicalDevice);

	COPY_OP_DEL(PhysicalDevice);
	MOVE_OP_DEL(PhysicalDevice);

public:
	inline Instance& GetInstance() const { return _Instance; }
	inline VkPhysicalDevice GetHandle() const { return _Handle; }
	inline const VkPhysicalDeviceFeatures& GetFeatures() const { return _Features; }
	inline const VkPhysicalDeviceFeatures& GetRequestedFeatures() const { return _RequestedFeatures; }
	inline const VkPhysicalDeviceProperties& GetProperties() const { return _Properties; }
	inline const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return _MemoryProperties; }
	inline const std::vector<VkQueueFamilyProperties>& GetQueueFamilyProperties() const { return _QueueFamilyProperties; }
	inline void* GetExtensionFeatureChain() const { return _LastRequestedExtensionFeature; }

	const VkFormatProperties GetFormatProperties(VkFormat format) const;
	VkBool32 IsPresentSupported(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const;

	// Requests a third party extension to be used.
	template <typename T>
	T& RequestExtensionFeatures(VkStructureType type)
	{
		// We cannot request extension features if the physical device properties 2 instance extension isn't enabled.
		ENSURE_MSG(instance.IsEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME),
			"Couldn't request feature from device as " + VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME + " isn't enabled!");

		// If the type already exists in the map, return a casted pointer to get the extension feature struct.
		auto extensionFeaturesIt = _ExtensionFeatures.find(type);
		if (extensionFeaturesIt != _ExtensionFeatures.end())
		{
			return *static_cast<T*>(extensionFeaturesIt->second.get());
		}

		// Get the extension feature
		VkPhysicalDeviceFeatures2KHR physicalDeviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR };
		T extension = { type };
		physicalDeviceFeatures.pNext = &extension;
		vkGetPhysicalDeviceFeatures2KHR(handle, &physicalDeviceFeatures);

		// Insert the extension feature into the extension feature map so its ownership is held.
		_ExtensionFeatures.insert({ type, std::make_shared<T>(extension) });

		// Pull out the dereferenced void pointer, we can assume its type based on the template.
		auto* extensionPtr = static_cast<T*>(_ExtensionFeatures.find(type)->second.get());

		// If an extension feature has already been requested, we shift the linked list down by one
		// Making this current extension the new base pointer.
		if (_LastRequestedExtensionFeature)
		{
			extensionPtr->pNext = _LastRequestedExtensionFeature;
		}
		_LastRequestedExtensionFeature = extensionPtr;

		return *extensionPtr;
	}

private:
	Instance& _Instance;
	VkPhysicalDevice _Handle = VK_NULL_HANDLE;
	VkPhysicalDeviceFeatures _Features = {};
	VkPhysicalDeviceFeatures _RequestedFeatures = {};
	VkPhysicalDeviceProperties _Properties;
	VkPhysicalDeviceMemoryProperties _MemoryProperties;
	void* _LastRequestedExtensionFeature = nullptr;
	std::vector<VkQueueFamilyProperties> _QueueFamilyProperties;
	std::map<VkStructureType, std::shared_ptr<void>> _ExtensionFeatures; // holds the extension feature structures, map is used to retain an order of requested structures
};
}

