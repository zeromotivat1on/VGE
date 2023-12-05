#include "Texture.h"
#include "Device.h"
#include "File.h"
#include "Buffer.h"
#include "Utils.h"

vge::Texture vge::Texture::Create(const TextureCreateInfo& data)
{
	Texture tex = {};
	tex.m_Id = data.Id;
	tex.m_Filename = data.Filename;
	tex.m_Device = data.Device;
	tex.m_Image = Image::CreateForTexture(data.Device, data.Filename);

	{
		ImageViewCreateInfo texImgViewCreateInfo = {};
		texImgViewCreateInfo.Device = data.Device;
		texImgViewCreateInfo.Format = VK_FORMAT_R8G8B8A8_UNORM;
		texImgViewCreateInfo.AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		texImgViewCreateInfo.Image = tex.m_Image.GetHandle();

		tex.m_View = Image::CreateView(texImgViewCreateInfo);
	}

	CreateTextureDescriptorSet(data.Device->GetHandle(), data.Sampler, data.DescriptorPool, data.DescriptorLayout, tex.m_View, tex.m_Descriptor);

	LOG(Log, "New - ID: %d, filename: %s", tex.GetId(), tex.GetFilename());

	return tex;
}

void vge::Texture::Destroy()
{
	vkDestroyImageView(m_Device->GetHandle(), m_View, nullptr);
	m_View = VK_NULL_HANDLE;
	m_Image.Destroy();
}
