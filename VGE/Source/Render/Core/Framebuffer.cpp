#include "Framebuffer.h"
#include "Device.h"

vge::Framebuffer::Framebuffer(Device& device, const RenderTarget& renderTarget, const RenderPass& renderPass)
	: _Device(device), _Extent(renderTarget.GetExtent())
{
	std::vector<VkImageView> attachments;

	for (auto& view : renderTarget.GetViews())
	{
		attachments.emplace_back(view.GetHandle());
	}

	VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	createInfo.renderPass = renderPass.GetHandle();
	createInfo.attachmentCount = ToU32(attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.width = _Extent.width;
	createInfo.height = _Extent.height;
	createInfo.layers = 1;

	VK_ENSURE(vkCreateFramebuffer(device.GetHandle(), &createInfo, nullptr, &_Handle));
}

vge::Framebuffer::Framebuffer(Framebuffer&& other) 
	: _Device(other._Device), _Handle(other._Handle), _Extent(other._Extent)
{
	other._Handle = VK_NULL_HANDLE;
}

vge::Framebuffer::~Framebuffer()
{
	if (_Handle)
	{
		vkDestroyFramebuffer(_Device.GetHandle(), _Handle, nullptr);
	}
}