#include "RenderTarget.h"

namespace vkb
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
}	// namespace vkb

const vge::RenderTarget::CreateFunc vge::RenderTarget::DefaultCreateFunction = [](Image&& swapchain_image) -> std::unique_ptr<RenderTarget> 
{
	VkFormat depth_format = get_suitable_depth_format(swapchain_image.get_device().get_gpu().get_handle());

	Image depth_image{ swapchain_image.get_device(), swapchain_image.get_extent(),
							depth_format,
							VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
							VMA_MEMORY_USAGE_GPU_ONLY };

	std::vector<Image> images;
	images.push_back(std::move(swapchain_image));
	images.push_back(std::move(depth_image));

	return std::make_unique<RenderTarget>(std::move(images));
};

vge::RenderTarget::RenderTarget(std::vector<Image>&& images) 
	: device(images.back().get_device()), images(std::move(images))
{
	ASSERT_MSG(images.empty(), "Should specify at least 1 image.");

	std::set<VkExtent2D, CompareExtent2D> unique_extent;

	// Returns the image extent as a VkExtent2D structure from a VkExtent3D
	auto get_image_extent = [](const Image& image) { return VkExtent2D{ image.get_extent().width, image.get_extent().height }; };

	// Constructs a set of unique image extents given a vector of images
	std::transform(this->images.begin(), this->images.end(), std::inserter(unique_extent, unique_extent.end()), get_image_extent);

	// Allow only one extent size for a render target
	if (unique_extent.size() != 1)
	{
		throw VulkanException{ VK_ERROR_INITIALIZATION_FAILED, "Extent size is not unique" };
	}

	extent = *unique_extent.begin();

	for (auto& image : this->images)
	{
		if (image.get_type() != VK_IMAGE_TYPE_2D)
		{
			throw VulkanException{ VK_ERROR_INITIALIZATION_FAILED, "Image type is not 2D" };
		}

		views.emplace_back(image, VK_IMAGE_VIEW_TYPE_2D);

		attachments.emplace_back(Attachment{ image.get_format(), image.get_sample_count(), image.get_usage() });
	}
}

vge::RenderTarget::RenderTarget(std::vector<ImageView>&& image_views) :
	device{ const_cast<Image&>(image_views.back().get_image()).get_device() },
	images{},
	views{ std::move(image_views) }
{
	assert(!views.empty() && "Should specify at least 1 image view");

	std::set<VkExtent2D, CompareExtent2D> unique_extent;

	// Returns the extent of the base mip level pointed at by a view
	auto get_view_extent = [](const ImageView& view) {
		const VkExtent3D mip0_extent = view.get_image().get_extent();
		const uint32_t   mip_level = view.get_subresource_range().baseMipLevel;
		return VkExtent2D{ mip0_extent.width >> mip_level, mip0_extent.height >> mip_level };
	};

	// Constructs a set of unique image extents given a vector of image views;
	// allow only one extent size for a render target
	std::transform(views.begin(), views.end(), std::inserter(unique_extent, unique_extent.end()), get_view_extent);
	if (unique_extent.size() != 1)
	{
		throw VulkanException{ VK_ERROR_INITIALIZATION_FAILED, "Extent size is not unique" };
	}
	extent = *unique_extent.begin();

	for (auto& view : views)
	{
		const auto& image = view.get_image();
		attachments.emplace_back(Attachment{ image.get_format(), image.get_sample_count(), image.get_usage() });
	}
}
