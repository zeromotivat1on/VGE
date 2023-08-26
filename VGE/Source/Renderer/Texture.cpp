#include "Texture.h"

vge::Texture vge::Texture::Create(const TextureCreateInfo& data)
{
	Texture tex = {};
	tex.m_Id = data.Id;
	tex.m_Filename = data.Filename;

	CreateTextureImage(VulkanContext::Allocator, VulkanContext::GfxQueue, data.CmdPool, data.Filename, tex.m_Image);
	CreateImageView(tex.m_Image.Handle, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, tex.m_View);
	CreateTextureDescriptorSet(data.Sampler, data.DescriptorPool, data.DescriptorLayout, tex.m_View, tex.m_Descriptor);

	LOG(Log, "New - ID: %d, name: %s", tex.GetId(), tex.GetFilename());

	return tex;
}

void vge::Texture::Destroy(VkDevice device)
{
	vkDestroyImageView(device, m_View, nullptr);
	vmaDestroyImage(VulkanContext::Allocator, m_Image.Handle, m_Image.Allocation);
}
