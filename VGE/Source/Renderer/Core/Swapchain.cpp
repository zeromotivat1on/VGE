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
		// If nothing found, always default to FIFO
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
	// Try to find the requested surface format in the supported surface formats
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

	// If the requested surface format isn't found, then try to request a format from the priority list
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

		// If nothing found, default the first supporte surface format
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
	:
	Swapchain{ oldSwapchain,
			  oldSwapchain.device,
			  oldSwapchain.surface,
			  oldSwapchain.properties.present_mode,
			  oldSwapchain.present_mode_priority_list,
			  oldSwapchain.surface_format_priority_list,
			  extent,
			  oldSwapchain.properties.image_count,
			  oldSwapchain.properties.pre_transform,
			  oldSwapchain.image_usage_flags }
{}

vge::Swapchain::Swapchain(Swapchain& oldSwapchain, const uint32_t image_count)
	:
	Swapchain{ oldSwapchain,
			  oldSwapchain.device,
			  oldSwapchain.surface,
			  oldSwapchain.properties.present_mode,
			  oldSwapchain.present_mode_priority_list,
			  oldSwapchain.surface_format_priority_list,
			  oldSwapchain.properties.extent,
			  image_count,
			  oldSwapchain.properties.pre_transform,
			  oldSwapchain.image_usage_flags }
{}

vge::Swapchain::Swapchain(Swapchain& oldSwapchain, const std::set<VkImageUsageFlagBits>& image_usage_flags)
	:
	Swapchain{ oldSwapchain,
			  oldSwapchain.device,
			  oldSwapchain.surface,
			  oldSwapchain.properties.present_mode,
			  oldSwapchain.present_mode_priority_list,
			  oldSwapchain.surface_format_priority_list,
			  oldSwapchain.properties.extent,
			  oldSwapchain.properties.image_count,
			  oldSwapchain.properties.pre_transform,
			  image_usage_flags }
{}

vge::Swapchain::Swapchain(Swapchain& oldSwapchain, const VkExtent2D& extent, const VkSurfaceTransformFlagBitsKHR transform)
	:
	Swapchain{ oldSwapchain,
			  oldSwapchain.device,
			  oldSwapchain.surface,
			  oldSwapchain.properties.present_mode,
			  oldSwapchain.present_mode_priority_list,
			  oldSwapchain.surface_format_priority_list,
			  extent,
			  oldSwapchain.properties.image_count,
			  transform,
			  oldSwapchain.image_usage_flags }
{}

vge::Swapchain::Swapchain(Device& device,
	VkSurfaceKHR                           surface,
	const VkPresentModeKHR                 present_mode,
	std::vector<VkPresentModeKHR> const& present_mode_priority_list,
	const std::vector<VkSurfaceFormatKHR>& surface_format_priority_list,
	const VkExtent2D& extent,
	const uint32_t                         image_count,
	const VkSurfaceTransformFlagBitsKHR    transform,
	const std::set<VkImageUsageFlagBits>& image_usage_flags) 
	:
	Swapchain{ *this, device, surface, present_mode, present_mode_priority_list, surface_format_priority_list, extent, image_count, transform, image_usage_flags }
{
}

vge::Swapchain::Swapchain(Swapchain& oldSwapchain,
	Device& device,
	VkSurfaceKHR                           surface,
	const VkPresentModeKHR                 present_mode,
	std::vector<VkPresentModeKHR> const& present_mode_priority_list,
	const std::vector<VkSurfaceFormatKHR>& surface_format_priority_list,
	const VkExtent2D& extent,
	const uint32_t                         image_count,
	const VkSurfaceTransformFlagBitsKHR    transform,
	const std::set<VkImageUsageFlagBits>& image_usage_flags) 
	:
	device{ device },
	surface{ surface }
{
	this->present_mode_priority_list = present_mode_priority_list;
	this->surface_format_priority_list = surface_format_priority_list;

	VkSurfaceCapabilitiesKHR surface_capabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->device.get_gpu().GetHandle(), surface, &surface_capabilities);

	uint32_t surface_format_count{ 0U };
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(this->device.get_gpu().GetHandle(), surface, &surface_format_count, nullptr));
	surface_formats.resize(surface_format_count);
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(this->device.get_gpu().GetHandle(), surface, &surface_format_count, surface_formats.data()));

	LOG(Log, "Surface supports the following surface formats:");
	for (auto& surface_format : surface_formats)
	{
		LOG(Log, "  \t{}", ToString(surface_format));
	}

	uint32_t present_mode_count{ 0U };
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(this->device.get_gpu().GetHandle(), surface, &present_mode_count, nullptr));
	present_modes.resize(present_mode_count);
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(this->device.get_gpu().GetHandle(), surface, &present_mode_count, present_modes.data()));

	LOG(Log, "Surface supports the following present modes:");
	for (auto& present_mode : present_modes)
	{
		LOG(Log, "  \t{}", ToString(present_mode));
	}

	// Chose best properties based on surface capabilities
	properties.image_count = ChooseImageCount(image_count, surface_capabilities.minImageCount, surface_capabilities.maxImageCount);
	properties.extent = ChooseExtent(extent, surface_capabilities.minImageExtent, surface_capabilities.maxImageExtent, surface_capabilities.currentExtent);
	properties.array_layers = ChooseImageArrayLayers(1U, surface_capabilities.maxImageArrayLayers);
	properties.surface_format = ChooseSurfaceFormat(properties.surface_format, surface_formats, surface_format_priority_list);
	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(this->device.get_gpu().GetHandle(), properties.surface_format.format, &format_properties);
	this->image_usage_flags = ChooseImageUsage(image_usage_flags, surface_capabilities.supportedUsageFlags, format_properties.optimalTilingFeatures);
	properties.image_usage = CompositeImageFlags(this->image_usage_flags);
	properties.pre_transform = ChooseTransform(transform, surface_capabilities.supportedTransforms, surface_capabilities.currentTransform);
	properties.composite_alpha = ChooseCompositeAlpha(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, surface_capabilities.supportedCompositeAlpha);

	// Pass through defaults to the create function
	properties.oldSwapchain = oldSwapchain.GetHandle();
	properties.present_mode = present_mode;

	properties.present_mode = ChoosePresentMode(properties.present_mode, present_modes, present_mode_priority_list);
	properties.surface_format = ChooseSurfaceFormat(properties.surface_format, surface_formats, surface_format_priority_list);

	VkSwapchainCreateInfoKHR create_info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	create_info.minImageCount = properties.image_count;
	create_info.imageExtent = properties.extent;
	create_info.presentMode = properties.present_mode;
	create_info.imageFormat = properties.surface_format.format;
	create_info.imageColorSpace = properties.surface_format.colorSpace;
	create_info.imageArrayLayers = properties.array_layers;
	create_info.imageUsage = properties.image_usage;
	create_info.preTransform = properties.pre_transform;
	create_info.compositeAlpha = properties.composite_alpha;
	create_info.oldSwapchain = properties.oldSwapchain;
	create_info.surface = surface;

	VkResult result = vkCreateSwapchainKHR(device.GetHandle(), &create_info, nullptr, &handle);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{ result, "Cannot create Swapchain" };
	}

	uint32_t image_available{ 0u };
	VK_CHECK(vkGetSwapchainImagesKHR(device.GetHandle(), handle, &image_available, nullptr));

	images.resize(image_available);

	VK_CHECK(vkGetSwapchainImagesKHR(device.GetHandle(), handle, &image_available, images.data()));
}

vge::Swapchain::~Swapchain()
{
	if (handle)
	{
		vkDestroySwapchainKHR(device.GetHandle(), handle, nullptr);
	}
}

vge::Swapchain::Swapchain(Swapchain&& other)
	:
	device{ other.device },
	surface{ std::exchange(other.surface, VK_NULL_HANDLE) },
	handle{ std::exchange(other.handle, VK_NULL_HANDLE) },
	images{ std::exchange(other.images, {}) },
	surface_formats{ std::exchange(other.surface_formats, {}) },
	present_modes{ std::exchange(other.present_modes, {}) },
	properties{ std::exchange(other.properties, {}) },
	present_mode_priority_list{ std::exchange(other.present_mode_priority_list, {}) },
	surface_format_priority_list{ std::exchange(other.surface_format_priority_list, {}) },
	image_usage_flags{ std::move(other.image_usage_flags) }
{
}

VkResult vge::Swapchain::AcquireNextImage(u32& imageIndex, VkSemaphore imageAcquiredSemaphore, VkFence fence /*= VK_NULL_HANDLE*/) const
{
	return vkAcquireNextImageKHR(device.GetHandle(), handle, std::numeric_limits<uint64_t>::max(), imageAcquiredSemaphore, fence, &imageIndex);
}

