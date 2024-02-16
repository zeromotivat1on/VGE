#pragma once

#include "Core/Common.h"
#include "Core/Image.h"
#include "Core/ImageView.h"

namespace vge
{
class Device;

// Description of render pass attachments.
// Attachment descriptions can be used to automatically create render target images.
struct Attachment
{
	Attachment() = default;
	Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage) : format(format), samples(samples), usage(usage) {}

	VkFormat format{ VK_FORMAT_UNDEFINED };
	VkSampleCountFlagBits samples{ VK_SAMPLE_COUNT_1_BIT };
	VkImageUsageFlags usage{ VK_IMAGE_USAGE_SAMPLED_BIT };
	VkImageLayout initial_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
};

/**
* RenderTarget contains three vectors for: Image, ImageView and Attachment.
* The first two are Vulkan images and corresponding image views respectively.
* Attachment (s) contain a description of the images, which has two main purposes:
* - RenderPass creation only needs a list of Attachment (s), not the actual images, so we keep
*   the minimum amount of information necessary
* - Creation of a RenderTarget becomes simpler, because the caller can just ask for some
*   Attachment (s) without having to create the images
*/
class RenderTarget
{
public:
	using CreateFunc = std::function<std::unique_ptr<RenderTarget>(Image&&)>;
	static const CreateFunc DefaultCreateFunction;

public:
	RenderTarget(std::vector<Image>&& images);
	RenderTarget(std::vector<ImageView>&& image_views);

	COPY_CTOR_DEL(RenderTarget);
	MOVE_CTOR_DEL(RenderTarget);

	COPY_OP_DEL(RenderTarget);
	MOVE_OP_DEL(RenderTarget);

public:
	inline const VkExtent2D& get_extent() const { return extent; }
	inline const std::vector<ImageView>& get_views() const { return views; }
	inline const std::vector<Attachment>& get_attachments() const { return attachments; }
	inline const std::vector<uint32_t>& get_input_attachments() const { return input_attachments; }
	inline const std::vector<uint32_t>& get_output_attachments() const { return output_attachments; }
	inline VkImageLayout get_layout(uint32_t attachment) const { return attachments[attachment].initial_layout;; }

	// Sets the current input attachments overwriting the current ones.
	// Should be set before beginning the render pass and before starting a new subpass.
	inline void set_input_attachments(std::vector<uint32_t>& input) { input_attachments = input;}

	// Sets the current output attachments overwriting the current ones.
	// Should be set before beginning the render pass and before starting a new subpass.
	inline void set_output_attachments(std::vector<uint32_t>& output) { output_attachments = output;}

	inline void set_layout(uint32_t attachment, VkImageLayout layout) { attachments[attachment].initial_layout = layout; }

private:
	Device const& device;
	VkExtent2D extent;
	std::vector<Image> images;
	std::vector<ImageView> views;
	std::vector<Attachment> attachments;
	std::vector<uint32_t> input_attachments = {}; // by default there are no input attachments
	std::vector<uint32_t> output_attachments = { 0 }; // by default the output attachments is attachment 0
};
}	// namespace vge
