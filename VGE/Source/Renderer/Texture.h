#pragma once

#include "Common.h"
#include "Image.h"

namespace vge
{
	class Device;

	struct TextureCreateInfo
	{
		int32 Id = INDEX_NONE;
		const char* Filename = nullptr;
		const Device* Device = nullptr;
		VkSampler Sampler = VK_NULL_HANDLE;
		VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout DescriptorLayout = VK_NULL_HANDLE;
	};

	class Texture
	{
	public:
		static Texture Create(const TextureCreateInfo& data);

	private:
		Texture() = default;

	public:
		void Destroy();

		inline int32 GetId() const { return m_Id; }
		inline const char* GetFilename() const { return m_Filename; }
		inline VkDescriptorSet GetDescriptor() const { return m_Descriptor; }

	private:
		int32 m_Id = INDEX_NONE;
		const char* m_Filename = nullptr;
		const Device* m_Device = nullptr;
		Image m_Image = {};
		VkImageView m_View = VK_NULL_HANDLE;
		VkDescriptorSet m_Descriptor = VK_NULL_HANDLE;
	};
}
