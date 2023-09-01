#include "Texture.h"
#include "Device.h"
#include "File.h"
#include "Buffer.h"
#include "Utils.h"

vge::Texture vge::Texture::Create(const TextureCreateInfo& data)
{
	ImageCreateInfo texImgCreateInfo = {};
	texImgCreateInfo.Device = data.Device;

	Texture tex = {};
	tex.m_Id = data.Id;
	tex.m_Filename = data.Filename;
	tex.m_Device = data.Device;
	tex.m_Image = Image::CreateForTexture(data.Device, data.Filename);

	CreateImageView(data.Device->GetHandle(), tex.m_Image.Handle, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, tex.m_View);
	CreateTextureDescriptorSet(data.Device->GetHandle(), data.Sampler, data.DescriptorPool, data.DescriptorLayout, tex.m_View, tex.m_Descriptor);

	LOG(Log, "New - ID: %d, name: %s", tex.GetId(), tex.GetFilename());

	return tex;
}

void vge::Texture::Destroy()
{
	vkDestroyImageView(m_Device->GetHandle(), m_View, nullptr);
	m_View = VK_NULL_HANDLE;
	m_Image.Destroy();
}
