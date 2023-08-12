#include "Renderer.h"
#include "Application.h"
#include "Window.h"
#include "Shader.h"
#include "File.h"

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

void vge::IncrementCurrentFrame()
{
	GCurrentFrame = (GCurrentFrame + 1) % GMaxDrawFrames;
}
#pragma endregion NamespaceFunctions

vge::Renderer::Renderer(GLFWwindow* window) : m_Window(window)
{}

void vge::Renderer::Initialize()
{
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	FindGpu();
	CreateDevice();
	CreateSwapchain();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandPool();

	const std::vector<Vertex> vertices =
	{
		{ {-0.4f, 0.4f, 0.0f}, {1.0f, 0.0f, 0.0f} },
		{ {-0.4f, -0.4f, 0.0f}, {0.0f, 1.0f, 0.0f} },
		{ {0.4f, -0.4f, 0.0f }, {0.0f, 0.0f, 1.0f} },
		{ {0.4f, 0.4f, 0.0f }, {1.0f, 1.0f, 0.0f} },
	};

	const std::vector<Vertex> vertices2 =
	{
		{ {-0.25f, 0.6f, 0.0f}, {1.0f, 0.0f, 0.0f} },
		{ {-0.25f, -0.6f, 0.0f}, {0.0f, 1.0f, 0.0f} },
		{ {0.25f, -0.6f, 0.0f }, {0.0f, 0.0f, 1.0f} },
		{ {0.25f, 0.6f, 0.0f }, {1.0f, 1.0f, 0.0f} },
	};

	const std::vector<uint32> indices = { 0, 1, 2, 2, 3, 0 };

	// Transfer queue = Graphics queue.
	m_Meshes.push_back(Mesh(m_Gpu, m_Device, m_GfxQueue, m_GfxCommandPool, vertices, indices));
	m_Meshes.push_back(Mesh(m_Gpu, m_Device, m_GfxQueue, m_GfxCommandPool, vertices2, indices));

	const float aspectRatio = static_cast<float>(m_SwapchainExtent.width) / static_cast<float>(m_SwapchainExtent.width);
	m_UboViewProjection.Projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	m_UboViewProjection.View = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	m_UboViewProjection.Projection[1][1] *= -1; // invert y-axis as glm uses positive y-axis for up, but vulkan uses it for down

	CreateCommandBuffers();
	AllocateDynamicBufferTransferSpace();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	RecordCommandBuffers();
	CreateSyncObjects();
}

void vge::Renderer::Draw()
{
	vkWaitForFences(m_Device, 1, &m_DrawFences[GCurrentFrame], VK_TRUE, UINT64_MAX);	// wait till open
	vkResetFences(m_Device, 1, &m_DrawFences[GCurrentFrame]);							// close after enter

	uint32 AcquiredImageIndex = 0;
	vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, m_ImageAvailableSemas[GCurrentFrame], VK_NULL_HANDLE, &AcquiredImageIndex);

	UpdateUniformBuffers(AcquiredImageIndex);

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_ImageAvailableSemas[GCurrentFrame];
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[AcquiredImageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_RenderFinishedSemas[GCurrentFrame];

	if (vkQueueSubmit(m_GfxQueue, 1, &submitInfo, m_DrawFences[GCurrentFrame]) != VK_SUCCESS) // open fence after successful render
	{
		LOG(Error, "Failed to submit info to graphics queue.");
		return;
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RenderFinishedSemas[GCurrentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_Swapchain;
	presentInfo.pImageIndices = &AcquiredImageIndex;

	if (vkQueuePresentKHR(m_PresentQueue, &presentInfo) != VK_SUCCESS)
	{
		LOG(Error, "Failed to present info to present queue.");
		return;
	}
}

void vge::Renderer::Cleanup()
{
	vkDeviceWaitIdle(m_Device);

	_aligned_free(m_ModelTransferSpace);

	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);

	for (size_t i = 0; i < m_SwapchainImages.size(); ++i)
	{
		vkDestroyBuffer(m_Device, m_VpUniformBuffers[i], nullptr);
		vkFreeMemory(m_Device, m_VpUniformBuffersMemory[i], nullptr);
		vkDestroyBuffer(m_Device, m_ModelDynamicUniformBuffers[i], nullptr);
		vkFreeMemory(m_Device, m_ModelDynamicUniformBuffersMemory[i], nullptr);
	}

	for (const Mesh& mesh : m_Meshes)
	{
		mesh.DestroyVertexBuffer();
		mesh.DestroyIndexBuffer();
	}

	for (int32 i = 0; i < GMaxDrawFrames; ++i)
	{
		vkDestroyFence(m_Device, m_DrawFences[i], nullptr);
		vkDestroySemaphore(m_Device, m_RenderFinishedSemas[i], nullptr);
		vkDestroySemaphore(m_Device, m_ImageAvailableSemas[i], nullptr);
	}

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
		LOG(Error, "Validation layers requested, but not supported.");
		return;
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
		LOG(Error, "Instance does not support requried extensions.");
		return;
	}

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	if (GEnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32>(C_ARRAY_NUM(GValidationLayers));
		createInfo.ppEnabledLayerNames = GValidationLayers;

		// Validate VkCreateInstance and VkDestroyInstance function calls.
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

		std::string layersString;
		for (int32 i = C_ARRAY_NUM(GValidationLayers) - 1; i >= 0; --i)
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
		LOG(Error, "Failed to create Vulkan instance.");
		return;
	}
}

void vge::Renderer::SetupDebugMessenger()
{
	if (!GEnableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
	{
		LOG(Error, "Failed to set up debug messenger.");
		return;
	}
}

void vge::Renderer::CreateSurface()
{
	if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS)
	{
		LOG(Error, "Failed to create window surface.");
		return;
	}
}

void vge::Renderer::FindGpu()
{
	uint32 gpuCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr);

	if (gpuCount == 0)
	{
		LOG(Error, "Can't find GPUs that support Vulkan.");
		return;
	}

	std::vector<VkPhysicalDevice> availableGpus(gpuCount);
	vkEnumeratePhysicalDevices(m_Instance, &gpuCount, availableGpus.data());

	for (const auto& gpu : availableGpus)
	{
		if (SuitableGpu(gpu, m_Surface))
		{
			m_Gpu = gpu;
			m_QueueIndices = GetQueueFamilies(m_Gpu, m_Surface);

			VkPhysicalDeviceProperties gpuProps;
			vkGetPhysicalDeviceProperties(m_Gpu, &gpuProps);
			LOG(Log, "Chosen GPU properties:");
			LOG(Log, " Name: %s", gpuProps.deviceName);
			LOG(Log, " Device ID: %u", gpuProps.deviceID);
			LOG(Log, " Vendor ID: %u", gpuProps.vendorID);
			LOG(Log, " Device type: %s", GpuTypeToString(gpuProps.deviceType));
			LOG(Log, " Driver version: %u", gpuProps.driverVersion);

			m_MinUniformBufferOffset = gpuProps.limits.minUniformBufferOffsetAlignment;

			return;
		}
	}

	LOG(Error, "Can't find suitable GPUs.");
}

void vge::Renderer::CreateDevice()
{
	std::unordered_set<int32> queueFamilyIndices = { m_QueueIndices.GraphicsFamily, m_QueueIndices.PresentFamily };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	for (int32 queueFamilyIndex : queueFamilyIndices)
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
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32>(C_ARRAY_NUM(GDeviceExtensions));
	deviceCreateInfo.ppEnabledExtensionNames = GDeviceExtensions;
	deviceCreateInfo.pEnabledFeatures = &gpuFeatures;

	{
		std::string extensionsString;
		for (int32 i = C_ARRAY_NUM(GDeviceExtensions) - 1; i >= 0; --i)
		{
			extensionsString.append(GDeviceExtensions[i]);
			extensionsString.append(" ");
		}
		LOG(Log, "Device extensions enabled: %s", extensionsString.c_str());
	}

	if (vkCreateDevice(m_Gpu, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS)
	{
		LOG(Error, "Failed to create Vulkan device.");
		return;
	}

	vkGetDeviceQueue(m_Device, m_QueueIndices.GraphicsFamily, 0, &m_GfxQueue);
	vkGetDeviceQueue(m_Device, m_QueueIndices.PresentFamily, 0, &m_PresentQueue);
}

void vge::Renderer::CreateSwapchain()
{
	SwapchainDetails swapchainDetails = GetSwapchainDetails(m_Gpu, m_Surface);
	
	VkSurfaceFormatKHR surfaceFormat = GetBestSurfaceFormat(swapchainDetails.SurfaceFormats);
	VkPresentModeKHR presentMode = GetBestPresentMode(swapchainDetails.PresentModes);
	VkExtent2D extent = GetBestSwapchainExtent(swapchainDetails.SurfaceCapabilities);

	uint32 imageCount = swapchainDetails.SurfaceCapabilities.minImageCount + 1;
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
	
	if (m_QueueIndices.GraphicsFamily != m_QueueIndices.PresentFamily)
	{
		const uint32 queueFamilyIndices[] = 
		{ 
			static_cast<uint32>(m_QueueIndices.GraphicsFamily),
			static_cast<uint32>(m_QueueIndices.PresentFamily)
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
		LOG(Error, "Failed to create swapchain.");
		return;
	}

	m_SwapchainImageFormat = surfaceFormat.format;
	m_SwapchainExtent = extent;

	uint32 swapchainImageCount = 0;
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
	renderPassCreateInfo.dependencyCount = static_cast<uint32>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	if (vkCreateRenderPass(m_Device, &renderPassCreateInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
	{
		LOG(Error, "Failed to create render pass.");
		return;
	}
}

void vge::Renderer::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding vpLayoutBinding = {};
	vpLayoutBinding.binding = 0;
	vpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vpLayoutBinding.descriptorCount = 1;
	vpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vpLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding modelLayoutBinding = {};
	modelLayoutBinding.binding = 1;
	modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	modelLayoutBinding.descriptorCount = 1;
	modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	modelLayoutBinding.pImmutableSamplers = nullptr;

	const std::array<VkDescriptorSetLayoutBinding, 2> layoutBindings = { vpLayoutBinding, modelLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = static_cast<uint32>(layoutBindings.size());
	layoutCreateInfo.pBindings = layoutBindings.data();

	if (vkCreateDescriptorSetLayout(m_Device, &layoutCreateInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
	{
		LOG(Error, "Failed to create descriptor set layout.");
		return;
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

	VkVertexInputBindingDescription vertexBindingDescription = {};
	vertexBindingDescription.binding = 0;
	vertexBindingDescription.stride = sizeof(Vertex);
	vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::array<VkVertexInputAttributeDescription, 2> vertexAttributeDescriptions;
	vertexAttributeDescriptions[0].binding = 0;
	vertexAttributeDescriptions[0].location = 0;
	vertexAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttributeDescriptions[0].offset = offsetof(Vertex, Position);
	vertexAttributeDescriptions[1].binding = 0;
	vertexAttributeDescriptions[1].location = 1;
	vertexAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttributeDescriptions[1].offset = offsetof(Vertex, Color);

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32>(vertexAttributeDescriptions.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

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
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32>(dynamicStates.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
#pragma endregion DynamicStateExample

	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // projection matrix y-axis is inverted, so use counter-clockwise
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
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(m_Device, &pipelineLayoutCreateInfo, nullptr, &m_GfxPipelineLayout) != VK_SUCCESS)
	{
		LOG(Error, "Failed to create graphics pipeline layout.");
		return;
	}

	VkGraphicsPipelineCreateInfo gfxPipelineCreateInfo = {};
	gfxPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gfxPipelineCreateInfo.stageCount = static_cast<uint32>(shaderStages.size());
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
		LOG(Error, "Failed to create graphics pipeline.");
		return;
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
		framebufferCreateInfo.attachmentCount = static_cast<uint32>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = m_SwapchainExtent.width;
		framebufferCreateInfo.height = m_SwapchainExtent.height;
		framebufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(m_Device, &framebufferCreateInfo, nullptr, &m_SwapchainFramebuffers[i]) != VK_SUCCESS)
		{
			LOG(Error, "Failed to create framebuffer.");
			return;
		}
	}
}

void vge::Renderer::CreateCommandPool()
{
	VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.queueFamilyIndex = m_QueueIndices.GraphicsFamily;

	if (vkCreateCommandPool(m_Device, &cmdPoolCreateInfo, nullptr, &m_GfxCommandPool) != VK_SUCCESS)
	{
		LOG(Error, "Failed to create graphics command pool.");
		return;
	}
}

void vge::Renderer::CreateCommandBuffers()
{
	m_CommandBuffers.resize(m_SwapchainFramebuffers.size());

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = m_GfxCommandPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = static_cast<uint32>(m_CommandBuffers.size());

	if (vkAllocateCommandBuffers(m_Device, &cmdBufferAllocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
	{
		LOG(Error, "Failed to allocate command buffers.");
		return;
	}
}

void vge::Renderer::AllocateDynamicBufferTransferSpace()
{
	// Calculate proper model alignment to ensure its fit into allocated block of memory.
	m_ModelUniformAlignment = (sizeof(UboModel) + m_MinUniformBufferOffset - 1) & ~(m_MinUniformBufferOffset - 1);
	m_ModelTransferSpace = (UboModel*)_aligned_malloc(m_ModelUniformAlignment * GMaxSceneObjects, m_ModelUniformAlignment);
}

void vge::Renderer::CreateUniformBuffers()
{
	constexpr VkDeviceSize vpBufferSize = sizeof(UboViewProjection);
	const VkDeviceSize modelBufferSize = m_ModelUniformAlignment * GMaxSceneObjects;

	m_VpUniformBuffers.resize(m_SwapchainImages.size());
	m_VpUniformBuffersMemory.resize(m_SwapchainImages.size());

	m_ModelDynamicUniformBuffers.resize(m_SwapchainImages.size());
	m_ModelDynamicUniformBuffersMemory.resize(m_SwapchainImages.size());

	constexpr VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	constexpr VkMemoryPropertyFlags props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (size_t i = 0; i < m_SwapchainImages.size(); ++i)
	{
		CreateBuffer(m_Gpu, m_Device, vpBufferSize, usage, props, m_VpUniformBuffers[i], m_VpUniformBuffersMemory[i]);
		CreateBuffer(m_Gpu, m_Device, modelBufferSize, usage, props, m_ModelDynamicUniformBuffers[i], m_ModelDynamicUniformBuffersMemory[i]);
	}
}

void vge::Renderer::CreateDescriptorPool()
{
	VkDescriptorPoolSize vpPoolSize = {};
	vpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vpPoolSize.descriptorCount = static_cast<uint32>(m_VpUniformBuffers.size());

	VkDescriptorPoolSize modelPoolSize = {};
	modelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	modelPoolSize.descriptorCount = static_cast<uint32>(m_ModelDynamicUniformBuffers.size());

	const std::array<VkDescriptorPoolSize, 2> poolSizes = { vpPoolSize, modelPoolSize };

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = static_cast<uint32>(m_SwapchainImages.size());
	poolCreateInfo.poolSizeCount = static_cast<uint32>(poolSizes.size());
	poolCreateInfo.pPoolSizes = poolSizes.data();

	if (vkCreateDescriptorPool(m_Device, &poolCreateInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
	{
		LOG(Error, "Failed to create descriptor pool.");
		return;
	}
}

void vge::Renderer::CreateDescriptorSets()
{
	m_DescriptorSets.resize(m_SwapchainImages.size());

	std::vector<VkDescriptorSetLayout> setLayouts(m_SwapchainImages.size(), m_DescriptorSetLayout);

	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = m_DescriptorPool;
	setAllocInfo.descriptorSetCount = static_cast<uint32>(m_SwapchainImages.size());
	setAllocInfo.pSetLayouts = setLayouts.data(); // 1 to 1 relationship with layout and set

	if (vkAllocateDescriptorSets(m_Device, &setAllocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
	{
		LOG(Error, "Failed to allocate descriptor sets.");
		return;
	}

	for (size_t i = 0; i < m_SwapchainImages.size(); ++i)
	{
		VkDescriptorBufferInfo vpDescriptorBufferInfo = {};
		vpDescriptorBufferInfo.buffer = m_VpUniformBuffers[i];
		vpDescriptorBufferInfo.offset = 0;
		vpDescriptorBufferInfo.range = sizeof(UboViewProjection);

		VkWriteDescriptorSet vpSetWrite = {};
		vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vpSetWrite.dstSet = m_DescriptorSets[i];
		vpSetWrite.dstBinding = 0;
		vpSetWrite.dstArrayElement = 0;
		vpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vpSetWrite.descriptorCount = 1;
		vpSetWrite.pBufferInfo = &vpDescriptorBufferInfo;

		VkDescriptorBufferInfo modelDescriptorBufferInfo = {};
		modelDescriptorBufferInfo.buffer = m_ModelDynamicUniformBuffers[i];
		modelDescriptorBufferInfo.offset = 0;
		modelDescriptorBufferInfo.range = m_ModelUniformAlignment;

		VkWriteDescriptorSet modelSetWrite = {};
		modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		modelSetWrite.dstSet = m_DescriptorSets[i];
		modelSetWrite.dstBinding = 1;
		modelSetWrite.dstArrayElement = 0;
		modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		modelSetWrite.descriptorCount = 1;
		modelSetWrite.pBufferInfo = &modelDescriptorBufferInfo;

		const std::array<VkWriteDescriptorSet, 2> setWrites = { vpSetWrite , modelSetWrite };

		vkUpdateDescriptorSets(m_Device, static_cast<uint32>(setWrites.size()), setWrites.data(), 0, nullptr);
	}
}

void vge::Renderer::RecordCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	// No need in this flag now as we control synchronization by ourselves.
	// cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	std::array<VkClearValue, 1> clearValues = 
	{
		{ 0.3f, 0.3f, 0.2f, 1.0f },
	};

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_RenderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = m_SwapchainExtent;
	renderPassBeginInfo.clearValueCount = static_cast<uint32>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	for (size_t i = 0; i < m_CommandBuffers.size(); ++i)
	{
		renderPassBeginInfo.framebuffer = m_SwapchainFramebuffers[i];

		if (vkBeginCommandBuffer(m_CommandBuffers[i], &cmdBufferBeginInfo) != VK_SUCCESS)
		{
			LOG(Error, "Failed to start command buffer record.");
			return;
		}

		vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		{
			vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GfxPipeline);

			for (size_t MeshIndex = 0; MeshIndex < m_Meshes.size(); ++MeshIndex)
			{
				const Mesh& mesh = m_Meshes[MeshIndex];

				VkBuffer vertexBuffers[] = { mesh.GetVertexBuffer() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(m_CommandBuffers[i], mesh.GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

				const uint32 dynamicOffset = static_cast<uint32>(m_ModelUniformAlignment * MeshIndex);
				vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GfxPipelineLayout, 0, 1, &m_DescriptorSets[i], 1, &dynamicOffset);

				vkCmdDrawIndexed(m_CommandBuffers[i], static_cast<uint32>(mesh.GetIndexCount()), 1, 0, 0, 0);
			}
		}

		vkCmdEndRenderPass(m_CommandBuffers[i]);

		if (vkEndCommandBuffer(m_CommandBuffers[i]) != VK_SUCCESS)
		{
			LOG(Error, "Failed to stop command buffer record.");
			return;
		}
	}
}

void vge::Renderer::CreateSyncObjects()
{
	m_ImageAvailableSemas.resize(GMaxDrawFrames);
	m_RenderFinishedSemas.resize(GMaxDrawFrames);
	m_DrawFences.resize(GMaxDrawFrames);

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int32 i = 0; i < GMaxDrawFrames; ++i)
	{
		if (vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_ImageAvailableSemas[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_RenderFinishedSemas[i]) != VK_SUCCESS)
		{
			LOG(Error, "Failed to create semaphore.");
			return;
		}

		if (vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_DrawFences[i]) != VK_SUCCESS)
		{
			LOG(Error, "Failed to create fence.");
			return;
		}
	}
}

void vge::Renderer::UpdateUniformBuffers(uint32 ImageIndex)
{
	void* data;
	vkMapMemory(m_Device, m_VpUniformBuffersMemory[ImageIndex], 0, sizeof(UboViewProjection), 0, &data);
	memcpy(data, &m_UboViewProjection, sizeof(UboViewProjection));
	vkUnmapMemory(m_Device, m_VpUniformBuffersMemory[ImageIndex]);

	for (size_t i = 0; i < m_Meshes.size(); ++i)
	{
		// TODO: Get proper pointer size for the current system (instead of just uint64_t).
		UboModel* meshModelStartAddress = (UboModel*)((ptr_size)m_ModelTransferSpace + (i * m_ModelUniformAlignment));
		*meshModelStartAddress = m_Meshes[i].GetModel();
	}

	vkMapMemory(m_Device, m_ModelDynamicUniformBuffersMemory[ImageIndex], 0, m_ModelUniformAlignment * m_Meshes.size(), 0, &data);
	memcpy(data, m_ModelTransferSpace, m_ModelUniformAlignment * m_Meshes.size());
	vkUnmapMemory(m_Device, m_ModelDynamicUniformBuffersMemory[ImageIndex]);
}
