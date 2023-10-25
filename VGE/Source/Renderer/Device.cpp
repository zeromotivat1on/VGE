#include "Device.h"
#include "Application.h"
#include "VulkanGlobals.h"
#include "Utils.h"

namespace vge
{
#pragma region DebugMessengerSetup
	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LOG(Warning, "%s", pCallbackData->pMessage);
			break;

		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			LOG(Error, "%s", pCallbackData->pMessage);
			break;
		}

	#if LOG_VK_VERBOSE
		LOG(Log, "%s", pCallbackData->pMessage);
	#endif

		return VK_FALSE;
	}

	static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		if (auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"))
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = VulkanDebugCallback;
	}
#pragma endregion DebugMessengerSetup

#pragma region Statics
	static bool SupportValidationLayers()
	{
		u32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const c8* layerName : vge::GValidationLayers)
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

	void GetRequriedInstanceExtensions(std::vector<const c8*>& outExtensions)
	{
		if (vge::GWindow)
		{
			vge::GWindow->GetInstanceExtensions(outExtensions);
		}

		outExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	bool SupportInstanceExtensions(const std::vector<const c8*>& checkExtensions)
	{
		u32 extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		if (extensionCount == 0) return false;

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		for (const auto& checkExtension : checkExtensions)
		{
			bool hasExtension = false;
			for (const auto& availableExtension : availableExtensions)
			{
				if (strcmp(checkExtension, availableExtension.extensionName) == 0)
				{
					hasExtension = true;
					break;
				}
			}

			if (!hasExtension)
			{
				return false;
			}
		}

		return true;
	}

	static vge::QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surface)
	{
		u32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilies.data());

		vge::QueueFamilyIndices indices = {};
		i32 queueFamilyIndex = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.GraphicsFamily = queueFamilyIndex;
			}

			if (surface)
			{
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(gpu, queueFamilyIndex, surface, &presentSupport);
				if (queueFamily.queueCount > 0 && presentSupport)
				{
					indices.PresentFamily = queueFamilyIndex;
				}
			}

			if (indices.IsValid())
			{
				break;
			}

			queueFamilyIndex++;
		}

		return indices;
	}

	static vge::SwapchainSupportDetails GetSwapchainSupportDetailsInternal(VkPhysicalDevice gpu, VkSurfaceKHR surface)
	{
		vge::SwapchainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &details.SurfaceCapabilities);

		u32 formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.SurfaceFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, details.SurfaceFormats.data());
		}

		u32 presentCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentCount, nullptr);

		if (presentCount != 0)
		{
			details.PresentModes.resize(presentCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentCount, details.PresentModes.data());
		}

		return details;
	}

	static bool SupportDeviceExtensions(VkPhysicalDevice gpu, const std::vector<const c8*>& checkExtensions)
	{
		u32 extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);

		if (extensionCount == 0) return false;

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, availableExtensions.data());

		for (const auto& checkExtension : checkExtensions)
		{
			bool hasExtension = false;
			for (const auto& availableExtension : availableExtensions)
			{
				if (strcmp(checkExtension, availableExtension.extensionName) == 0)
				{
					hasExtension = true;
					break;
				}
			}

			if (!hasExtension)
			{
				return false;
			}
		}

		return true;
	}

	static bool SuitableGpu(VkPhysicalDevice gpu, VkSurfaceKHR surface)
	{
		//VkPhysicalDeviceProperties gpuProps;
		//vkGetPhysicalDeviceProperties(gpu, &gpuProps);

		VkPhysicalDeviceFeatures gpuFeatures;
		vkGetPhysicalDeviceFeatures(gpu, &gpuFeatures);

		vge::QueueFamilyIndices indices = GetQueueFamilies(gpu, surface);

		std::vector<const c8*> deviceExtensions;
		deviceExtensions.assign(vge::GDeviceExtensions, vge::GDeviceExtensions + C_ARRAY_NUM(vge::GDeviceExtensions));

		vge::SwapchainSupportDetails swapchainDetails = GetSwapchainSupportDetailsInternal(gpu, surface);

		return indices.IsValid() && SupportDeviceExtensions(gpu, deviceExtensions) && swapchainDetails.IsValid() && gpuFeatures.samplerAnisotropy;
	}
#pragma endregion Statics
}

vge::Device::Device(Window* window) : m_Window(window)
{
	ENSURE(m_Window);
}

void vge::Device::Initialize()
{
	CreateInstance();
	SetupDebugMessenger();
	CreateInitialSurface();
	FindGpu();
	CreateDevice();
	FindQueues();
	CreateCustomAllocator();
	CreateCommandPool();
}

void vge::Device::Destroy()
{
	WaitIdle();

	vkDestroyCommandPool(m_Handle, m_CommandPool, nullptr);
	vmaDestroyAllocator(m_Allocator);
	vkDestroyDevice(m_Handle, nullptr);

	if (GEnableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
	}

	vkDestroyInstance(m_Instance, nullptr);
}

vge::SwapchainSupportDetails vge::Device::GetSwapchainSupportDetails(VkSurfaceKHR surface) const
{
	if (surface == VK_NULL_HANDLE)
	{
		surface = m_InitialSurface;
	}

	return GetSwapchainSupportDetailsInternal(m_Gpu, surface);
}

void vge::Device::CreateInstance()
{
	if (GEnableValidationLayers)
	{
		ENSURE_MSG(SupportValidationLayers(), "Validation layers requested, but not supported.");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = GApplication->Specs.Name;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = GApplication->Specs.InternalName;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	std::vector<const c8*> instanceExtensions = {};
	GetRequriedInstanceExtensions(instanceExtensions);

	ENSURE_MSG(SupportInstanceExtensions(instanceExtensions), "Instance does not support requried extensions.");

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<u32>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	if (GEnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<u32>(C_ARRAY_NUM(GValidationLayers));
		createInfo.ppEnabledLayerNames = GValidationLayers;

		// Validate VkCreateInstance and VkDestroyInstance function calls.
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

		std::string layersString;
		for (i32 i = C_ARRAY_NUM(GValidationLayers) - 1; i >= 0; --i)
		{
			layersString.append(GValidationLayers[i]);
			layersString.append(" ");
		}

		LOG(Log, "Validation layers in use: %s", layersString.c_str());
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	VK_ENSURE(vkCreateInstance(&createInfo, nullptr, &m_Instance));
}

void vge::Device::SetupDebugMessenger()
{
	if (!GEnableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	VK_ENSURE_MSG(CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger), "Failed to set up debug messenger.");
}

void vge::Device::CreateInitialSurface()
{
	CreateWindowSurface(m_InitialSurface);
}

void vge::Device::FindGpu()
{
	u32 gpuCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr);

	ENSURE_MSG(gpuCount > 0, "Can't find GPUs that support Vulkan.");

	std::vector<VkPhysicalDevice> availableGpus(gpuCount);
	vkEnumeratePhysicalDevices(m_Instance, &gpuCount, availableGpus.data());

	for (const VkPhysicalDevice& gpu : availableGpus)
	{
		if (SuitableGpu(gpu, m_InitialSurface))
		{
			m_Gpu = gpu;
			m_QueueIndices = GetQueueFamilies(m_Gpu, m_InitialSurface);

			VkPhysicalDeviceProperties gpuProps;
			vkGetPhysicalDeviceProperties(m_Gpu, &gpuProps);
			LOG(Log, "Chosen GPU properties:");
			LOG(Log, " Name: %s", gpuProps.deviceName);
			LOG(Log, " Device ID: %u", gpuProps.deviceID);
			LOG(Log, " Vendor ID: %u", gpuProps.vendorID);
			LOG(Log, " Device type: %s", vge::GpuTypeToString(gpuProps.deviceType));
			LOG(Log, " Driver version: %u", gpuProps.driverVersion);

			//m_MinUniformBufferOffset = gpuProps.limits.minUniformBufferOffsetAlignment;

			return;
		}
	}

	ENSURE_MSG(false, "Can't find suitable GPUs.");
}

void vge::Device::CreateDevice()
{
	std::unordered_set<i32> queueFamilyIndices = { m_QueueIndices.GraphicsFamily, m_QueueIndices.PresentFamily };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	for (i32 queueFamilyIndex : queueFamilyIndices)
	{
		const f32 priority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &priority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures gpuFeatures = {};
	gpuFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<u32>(C_ARRAY_NUM(GDeviceExtensions));
	deviceCreateInfo.ppEnabledExtensionNames = GDeviceExtensions;
	deviceCreateInfo.pEnabledFeatures = &gpuFeatures;

	{
		std::string extensionsString;
		for (i32 i = C_ARRAY_NUM(GDeviceExtensions) - 1; i >= 0; --i)
		{
			extensionsString.append(GDeviceExtensions[i]);
			extensionsString.append(" ");
		}
		LOG(Log, "Device extensions enabled: %s", extensionsString.c_str());
	}

	VK_ENSURE(vkCreateDevice(m_Gpu, &deviceCreateInfo, nullptr, &m_Handle));
}

void vge::Device::FindQueues()
{
	vkGetDeviceQueue(m_Handle, m_QueueIndices.GraphicsFamily, 0, &m_GfxQueue);
	vkGetDeviceQueue(m_Handle, m_QueueIndices.PresentFamily, 0, &m_PresentQueue);
}

void vge::Device::CreateCustomAllocator()
{
	VmaAllocatorCreateInfo vmaAllocatorCreateInfo = {};
	vmaAllocatorCreateInfo.instance = m_Instance;
	vmaAllocatorCreateInfo.physicalDevice = m_Gpu;
	vmaAllocatorCreateInfo.device = m_Handle;

	VK_ENSURE(vmaCreateAllocator(&vmaAllocatorCreateInfo, &m_Allocator));
}

void vge::Device::CreateCommandPool()
{
	VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // enable cmd buffers reset (re-record)
	cmdPoolCreateInfo.queueFamilyIndex = m_QueueIndices.GraphicsFamily;

	VK_ENSURE(vkCreateCommandPool(m_Handle, &cmdPoolCreateInfo, nullptr, &m_CommandPool));
}
