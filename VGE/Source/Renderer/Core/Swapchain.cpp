#include "Swapchain.h"
#include "Device.h"

namespace vge
{
namespace
{
inline u32 ChooseImageCount(u32 requestImageCount, u32 minImageCount, u32 maxImageCount)
{
	if (maxImageCount != 0)
	{
		requestImageCount = std::min(requestImageCount, maxImageCount);
	}

	requestImageCount = std::max(requestImageCount, minImageCount);

	return requestImageCount;
}

inline u32 ChooseImageArrayLayers(u32 requestImageArrayLayers, u32 maxImageArrayLayers)
{
	requestImageArrayLayers = std::min(requestImageArrayLayers, maxImageArrayLayers);
	requestImageArrayLayers = std::max(requestImageArrayLayers, 1u);

	return requestImageArrayLayers;
}

inline VkExtent2D ChooseExtent(VkExtent2D requestExtent, const VkExtent2D& minImageExtent, const VkExtent2D& maxImageExtent, const VkExtent2D& currentExtent)
{
	if (currentExtent.width == 0xFFFFFFFF)
	{
		return requestExtent;
	}

	if (requestExtent.width < 1 || requestExtent.height < 1)
	{
		LOG(Warning, "Image extent (%d, %d) not supported, selecting (%d, %d).", requestExtent.width, requestExtent.height, currentExtent.width, currentExtent.height);
		return currentExtent;
	}

	requestExtent.width = std::max(requestExtent.width, minImageExtent.width);
	requestExtent.width = std::min(requestExtent.width, maxImageExtent.width);

	requestExtent.height = std::max(requestExtent.height, minImageExtent.height);
	requestExtent.height = std::min(requestExtent.height, maxImageExtent.height);

	return requestExtent;
}

inline VkPresentModeKHR ChoosePresentMode(
	VkPresentModeKHR requestPresentMode,
	const std::vector<VkPresentModeKHR>& availablePresentModes,
	const std::vector<VkPresentModeKHR>& presentModePriorityList)
{
	auto presentModeIt = std::find(availablePresentModes.begin(), availablePresentModes.end(), requestPresentMode);

	if (presentModeIt == availablePresentModes.end())
	{
		// If nothing found, always default to FIFO.
		VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		for (auto& present_mode : presentModePriorityList)
		{
			if (std::find(availablePresentModes.begin(), availablePresentModes.end(), present_mode) != availablePresentModes.end())
			{
				chosenPresentMode = present_mode;
				break;
			}
		}

		LOG(Warning, "Present mode %s not supported, selecting %s.", ToString(requestPresentMode), ToString(chosenPresentMode));
		return chosenPresentMode;
	}
	else
	{
		LOG(Log, "Present mode selected %s.", ToString(requestPresentMode));
		return *presentModeIt;
	}
}

inline VkSurfaceFormatKHR ChooseSurfaceFormat(
	const VkSurfaceFormatKHR requestedSurfaceFormat,
	const std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats,
	const std::vector<VkSurfaceFormatKHR>& surfaceFormatPriorityList)
{
	// Try to find the requested surface format in the supported surface formats.
	auto surfaceFormatIt = std::find_if(availableSurfaceFormats.begin(), availableSurfaceFormats.end(),
		[&requestedSurfaceFormat](const VkSurfaceFormatKHR& surface)
		{
			if (surface.format == requestedSurfaceFormat.format &&
				surface.colorSpace == requestedSurfaceFormat.colorSpace)
			{
				return true;
			}

			return false;
		});

	// If the requested surface format isn't found, then try to request a format from the priority list.
	if (surfaceFormatIt == availableSurfaceFormats.end())
	{
		for (auto& surfaceFormat : surfaceFormatPriorityList)
		{
			surfaceFormatIt = std::find_if(availableSurfaceFormats.begin(), availableSurfaceFormats.end(),
				[&surfaceFormat](const VkSurfaceFormatKHR& surface) {
					if (surface.format == surfaceFormat.format &&
						surface.colorSpace == surfaceFormat.colorSpace)
					{
						return true;
					}

					return false;
				});

			if (surfaceFormatIt != availableSurfaceFormats.end())
			{
				LOG(Warning, "Surface format %s not supported, selecting %s.", ToString(requestedSurfaceFormat), ToString(*surfaceFormatIt));
				return *surfaceFormatIt;
			}
		}

		// If nothing found, default the first supporte surface format.
		surfaceFormatIt = availableSurfaceFormats.begin();
		LOG(Warning, "Surface format %s not supported, selecting %s.", ToString(requestedSurfaceFormat), ToString(*surfaceFormatIt));
	}
	else
	{
		LOG(Log, "Surface format selected %s.", ToString(requestedSurfaceFormat));
	}

	return *surfaceFormatIt;
}

inline VkSurfaceTransformFlagBitsKHR ChooseTransform(VkSurfaceTransformFlagBitsKHR requestTransform, VkSurfaceTransformFlagsKHR supportedTransform, VkSurfaceTransformFlagBitsKHR currentTransform)
{
	if (requestTransform & supportedTransform)
	{
		return requestTransform;
	}

	LOG(Warning, "Surface transform %s not supported, selecting %s.", ToString(requestTransform), ToString(currentTransform));

	return currentTransform;
}

inline VkCompositeAlphaFlagBitsKHR ChooseCompositeAlpha(VkCompositeAlphaFlagBitsKHR requestCompositeAlpha, VkCompositeAlphaFlagsKHR supportedCompositeAlpha)
{
	if (requestCompositeAlpha & supportedCompositeAlpha)
	{
		return requestCompositeAlpha;
	}

	static const std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags =
	{
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
	};

	for (VkCompositeAlphaFlagBitsKHR compositeAlpha : compositeAlphaFlags)
	{
		if (compositeAlpha & supportedCompositeAlpha)
		{
			LOG(Warning, "Composite alpha %s not supported, selecting %s.", ToString(requestCompositeAlpha), ToString(compositeAlpha));
			return compositeAlpha;
		}
	}

	ENSURE_MSG(false, "No compatible composite alpha found.");
}

inline bool ValidateFormatFeature(VkImageUsageFlagBits imageUsage, VkFormatFeatureFlags supportedFeatures)
{
	switch (imageUsage)
	{
	case VK_IMAGE_USAGE_STORAGE_BIT:
		return VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT & supportedFeatures;
	default:
		return true;
	}
}

inline std::set<VkImageUsageFlagBits> ChooseImageUsage(const std::set<VkImageUsageFlagBits>& requestedImageUsageFlags, VkImageUsageFlags supportedImageUsage, VkFormatFeatureFlags supportedFeatures)
{
	std::set<VkImageUsageFlagBits> validatedImageUsageFlags;
	for (auto flag : requestedImageUsageFlags)
	{
		if ((flag & supportedImageUsage) && ValidateFormatFeature(flag, supportedFeatures))
		{
			validatedImageUsageFlags.insert(flag);
		}
		else
		{
			LOG(Warning, "Image usage %s requested but not supported.", ToString(flag));
		}
	}

	if (validatedImageUsageFlags.empty())
	{
		// Pick the first format from list of defaults, if supported.
		static const std::vector<VkImageUsageFlagBits> imageUsageFlags = {
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_IMAGE_USAGE_STORAGE_BIT,
			VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT };

		for (VkImageUsageFlagBits imageUsage : imageUsageFlags)
		{
			if ((imageUsage & supportedImageUsage) && ValidateFormatFeature(imageUsage, supportedFeatures))
			{
				validatedImageUsageFlags.insert(imageUsage);
				break;
			}
		}
	}

	if (!validatedImageUsageFlags.empty())
	{
		// Log image usage flags used.
		std::string usageList;
		for (VkImageUsageFlagBits imageUsage : validatedImageUsageFlags)
		{
			usageList += ToString(imageUsage) + " ";
		}
		LOG(Log, "Image usage flags: %s.", usageList);
	}
	else
	{
		ENSURE_MSG(false, "No compatible image usage found.");
	}

	return validatedImageUsageFlags;
}

inline VkImageUsageFlags CompositeImageFlags(std::set<VkImageUsageFlagBits>& imageUsageFlags)
{
	VkImageUsageFlags imageUsage = {};
	for (auto flag : imageUsageFlags)
	{
		imageUsage |= flag;
	}
	return imageUsage;
}
}	// namespace
}	// namespace vge

vge::Swapchain::Swapchain(Swapchain& oldSwapchain, const VkExtent2D& extent)
	: Swapchain(oldSwapchain,
		oldSwapchain._Device,
		oldSwapchain._Surface,
		oldSwapchain._Properties.PresentMode,
		oldSwapchain._PresentModePriorityList,
		oldSwapchain._SurfaceFormatPriorityList,
		extent,
		oldSwapchain._Properties.ImageCount,
		oldSwapchain._Properties.PreTransform,
		oldSwapchain._ImageUsageFlags)
{}

vge::Swapchain::Swapchain(Swapchain& oldSwapchain, const u32 imageCount)
	: Swapchain(oldSwapchain,
		oldSwapchain._Device,
		oldSwapchain._Surface,
		oldSwapchain._Properties.PresentMode,
		oldSwapchain._PresentModePriorityList,
		oldSwapchain._SurfaceFormatPriorityList,
		oldSwapchain._Properties.Extent,
		imageCount,
		oldSwapchain._Properties.PreTransform,
		oldSwapchain._ImageUsageFlags)
{}

vge::Swapchain::Swapchain(Swapchain& oldSwapchain, const std::set<VkImageUsageFlagBits>& imageUsageFlags)
	: Swapchain(oldSwapchain,
		oldSwapchain._Device,
		oldSwapchain._Surface,
		oldSwapchain._Properties.PresentMode,
		oldSwapchain._PresentModePriorityList,
		oldSwapchain._SurfaceFormatPriorityList,
		oldSwapchain._Properties.Extent,
		oldSwapchain._Properties.ImageCount,
		oldSwapchain._Properties.PreTransform,
		imageUsageFlags)
{}

vge::Swapchain::Swapchain(Swapchain& oldSwapchain, const VkExtent2D& extent, const VkSurfaceTransformFlagBitsKHR transform)
	: Swapchain(oldSwapchain,
		oldSwapchain._Device,
		oldSwapchain._Surface,
		oldSwapchain._Properties.PresentMode,
		oldSwapchain._PresentModePriorityList,
		oldSwapchain._SurfaceFormatPriorityList,
		extent,
		oldSwapchain._Properties.ImageCount,
		transform,
		oldSwapchain._ImageUsageFlags)
{}

vge::Swapchain::Swapchain(
	Device& device,
	VkSurfaceKHR surface,
	const VkPresentModeKHR presentMode,
	const std::vector<VkPresentModeKHR>& presentModePriorityList,
	const std::vector<VkSurfaceFormatKHR>& surfaceFormatPriorityList,
	const VkExtent2D& extent,
	const u32 imageCount,
	const VkSurfaceTransformFlagBitsKHR transform,
	const std::set<VkImageUsageFlagBits>& imageUsageFlags)
	:
	Swapchain(*this, device, surface, presentMode, presentModePriorityList, surfaceFormatPriorityList, extent, imageCount, transform, imageUsageFlags)
{}

vge::Swapchain::Swapchain(
	Swapchain& oldSwapchain,
	Device& device,
	VkSurfaceKHR surface,
	const VkPresentModeKHR presentMode,
	std::vector<VkPresentModeKHR> const& presentModePriorityList,
	const std::vector<VkSurfaceFormatKHR>& surfaceFormatPriorityList,
	const VkExtent2D& extent,
	const u32 imageCount,
	const VkSurfaceTransformFlagBitsKHR transform,
	const std::set<VkImageUsageFlagBits>& imageUsageFlags)
	:
	_Device(device), _Surface(surface)
{
	_PresentModePriorityList = presentModePriorityList;
	_SurfaceFormatPriorityList = surfaceFormatPriorityList;

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_Device.GetGpu().GetHandle(), surface, &surfaceCapabilities);

	u32 surfaceFormatCount = 0;
	VK_ENSURE(vkGetPhysicalDeviceSurfaceFormatsKHR(_Device.GetGpu().GetHandle(), surface, &surfaceFormatCount, nullptr));
	_SurfaceFormats.resize(surfaceFormatCount);
	VK_ENSURE(vkGetPhysicalDeviceSurfaceFormatsKHR(_Device.GetGpu().GetHandle(), surface, &surfaceFormatCount, _SurfaceFormats.data()));

	LOG(Log, "Surface supports the following surface formats:");
	for (auto& surface_format : _SurfaceFormats)
	{
		LOG(Log, "  \t%s", ToString(surface_format));
	}

	u32 presentModeCount = 0;
	VK_ENSURE(vkGetPhysicalDeviceSurfacePresentModesKHR(_Device.GetGpu().GetHandle(), surface, &presentModeCount, nullptr));
	_PresentModes.resize(presentModeCount);
	VK_ENSURE(vkGetPhysicalDeviceSurfacePresentModesKHR(_Device.GetGpu().GetHandle(), surface, &presentModeCount, _PresentModes.data()));

	LOG(Log, "Surface supports the following present modes:");
	for (auto& present_mode : _PresentModes)
	{
		LOG(Log, "  \t%s", ToString(present_mode));
	}

	// Chose best properties based on surface capabilities.
	_Properties.ImageCount = ChooseImageCount(imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
	_Properties.Extent = ChooseExtent(extent, surfaceCapabilities.minImageExtent, surfaceCapabilities.maxImageExtent, surfaceCapabilities.currentExtent);
	_Properties.ArrayLayers = ChooseImageArrayLayers(1U, surfaceCapabilities.maxImageArrayLayers);
	_Properties.SurfaceFormat = ChooseSurfaceFormat(_Properties.SurfaceFormat, _SurfaceFormats, surfaceFormatPriorityList);
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(_Device.GetGpu().GetHandle(), _Properties.SurfaceFormat.format, &formatProperties);
	_ImageUsageFlags = ChooseImageUsage(imageUsageFlags, surfaceCapabilities.supportedUsageFlags, formatProperties.optimalTilingFeatures);
	_Properties.ImageUsage = CompositeImageFlags(_ImageUsageFlags);
	_Properties.PreTransform = ChooseTransform(transform, surfaceCapabilities.supportedTransforms, surfaceCapabilities.currentTransform);
	_Properties.CompositeAlpha = ChooseCompositeAlpha(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, surfaceCapabilities.supportedCompositeAlpha);

	// Pass through defaults to the create function.
	_Properties.OldSwapchain = oldSwapchain.GetHandle();
	_Properties.PresentMode = presentMode;

	_Properties.PresentMode = ChoosePresentMode(_Properties.PresentMode, _PresentModes, presentModePriorityList);
	_Properties.SurfaceFormat = ChooseSurfaceFormat(_Properties.SurfaceFormat, _SurfaceFormats, surfaceFormatPriorityList);

	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.minImageCount = _Properties.ImageCount;
	createInfo.imageExtent = _Properties.Extent;
	createInfo.presentMode = _Properties.PresentMode;
	createInfo.imageFormat = _Properties.SurfaceFormat.format;
	createInfo.imageColorSpace = _Properties.SurfaceFormat.colorSpace;
	createInfo.imageArrayLayers = _Properties.ArrayLayers;
	createInfo.imageUsage = _Properties.ImageUsage;
	createInfo.preTransform = _Properties.PreTransform;
	createInfo.compositeAlpha = _Properties.CompositeAlpha;
	createInfo.oldSwapchain = _Properties.OldSwapchain;
	createInfo.surface = surface;

	VK_ENSURE(vkCreateSwapchainKHR(device.GetHandle(), &createInfo, nullptr, &_Handle));

	u32 swapchainImageCount = 0;
	VK_ENSURE(vkGetSwapchainImagesKHR(device.GetHandle(), _Handle, &swapchainImageCount, nullptr));
	_Images.resize(swapchainImageCount);
	VK_ENSURE(vkGetSwapchainImagesKHR(device.GetHandle(), _Handle, &swapchainImageCount, _Images.data()));
}

vge::Swapchain::~Swapchain()
{
	if (_Handle)
	{
		vkDestroySwapchainKHR(_Device.GetHandle(), _Handle, nullptr);
	}
}

vge::Swapchain::Swapchain(Swapchain&& other)
	:
	_Device(other._Device),
	_Surface(std::exchange(other._Surface, VK_NULL_HANDLE)),
	_Handle(std::exchange(other._Handle, VK_NULL_HANDLE)),
	_Images(std::exchange(other._Images, {})),
	_SurfaceFormats(std::exchange(other._SurfaceFormats, {})),
	_PresentModes(std::exchange(other._PresentModes, {})),
	_Properties(std::exchange(other._Properties, {})),
	_PresentModePriorityList(std::exchange(other._PresentModePriorityList, {})),
	_SurfaceFormatPriorityList(std::exchange(other._SurfaceFormatPriorityList, {})),
	_ImageUsageFlags(std::move(other._ImageUsageFlags))
{
}

VkResult vge::Swapchain::AcquireNextImage(u32& imageIndex, VkSemaphore imageAcquiredSemaphore, VkFence fence /*= VK_NULL_HANDLE*/) const
{
	return vkAcquireNextImageKHR(_Device.GetHandle(), _Handle, std::numeric_limits<uint64_t>::max(), imageAcquiredSemaphore, fence, &imageIndex);
}

