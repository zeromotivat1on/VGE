#pragma once

#include "Common.h"
#include "Device.h"
#include "Buffer.h"

namespace vge
{
	class RenderPass;

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
		Swapchain(const Device* device);
		NOT_COPYABLE(Swapchain);

	public:
		void Initialize(SwapchainRecreateInfo* recreateInfo = nullptr);
		void Destroy(SwapchainRecreateInfo* recreateInfo = nullptr);

		inline VkSwapchainKHR GetHandle() const { return m_Handle; }
		inline VkSurfaceKHR GetSurface() const { return m_Surface; }
		inline VkFormat GetImageFormat() const { return m_ImageFormat; }
		inline VkExtent2D GetExtent() const { return m_Extent; }
		inline u32 GetExtentWidth() const { return m_Extent.width; }
		inline u32 GetExtentHeight() const { return m_Extent.height; }
		inline size_t GetImageCount() const { return m_Images.size(); }
		inline size_t GetFramebufferCount() const { return m_Framebuffers.size(); }
		inline FrameBuffer* GetFramebuffer(size_t index) { return index < GetFramebufferCount() ? &m_Framebuffers[index] : nullptr; }
		inline f32 GetAspectRatio() const { return static_cast<f32>(m_Extent.width) / static_cast<f32>(m_Extent.height); }

		inline const SwapchainImage* GetImage(size_t index) const { return index < GetImageCount() ? &m_Images[index] : nullptr; }
		inline 		 SwapchainImage* GetImage(size_t index)		  { return index < GetImageCount() ? &m_Images[index] : nullptr; }

		inline u32 GetCurrentImageIndex() const { return m_CurrentImageIndex; }
		inline VkResult AcquireNextImage(VkSemaphore semaphore, u64 timeout = UINT64_MAX, VkFence fence = VK_NULL_HANDLE)
		{
			return vkAcquireNextImageKHR(m_Device->GetHandle(), m_Handle, timeout, semaphore, fence, &m_CurrentImageIndex);
		}

		inline SwapchainSupportDetails GetSupportDetails() const { return m_Device->GetSwapchainSupportDetails(m_Surface); }

		void CreateFramebuffer(const RenderPass* renderPass, u32 attachmentCount, const VkImageView* attachments);

	private:
		const Device* m_Device = nullptr;
		u32 m_CurrentImageIndex = 0;
		VkSwapchainKHR m_Handle = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkFormat m_ImageFormat = {};
		VkExtent2D m_Extent = {};
		std::vector<SwapchainImage> m_Images = {};
		std::vector<FrameBuffer> m_Framebuffers = {};
	};
}
