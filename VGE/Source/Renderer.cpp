#include "Renderer.h"

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
			if (strcmp(checkExtension, availableExtension.extensionName))
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

QueueFamilyIndices vge::GetQueueFamilies(VkPhysicalDevice gpu)
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
	CreateInstance();
}

int32_t vge::Renderer::Initialize()
{
	try 
	{
		CreateInstance();
		FindGpu();
		CreateDevice();
	}
	catch (const std::runtime_error& err)
	{
		printf("Runtime Error in %s: %s", __FUNCTION__, err.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void vge::Renderer::Cleanup()
{
	vkDestroyDevice(m_Device, nullptr);
	vkDestroyInstance(m_Instance, nullptr);
}

void vge::Renderer::CreateInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Game Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Spicy Cake";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	std::vector<const char*> instanceExtensions = {};
	GetGlfwExtensions(instanceExtensions);

	if (!SupportInstanceExtensions(instanceExtensions))
	{
		throw std::runtime_error("Instance does not support requried extensions.");
	}

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;

	if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan instance.");
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
