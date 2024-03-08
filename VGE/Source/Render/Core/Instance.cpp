#include "Instance.h"
#include "PhysicalDevice.h"
#include "Core/Helpers.h"
#include <algorithm>
#include <functional>

#if defined(VGE_DEBUG) || defined(VGE_VALIDATION_LAYERS)
	#define USE_VALIDATION_LAYERS
#endif

#if defined(USE_VALIDATION_LAYERS) && (defined(VGE_VALIDATION_LAYERS_GPU_ASSISTED) || defined(VGE_VALIDATION_LAYERS_BEST_PRACTICES) || defined(VGE_VALIDATION_LAYERS_SYNCHRONIZATION))
	#define USE_VALIDATION_LAYER_FEATURES
#endif

namespace vge
{
namespace
{
#ifdef USE_VALIDATION_LAYERS
VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData)
{
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		LOG(Warning, "[%d %s]: %s", callbackData->messageIdNumber, callbackData->pMessageIdName, callbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		LOG(Error, "[%d %s]: %s", callbackData->messageIdNumber, callbackData->pMessageIdName, callbackData->pMessage);
	}
	else
	{
#if LOG_VK_VERBOSE
		LOG(Log, "[%d %s]: %s", callbackData->messageIdNumber, callbackData->pMessageIdName, callbackData->pMessage);
#endif
	}

	return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT /*type*/,
	uint64_t /*object*/, size_t /*location*/, int32_t /*message_code*/,
	const char* layerPrefix, const char* message, void* /*user_data*/)
{
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		LOG(Error, "%s: %s", layerPrefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		LOG(Warning, "%s: %s", layerPrefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		LOG(Warning, "%s: %s", layerPrefix, message);
	}
	else
	{
		LOG(Log, "%s: %s", layerPrefix, message);
	}

	return VK_FALSE;
}
#endif

bool ValidateLayers(const std::vector<const char*>& required, const std::vector<VkLayerProperties>& available)
{
	for (auto layer : required)
	{
		bool found = false;
		for (auto& availableLayer : available)
		{
			if (strcmp(availableLayer.layerName, layer) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			LOG(Error, "Validation Layer %s not found", layer);
			return false;
		}
	}

	return true;
}
}	// namespace

std::vector<const char*> GetOptimalValidationLayers(const std::vector<VkLayerProperties>& supported_instance_layers)
{
	std::vector<std::vector<const char*>> validationLayerPriorityList =
	{
		// The preferred validation layer is "VK_LAYER_KHRONOS_validation".
		{"VK_LAYER_KHRONOS_validation"},

		// Otherwise we fallback to using the LunarG meta layer.
		{"VK_LAYER_LUNARG_standard_validation"},

		// Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist.
		{
			"VK_LAYER_GOOGLE_threading",
			"VK_LAYER_LUNARG_parameter_validation",
			"VK_LAYER_LUNARG_object_tracker",
			"VK_LAYER_LUNARG_core_validation",
			"VK_LAYER_GOOGLE_unique_objects",
		},

		// Otherwise as a last resort we fallback to attempting to enable the LunarG core layer.
		{"VK_LAYER_LUNARG_core_validation"} };

	for (auto& validationLayers : validationLayerPriorityList)
	{
		if (ValidateLayers(validationLayers, supported_instance_layers))
		{
			return validationLayers;
		}

		LOG(Warning, "Couldn't enable validation layers (see log for error) - falling back.");
	}

	// Else return nothing.
	return {};
}

namespace
{
bool EnableExtension(const char* requiredExtName, const std::vector<VkExtensionProperties>& availableExts, std::vector<const char*>& enabledExts)
{
	for (auto& availExtIt : availableExts)
	{
		if (strcmp(availExtIt.extensionName, requiredExtName) == 0)
		{
			auto it = std::find_if(enabledExts.begin(), enabledExts.end(),
				[requiredExtName](const char* enabled_ext_name) {
					return strcmp(enabled_ext_name, requiredExtName) == 0;
				});
			if (it != enabledExts.end())
			{
				// Extension is already enabled
			}
			else
			{
				LOG(Log, "Extension %s found, enabling it.", requiredExtName);
				enabledExts.emplace_back(requiredExtName);
			}
			return true;
		}
	}

	LOG(Warning, "Extension %s not found.", requiredExtName);
	return false;
}

bool EnableAllExtensions(const std::vector<const char*> requiredExtNames, const std::vector<VkExtensionProperties>& availableExts, std::vector<const char*>& enabledExts)
{
	using std::placeholders::_1;

	return std::all_of(requiredExtNames.begin(), requiredExtNames.end(),
		std::bind(EnableExtension, _1, availableExts, enabledExts));
}

}	// namespace
}	// namespace vge

vge::Instance::Instance(
	const std::string& appName, 
	const std::unordered_map<const char*, bool>& requiredExtensions /*= {}*/, 
	const std::vector<const char*>& requiredValidationLayers /*= {}*/,
	bool headless /*= false*/,
	u32 apiVersion /*= VK_API_VERSION_1_0*/)
{
	VK_ENSURE(volkInitialize());
	
	u32 instanceExtCount = 0;
	VK_ENSURE(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtCount, nullptr));

	std::vector<VkExtensionProperties> availableInstanceExts(instanceExtCount);
	VK_ENSURE(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtCount, availableInstanceExts.data()));

#ifdef USE_VALIDATION_LAYERS
	// Check if VK_EXT_debug_utils is supported, which supersedes VK_EXT_Debug_Report.
	const bool hasDebugUtils = EnableExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, availableInstanceExts, _EnabledExtensions);
	if (!hasDebugUtils)
	{
		const bool hasDebugReport = EnableExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, availableInstanceExts, _EnabledExtensions);
		if (!hasDebugReport)
		{
			LOG(Warning, "Neither of %s or %s are available, disabling debug reporting.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
	}
#endif

#if (defined(VGE_ENABLE_PORTABILITY))
	EnableExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, availableInstanceExts, _EnabledExtensions);
	EnableExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, availableInstanceExts, _EnabledExtensions);
#endif

#ifdef USE_VALIDATION_LAYER_FEATURES
	bool validationFeatures = false;
	{
		u32 layerInstanceExtCount;
		VK_ENSURE(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerInstanceExtCount, nullptr));

		std::vector<VkExtensionProperties> availableLayerInstanceExts(layerInstanceExtCount);
		VK_ENSURE(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerInstanceExtCount, availableLayerInstanceExts.data()));

		for (auto& available_extension : availableLayerInstanceExts)
		{
			if (strcmp(available_extension.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0)
			{
				validationFeatures = true;
				LOG(Log, "%s is available, enabling it.", VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
				_EnabledExtensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
			}
		}
	}
#endif

	// Try to enable headless surface extension if it exists
	if (headless)
	{
		const bool hasHeadlessSurface = EnableExtension(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME, availableInstanceExts, _EnabledExtensions);
		if (!hasHeadlessSurface)
		{
			LOG(Warning, "%s is not available, disabling swapchain creation.", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
		}
	}
	else
	{
		_EnabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	}

	// VK_KHR_get_physical_device_properties2 is a prerequisite of VK_KHR_performance_query
	// which will be used for stats gathering where available.
	EnableExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, availableInstanceExts, _EnabledExtensions);

	bool extensionError = false;
	for (auto extension : requiredExtensions)
	{
		const char* extensionName = extension.first;
		const bool extensionIsOptional = extension.second;
		if (!EnableExtension(extensionName, availableInstanceExts, _EnabledExtensions))
		{
			if (extensionIsOptional)
			{
				LOG(Warning, "Optional instance extension %s is not available, some features may be disabled.", extensionName);
			}
			else
			{
				LOG(Error, "Required instance extension %s is not available, cannot run", extensionName);
				extensionError = true;
			}
			extensionError = extensionError || !extensionIsOptional;
		}
	}

	ENSURE_MSG(!extensionError, "Required instance extensions are missing.");

	u32 instanceLayerCount;
	VK_ENSURE(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));

	std::vector<VkLayerProperties> supportedValidationLayers(instanceLayerCount);
	VK_ENSURE(vkEnumerateInstanceLayerProperties(&instanceLayerCount, supportedValidationLayers.data()));

	std::vector<const char*> requestedValidationLayers(requiredValidationLayers);

#ifdef USE_VALIDATION_LAYERS
	// Determine the optimal validation layers to enable that are necessary for useful debugging
	std::vector<const char*> optimalValidationLayers = GetOptimalValidationLayers(supportedValidationLayers);
	requestedValidationLayers.insert(requestedValidationLayers.end(), optimalValidationLayers.begin(), optimalValidationLayers.end());
#endif

	if (ValidateLayers(requestedValidationLayers, supportedValidationLayers))
	{
		LOG(Log, "Enabled Validation Layers:")
		for (const char* layer : requestedValidationLayers)
		{
			LOG(Log, "  \t%s", layer);
		}
	}
	else
	{
		ENSURE_MSG(false, "Required validation layers are missing.");
	}

	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.pApplicationName = appName.c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "VGE - Vulkan Game Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = apiVersion;

	VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = ToU32(_EnabledExtensions.size());
	instanceInfo.ppEnabledExtensionNames = _EnabledExtensions.data();
	instanceInfo.enabledLayerCount = ToU32(requestedValidationLayers.size());
	instanceInfo.ppEnabledLayerNames = requestedValidationLayers.data();

#ifdef USE_VALIDATION_LAYERS
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	VkDebugReportCallbackCreateInfoEXT debugReportInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
	if (hasDebugUtils)
	{
		debugUtilsInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debugUtilsInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugUtilsInfo.pfnUserCallback = DebugUtilsMessengerCallback;

		instanceInfo.pNext = &debugUtilsInfo;
	}
	else
	{
		debugReportInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		debugReportInfo.pfnCallback = DebugCallback;

		instanceInfo.pNext = &debugReportInfo;
	}
#endif

#if (defined(VGE_ENABLE_PORTABILITY))
	instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

	// Some of the specialized layers need to be enabled explicitly
#ifdef USE_VALIDATION_LAYER_FEATURES
	VkValidationFeaturesEXT validationFeaturesInfo = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
	std::vector<VkValidationFeatureEnableEXT> enableFeatures{};
	if (validationFeatures)
	{
#if defined(VGE_VALIDATION_LAYERS_GPU_ASSISTED)
		enableFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT);
		enableFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT);
#endif
#if defined(VGE_VALIDATION_LAYERS_BEST_PRACTICES)
		enableFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT);
#endif
#if defined(VGE_VALIDATION_LAYERS_SYNCHRONIZATION)
		enableFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT);
#endif
		validationFeaturesInfo.enabledValidationFeatureCount = ToU32(enableFeatures.size());
		validationFeaturesInfo.pEnabledValidationFeatures = enableFeatures.data();
		validationFeaturesInfo.pNext = instanceInfo.pNext;
		instanceInfo.pNext = &validationFeaturesInfo;
	}
#endif

	// Create the Vulkan instance
	VK_ENSURE(vkCreateInstance(&instanceInfo, nullptr, &_Handle));

	volkLoadInstance(_Handle);

#ifdef USE_VALIDATION_LAYERS
	if (hasDebugUtils)
	{
		VK_ENSURE(vkCreateDebugUtilsMessengerEXT(_Handle, &debugUtilsInfo, nullptr, &_DebugUtilsMessenger));
	}
	else
	{
		VK_ENSURE(vkCreateDebugReportCallbackEXT(_Handle, &debugReportInfo, nullptr, &_DebugReportCallback));
	}
#endif

	QueryGpus();
}

vge::Instance::Instance(VkInstance instance) : _Handle(instance)
{
	ENSURE(_Handle);
	VK_ENSURE(volkInitialize());
	QueryGpus();
}

vge::Instance::~Instance()
{
#ifdef USE_VALIDATION_LAYERS
	if (_DebugUtilsMessenger)
	{
		vkDestroyDebugUtilsMessengerEXT(_Handle, _DebugUtilsMessenger, nullptr);
	}
	if (_DebugReportCallback)
	{
		vkDestroyDebugReportCallbackEXT(_Handle, _DebugReportCallback, nullptr);
	}
#endif

	if (_Handle)
	{
		vkDestroyInstance(_Handle, nullptr);
	}
}

vge::PhysicalDevice& vge::Instance::GetFirstGpu()
{
	ASSERT_MSG(!_Gpus.empty(), "No physical devices were found on the system.")

	// Find a discrete GPU
	for (auto& gpu : _Gpus)
	{
		if (gpu->GetProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			return *gpu;
		}
	}

	// Otherwise just pick the first one
	LOG(Warning, "Couldn't find a discrete physical device, picking default GPU.");
	return *_Gpus[0];
}

vge::PhysicalDevice& vge::Instance::GetSuitableGpu(VkSurfaceKHR surface)
{
	ASSERT_MSG(!_Gpus.empty(), "No physical devices were found on the system.")

	// A GPU can be explicitly selected via the command line (see plugins/gpu_selection.cpp), this overrides the below GPU selection algorithm.
	//if (selected_gpu_index.has_value())
	//{
	//	LOGI("Explicitly selecting GPU {}", selected_gpu_index.value());
	//	if (selected_gpu_index.value() > gpus.size() - 1)
	//	{
	//		throw std::runtime_error("Selected GPU index is not within no. of available GPUs");
	//	}
	//	return *gpus[selected_gpu_index.value()];
	//}

	// Find a discrete GPU.
	for (auto& gpu : _Gpus)
	{
		if (gpu->GetProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			// See if it work with the surface..
			const size_t queueCount = gpu->GetQueueFamilyProperties().size();
			for (u32 queueIdx = 0; static_cast<size_t>(queueIdx) < queueCount; queueIdx++)
			{
				if (gpu->IsPresentSupported(surface, queueIdx))
				{
					return *gpu;
				}
			}
		}
	}

	// Otherwise just pick the first one.
	LOG(Warning, "Couldn't find a discrete physical device, picking default GPU.");
	return *_Gpus[0];
}

bool vge::Instance::IsExtensionEnabled(const char* extension) const
{
	return std::find_if(_EnabledExtensions.begin(), _EnabledExtensions.end(), 
		[extension](const char *enabled_extension) { return strcmp(extension, enabled_extension) == 0; }) != _EnabledExtensions.end();
}

void vge::Instance::QueryGpus()
{
	u32 physicalDeviceCount = 0;
	VK_ENSURE(vkEnumeratePhysicalDevices(_Handle, &physicalDeviceCount, nullptr));

	ENSURE_MSG(physicalDeviceCount > 0, "Couldn't find a physical device that supports Vulkan.");

	std::vector<VkPhysicalDevice> physicalDevices;
	physicalDevices.resize(physicalDeviceCount);
	VK_ENSURE(vkEnumeratePhysicalDevices(_Handle, &physicalDeviceCount, physicalDevices.data()));

	// Create gpus wrapper objects from the VkPhysicalDevice's
	for (VkPhysicalDevice& physicalDevice : physicalDevices)
	{
		_Gpus.push_back(std::make_unique<PhysicalDevice>(*this, physicalDevice));
	}
}
