#include "Swapchain.h"
#include "Window.h"
#include "Utils.h"
#include "RenderPass.h"

namespace vge
{
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

		i32 width, height;
		vge::GWindow->GetFramebufferSize(width, height);

		VkExtent2D newExtent = {};
		newExtent.width = std::clamp(static_cast<u32>(width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
		newExtent.height = std::clamp(static_cast<u32>(height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

		return newExtent;
	}
#pragma endregion Statics
}

vge::Swapchain::Swapchain(const Device* device) : m_Device(device) 
{
}

void vge::Swapchain::Initialize(SwapchainRecreateInfo* recreateInfo /*= nullptr*/)
{
	if (recreateInfo && recreateInfo->IsValid())
	{
		m_Surface = recreateInfo->Surface;
		recreateInfo->Surface = VK_NULL_HANDLE;
	}
	else
	{
		m_Device->CreateWindowSurface(m_Surface);
	}

	SwapchainSupportDetails swapchainDetails = GetSupportDetails();
	QueueFamilyIndices queueIndices = m_Device->GetQueueIndices();

	VkSurfaceFormatKHR surfaceFormat = GetBestSurfaceFormat(swapchainDetails.SurfaceFormats);
	VkPresentModeKHR presentMode = GetBestPresentMode(swapchainDetails.PresentModes);
	VkExtent2D extent = GetBestSwapchainExtent(swapchainDetails.SurfaceCapabilities);

	u32 imageCount = swapchainDetails.SurfaceCapabilities.minImageCount + 1;
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

	if (queueIndices.GraphicsFamily != queueIndices.PresentFamily)
	{
		const u32 queueFamilyIndices[] =
		{
			static_cast<u32>(queueIndices.GraphicsFamily),
			static_cast<u32>(queueIndices.PresentFamily)
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

	if (recreateInfo && recreateInfo->Swapchain != VK_NULL_HANDLE)
	{
		swapchainCreateInfo.oldSwapchain = recreateInfo->Swapchain;
	}
	else 
	{
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	}

	VK_ENSURE(vkCreateSwapchainKHR(m_Device->GetHandle(), &swapchainCreateInfo, nullptr, &m_Handle));

	if (recreateInfo)
	{
		if (recreateInfo->Swapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(m_Device->GetHandle(), recreateInfo->Swapchain, nullptr);
			recreateInfo->Swapchain = VK_NULL_HANDLE;
		}

		if (recreateInfo->Surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(m_Device->GetInstance(), recreateInfo->Surface, nullptr);
			recreateInfo->Surface = VK_NULL_HANDLE;
		}
	}

	m_ImageFormat = surfaceFormat.format;
	m_Extent = extent;

	LOG(Log, "Dimensions: %dx%d", m_Extent.width, m_Extent.height);

	u32 swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(m_Device->GetHandle(), m_Handle, &swapchainImageCount, nullptr);

	std::vector<VkImage> images(swapchainImageCount);
	vkGetSwapchainImagesKHR(m_Device->GetHandle(), m_Handle, &swapchainImageCount, images.data());

	for (const VkImage& image : images)
	{
		ImageViewCreateInfo imgViewCreateInfo = {};
		imgViewCreateInfo.Device = m_Device;
		imgViewCreateInfo.Format = m_ImageFormat;
		imgViewCreateInfo.AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		imgViewCreateInfo.Image = image;

		SwapchainImage swapchainImage = {};
		swapchainImage.Handle = image;
		swapchainImage.View = Image::CreateView(imgViewCreateInfo);

		m_Images.push_back(swapchainImage);
	}
}

void vge::Swapchain::Destroy(SwapchainRecreateInfo* recreateInfo /*= nullptr*/)
{
	for (FrameBuffer& framebuffer : m_Framebuffers)
	{
		framebuffer.Destroy();
	}

	for (SwapchainImage& swapchainImage : m_Images)
	{
		vkDestroyImageView(m_Device->GetHandle(), swapchainImage.View, nullptr);
	}

	if (recreateInfo && recreateInfo->IsValid())
	{
		recreateInfo->Swapchain = m_Handle;
		recreateInfo->Surface = m_Surface;
	}
	else
	{
		vkDestroySwapchainKHR(m_Device->GetHandle(), m_Handle, nullptr);
		vkDestroySurfaceKHR(m_Device->GetInstance(), m_Surface, nullptr);
	}
}

void vge::Swapchain::CreateFramebuffer(const RenderPass* renderPass, u32 attachmentCount, const VkImageView* attachments)
{
	FrameBufferCreateInfo createInfo = {};
	createInfo.Device = m_Device;
	createInfo.RenderPass = renderPass->GetHandle();
	createInfo.AttachmentCount = attachmentCount;
	createInfo.Attachments = attachments;
	createInfo.Extent = m_Extent;

	m_Framebuffers.push_back(FrameBuffer::Create(createInfo));
}
