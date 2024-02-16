#include "Image.h"
#include "Device.h"
#include "ImageView.h"

namespace vge
{
namespace
{
inline VkImageType find_image_type(VkExtent3D extent)
{
	VkImageType result;

	uint32_t dimNum = 0;

	if (extent.width >= 1)
	{
		dimNum++;
	}

	if (extent.height >= 1)
	{
		dimNum++;
	}

	if (extent.depth > 1)
	{
		dimNum++;
	}

	switch (dimNum)
	{
	case 1:
		result = VK_IMAGE_TYPE_1D;
		break;
	case 2:
		result = VK_IMAGE_TYPE_2D;
		break;
	case 3:
		result = VK_IMAGE_TYPE_3D;
		break;
	default:
		ENSURE_MSG(false, "No image type found.");
		break;
	}

	return result;
}
}	// namespace
}	// namespace vge

vge::Image::Image(
	const Device& device, const VkExtent3D& extent, VkFormat format,
	VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage,
	VkSampleCountFlagBits sampleCount /*= VK_SAMPLE_COUNT_1_BIT*/, u32 mipLevels /*= 1*/, u32 arrayLayers /*= 1*/,
	VkImageTiling tiling /*= VK_IMAGE_TILING_OPTIMAL*/, VkImageCreateFlags flags /*= 0*/,
	u32 numQueueFamilies /*= 0*/, const u32* queueFamilies /*= nullptr*/)
	: VulkanResource(VK_NULL_HANDLE, &device), _Type(find_image_type(extent)),
	_Extent(extent), _Format(format), _SampleCount(_SampleCount),
	_Usage(imageUsage), _ArrayLayerCount(arrayLayers), _Tiling(tiling)
{
	ASSERT_MSG(mipLevels > 0, "Image should have at least one level.");
	ASSERT_MSG(arrayLayers > 0, "Image should have at least one layer.");

	_Subresource.mipLevel = mipLevels;
	_Subresource.arrayLayer = arrayLayers;

	VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.flags = flags;
	imageInfo.imageType = _Type;
	imageInfo.format = format;
	imageInfo.extent = extent;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.samples = _SampleCount;
	imageInfo.tiling = tiling;
	imageInfo.usage = imageUsage;

	if (numQueueFamilies != 0)
	{
		imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		imageInfo.queueFamilyIndexCount = numQueueFamilies;
		imageInfo.pQueueFamilyIndices = queueFamilies;
	}

	VmaAllocationCreateInfo memoryInfo = {};
	memoryInfo.usage = memoryUsage;

	if (imageUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
	{
		memoryInfo.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
	}

	VK_ENSURE(vmaCreateImage(device.GetMemoryAllocator(), &imageInfo, &memoryInfo, &_Handle, &_Memory, nullptr));
}

vge::Image::Image(
	const Device& device, VkImage handle, const VkExtent3D& extent,
	VkFormat format, VkImageUsageFlags image_usage, VkSampleCountFlagBits sample_count) 
	: VulkanResource(handle, &device), _Type(find_image_type(extent)), _Extent(extent),
	_Format(format), _SampleCount(sample_count), _Usage(image_usage)
{
	_Subresource.mipLevel = 1;
	_Subresource.arrayLayer = 1;
}

vge::Image::Image(Image&& other) 
	: VulkanResource(std::move(other)), _Memory(other._Memory), _Type(other._Type),
	_Extent(other._Extent), _Format(other._Format), _SampleCount(other._SampleCount),
	_Usage(other._Usage), _Tiling(other._Tiling), _Subresource(other._Subresource),
	_Views(std::exchange(other._Views, {})), _MappedData(other._MappedData), _Mapped(other._Mapped)
{
	other._Memory = VK_NULL_HANDLE;
	other._MappedData = nullptr;
	other._Mapped = false;

	// Update image views references to this image to avoid dangling pointers.
	for (auto& view : _Views)
	{
		view->SetImage(*this);
	}
}

vge::Image::~Image()
{
	if (_Handle && _Memory)
	{
		Unmap();
		vmaDestroyImage(_Device->GetMemoryAllocator(), _Handle, _Memory);
	}
}

uint8_t* vge::Image::Map()
{
	if (!_MappedData && !_Mapped)
	{
		CLOG(_Tiling != VK_IMAGE_TILING_LINEAR, Warning, "Mapping image memory that is not linear.");
		VK_ENSURE(vmaMapMemory(_Device->GetMemoryAllocator(), _Memory, reinterpret_cast<void**>(&_MappedData)));
		_Mapped = true;
	}
	return _MappedData;
}

void vge::Image::Unmap()
{
	if (_MappedData && _Mapped)
	{
		vmaUnmapMemory(_Device->GetMemoryAllocator(), _Memory);
		_MappedData = nullptr;
		_Mapped = false;
	}
}