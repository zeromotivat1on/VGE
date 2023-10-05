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
		int32 SubpassCount = INDEX_NONE;
		VkFormat ColorFormat = VK_FORMAT_UNDEFINED;
		VkFormat DepthFormat = VK_FORMAT_UNDEFINED;
		std::vector<VkSubpassDescription>* Subpasses = nullptr;
		std::vector<VkSubpassDependency>* Dependencies = nullptr;
		std::vector<VkAttachmentDescription>* Attachments = nullptr;
	};

	class RenderPass
	{
	public:
		RenderPass() = default;

		void Initialize(const RenderPassCreateInfo& data);
		void Destroy();

		inline VkRenderPass GetHandle() const { return m_Handle; }
		inline int32 GeSubpassCount() const { return m_SubpassCount; }
		inline const Subpass* GetSubpass(int32 index) const { return index < m_SubpassCount ? &m_Subpasses[index] : nullptr; }

	private:
		// Creates attachments for render pass (images, views) according to description.
		void InitSubpasses(const RenderPassCreateInfo& data);

	private:
		const Device* m_Device = nullptr;
		const Swapchain* m_Swapchain = nullptr;
		VkRenderPass m_Handle = VK_NULL_HANDLE;
		int32 m_SubpassCount = INDEX_NONE;
		std::vector<Subpass> m_Subpasses = {};
	};
}

