#pragma once

#include "VkCommon.h"

namespace vge
{
class PhysicalDevice;

std::vector<const char*> GetOptimalValidationLayers(const std::vector<VkLayerProperties>& supported_instance_layers);

class Instance
{
public:
	Instance(
		const std::string& appName,
		const std::unordered_map<const char*, bool>& requiredExtensions = {},
		const std::vector<const char*>& requiredValidationLayers = {},
		bool headless = false,
		u32 apiVersion = VK_API_VERSION_1_0);

	Instance(VkInstance instance);

	COPY_CTOR_DEL(Instance);
	MOVE_CTOR_DEL(Instance);

	~Instance();

	COPY_OP_DEL(Instance);
	MOVE_OP_DEL(Instance);

public:
	VkInstance GetHandle() const { return _Handle; }

	PhysicalDevice& GetGirstGpu(); // find first discrete gpu
	PhysicalDevice& GetSuitableGpu(VkSurfaceKHR surface); // find first discrete gpu that can render to a given surface

	bool IsExtensionEnabled(const char* extension) const;
	const std::vector<const char*>& GetExtensions() { return _EnabledExtensions; }
	//const std::vector<VkLayerProperties>& GetLayerProperties();
	//bool GetLayerProperties(const char* layerName, VkLayerProperties& properties);

private:
	void QueryGpus();

private:
	VkInstance _Handle = VK_NULL_HANDLE;
	std::vector<const char*> _EnabledExtensions;
	std::vector<std::unique_ptr<PhysicalDevice>> _Gpus;

#if defined(VGE_DEBUG) || defined(VGE_VALIDATION_LAYERS)
	VkDebugUtilsMessengerEXT _DebugUtilsMessenger = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT _DebugReportCallback = VK_NULL_HANDLE;
#endif
};
}	// namespace vge

