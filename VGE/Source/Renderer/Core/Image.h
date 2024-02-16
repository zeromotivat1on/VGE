#pragma once

#include "Core/VulkanResource.h"
#include <unordered_set>

namespace vge
{
class Device;
class ImageView;

class Image : public VulkanResource<VkImage, VK_OBJECT_TYPE_IMAGE, const Device>
{
public:
	Image(
		const Device& device, VkImage handle, const VkExtent3D& extent,
		VkFormat format, VkImageUsageFlags imageUsage,
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

	Image(
		const Device& device, const VkExtent3D& extent, VkFormat format,
		VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage,
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, u32 mipLevels = 1, u32 arrayLayers = 1,
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageCreateFlags flags = 0,
		u32 numQueueFamilies = 0, const u32* queueFamilies = nullptr);

	COPY_CTOR_DEL(Image);
	Image(Image&& other);

	~Image() override;

	COPY_OP_DEL(Image);
	MOVE_OP_DEL(Image);

public:
	inline VmaAllocation GetMemory() const { return _Memory; }
	inline VkImageType GetType() const { return _Type; }
	inline const VkExtent3D& GetExtent() const { return _Extent; }
	inline VkFormat GetFormat() const { return _Format; }
	inline VkSampleCountFlagBits GetSampleCount() const { return _SampleCount; }
	inline VkImageUsageFlags GetUsage() const { return _Usage; }
	inline VkImageTiling GetTiling() const { return _Tiling; }
	inline VkImageSubresource GetSubresource() const { return _Subresource; }
	inline u32 GetArrayLayerCount() const { return _ArrayLayerCount; }
	inline std::unordered_set<ImageView*>& GetViews() { return _Views; }

	u8* Map(); // maps vulkan memory to an host visible address
	void Unmap(); // unmaps vulkan memory from the host visible address

private:
	VmaAllocation _Memory = VK_NULL_HANDLE;
	VkImageType _Type;
	VkExtent3D _Extent;
	VkFormat _Format;
	VkImageUsageFlags _Usage;
	VkSampleCountFlagBits _SampleCount;
	VkImageTiling _Tiling;
	VkImageSubresource _Subresource;
	u32 _ArrayLayerCount = 0;
	std::unordered_set<ImageView*> _Views; // image views referring to this image
	u8* _MappedData = nullptr;
	bool _Mapped = false; // whether it was mapped with vmaMapMemory
};
}	// namespace vge
