#include "RenderPass.h"
#include "Device.h"

namespace vge
{
namespace
{
inline void SetStructureType(VkAttachmentDescription& attachment)
{
	// VkAttachmentDescription has no sType field.
}

inline void SetStructureType(VkAttachmentDescription2KHR& attachment)
{
	attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
}

inline void SetStructureType(VkAttachmentReference& reference)
{
	// VkAttachmentReference has no sType field.
}

inline void SetStructureType(VkAttachmentReference2KHR& reference)
{
	reference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
}

inline void SetStructureType(VkRenderPassCreateInfo& createInfo)
{
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
}

inline void SetStructureType(VkRenderPassCreateInfo2KHR& createInfo)
{
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR;
}

inline void SetStructureType(VkSubpassDescription& description)
{
	// VkSubpassDescription has no sType field.
}

inline void SetStructureType(VkSubpassDescription2KHR& description)
{
	description.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR;
}

inline void SetPointerNext(VkSubpassDescription& subpassDescription, VkSubpassDescriptionDepthStencilResolveKHR& depthResolve, VkAttachmentReference& depthResolveAttachment)
{
	// VkSubpassDescription cannot have pNext point to a VkSubpassDescriptionDepthStencilResolveKHR containing a VkAttachmentReference.
}

inline void SetPointerNext(VkSubpassDescription2KHR& subpassDescription, VkSubpassDescriptionDepthStencilResolveKHR& depthResolve, VkAttachmentReference2KHR& depthResolveAttachment)
{
	depthResolve.pDepthStencilResolveAttachment = &depthResolveAttachment;
	subpassDescription.pNext = &depthResolve;
}

inline const VkAttachmentReference2KHR* GetDepthResolveReference(const VkSubpassDescription& subpassDescription)
{
	// VkSubpassDescription cannot have pNext point to a VkSubpassDescriptionDepthStencilResolveKHR containing a VkAttachmentReference2KHR.
	return nullptr;
}

inline const VkAttachmentReference2KHR* GetDepthResolveReference(const VkSubpassDescription2KHR& subpassDescription)
{
	auto descriptionDepthResolve = static_cast<const VkSubpassDescriptionDepthStencilResolveKHR*>(subpassDescription.pNext);

	const VkAttachmentReference2KHR* depthResolveAttachment = nullptr;
	if (descriptionDepthResolve)
	{
		depthResolveAttachment = descriptionDepthResolve->pDepthStencilResolveAttachment;
	}

	return depthResolveAttachment;
}

inline VkResult CreateVkRenderpass(VkDevice device, VkRenderPassCreateInfo& createInfo, VkRenderPass* handle)
{
	return vkCreateRenderPass(device, &createInfo, nullptr, handle);
}

inline VkResult CreateVkRenderpass(VkDevice device, VkRenderPassCreateInfo2KHR& createInfo, VkRenderPass* handle)
{
	return vkCreateRenderPass2KHR(device, &createInfo, nullptr, handle);
}
}	// namespace

template <typename T>
std::vector<T> GetAttachmentDescriptions(const std::vector<Attachment>& attachments, const std::vector<LoadStoreInfo>& loadStoreInfos)
{
	std::vector<T> attachmentDescriptions;

	for (size_t i = 0U; i < attachments.size(); ++i)
	{
		T attachment{};
		SetStructureType(attachment);

		attachment.format = attachments[i].Format;
		attachment.samples = attachments[i].Samples;
		attachment.initialLayout = attachments[i].InitialLayout;
		attachment.finalLayout = IsDepthFormat(attachment.format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		if (i < loadStoreInfos.size())
		{
			attachment.loadOp = loadStoreInfos[i].LoadOp;
			attachment.storeOp = loadStoreInfos[i].StoreOp;
			attachment.stencilLoadOp = loadStoreInfos[i].LoadOp;
			attachment.stencilStoreOp = loadStoreInfos[i].StoreOp;
		}

		attachmentDescriptions.push_back(std::move(attachment));
	}

	return attachmentDescriptions;
}

template <typename TSubpassDescription, typename TAttachmentDescription, typename TAttachmentReference>
void SetAttachmentLayouts(std::vector<TSubpassDescription>& subpassDescriptions, std::vector<TAttachmentDescription>& attachmentDescriptions)
{
	// Make the initial layout same as in the first subpass using that attachment.
	for (auto& subpass : subpassDescriptions)
	{
		for (size_t k = 0U; k < subpass.colorAttachmentCount; ++k)
		{
			auto& reference = subpass.pColorAttachments[k];
			// Set it only if not defined yet.
			if (attachmentDescriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
			{
				attachmentDescriptions[reference.attachment].initialLayout = reference.layout;
			}
		}

		for (size_t k = 0U; k < subpass.inputAttachmentCount; ++k)
		{
			auto& reference = subpass.pInputAttachments[k];
			// Set it only if not defined yet.
			if (attachmentDescriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
			{
				attachmentDescriptions[reference.attachment].initialLayout = reference.layout;
			}
		}

		if (subpass.pDepthStencilAttachment)
		{
			auto& reference = *subpass.pDepthStencilAttachment;
			// Set it only if not defined yet.
			if (attachmentDescriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
			{
				attachmentDescriptions[reference.attachment].initialLayout = reference.layout;
			}
		}

		if (subpass.pResolveAttachments)
		{
			for (size_t k = 0U; k < subpass.colorAttachmentCount; ++k)
			{
				auto& reference = subpass.pResolveAttachments[k];
				// Set it only if not defined yet
				if (attachmentDescriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
				{
					attachmentDescriptions[reference.attachment].initialLayout = reference.layout;
				}
			}
		}

		if (const auto depth_resolve = GetDepthResolveReference(subpass))
		{
			// Set it only if not defined yet.
			if (attachmentDescriptions[depth_resolve->attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
			{
				attachmentDescriptions[depth_resolve->attachment].initialLayout = depth_resolve->layout;
			}
		}
	}

	// Make the final layout same as the last subpass layout.
	{
		auto& subpass = subpassDescriptions.back();

		for (size_t k = 0U; k < subpass.colorAttachmentCount; ++k)
		{
			const auto& reference = subpass.pColorAttachments[k];

			attachmentDescriptions[reference.attachment].finalLayout = reference.layout;
		}

		for (size_t k = 0U; k < subpass.inputAttachmentCount; ++k)
		{
			const auto& reference = subpass.pInputAttachments[k];

			attachmentDescriptions[reference.attachment].finalLayout = reference.layout;

			// Do not use depth attachment if used as input.
			if (IsDepthFormat(attachmentDescriptions[reference.attachment].format))
			{
				subpass.pDepthStencilAttachment = nullptr;
			}
		}

		if (subpass.pDepthStencilAttachment)
		{
			const auto& reference = *subpass.pDepthStencilAttachment;

			attachmentDescriptions[reference.attachment].finalLayout = reference.layout;
		}

		if (subpass.pResolveAttachments)
		{
			for (size_t k = 0U; k < subpass.colorAttachmentCount; ++k)
			{
				const auto& reference = subpass.pResolveAttachments[k];

				attachmentDescriptions[reference.attachment].finalLayout = reference.layout;
			}
		}

		if (const auto depthResolve = GetDepthResolveReference(subpass))
		{
			attachmentDescriptions[depthResolve->attachment].finalLayout = depthResolve->layout;
		}
	}
}

template <typename T>
std::vector<T> GetSubpassDependencies(const size_t subpassCount)
{
	std::vector<T> dependencies(subpassCount - 1);

	if (subpassCount > 1)
	{
		for (u32 i = 0; i < ToU32(dependencies.size()); ++i)
		{
			// Transition input attachments from color attachment to shader read.
			dependencies[i].srcSubpass = i;
			dependencies[i].dstSubpass = i + 1;
			dependencies[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[i].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[i].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}
	}

	return dependencies;
}

template <typename T>
T GetAttachmentReference(const u32 attachment, const VkImageLayout layout)
{
	T reference{};
	SetStructureType(reference);

	reference.attachment = attachment;
	reference.layout = layout;

	return reference;
}	// namespace vge

vge::RenderPass::RenderPass(Device& device, const std::vector<Attachment>& attachments, const std::vector<LoadStoreInfo>& loadStoreInfos, const std::vector<SubpassInfo>& subpasses)
	: VulkanResource(VK_NULL_HANDLE, &device), _SubpassCount(std::max<size_t>(1, subpasses.size())) /* at least 1 subpass */
{
	if (device.IsExtensionEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME))
	{
		CreateRenderpass<
			VkSubpassDescription2KHR, 
			VkAttachmentDescription2KHR, 
			VkAttachmentReference2KHR, 
			VkSubpassDependency2KHR, 
			VkRenderPassCreateInfo2KHR>
			(attachments, loadStoreInfos, subpasses);
	}
	else
	{
		CreateRenderpass<
			VkSubpassDescription, 
			VkAttachmentDescription, 
			VkAttachmentReference, 
			VkSubpassDependency, 
			VkRenderPassCreateInfo>
			(attachments, loadStoreInfos, subpasses);
	}
}

vge::RenderPass::RenderPass(RenderPass&& other)
	: VulkanResource(std::move(other)), _SubpassCount(other._SubpassCount), _ColorOutputCount(other._ColorOutputCount)
{
	other._Handle = VK_NULL_HANDLE;
}

vge::RenderPass::~RenderPass()
{
	if (_Handle)
	{
		vkDestroyRenderPass(_Device->GetHandle(), _Handle, nullptr);
	}
}

const VkExtent2D vge::RenderPass::GetRenderAreaGranularity() const
{
	VkExtent2D renderAreaGranularity = {};
	vkGetRenderAreaGranularity(_Device->GetHandle(), GetHandle(), &renderAreaGranularity);
	return renderAreaGranularity;
}

template <typename TSubpassDescription, typename TAttachmentDescription, typename TAttachmentReference, typename TSubpassDependency, typename TRenderPassCreateInfo>
void vge::RenderPass::CreateRenderpass(const std::vector<Attachment>& attachments, const std::vector<LoadStoreInfo>& loadStoreInfos, const std::vector<SubpassInfo>& subpasses)
{
	auto attachmentDescriptions = GetAttachmentDescriptions<TAttachmentDescription>(attachments, loadStoreInfos);

	// Store attachments for every subpass.
	std::vector<std::vector<TAttachmentReference>> inputAttachments(_SubpassCount);
	std::vector<std::vector<TAttachmentReference>> colorAttachments(_SubpassCount);
	std::vector<std::vector<TAttachmentReference>> depthStencilAttachments(_SubpassCount);
	std::vector<std::vector<TAttachmentReference>> colorResolveAttachments(_SubpassCount);
	std::vector<std::vector<TAttachmentReference>> depthResolveAttachments(_SubpassCount);

	//std::string newDebugName{};
	//const bool  needsDebugName = get_debug_name().empty();
	//if (needsDebugName)
	//{
	//	newDebugName = fmt::format("RP with {} subpasses:\n", subpasses.size());
	//}

	for (size_t i = 0; i < subpasses.size(); ++i)
	{
		auto& subpass = subpasses[i];

		//if (needsDebugName)
		//{
		//	newDebugName += fmt::format("\t[{}]: {}\n", i, subpass.DebugName);
		//}

		// Fill color attachments references.
		for (auto outputAttachment : subpass.OutputAttachments)
		{
			auto initialLayout = attachments[outputAttachment].InitialLayout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : attachments[outputAttachment].InitialLayout;
			auto& description = attachmentDescriptions[outputAttachment];
			if (!IsDepthFormat(description.format))
			{
				colorAttachments[i].push_back(GetAttachmentReference<TAttachmentReference>(outputAttachment, initialLayout));
			}
		}

		// Fill input attachments references.
		for (auto inputAttachment : subpass.InputAttachments)
		{
			auto defaultLayout = IsDepthFormat(attachmentDescriptions[inputAttachment].format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			auto initialLayout = attachments[inputAttachment].InitialLayout == VK_IMAGE_LAYOUT_UNDEFINED ? defaultLayout : attachments[inputAttachment].InitialLayout;
			inputAttachments[i].push_back(GetAttachmentReference<TAttachmentReference>(inputAttachment, initialLayout));
		}

		for (auto r_attachment : subpass.ColorResolveAttachments)
		{
			auto initialLayout = attachments[r_attachment].InitialLayout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : attachments[r_attachment].InitialLayout;
			colorResolveAttachments[i].push_back(GetAttachmentReference<TAttachmentReference>(r_attachment, initialLayout));
		}

		if (!subpass.DisableDepthStencilAttachment)
		{
			// Assumption: depth stencil attachment appears in the list before any depth stencil resolve attachment.
			auto it = find_if(attachments.begin(), attachments.end(), [](const Attachment attachment) { return IsDepthFormat(attachment.Format); });
			if (it != attachments.end())
			{
				auto i_depth_stencil = ToU32(std::distance(attachments.begin(), it));
				auto initialLayout = it->InitialLayout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : it->InitialLayout;
				depthStencilAttachments[i].push_back(GetAttachmentReference<TAttachmentReference>(i_depth_stencil, initialLayout));

				if (subpass.DepthStencilResolveMode != VK_RESOLVE_MODE_NONE)
				{
					auto i_depth_stencil_resolve = subpass.DepthStencilResolveAttachment;
					initialLayout = attachments[i_depth_stencil_resolve].InitialLayout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : attachments[i_depth_stencil_resolve].InitialLayout;
					depthResolveAttachments[i].push_back(GetAttachmentReference<TAttachmentReference>(i_depth_stencil_resolve, initialLayout));
				}
			}
		}
	}

	std::vector<TSubpassDescription> subpassDescriptions;
	subpassDescriptions.reserve(_SubpassCount);
	
	VkSubpassDescriptionDepthStencilResolveKHR depthResolve = {};

	for (size_t i = 0; i < subpasses.size(); ++i)
	{
		auto& subpass = subpasses[i];

		TSubpassDescription subpassDescription{};
		SetStructureType(subpassDescription);
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		subpassDescription.pInputAttachments = inputAttachments[i].empty() ? nullptr : inputAttachments[i].data();
		subpassDescription.inputAttachmentCount = ToU32(inputAttachments[i].size());

		subpassDescription.pColorAttachments = colorAttachments[i].empty() ? nullptr : colorAttachments[i].data();
		subpassDescription.colorAttachmentCount = ToU32(colorAttachments[i].size());

		subpassDescription.pResolveAttachments = colorResolveAttachments[i].empty() ? nullptr : colorResolveAttachments[i].data();

		subpassDescription.pDepthStencilAttachment = nullptr;
		if (!depthStencilAttachments[i].empty())
		{
			subpassDescription.pDepthStencilAttachment = depthStencilAttachments[i].data();

			if (!depthResolveAttachments[i].empty())
			{
				// If the pNext list of VkSubpassDescription2 includes a VkSubpassDescriptionDepthStencilResolve structure,
				// then that structure describes multisample resolve operations for the depth/stencil attachment in a subpass.
				depthResolve.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR;
				depthResolve.depthResolveMode = subpass.DepthStencilResolveMode;
				SetPointerNext(subpassDescription, depthResolve, depthResolveAttachments[i][0]);

				auto& reference = depthResolveAttachments[i][0];
				// Set it only if not defined yet.
				if (attachmentDescriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
				{
					attachmentDescriptions[reference.attachment].initialLayout = reference.layout;
				}
			}
		}

		subpassDescriptions.push_back(subpassDescription);
	}

	// Default subpass
	if (subpasses.empty())
	{
		TSubpassDescription subpassDescription = {};
		SetStructureType(subpassDescription);
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		u32 defaultDepthStencilAttachment = VK_ATTACHMENT_UNUSED;

		for (u32 k = 0U; k < ToU32(attachmentDescriptions.size()); ++k)
		{
			if (IsDepthFormat(attachments[k].Format))
			{
				if (defaultDepthStencilAttachment == VK_ATTACHMENT_UNUSED)
				{
					defaultDepthStencilAttachment = k;
				}
				continue;
			}

			colorAttachments[0].push_back(GetAttachmentReference<TAttachmentReference>(k, VK_IMAGE_LAYOUT_GENERAL));
		}

		subpassDescription.pColorAttachments = colorAttachments[0].data();

		if (defaultDepthStencilAttachment != VK_ATTACHMENT_UNUSED)
		{
			depthStencilAttachments[0].push_back(GetAttachmentReference<TAttachmentReference>(defaultDepthStencilAttachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

			subpassDescription.pDepthStencilAttachment = depthStencilAttachments[0].data();
		}

		subpassDescriptions.push_back(subpassDescription);
	}

	SetAttachmentLayouts<TSubpassDescription, TAttachmentDescription, TAttachmentReference>(subpassDescriptions, attachmentDescriptions);

	_ColorOutputCount.reserve(_SubpassCount);
	for (size_t i = 0; i < _SubpassCount; i++)
	{
		_ColorOutputCount.push_back(ToU32(colorAttachments[i].size()));
	}

	const auto& subpassDependencies = GetSubpassDependencies<TSubpassDependency>(_SubpassCount);

	TRenderPassCreateInfo createInfo = {};
	SetStructureType(createInfo);
	createInfo.attachmentCount = ToU32(attachmentDescriptions.size());
	createInfo.pAttachments = attachmentDescriptions.data();
	createInfo.subpassCount = ToU32(subpassDescriptions.size());
	createInfo.pSubpasses = subpassDescriptions.data();
	createInfo.dependencyCount = ToU32(subpassDependencies.size());
	createInfo.pDependencies = subpassDependencies.data();

	VK_ENSURE(CreateVkRenderpass(device->GetHandle, &createInfo, nullptr, &handle);)

	//if (needsDebugName)
	//{
	//	set_debug_name(newDebugName);
	//}
}