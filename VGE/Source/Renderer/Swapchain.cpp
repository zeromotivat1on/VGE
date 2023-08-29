#include "Swapchain.h"
#include "Window.h"
#include "VulkanUtils.h"

#pragma region Statics
static VkSurfaceFormatKHR GetBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
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

static VkPresentModeKHR GetBestPresentMode(const std::vector<VkPresentModeKHR>& modes)
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

static VkExtent2D GetBestSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities)
{
	if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
	{
		return surfaceCapabilities.currentExtent;
	}

	int32 width, height;
	vge::GWindow->GetFramebufferSize(width, height);

	VkExtent2D newExtent = {};
	newExtent.width = std::clamp(static_cast<uint32>(width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
	newExtent.height = std::clamp(static_cast<uint32>(height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

	return newExtent;
}
#pragma endregion Statics

vge::Swapchain::Swapchain(Device& device) : m_Device(device) {}

void vge::Swapchain::Initialize()
{
	SwapchainSupportDetails swapchainDetails = m_Device.GetSwapchainSupportDetails();
	QueueFamilyIndices queueIndices = m_Device.GetQueueIndices();

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
	swapchainCreateInfo.surface = m_Device.GetSurface();
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

	if (queueIndices.GraphicsFamily != queueIndices.PresentFamily)
	{
		const uint32 queueFamilyIndices[] =
		{
			static_cast<uint32>(queueIndices.GraphicsFamily),
			static_cast<uint32>(queueIndices.PresentFamily)
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

	VK_ENSURE(vkCreateSwapchainKHR(m_Device.GetHandle(), &swapchainCreateInfo, nullptr, &m_Handle));

	m_ImageFormat = surfaceFormat.format;
	m_Extent = extent;

	uint32 swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(m_Device.GetHandle(), m_Handle, &swapchainImageCount, nullptr);

	std::vector<VkImage> images(swapchainImageCount);
	vkGetSwapchainImagesKHR(m_Device.GetHandle(), m_Handle, &swapchainImageCount, images.data());

	for (const auto& image : images)
	{
		SwapchainImage swapchainImage = {};
		swapchainImage.Handle = image;
		CreateImageView(image, m_ImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapchainImage.View);

		m_Images.push_back(swapchainImage);
	}
}

void vge::Swapchain::Destroy()
{
	for (VkFramebuffer& framebuffer : m_Framebuffers)
	{
		vkDestroyFramebuffer(m_Device.GetHandle(), framebuffer, nullptr);
	}

	for (SwapchainImage& swapchainImage : m_Images)
	{
		vkDestroyImageView(m_Device.GetHandle(), swapchainImage.View, nullptr);
	}

	vkDestroySwapchainKHR(m_Device.GetHandle(), m_Handle, nullptr);
}

void vge::Swapchain::CreateFramebuffer(VkRenderPass renderPass, uint32 attachmentCount, const VkImageView* attachments)
{
	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = attachmentCount;
	framebufferCreateInfo.pAttachments = attachments;
	framebufferCreateInfo.width = m_Extent.width;
	framebufferCreateInfo.height = m_Extent.height;
	framebufferCreateInfo.layers = 1;

	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	VK_ENSURE(vkCreateFramebuffer(m_Device.GetHandle(), &framebufferCreateInfo, nullptr, &framebuffer));
	m_Framebuffers.push_back(framebuffer);
}