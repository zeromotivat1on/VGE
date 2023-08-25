#pragma once

#include "Common.h"
#include "VulkanUtils.h"

namespace vge
{
	struct TextureCreateInfo
	{
		int32 Id = INDEX_NONE;
		const char* Filename = nullptr;
		VkSampler Sampler = VK_NULL_HANDLE;
		VkCommandPool CmdPool = VK_NULL_HANDLE;
		VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout DescriptorLayout = VK_NULL_HANDLE;
	};

	class Texture
	{
	public:
		static Texture Create(const TextureCreateInfo& data);

	public:
		Texture() = default;

		void Destroy(VkDevice device);

		int32 GetId() const { return m_Id; }
		const char* GetFilename() const { return m_Filename; }
		VkDescriptorSet GetDescriptor() const { return m_Descriptor; }

	private:
		int32 m_Id = INDEX_NONE;
		const char* m_Filename = nullptr;
		VmaImage m_Image = {};
		VkImageView m_View = VK_NULL_HANDLE;
		VkDescriptorSet m_Descriptor = VK_NULL_HANDLE;
	};
}
