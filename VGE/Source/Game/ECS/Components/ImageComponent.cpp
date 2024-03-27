#include "ImageComponent.h"
#include "Core/Error.h"
#include "Platform/FileSystem.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

//#include "scene_graph/components/image/astc.h"
//#include "scene_graph/components/image/ktx.h"
//#include "scene_graph/components/image/stb.h"

namespace vge
{
bool IsAstc(const VkFormat format)
{
	return (format == VK_FORMAT_ASTC_4x4_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_4x4_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_5x4_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_5x4_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_5x5_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_5x5_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_6x5_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_6x5_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_6x6_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_6x6_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_8x5_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_8x5_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_8x6_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_8x6_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_8x8_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_8x8_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_10x5_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_10x5_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_10x6_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_10x6_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_10x8_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_10x8_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_10x10_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_10x10_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_12x10_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_12x10_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_12x12_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_12x12_SRGB_BLOCK);
}

// When the color-space of a loaded image is unknown (from KTX1 for example) we
// may want to assume that the loaded data is in sRGB format (since it usually is).
// In those cases, this helper will get called which will force an existing unorm
// format to become an srgb format where one exists. If none exist, the format will
// remain unmodified.
static VkFormat MaybeCoerceToSrgb(VkFormat fmt)
{
	switch (fmt)
	{
		case VK_FORMAT_R8_UNORM:
			return VK_FORMAT_R8_SRGB;
		case VK_FORMAT_R8G8_UNORM:
			return VK_FORMAT_R8G8_SRGB;
		case VK_FORMAT_R8G8B8_UNORM:
			return VK_FORMAT_R8G8B8_SRGB;
		case VK_FORMAT_B8G8R8_UNORM:
			return VK_FORMAT_B8G8R8_SRGB;
		case VK_FORMAT_R8G8B8A8_UNORM:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case VK_FORMAT_B8G8R8A8_UNORM:
			return VK_FORMAT_B8G8R8A8_SRGB;
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
			return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
			return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
			return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		case VK_FORMAT_BC2_UNORM_BLOCK:
			return VK_FORMAT_BC2_SRGB_BLOCK;
		case VK_FORMAT_BC3_UNORM_BLOCK:
			return VK_FORMAT_BC3_SRGB_BLOCK;
		case VK_FORMAT_BC7_UNORM_BLOCK:
			return VK_FORMAT_BC7_SRGB_BLOCK;
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
			return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
		case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
			return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
		case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
			return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
		case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
			return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
		case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
			return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
		case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
			return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
		case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
			return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
		case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
			return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
		case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
			return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
		case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
			return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
		case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
			return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
		case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
			return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
		case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
			return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
		case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
			return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
		case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
			return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
		case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
			return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
		case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
			return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
		case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
			return VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;
		case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
			return VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;
		case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
			return VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;
		case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
			return VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;
		default:
			return fmt;
	}
}
}	// namespace vge

vge::ImageComponent vge::ImageComponent::CreateFromPath(const std::string& uri, ImageContentType contentType)
{
	const auto extension = fs::GetExtension(uri);

	ImageComponent image = {};

	if (extension == "png" || extension == "jpg")
	{
		image.Data = fs::ReadAsset(uri);
		image.ImagePath = uri;
		image.ContentType = contentType;
	}

	LOG(Warning, "Can't load image of unsupported type '%s', returning empty.", extension.c_str());
	
	return image;
}

void vge::ImageComponent::GenerateMipmaps()
{
	ASSERT_MSG(Mipmaps.size() > 1, "Mipmaps already generated.");

	constexpr i32 channels = 4;
	const auto extent = GetExtent();
	u32 nextWidth  = std::max<u32>(1u, extent.width / 2);
	u32 nextHeight = std::max<u32>(1u, extent.height / 2);
	u32 nextSize = nextWidth * nextHeight * channels;

	while (true)
	{
		// Make space for next mipmap.
		const u32 oldSize = ToU32(Data.size());
		Data.resize(oldSize + nextSize);

		const auto& prevMipmap = Mipmaps.back();

		// Update mipmaps.
		Mipmap nextMipmap = {};
		nextMipmap.Level = prevMipmap.Level + 1;
		nextMipmap.Offset = oldSize;
		nextMipmap.Extent = { nextWidth, nextHeight, 1u };

		// Fill next mipmap memory.
		stbir_resize_uint8(Data.data() + prevMipmap.Offset, prevMipmap.Extent.width, prevMipmap.Extent.height, 0,
		                   Data.data() + nextMipmap.Offset, nextMipmap.Extent.width, nextMipmap.Extent.height, 0, channels);

		Mipmaps.emplace_back(nextMipmap);

		// Next mipmap values.
		nextWidth = std::max<u32>(1u, nextWidth / 2);
		nextHeight = std::max<u32>(1u, nextHeight / 2);
		nextSize = nextWidth * nextHeight * channels;

		if (nextWidth == 1 && nextHeight == 1)
		{
			break;
		}
	}
}

void vge::ImageComponent::CreateVkImage(const Device& device, VkImageViewType imageViewType, VkImageCreateFlags flags)
{
	ASSERT_MSG(!VkImage && !VkImageView, "Vulkan image already constructed");

	VkImage = std::make_unique<Image>(device, GetExtent(), Format,
										VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
										VMA_MEMORY_USAGE_GPU_ONLY, VK_SAMPLE_COUNT_1_BIT,
										ToU32(Mipmaps.size()), Layers, VK_IMAGE_TILING_OPTIMAL, flags);

	VkImageView = std::make_unique<ImageView>(*VkImage, imageViewType);
}