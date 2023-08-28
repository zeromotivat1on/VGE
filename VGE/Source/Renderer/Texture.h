#pragma once

#include "Common.h"
#include "VulkanUtils.h"

namespace vge
{
	struct VmaImage
	{
		VkImage Handle = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo AllocInfo = {};

		inline void Destroy()
		{
			vmaDestroyImage(VulkanContext::Allocator, Handle, Allocation);
			Handle = VK_NULL_HANDLE;
			Allocation = VK_NULL_HANDLE;
			memory::memzero(&AllocInfo, sizeof(VmaAllocationInfo));
		}
	};

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

		inline void Destroy()
		{
			vkDestroyImageView(VulkanContext::Device, m_View, nullptr);
			m_View = VK_NULL_HANDLE;
			vmaDestroyImage(VulkanContext::Allocator, m_Image.Handle, m_Image.Allocation);
			memory::memzero(&m_Image, sizeof(VmaImage));
		}

		inline int32 GetId() const { return m_Id; }
		inline const char* GetFilename() const { return m_Filename; }
		inline VkDescriptorSet GetDescriptor() const { return m_Descriptor; }

	private:
		int32 m_Id = INDEX_NONE;
		const char* m_Filename = nullptr;
		VmaImage m_Image = {};
		VkImageView m_View = VK_NULL_HANDLE;
		VkDescriptorSet m_Descriptor = VK_NULL_HANDLE;
	};
}
