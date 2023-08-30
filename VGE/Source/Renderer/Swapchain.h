#pragma once

#include "Common.h"
#include "Device.h"

namespace vge
{
	struct SwapchainImage
	{
		VkImage Handle = VK_NULL_HANDLE;
		VkImageView View = VK_NULL_HANDLE;
	};

	struct SwapchainRecreateInfo
	{
		VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
		VkSurfaceKHR Surface = VK_NULL_HANDLE;

		inline bool IsValid() { return Swapchain && Surface; }
	};

	class Swapchain
	{
	public:
		Swapchain() = default;
		Swapchain(Device& device);
		NOT_COPYABLE(Swapchain);

	public:
		void Initialize(SwapchainRecreateInfo* recreateInfo = nullptr);
		void Destroy(SwapchainRecreateInfo* recreateInfo = nullptr);

		inline VkSwapchainKHR GetHandle() const { return m_Handle; }
		inline VkSurfaceKHR GetSurface() const { return m_Surface; }
		inline VkFormat GetImageFormat() const { return m_ImageFormat; }
		inline VkExtent2D GetExtent() const { return m_Extent; }
		inline uint32 GetExtentWidth() const { return m_Extent.width; }
		inline uint32 GetExtentHeight() const { return m_Extent.height; }
		inline size_t GetImageCount() const { return m_Images.size(); }
		inline size_t GetFramebufferCount() const { return m_Framebuffers.size(); }
		inline VkFramebuffer GetFramebuffer(size_t index) { return index < GetFramebufferCount() ? m_Framebuffers[index] : VK_NULL_HANDLE; }

		inline const SwapchainImage* GetImage(size_t index) const { return index < GetImageCount() ? &m_Images[index] : nullptr; }
		inline 		 SwapchainImage* GetImage(size_t index)		  { return index < GetImageCount() ? &m_Images[index] : nullptr; }

		inline VkResult AcquireNextImage(VkSemaphore semaphore, uint32& outImage, uint64 timeout = UINT64_MAX, VkFence fence = VK_NULL_HANDLE)
		{
			return vkAcquireNextImageKHR(m_Device.GetHandle(), m_Handle, timeout, semaphore, fence, &outImage);
		}

		inline SwapchainSupportDetails GetSupportDetails() const { return m_Device.GetSwapchainSupportDetails(m_Surface); }

		void CreateFramebuffer(VkRenderPass renderPass, uint32 attachmentCount, const VkImageView* attachments);

	private:
		Device& m_Device;
		VkSwapchainKHR m_Handle = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkFormat m_ImageFormat = {};
		VkExtent2D m_Extent = {};
		std::vector<SwapchainImage> m_Images = {};
		std::vector<VkFramebuffer> m_Framebuffers = {};
	};
}
