#pragma once

#include "Common.h"
#include "Image.h"

namespace vge
{
	class Device;

	struct TextureCreateInfo
	{
		i32 Id = INDEX_NONE;
		const c8* Filename = nullptr;
		const Device* Device = nullptr;
		VkSampler Sampler = VK_NULL_HANDLE;
		VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout DescriptorLayout = VK_NULL_HANDLE;
	};

	class Texture
	{
	public:
		static Texture Create(const TextureCreateInfo& data);

	public:
		Texture() = default;

		void Destroy();

		inline i32 GetId() const { return m_Id; }
		inline const c8* GetFilename() const { return m_Filename; }
		inline VkDescriptorSet GetDescriptor() const { return m_Descriptor; }

	private:
		i32 m_Id = INDEX_NONE;
		const c8* m_Filename = nullptr;
		const Device* m_Device = nullptr;
		Image m_Image = {};
		VkImageView m_View = VK_NULL_HANDLE;
		VkDescriptorSet m_Descriptor = VK_NULL_HANDLE;
	};
}
