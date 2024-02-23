#include "ImageView.h"
#include "Device.h"

vge::ImageView::ImageView(
	Image& image, VkImageViewType viewType, VkFormat format /*= VK_FORMAT_UNDEFINED*/,
	u32 baseMipLevel /*= 0*/, u32 baseArrayLayer /*= 0*/, u32 nMipLevels /*= 0*/, u32 nArrayLayers /*= 0*/)
	: VulkanResource(VK_NULL_HANDLE, &image.GetDevice()), _Image(&image), _Format(format)
{
	if (format == VK_FORMAT_UNDEFINED)
	{
		this->_Format = format = _Image->GetFormat();
	}

	_SubresourceRange.baseMipLevel = baseMipLevel;
	_SubresourceRange.baseArrayLayer = baseArrayLayer;
	_SubresourceRange.levelCount = nMipLevels == 0 ? _Image->GetSubresource().mipLevel : nMipLevels;
	_SubresourceRange.layerCount = nArrayLayers == 0 ? _Image->GetSubresource().arrayLayer : nArrayLayers;

	if (IsDepthFormat(format))
	{
		_SubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else
	{
		_SubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.image = _Image->GetHandle();
	viewInfo.viewType = viewType;
	viewInfo.format = format;
	viewInfo.subresourceRange = _SubresourceRange;

	VK_ENSURE(vkCreateImageView(_Device->GetHandle(), &viewInfo, nullptr, &_Handle));

	// Register this image view to its image
	// in order to be notified when it gets moved
	_Image->GetViews().emplace(this);
}

vge::ImageView::ImageView(ImageView&& other) 
	: VulkanResource(std::move(other)), _Image(other._Image), _Format(other._Format), _SubresourceRange(other._SubresourceRange)
{
	// Remove old view from image set and add this new one
	auto& views = _Image->GetViews();
	views.erase(&other);
	views.emplace(this);

	other._Handle = VK_NULL_HANDLE;
}

vge::ImageView::~ImageView()
{
	if (_Handle)
	{
		vkDestroyImageView(_Device->GetHandle(), _Handle, nullptr);
	}
}

VkImageSubresourceLayers vge::ImageView::GetSubresourceLayers() const
{
	VkImageSubresourceLayers subresource{};
	subresource.aspectMask = _SubresourceRange.aspectMask;
	subresource.baseArrayLayer = _SubresourceRange.baseArrayLayer;
	subresource.layerCount = _SubresourceRange.layerCount;
	subresource.mipLevel = _SubresourceRange.baseMipLevel;

	return subresource;
}
