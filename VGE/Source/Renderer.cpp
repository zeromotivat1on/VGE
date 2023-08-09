#include "Renderer.h"
#include "Window.h"
#include "Application.h"
#include "File.h"
#include "Shader.h"

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

	// Do not log verbose info.
	//LOG(Log, "%s", pCallbackData->pMessage);
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
	GRenderer = nullptr;
	return true;
}

void vge::GetGlfwInstanceExtensions(std::vector<const char*>& outExtensions)
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
	{
		outExtensions.push_back(glfwExtensions[i]);
	}
}

void vge::GetRequriedInstanceExtensions(std::vector<const char*>& outExtensions)
{
	GetGlfwInstanceExtensions(outExtensions);
	outExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}

bool vge::SupportInstanceExtensions(const std::vector<const char*>& checkExtensions)
{
	uint32_t extensionCount = 0;
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

bool vge::SupportDeviceExtensions(VkPhysicalDevice gpu, const std::vector<const char*>& checkExtensions)
{
	uint32_t extensionCount = 0;
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

bool vge::SuitableGpu(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	//VkPhysicalDeviceProperties gpuProps;
	//vkGetPhysicalDeviceProperties(gpu, &gpuProps);

	//VkPhysicalDeviceFeatures gpuFeatures;
	//vkGetPhysicalDeviceFeatures(gpu, &gpuFeatures);

	QueueFamilyIndices indices = GetQueueFamilies(gpu, surface);

	std::vector<const char*> deviceExtensions;
	deviceExtensions.assign(GDeviceExtensions, GDeviceExtensions + C_ARRAY_NUM(GDeviceExtensions));

	SwapchainDetails swapchainDetails = GetSwapchainDetails(gpu, surface);

	return indices.IsValid() && SupportDeviceExtensions(gpu, deviceExtensions) && swapchainDetails.IsValid();
}

vge::QueueFamilyIndices vge::GetQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surface)
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

vge::SwapchainDetails vge::GetSwapchainDetails(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	SwapchainDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &details.SurfaceCapabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.SurfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, details.SurfaceFormats.data());
	}

	uint32_t presentCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentCount, nullptr);
	
	if (presentCount != 0)
	{
		details.PresentModes.resize(presentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentCount, details.PresentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR vge::GetBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	static constexpr VkSurfaceFormatKHR defaultFormat = { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		LOG(Warning, "Given format is undefined, returning default surface format.");
		return defaultFormat;
	}

	for (const VkSurfaceFormatKHR& format : formats)
	{
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	return formats[0];
}

VkPresentModeKHR vge::GetBestPresentMode(const std::vector<VkPresentModeKHR>& modes)
{
	static constexpr VkPresentModeKHR desiredMode = VK_PRESENT_MODE_MAILBOX_KHR;
	static constexpr VkPresentModeKHR defaultMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const VkPresentModeKHR& mode : modes)
	{
		if (mode == desiredMode)
		{
			return desiredMode;
		}
	}

	return defaultMode;
}

VkExtent2D vge::GetBestSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities)
{
	if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
	{
		return surfaceCapabilities.currentExtent;
	}

	int width, height;
	glfwGetFramebufferSize(GWindow, &width, &height);

	VkExtent2D newExtent = {};
	newExtent.width = std::clamp(static_cast<uint32_t>(width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
	newExtent.height = std::clamp(static_cast<uint32_t>(height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
	
	return newExtent;
}

VkImageView vge::CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags)
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	VkImageView imageView = VK_NULL_HANDLE;
	if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		LOG(Error, "Failed to create image view");
	}

	return imageView;
}
#pragma endregion NamespaceFunctions

vge::Renderer::Renderer(GLFWwindow* window) : m_Window(window)
{}

int32_t vge::Renderer::Initialize()
{
	try
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		FindGpu();
		CreateDevice();
		CreateSwapchain();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFramebuffers();
		CreateCommandPool();
		CreateCommandBuffers();
		RecordCommandBuffers();
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
	vkDestroyCommandPool(m_Device, m_GfxCommandPool, nullptr);

	for (auto framebuffer : m_SwapchainFramebuffers)
	{
		vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
	}

	vkDestroyPipeline(m_Device, m_GfxPipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_GfxPipelineLayout, nullptr);
	vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

	for (auto swapchainImage : m_SwapchainImages)
	{
		vkDestroyImageView(m_Device, swapchainImage.ImageView, nullptr);
	}

	vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	vkDestroyDevice(m_Device, nullptr);

	if (GEnableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
	}

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
	appInfo.pApplicationName = GApplication->Specs.Name;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = GApplication->Specs.InternalName;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	std::vector<const char*> instanceExtensions = {};
	GetRequriedInstanceExtensions(instanceExtensions);

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

		std::string layersString;
		for (int32_t i = C_ARRAY_NUM(GValidationLayers) - 1; i >= 0; --i)
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

void vge::Renderer::CreateSurface()
{
	if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface.");
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
		if (SuitableGpu(gpu, m_Surface))
		{
			m_Gpu = gpu;

			VkPhysicalDeviceProperties gpuProps;
			vkGetPhysicalDeviceProperties(m_Gpu, &gpuProps);
			LOG(Log, "Chosen GPU properties:");
			LOG(Log, " Name: %s", gpuProps.deviceName);
			LOG(Log, " Device ID: %u", gpuProps.deviceID);
			LOG(Log, " Vendor ID: %u", gpuProps.vendorID);
			LOG(Log, " Device type: %s", GpuTypeToString(gpuProps.deviceType));
			LOG(Log, " Driver version: %u", gpuProps.driverVersion);

			return;
		}
	}

	throw std::runtime_error("Can't find suitable GPUs.");
}

void vge::Renderer::CreateDevice()
{
	QueueFamilyIndices indices = GetQueueFamilies(m_Gpu, m_Surface);

	std::unordered_set<int32_t> queueFamilyIndices = { indices.GraphicsFamily, indices.PresentFamily };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	for (int32_t queueFamilyIndex : queueFamilyIndices)
	{
		const float priority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &priority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures gpuFeatures = {};

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(C_ARRAY_NUM(GDeviceExtensions));
	deviceCreateInfo.ppEnabledExtensionNames = GDeviceExtensions;
	deviceCreateInfo.pEnabledFeatures = &gpuFeatures;

	{
		std::string extensionsString;
		for (int32_t i = C_ARRAY_NUM(GDeviceExtensions) - 1; i >= 0; --i)
		{
			extensionsString.append(GDeviceExtensions[i]);
			extensionsString.append(" ");
		}
		LOG(Log, "Device extensions enabled: %s", extensionsString.c_str());
	}

	if (vkCreateDevice(m_Gpu, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan device.");
	}

	vkGetDeviceQueue(m_Device, indices.GraphicsFamily, 0, &m_GrpahicsQueue);
	vkGetDeviceQueue(m_Device, indices.PresentFamily, 0, &m_PresentQueue);
}

void vge::Renderer::CreateSwapchain()
{
	SwapchainDetails swapchainDetails = GetSwapchainDetails(m_Gpu, m_Surface);
	
	VkSurfaceFormatKHR surfaceFormat = GetBestSurfaceFormat(swapchainDetails.SurfaceFormats);
	VkPresentModeKHR presentMode = GetBestPresentMode(swapchainDetails.PresentModes);
	VkExtent2D extent = GetBestSwapchainExtent(swapchainDetails.SurfaceCapabilities);

	uint32_t imageCount = swapchainDetails.SurfaceCapabilities.minImageCount + 1;
	if (swapchainDetails.SurfaceCapabilities.maxImageCount > 0 && swapchainDetails.SurfaceCapabilities.maxImageCount < imageCount)
	{
		imageCount = swapchainDetails.SurfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = m_Surface;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform = swapchainDetails.SurfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	
	QueueFamilyIndices indices = GetQueueFamilies(m_Gpu, m_Surface);
	if (indices.GraphicsFamily != indices.PresentFamily)
	{
		const uint32_t queueFamilyIndices[] = 
		{ 
			static_cast<uint32_t>(indices.GraphicsFamily), 
			static_cast<uint32_t>(indices.PresentFamily) 
		};

		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else 
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_Device, &swapchainCreateInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swapchain.");
	}

	m_SwapchainImageFormat = surfaceFormat.format;
	m_SwapchainExtent = extent;

	uint32_t swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchainImageCount, nullptr);

	std::vector<VkImage> images(swapchainImageCount);
	vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchainImageCount, images.data());

	for (const auto& image : images)
	{
		SwapchainImage swapchainImage = {};
		swapchainImage.Image = image;
		swapchainImage.ImageView = CreateImageView(m_Device, image, m_SwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		m_SwapchainImages.push_back(swapchainImage);
	}
}

void vge::Renderer::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_SwapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;

	std::array<VkSubpassDependency, 2> subpassDependencies;
	// Image layout transition must happen after ...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	// ... and before ...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;
	// Image layout transition must happen after ...
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// ... and before ...
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	if (vkCreateRenderPass(m_Device, &renderPassCreateInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass.");
	}
}

void vge::Renderer::CreateGraphicsPipeline()
{
	std::vector<char> vertexShaderCode = file::ReadShader("Shaders/vert.spv");
	std::vector<char> fragmentShaderCode = file::ReadShader("Shaders/frag.spv");

	VkShaderModule vertexShaderModule = CreateShaderModule(m_Device, vertexShaderCode);
	VkShaderModule fragmentShaderModule = CreateShaderModule(m_Device, fragmentShaderCode);

	VkPipelineShaderStageCreateInfo vertexStageCreateInfo = {};
	vertexStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexStageCreateInfo.module = vertexShaderModule;
	vertexStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentStageCreateInfo = {};
	fragmentStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentStageCreateInfo.module = fragmentShaderModule;
	fragmentStageCreateInfo.pName = "main";

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { vertexStageCreateInfo, fragmentStageCreateInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_SwapchainExtent.width);
	viewport.height = static_cast<float>(m_SwapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapchainExtent;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors= &scissor;

#pragma region DynamicStateExample
	std::vector<VkDynamicState> dynamicStates;
	dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT); // can be set from vkCmdSetViewport(cmdBuffer, pos, amount, newViewport)
	dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);	// can be set from vkCmdSetScissor(cmdBuffer, pos, amount, newScissor)
	
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
#pragma endregion DynamicStateExample

	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
	multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
	colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendingCreateInfo.attachmentCount = 1;
	colorBlendingCreateInfo.pAttachments = &colorBlendAttachment;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(m_Device, &pipelineLayoutCreateInfo, nullptr, &m_GfxPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline layout.");
	}

	VkGraphicsPipelineCreateInfo gfxPipelineCreateInfo = {};
	gfxPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gfxPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	gfxPipelineCreateInfo.pStages = shaderStages.data();
	gfxPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	gfxPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	gfxPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	gfxPipelineCreateInfo.pDynamicState = nullptr;
	gfxPipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	gfxPipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	gfxPipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
	gfxPipelineCreateInfo.pDepthStencilState = nullptr;
	gfxPipelineCreateInfo.layout = m_GfxPipelineLayout;
	gfxPipelineCreateInfo.renderPass = m_RenderPass;
	gfxPipelineCreateInfo.subpass = 0;
	gfxPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	gfxPipelineCreateInfo.basePipelineIndex = INDEX_NONE;

	if (vkCreateGraphicsPipelines(m_Device, nullptr, 1, &gfxPipelineCreateInfo, nullptr, &m_GfxPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline.");
	}

	vkDestroyShaderModule(m_Device, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(m_Device, vertexShaderModule, nullptr);
}

void vge::Renderer::CreateFramebuffers()
{
	m_SwapchainFramebuffers.resize(m_SwapchainImages.size());

	for (size_t i = 0; i < m_SwapchainFramebuffers.size(); ++i)
	{
		std::array<VkImageView, 1> attachments = { m_SwapchainImages[i].ImageView };

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_RenderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = m_SwapchainExtent.width;
		framebufferCreateInfo.height = m_SwapchainExtent.height;
		framebufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(m_Device, &framebufferCreateInfo, nullptr, &m_SwapchainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer.");
		}
	}
}

void vge::Renderer::CreateCommandPool()
{
	QueueFamilyIndices queueIndices = GetQueueFamilies(m_Gpu, m_Surface);

	VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.queueFamilyIndex = queueIndices.GraphicsFamily;

	if (vkCreateCommandPool(m_Device, &cmdPoolCreateInfo, nullptr, &m_GfxCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics command pool");
	}
}

void vge::Renderer::CreateCommandBuffers()
{
	m_CommandBuffers.resize(m_SwapchainFramebuffers.size());

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = m_GfxCommandPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	if (vkAllocateCommandBuffers(m_Device, &cmdBufferAllocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers.");
	}
}

void vge::Renderer::RecordCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	std::array<VkClearValue, 1> clearValues = 
	{
		{ 0.6f, 0.65f, 0.4f, 1.0f },
	};

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_RenderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = m_SwapchainExtent;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	for (size_t i = 0; i < m_CommandBuffers.size(); ++i)
	{
		renderPassBeginInfo.framebuffer = m_SwapchainFramebuffers[i];

		if (vkBeginCommandBuffer(m_CommandBuffers[i], &cmdBufferBeginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to start command buffer record.");
		}

		vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GfxPipeline);

		vkCmdDraw(m_CommandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(m_CommandBuffers[i]);

		if (vkEndCommandBuffer(m_CommandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to stop command buffer record.");
		}
	}
}
