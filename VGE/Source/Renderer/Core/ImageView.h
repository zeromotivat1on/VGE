#pragma once

#include "Core/VulkanResource.h"
#include "Core/Image.h"

namespace vge
{
class ImageView : public VulkanResource<VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW, const Device>
{
public:
	ImageView(
		Image& image, VkImageViewType viewType, VkFormat format = VK_FORMAT_UNDEFINED,
		u32 baseMipLevel = 0, u32 baseArrayLayer = 0, u32 nMipLevels = 0, u32 nArrayLayers = 0);

	COPY_CTOR_DEL(ImageView);
	ImageView(ImageView&& other);

	~ImageView() override;

	COPY_OP_DEL(ImageView);
	MOVE_OP_DEL(ImageView);

public:
	inline const Image& GetImage() const { ASSERT(_Image); return *_Image; }
	inline VkFormat GetFormat() const { return _Format; }
	inline VkImageSubresourceRange GetSubresourceRange() const { return _SubresourceRange; }

	inline void SetImage(Image& image) { _Image = &image; }

	VkImageSubresourceLayers GetSubresourceLayers() const;

private:
	Image* _Image = nullptr;
	VkFormat _Format = {};
	VkImageSubresourceRange _SubresourceRange = {};
};
}	// namespace vge
