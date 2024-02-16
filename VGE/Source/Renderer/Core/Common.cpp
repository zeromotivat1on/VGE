#include "Common.h"

namespace vge
{
VkAccessFlags GetAccessFlags(VkImageLayout layout)
{
	switch (layout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		return 0;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		return VK_ACCESS_HOST_WRITE_BIT;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
		return VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VK_ACCESS_TRANSFER_READ_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		return VK_ACCESS_TRANSFER_WRITE_BIT;
	case VK_IMAGE_LAYOUT_GENERAL:
		assert(false && "Don't know how to get a meaningful VkAccessFlags for VK_IMAGE_LAYOUT_GENERAL! Don't use it!");
		return 0;
	default:
		assert(false);
		return 0;
	}
}

VkPipelineStageFlags GetPipelineStageFlags(VkImageLayout layout)
{
	switch (layout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		return VK_PIPELINE_STAGE_HOST_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
		return VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	case VK_IMAGE_LAYOUT_GENERAL:
		ASSERT_MSG(false, "Don't know how to get a meaningful VkPipelineStageFlags for VK_IMAGE_LAYOUT_GENERAL, don't use it.");
		return 0;
	default:
		ASSERT_MSG(false, "Unsupported layout.");
		return 0;
	}
}

void ImageLayoutTransition(
	VkCommandBuffer commandBuffer, VkImage image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
	VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& subresourceRange)
{
	VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	imageMemoryBarrier.srcAccessMask = srcAccessMask;
	imageMemoryBarrier.dstAccessMask = dstAccessMask;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// Put barrier inside setup command buffer.
	vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void ImageLayoutTransition(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& subresourceRange)
{
	VkPipelineStageFlags srcStageMask = GetPipelineStageFlags(oldLayout);
	VkPipelineStageFlags dstStageMask = GetPipelineStageFlags(newLayout);
	VkAccessFlags srcAccessMask = GetAccessFlags(oldLayout);
	VkAccessFlags dstAccessMask = GetAccessFlags(newLayout);

	ImageLayoutTransition(commandBuffer, image, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask, oldLayout, newLayout, subresourceRange);
}

void ImageLayoutTransition(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	ImageLayoutTransition(commandBuffer, image, oldLayout, newLayout, subresourceRange);
}

void ImageLayoutTransition(VkCommandBuffer commandBuffer, const std::vector<std::pair<VkImage, VkImageSubresourceRange>>& imagesAndRanges, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkPipelineStageFlags srcStageMask = GetPipelineStageFlags(oldLayout);
	VkPipelineStageFlags dstStageMask = GetPipelineStageFlags(newLayout);
	VkAccessFlags srcAccessMask = GetAccessFlags(oldLayout);
	VkAccessFlags dstAccessMask = GetAccessFlags(newLayout);

	// Create image barrier objects
	std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
	imageMemoryBarriers.reserve(imagesAndRanges.size());

	for (size_t i = 0; i < imagesAndRanges.size(); i++)
	{
		imageMemoryBarriers.emplace_back(VkImageMemoryBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
																nullptr,
																srcAccessMask,
																dstAccessMask,
																oldLayout,
																newLayout,
																VK_QUEUE_FAMILY_IGNORED,
																VK_QUEUE_FAMILY_IGNORED,
																imagesAndRanges[i].first,
																imagesAndRanges[i].second });
	}

	// Put barriers inside setup command buffer.
	vkCmdPipelineBarrier(commandBuffer,
		srcStageMask,
		dstStageMask,
		0,
		0,
		nullptr,
		0,
		nullptr,
		ToU32(imageMemoryBarriers.size()),
		imageMemoryBarriers.data());
}
}	// namespace vge