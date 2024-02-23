#include "RenderTarget.h"
#include "Device.h"

namespace vge
{
namespace
{
struct CompareExtent2D
{
	bool operator()(const VkExtent2D& lhs, const VkExtent2D& rhs) const
	{
		return !(lhs.width == rhs.width && lhs.height == rhs.height) && (lhs.width < rhs.width && lhs.height < rhs.height);
	}
};
}	// namespace
}	// namespace vge

const vge::RenderTarget::CreateFunc vge::RenderTarget::DefaultCreateFunction = [](Image&& swapchainImage) -> std::unique_ptr<RenderTarget> 
{
	VkFormat depthFormat = GetSuitableDepthFormat(swapchainImage.GetDevice().GetGpu().GetHandle());

	Image depthImage = Image(swapchainImage.GetDevice(), swapchainImage.GetExtent(), depthFormat,
							VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
							VMA_MEMORY_USAGE_GPU_ONLY);

	std::vector<Image> images;
	images.push_back(std::move(swapchainImage));
	images.push_back(std::move(depthImage));

	return std::make_unique<RenderTarget>(std::move(images));
};

vge::RenderTarget::RenderTarget(std::vector<Image>&& images) 
	: _Device(images.back().GetDevice()), _Images(std::move(images))
{
	ASSERT_MSG(images.empty(), "Should specify at least 1 image.");

	std::set<VkExtent2D, CompareExtent2D> uniqueExtent;

	// Returns the image extent as a VkExtent2D structure from a VkExtent3D.
	auto getImageExtent2D = [](const Image& image) { return VkExtent2D{ image.GetExtent().width, image.GetExtent().height }; };

	// Constructs a set of unique image extents given a vector of images
	std::transform(_Images.begin(), _Images.end(), std::inserter(uniqueExtent, uniqueExtent.end()), getImageExtent2D);

	// Allow only one extent size for a render target
	ENSURE_MSG(uniqueExtent.size() == 1, "Extent size is not unique.");

	_Extent = *uniqueExtent.begin();

	for (auto& image : _Images)
	{
		ENSURE_MSG(image.GetType() == VK_IMAGE_TYPE_2D, "Image type is not 2D.")

		_Views.emplace_back(image, VK_IMAGE_VIEW_TYPE_2D);
		_Attachments.emplace_back(Attachment(image.GetFormat(), image.GetSampleCount(), image.GetUsage()));
	}
}

vge::RenderTarget::RenderTarget(std::vector<ImageView>&& imageViews) 
	: _Device(const_cast<Image&>(imageViews.back().GetImage()).GetDevice()), _Images(), _Views(std::move(imageViews))
{
	assert(!_Views.empty() && "Should specify at least 1 image view");

	std::set<VkExtent2D, CompareExtent2D> uniqueExtent;

	// Returns the extent of the base mip level pointed at by a view.
	auto getViewExtent = [](const ImageView& view) 
	{
		const VkExtent3D mip0Extent = view.GetImage().GetExtent();
		const u32 mipLevel = view.GetSubresourceRange().baseMipLevel;
		return VkExtent2D{ mip0Extent.width >> mipLevel, mip0Extent.height >> mipLevel };
	};

	// Constructs a set of unique image extents given a vector of image views,
	// allow only one extent size for a render target.
	std::transform(_Views.begin(), _Views.end(), std::inserter(uniqueExtent, uniqueExtent.end()), getViewExtent);

	ENSURE_MSG(uniqueExtent.size() == 1, "Extent size is not unique.");
	
	_Extent = *uniqueExtent.begin();

	for (auto& view : _Views)
	{
		const auto& image = view.GetImage();
		_Attachments.emplace_back(Attachment(image.GetFormat(), image.GetSampleCount(), image.GetUsage()));
	}
}
