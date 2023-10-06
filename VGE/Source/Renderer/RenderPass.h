#pragma once

#include "Common.h"
#include "RenderCommon.h"
#include "Image.h"

namespace vge
{
	class Device;
	class Swapchain;

	struct RenderPassAttachment
	{
		vge::Image Image = {};
		VkImageView View = VK_NULL_HANDLE;
	};

	struct SubpassCreateInfo
	{
		int32 Index = INDEX_NONE;
		VkSubpassDescription Description = {};
	};

	class Subpass
	{
	public:
		Subpass() = default;

		void Initialize(const SubpassCreateInfo& data);

		inline int32 GetIndex() const { return m_Index; }
		inline const VkSubpassDescription& GetDescription() const { return m_Description; }

	private:
		int32 m_Index = INDEX_NONE;
		VkSubpassDescription m_Description = {};
	};

	struct RenderPassCreateInfo
	{
		vge::Device* Device = nullptr;
		vge::Swapchain* Swapchain = nullptr;
		uint32 SubpassCount = 0;
		VkFormat ColorFormat = VK_FORMAT_UNDEFINED;
		VkFormat DepthFormat = VK_FORMAT_UNDEFINED;
		std::vector<VkSubpassDescription>* Subpasses = nullptr;
		std::vector<VkSubpassDependency>* Dependencies = nullptr;
		std::vector<VkAttachmentDescription>* Attachments = nullptr;
		std::vector<VkClearValue>* ClearValues = nullptr;
	};

	class RenderPass
	{
	public:
		RenderPass() = default;

		void Initialize(const RenderPassCreateInfo& data);
		void Destroy();

		inline VkRenderPass GetHandle() const { return m_Handle; }
		inline uint32 GeSubpassCount() const { return m_SubpassCount; }
		inline uint32 GeAttachmentCount() const { return m_AttachmentCount; }
		inline uint32 GetClearValueCount() const { return m_AttachmentCount; }
		inline const VkClearValue* GetClearValues() const { return m_ClearValues.data(); }
		inline const Subpass* GetSubpass(uint32 index) const { return index < m_SubpassCount ? &m_Subpasses[index] : nullptr; }
		inline const VkClearValue* GetClearValue(uint32 attachmentIdx) const { return attachmentIdx < m_AttachmentCount ? &m_ClearValues[attachmentIdx] : nullptr; }

	private:
		// Creates attachments for render pass (images, views) according to description.
		void InitSubpasses(const RenderPassCreateInfo& data);

	private:
		const Device* m_Device = nullptr;
		const Swapchain* m_Swapchain = nullptr;
		VkRenderPass m_Handle = VK_NULL_HANDLE;
		uint32 m_AttachmentCount = 0;
		uint32 m_SubpassCount = 0;
		std::vector<Subpass> m_Subpasses = {};
		std::vector<VkClearValue> m_ClearValues = {};
	};
}

