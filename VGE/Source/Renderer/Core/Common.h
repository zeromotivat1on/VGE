#pragma once

#include <cstdio>
#include <vector>
#include <array>
#include <memory>
#include <string>
#include <map>
#include <unordered_map>
#include <functional>
#include <assert.h>

#include "vk_mem_alloc.h"
#include "volk.h"

#include "Types.h"
#include "Macros.h"
#include "Logging.h"
#include "Helpers.h"
#include "GlmCommon.h"
#include "StringHelpers.h"

#define VK_FLAGS_NONE 0

#define DEFAULT_FENCE_TIMEOUT 100000000000	// default fence timeout in nanoseconds

namespace vge
{
template <class T>
using ShaderStageMap = std::map<VkShaderStageFlagBits, T>;

template <class T>
using BindingMap = std::map<u32, std::map<u32, T>>;

struct LoadStoreInfo
{
	VkAttachmentLoadOp LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	VkAttachmentStoreOp StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
};

// Image memory barrier structure used to define memory access for an image view during command recording.
struct ImageMemoryBarrier
{
	VkPipelineStageFlags SrcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkPipelineStageFlags DstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	
	VkAccessFlags SrcAccessMask = 0;
	VkAccessFlags DstAccessMask = 0;

	VkImageLayout OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout NewLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	u32 OldQueueFamily = VK_QUEUE_FAMILY_IGNORED;
	u32 NewQueueFamily = VK_QUEUE_FAMILY_IGNORED;
};

// Buffer memory barrier structure used to define memory access for a buffer during command recording.
struct BufferMemoryBarrier
{
	VkPipelineStageFlags SrcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkPipelineStageFlags DstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	
	VkAccessFlags SrcAccessMask = 0;
	VkAccessFlags DstAccessMask = 0;
};

inline bool IsDepthOnlyFormat(VkFormat format) { return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT; }
inline bool IsDepthStencilFormat(VkFormat format) { return format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT; }
inline bool IsDepthFormat(VkFormat format) { return IsDepthOnlyFormat(format) || IsDepthStencilFormat(format); }

bool IsDynamicBufferDescriptorType(VkDescriptorType descriptor_type)
{ return descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC || descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; }

bool IsBufferDescriptorType(VkDescriptorType descriptor_type) 
{ return descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || IsDynamicBufferDescriptorType(descriptor_type); }

void ImageLayoutTransition(
	VkCommandBuffer commandBuffer, VkImage image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, 
	VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& subresourceRange);
void ImageLayoutTransition(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& subresourceRange);
void ImageLayoutTransition(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
void ImageLayoutTransition(VkCommandBuffer commandBuffer, const std::vector<std::pair<VkImage, VkImageSubresourceRange>>& imagesAndRanges, VkImageLayout oldLayout, VkImageLayout newLayout);
}	// namespace vge
