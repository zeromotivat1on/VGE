#include "RenderPass.h"
#include "Device.h"
#include "Swapchain.h"
#include "Utils.h"

void vge::RenderPass::Initialize(const RenderPassCreateInfo& data)
{
	ENSURE(data.SubpassCount == data.Subpasses->size());
	ENSURE(data.SubpassCount == data.Dependencies->size() - 1);
	ENSURE(data.ClearValues->size() == data.Attachments->size())

	m_Device = data.Device;
	m_Swapchain = data.Swapchain;
	m_SubpassCount = data.SubpassCount;
	m_ClearValues = *data.ClearValues;

	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = static_cast<u32>(data.Attachments->size());
	createInfo.pAttachments = data.Attachments->data();
	createInfo.subpassCount = static_cast<u32>(data.Subpasses->size());
	createInfo.pSubpasses = data.Subpasses->data();
	createInfo.dependencyCount = static_cast<u32>(data.Dependencies->size());
	createInfo.pDependencies = data.Dependencies->data();

	VK_ENSURE(vkCreateRenderPass(m_Device->GetHandle(), &createInfo, nullptr, &m_Handle));

	m_AttachmentCount = createInfo.attachmentCount;

	InitSubpasses(data);
}

void vge::RenderPass::Destroy()
{
	vkDestroyRenderPass(m_Device->GetHandle(), m_Handle, nullptr);
}

void vge::RenderPass::InitSubpasses(const RenderPassCreateInfo& data)
{
	m_Subpasses.resize(data.SubpassCount);

	for (u32 subpassIdx = 0 ; subpassIdx < data.SubpassCount; ++subpassIdx)
	{
		SubpassCreateInfo createInfo = {};
		createInfo.Index = subpassIdx;
		createInfo.Description = data.Subpasses->at(subpassIdx);

		m_Subpasses[subpassIdx].Initialize(createInfo);
	}
}

void vge::Subpass::Initialize(const SubpassCreateInfo& data)
{
	ENSURE(data.Index >= 0);

	m_Index = data.Index;
	m_Description = data.Description;
}
