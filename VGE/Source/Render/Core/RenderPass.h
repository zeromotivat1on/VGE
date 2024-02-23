#pragma once

#include "Core/VulkanResource.h"

namespace vge
{
struct Attachment;
class Device;

struct SubpassInfo
{
	std::vector<u32> InputAttachments;
	std::vector<u32> OutputAttachments;
	std::vector<u32> ColorResolveAttachments;
	bool DisableDepthStencilAttachment;
	u32 DepthStencilResolveAttachment;
	VkResolveModeFlagBits DepthStencilResolveMode;
	std::string DebugName;
};

class RenderPass : public VulkanResource<VkRenderPass, VK_OBJECT_TYPE_RENDER_PASS>
{
public:
	RenderPass(
		Device& device,
		const std::vector<Attachment>& attachments,
		const std::vector<LoadStoreInfo>& loadStoreInfos,
		const std::vector<SubpassInfo>& subpasses);

	COPY_CTOR_DEL(RenderPass);
	RenderPass(RenderPass&& other);

	~RenderPass();

	COPY_OP_DEL(RenderPass);
	MOVE_OP_DEL(RenderPass);

public:
	inline const u32 GetColorOutputCount(u32 subpassIndex) const { return _ColorOutputCount[subpassIndex]; }

	const VkExtent2D GetRenderAreaGranularity() const;

private:
	template <typename TSubpassDescription, typename TAttachmentDescription, typename TAttachmentReference, typename TSubpassDependency, typename TRenderPassCreateInfo>
	void CreateRenderpass(const std::vector<Attachment>& attachments, const std::vector<LoadStoreInfo>& loadStoreInfos, const std::vector<SubpassInfo>& subpasses);

private:
	size_t _SubpassCount;
	std::vector<u32> _ColorOutputCount;
};
}	// namespace vge

