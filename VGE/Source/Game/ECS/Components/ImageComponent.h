#pragma once

#include <memory>
#include <string>
#include <vector>

#include <volk.h>

#include "Types.h"
#include "Core/Image.h"
#include "Core/ImageView.h"

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

struct ImageComponent
{
	u32 Layers = 1;
	VkFormat Format = VK_FORMAT_UNDEFINED;
	std::string ImagePath;
	std::vector<u8> Data;
	std::vector<Mipmap> Mipmaps;
	std::vector<std::vector<VkDeviceSize>> Offsets; // stored like - Offsets[ArrayLayer][MipmapLayer]
	std::unique_ptr<Image> VkImage;
	std::unique_ptr<ImageView> VkImageView;
	ImageContentType ContentType = ImageContentType::Unknown;
};

ImageComponent CreateFromPath(const std::string& uri, ImageContentType);
void GenerateMipmaps(ImageComponent&);
void CreateVkImage(ImageComponent&, const Device&, VkImageViewType = VK_IMAGE_VIEW_TYPE_2D, VkImageCreateFlags = 0);
const VkExtent3D& GetExtent(const ImageComponent&);
	
// struct ImageComponent
// {
// public:
// 	// Type of content held in image. This helps to steer the image loaders when deciding what the format should be.
// 	// Some image containers don't know whether the data they contain is sRGB or not.
// 	// Since most applications save color images in sRGB, knowing that an image contains color data helps us to better guess its format when unknown.
// 	enum ContentType
// 	{
// 		Unknown,
// 		Color,
// 		Other
// 	};
//
// 	static std::unique_ptr<ImageComponent> CreateFromPath(const std::string& uri, ContentType);
//
// public:
// 	ImageComponent(const std::string& imagePath, std::vector<u8>&& data = {}, std::vector<Mipmap>&& mipmaps = {{}});
// 	virtual ~ImageComponent() = default;
// 	
// public:
// 	inline u32 GetLayers() const { return _Layers; }
// 	inline VkFormat GetFormat() const { return _Format; }
// 	inline const std::vector<u8>& GetData() const { return _Data; }
// 	inline const VkExtent3D& GetExtent() const { ASSERT(!_Mipmaps.empty()); return _Mipmaps[0].Extent; }
// 	inline const std::vector<Mipmap>& GetMipmaps() const { return _Mipmaps; }
// 	inline const std::vector<std::vector<VkDeviceSize>>& GetOffsets() const { return _Offsets; }
// 	inline const Image& GetVkImage() const { ASSERT(_VkImage); return *_VkImage; }
// 	inline const ImageView& GetVkImageView() const { ASSERT(_VkImageView); return *_VkImageView; }
//
// 	inline void ClearData() { _Data.clear(); _Data.shrink_to_fit(); }
//
// 	void GenerateMipmaps();
// 	void CreateVkImage(const Device&, VkImageViewType = VK_IMAGE_VIEW_TYPE_2D, VkImageCreateFlags = 0);
// 	void coerce_format_to_srgb();
//
// protected:
// 	std::vector<u8> &get_mut_data();
//
// 	void set_data(const u8* rawData, size_t size);
//  
// 	void set_format(VkFormat format);
//
// 	void set_width(u32 width);
//
// 	void set_height(u32 height);
//
// 	void set_depth(u32 depth);
//
// 	void set_layers(u32 layers);
//
// 	void set_offsets(const std::vector<std::vector<VkDeviceSize>>& offsets);
//
// 	Mipmap& get_mipmap(size_t index);
//
// 	std::vector<Mipmap>& get_mut_mipmaps();
//
// private:
// 	std::vector<u8> _Data;
// 	VkFormat _Format = VK_FORMAT_UNDEFINED;
// 	u32 _Layers = 1;
// 	std::vector<Mipmap> _Mipmaps = {{}};
// 	std::vector<std::vector<VkDeviceSize>> _Offsets; // stored like offsets[arrayLayer][mipmapLayer]
// 	std::unique_ptr<Image> _VkImage;
// 	std::unique_ptr<ImageView> _VkImageView;
// };
}	// namespace vge
