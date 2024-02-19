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
	VkSwapchainKHR                old_swapchain;
	u32                      image_count{ 3 };
	VkExtent2D                    extent{};
	VkSurfaceFormatKHR            surface_format{};
	u32                      array_layers;
	VkImageUsageFlags             image_usage;
	VkSurfaceTransformFlagBitsKHR pre_transform;
	VkCompositeAlphaFlagBitsKHR   composite_alpha;
	VkPresentModeKHR              present_mode;
};

class Swapchain
{
public:
	Swapchain(Swapchain& old_swapchain, const VkExtent2D& extent);
	Swapchain(Swapchain& old_swapchain, const u32 image_count);
	Swapchain(Swapchain& old_swapchain, const std::set<VkImageUsageFlagBits>& image_usage_flags);
	Swapchain(Swapchain& swapchain, const VkExtent2D& extent, const VkSurfaceTransformFlagBitsKHR transform);

	Swapchain(
		Device& device,
		VkSurfaceKHR surface,
		const VkPresentModeKHR present_mode,
		const std::vector<VkPresentModeKHR>& present_mode_priority_list = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR },
		const std::vector<VkSurfaceFormatKHR>& surface_format_priority_list = { {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} },
		const VkExtent2D& extent = {},
		const u32 image_count = 3,
		const VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		const std::set<VkImageUsageFlagBits>& image_usage_flags = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT });

	Swapchain(
		Swapchain& old_swapchain,
		Device& device,
		VkSurfaceKHR surface,
		const VkPresentModeKHR present_mode,
		const std::vector<VkPresentModeKHR>& present_mode_priority_list = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR },
		const std::vector<VkSurfaceFormatKHR>& surface_format_priority_list = { {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} },
		const VkExtent2D& extent = {},
		const u32 image_count = 3,
		const VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		const std::set<VkImageUsageFlagBits>& image_usage_flags = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT });

	COPY_CTOR_DEL(Swapchain);
	Swapchain(Swapchain&& other);

	~Swapchain();

	COPY_OP_DEL(Swapchain);
	MOVE_OP_DEL(Swapchain);

public:
	inline bool is_valid() const { return handle != VK_NULL_HANDLE; }
	inline Device& get_device() { return device; }
	inline VkSwapchainKHR get_handle() const { return handle; }
	inline const VkExtent2D& get_extent() const { return properties.extent; }
	inline VkFormat get_format() const { return properties.surface_format.format; }
	inline const std::vector<VkImage>& get_images() const { return images; }
	inline VkSurfaceTransformFlagBitsKHR get_transform() const { return properties.pre_transform; }
	inline VkSurfaceKHR get_surface() const { return surface; }
	inline VkImageUsageFlags get_usage() const { return properties.image_usage; }
	inline VkPresentModeKHR get_present_mode() const { return properties.present_mode; }

	VkResult AcquireNextImage(u32& imageIndex, VkSemaphore imageAcquiredSemaphore, VkFence fence = VK_NULL_HANDLE) const;

private:
	Device& device;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR handle = VK_NULL_HANDLE;
	std::vector<VkImage> images;
	std::vector<VkSurfaceFormatKHR> surface_formats;
	std::vector<VkPresentModeKHR> present_modes;
	SwapchainProperties properties;
	std::vector<VkPresentModeKHR> present_mode_priority_list = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR };
	std::vector<VkSurfaceFormatKHR> surface_format_priority_list = { {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} };
	std::set<VkImageUsageFlagBits> image_usage_flags;
};
}	// namespace vge
