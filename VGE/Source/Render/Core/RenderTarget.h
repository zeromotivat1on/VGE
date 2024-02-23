#pragma once

#include "VkCommon.h"
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
	Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage) : Format(format), Samples(samples), Usage(usage) {}

	VkFormat Format = VK_FORMAT_UNDEFINED;
	VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageUsageFlags Usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	VkImageLayout InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

/**
* RenderTarget contains three vectors for: Image, ImageView and Attachment.
* The first two are Vulkan images and corresponding image views respectively.
* Attachment (s) contain a description of the images, which has two main purposes:
* - RenderPass creation only needs a list of Attachment (s), not the actual images, so we keep
*   the minimum amount of information necessary.
* - Creation of a RenderTarget becomes simpler, because the caller can just ask for some
*   Attachment (s) without having to create the images.
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
	inline const VkExtent2D& GetExtent() const { return _Extent; }
	inline const std::vector<ImageView>& GetViews() const { return _Views; }
	inline const std::vector<Attachment>& GetAttachments() const { return _Attachments; }
	inline const std::vector<u32>& GetInputAttachments() const { return _InputAttachments; }
	inline const std::vector<u32>& GetOutputAttachments() const { return _OutputAttachments; }
	inline VkImageLayout GetLayout(u32 attachment) const { return _Attachments[attachment].InitialLayout;; }

	// Sets the current input attachments overwriting the current ones.
	// Should be set before beginning the render pass and before starting a new subpass.
	inline void SetInputAttachments(std::vector<u32>& input) { _InputAttachments = input;}

	// Sets the current output attachments overwriting the current ones.
	// Should be set before beginning the render pass and before starting a new subpass.
	inline void SetOutputAttachments(std::vector<u32>& output) { _OutputAttachments = output;}

	inline void SetLayout(u32 attachment, VkImageLayout layout) { _Attachments[attachment].InitialLayout = layout; }

private:
	const Device& _Device;
	VkExtent2D _Extent;
	std::vector<Image> _Images;
	std::vector<ImageView> _Views;
	std::vector<Attachment> _Attachments;
	std::vector<u32> _InputAttachments = {}; // by default there are no input attachments
	std::vector<u32> _OutputAttachments = { 0 }; // by default the output attachments is attachment 0
};
}	// namespace vge
