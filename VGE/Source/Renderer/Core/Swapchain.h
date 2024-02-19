#pragma once

#include "Core/Common.h"

namespace vge
{
class Device;

enum class ImageFormat : u8
{
	sRGB,
	UNORM
};

struct SwapchainProperties
{
	VkSwapchainKHR OldSwapchain;
	u32 ImageCount = 3;
	VkExtent2D Extent = {};
	VkSurfaceFormatKHR SurfaceFormat = {};
	u32 ArrayLayers;
	VkImageUsageFlags ImageUsage;
	VkSurfaceTransformFlagBitsKHR PreTransform;
	VkCompositeAlphaFlagBitsKHR CompositeAlpha;
	VkPresentModeKHR PresentMode;
};

class Swapchain
{
public:
	Swapchain(Swapchain& oldSwapchain, const VkExtent2D& extent);
	Swapchain(Swapchain& oldSwapchain, const u32 imageCount);
	Swapchain(Swapchain& oldSwapchain, const std::set<VkImageUsageFlagBits>& imageUsageFlags);
	Swapchain(Swapchain& swapchain, const VkExtent2D& extent, const VkSurfaceTransformFlagBitsKHR transform);

	Swapchain(
		Device& device,
		VkSurfaceKHR surface,
		const VkPresentModeKHR present_mode,
		const std::vector<VkPresentModeKHR>& presentModePriorityList = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR },
		const std::vector<VkSurfaceFormatKHR>& surfaceFormatPriorityList = { {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} },
		const VkExtent2D& extent = {},
		const u32 imageCount = 3,
		const VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		const std::set<VkImageUsageFlagBits>& imageUsageFlags = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT });

	Swapchain(
		Swapchain& oldSwapchain,
		Device& device,
		VkSurfaceKHR surface,
		const VkPresentModeKHR present_mode,
		const std::vector<VkPresentModeKHR>& presentModePriorityList = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR },
		const std::vector<VkSurfaceFormatKHR>& surfaceFormatPriorityList = { {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} },
		const VkExtent2D& extent = {},
		const u32 imageCount = 3,
		const VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		const std::set<VkImageUsageFlagBits>& imageUsageFlags = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT });

	COPY_CTOR_DEL(Swapchain);
	Swapchain(Swapchain&& other);

	~Swapchain();

	COPY_OP_DEL(Swapchain);
	MOVE_OP_DEL(Swapchain);

public:
	inline bool IsValid() const { return _Handle != VK_NULL_HANDLE; }
	inline Device& GetDevice() { return _Device; }
	inline VkSwapchainKHR GetHandle() const { return _Handle; }
	inline const VkExtent2D& GetExtent() const { return _Properties.Extent; }
	inline VkFormat GetFormat() const { return _Properties.SurfaceFormat.format; }
	inline const std::vector<VkImage>& GetImages() const { return _Images; }
	inline VkSurfaceTransformFlagBitsKHR GetTransform() const { return _Properties.PreTransform; }
	inline VkSurfaceKHR GetSurface() const { return _Surface; }
	inline VkImageUsageFlags GetUsage() const { return _Properties.ImageUsage; }
	inline VkPresentModeKHR GetPresentMode() const { return _Properties.PresentMode; }

	VkResult AcquireNextImage(u32& imageIndex, VkSemaphore imageAcquiredSemaphore, VkFence fence = VK_NULL_HANDLE) const;

private:
	Device& _Device;
	VkSurfaceKHR _Surface = VK_NULL_HANDLE;
	VkSwapchainKHR _Handle = VK_NULL_HANDLE;
	std::vector<VkImage> _Images;
	std::vector<VkSurfaceFormatKHR> _SurfaceFormats;
	std::vector<VkPresentModeKHR> _PresentModes;
	SwapchainProperties _Properties;
	std::vector<VkPresentModeKHR> _PresentModePriorityList = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR };
	std::vector<VkSurfaceFormatKHR> _SurfaceFormatPriorityList = { {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} };
	std::set<VkImageUsageFlagBits> _ImageUsageFlags;
};
}	// namespace vge
