#pragma once

#include "Common.h"

namespace vge
{
	class Device;

	struct ImageCreateInfo
	{
		const Device* Device = nullptr;
		VkExtent2D Extent = {};
		VkFormat Format = VkFormat::VK_FORMAT_UNDEFINED;
		VkImageTiling Tiling = VkImageTiling::VK_IMAGE_TILING_MAX_ENUM;
		VkImageUsageFlags Usage = 0;
		VmaMemoryUsage MemAllocUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN;
	};

	struct Image
	{
	public:
		static Image Create(const ImageCreateInfo& data);
		static Image CreateForTexture(const Device* device, const char* filename);

	public:
		Image() = default;
		void Destroy();

	public:
		VkImage Handle = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo AllocInfo = {};

	private:
		VmaAllocator m_Allocator = VK_NULL_HANDLE;
	};
}
