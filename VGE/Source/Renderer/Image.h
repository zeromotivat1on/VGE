#pragma once

#include "Common.h"
#include "RenderCommon.h"

namespace vge
{
	class Device;

	struct ImageCreateInfo
	{
		const vge::Device* Device = nullptr;
		VkExtent2D Extent = {};
		VkFormat Format = VkFormat::VK_FORMAT_UNDEFINED;
		VkImageTiling Tiling = VkImageTiling::VK_IMAGE_TILING_MAX_ENUM;
		VkImageUsageFlags Usage = 0;
		VmaMemoryUsage MemAllocUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN;
	};

	struct ImageViewCreateInfo
	{
		const vge::Device* Device = nullptr;
		VkImage Image = VK_NULL_HANDLE;
		VkFormat Format = VK_FORMAT_UNDEFINED; 
		VkImageAspectFlagBits AspectFlags = VK_IMAGE_ASPECT_NONE;
	};

	class Image
	{
	public:
		static Image Create(const ImageCreateInfo& data);
		static Image CreateForTexture(const Device* device, const c8* filename);
		static VkImageView CreateView(const ImageViewCreateInfo& data);
		static VkFormat GetBestFormat(const Device* device, const std::vector<VkFormat>& formats, VkFormatFeatureFlags features, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL);

	public:
		Image() = default;
		void Destroy();

		inline VkImage GetHandle() const { return m_Handle; }

	private:
		VkImage m_Handle = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo m_AllocInfo = {};
		VmaAllocator m_Allocator = VK_NULL_HANDLE;
	};
}
