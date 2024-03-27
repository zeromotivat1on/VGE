#pragma once

#include <memory>
#include <string>
#include <vector>

#include <volk.h>

#include "Types.h"
#include "Core/Image.h"
#include "Core/ImageView.h"
#include "ECS/Component.h"

namespace vge
{
// Check whether the vulkan format is ASTC.
bool IsAstc(VkFormat format);

enum class ImageContentType : u8
{
	Unknown,
	Color,
	Other
};
	
struct Mipmap
{
	u32 Level = 0; // mipmap level
	u32 Offset = 0; // byte offset used for uploading
	VkExtent3D Extent = {0, 0, 0}; // width depth and height of the mipmap
};

struct ImageComponent : public Component
{
public:
	static ImageComponent CreateFromPath(const std::string& uri, ImageContentType);

public:
	u32 Layers = 1;
	VkFormat Format = VK_FORMAT_UNDEFINED;
	std::string ImagePath;
	std::vector<u8> Data;
	std::vector<Mipmap> Mipmaps;
	std::vector<std::vector<VkDeviceSize>> Offsets; // stored like - Offsets[ArrayLayer][MipmapLayer]
	std::unique_ptr<Image> VkImage;
	std::unique_ptr<ImageView> VkImageView;
	ImageContentType ContentType = ImageContentType::Unknown;

public:
	inline const VkExtent3D& GetExtent() const { ASSERT(!Mipmaps.empty()); return Mipmaps[0].Extent; }
	
	void GenerateMipmaps();
	void CreateVkImage(const Device&, VkImageViewType = VK_IMAGE_VIEW_TYPE_2D, VkImageCreateFlags = 0);
};
}	// namespace vge
