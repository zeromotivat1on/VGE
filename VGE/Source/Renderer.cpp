#include "Renderer.h"

#pragma region DebugMessengerSetup
static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	LogCategory category = Log;

	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		category = Warning;
		break;

	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		category = Error;
		break;
	}

	LOG(category, "%s", pCallbackData->pMessage);
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

#pragma region NamespaceFunctions
vge::Renderer* vge::CreateRenderer(GLFWwindow* window)
{
	if (GRenderer) return GRenderer;
	return (GRenderer = new Renderer(window));
}

bool vge::DestroyRenderer()
{
	if (!GRenderer) return false;
	GRenderer->Cleanup();
	delete GRenderer;
	return true;
}

void vge::GetGlfwExtensions(std::vector<const char*>& outExtensions)
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
	{
		outExtensions.push_back(glfwExtensions[i]);
	}
}

void vge::GetRequriedExtensions(std::vector<const char*>& outExtensions)
{
	GetGlfwExtensions(outExtensions);
	outExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}

bool vge::SupportInstanceExtensions(const std::vector<const char*>& checkExtensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

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

bool vge::SuitableGpu(VkPhysicalDevice gpu)
{
	//VkPhysicalDeviceProperties gpuProps;
	//vkGetPhysicalDeviceProperties(gpu, &gpuProps);

	//VkPhysicalDeviceFeatures gpuFeatures;
	//vkGetPhysicalDeviceFeatures(gpu, &gpuFeatures);

	QueueFamilyIndices indices = GetQueueFamilies(gpu);

	return indices.IsValid();
}

vge::QueueFamilyIndices vge::GetQueueFamilies(VkPhysicalDevice gpu)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilies.data());

	QueueFamilyIndices indices = {};
	int32_t queueFamilyIndex = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.GraphicsFamily = queueFamilyIndex;
		}

		if (indices.IsValid())
		{
			break;
		}

		queueFamilyIndex++;
	}

	return indices;
}
#pragma endregion NamespaceFunctions

vge::Renderer::Renderer(GLFWwindow* window) : m_Window(window)
{
}

int32_t vge::Renderer::Initialize()
{
	try
	{
		CreateInstance();
		SetupDebugMessenger();
		FindGpu();
		CreateDevice();
	}
	catch (const std::runtime_error& err)
	{
		LOG(Error, "%s", err.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void vge::Renderer::Cleanup()
{
	if (GEnableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
	}

	vkDestroyDevice(m_Device, nullptr);
	vkDestroyInstance(m_Instance, nullptr);
}

void vge::Renderer::CreateInstance()
{
	if (GEnableValidationLayers && !SupportValidationLayers())
	{
		throw std::runtime_error("Validation layers requested, but not supported.");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Game Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Spicy Cake";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	std::vector<const char*> instanceExtensions = {};
	GetRequriedExtensions(instanceExtensions);

	if (!SupportInstanceExtensions(instanceExtensions))
	{
		throw std::runtime_error("Instance does not support requried extensions.");
	}

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	if (GEnableValidationLayers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(C_ARRAY_NUM(GValidationLayers));
		createInfo.ppEnabledLayerNames = GValidationLayers;

		// Validate VkCreateInstance and VkDestroyInstance function calls.
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

		LOG(Log, "Validation layers in use:");
		for (int32_t i = C_ARRAY_NUM(GValidationLayers) - 1; i >= 0; --i)
		{
			LOG(Log, "\t%s", GValidationLayers[i]);
		}
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan instance.");
	}
}

void vge::Renderer::SetupDebugMessenger()
{
	if (!GEnableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to set up debug messenger.");
	}
}

void vge::Renderer::FindGpu()
{
	uint32_t gpuCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr);

	if (gpuCount == 0)
	{
		throw std::runtime_error("Can't find GPUs that support Vulkan.");
	}

	std::vector<VkPhysicalDevice> availableGpus(gpuCount);
	vkEnumeratePhysicalDevices(m_Instance, &gpuCount, availableGpus.data());

	for (const auto& gpu : availableGpus)
	{
		if (SuitableGpu(gpu))
		{
			m_Gpu = gpu;

			VkPhysicalDeviceProperties gpuProps;
			vkGetPhysicalDeviceProperties(m_Gpu, &gpuProps);
			LOG(Log, "Chosen GPU properties:");
			LOG(Log, "\tName: %s", gpuProps.deviceName);
			LOG(Log, "\tDevice ID: %d", gpuProps.deviceID);
			LOG(Log, "\tVendor ID: %d", gpuProps.vendorID);
			LOG(Log, "\tDevice type: %d", gpuProps.deviceType);
			LOG(Log, "\tDriver version: %d", gpuProps.driverVersion);

			break;
		}
	}
}

void vge::Renderer::CreateDevice()
{
	QueueFamilyIndices indices = GetQueueFamilies(m_Gpu);

	float priority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.GraphicsFamily;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &priority;

	VkPhysicalDeviceFeatures gpuFeatures = {};

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.ppEnabledExtensionNames = nullptr;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.pEnabledFeatures = &gpuFeatures;

	if (vkCreateDevice(m_Gpu, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan device.");
	}

	vkGetDeviceQueue(m_Device, indices.GraphicsFamily, 0, &m_GrpahicsQueue);
}
