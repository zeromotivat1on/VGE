#pragma once

#include "Common.h"

namespace vge
{
	struct PipelineCreateInfo
	{
		VkViewport Viewport = {};
		VkRect2D Scissor = {};
		VkPipelineVertexInputStateCreateInfo VertexInfo = {};
		VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo = {};
		VkPipelineRasterizationStateCreateInfo RasterizationInfo = {};
		VkPipelineMultisampleStateCreateInfo MultisampleInfo = {};
		VkPipelineColorBlendAttachmentState ColorBlendAttachment = {};
		VkPipelineColorBlendStateCreateInfo ColorBlendInfo = {};
		VkPipelineDepthStencilStateCreateInfo DepthStencilInfo = {};
		VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
		VkRenderPass RenderPass = VK_NULL_HANDLE;
		int32 SubpassIndex = INDEX_NONE;
	};

	class Pipeline
	{
	public:
		static PipelineCreateInfo DefaultCreateInfo(VkExtent2D extent);

	public:
		Pipeline() = default;
		NOT_COPYABLE(Pipeline);

	public:
		void Initialize(const char* vertexShader, const char* fragmentShader, const PipelineCreateInfo& data);
		void Destroy();

		inline VkPipeline GetHandle() const { return m_Handle; }
		inline VkPipelineLayout GetLayout() const { return m_Layout; }

	private:
		VkPipeline m_Handle = VK_NULL_HANDLE;
		VkPipelineLayout m_Layout = VK_NULL_HANDLE;
		VkShaderModule m_VertexShaderModule = VK_NULL_HANDLE;
		VkShaderModule m_FragmentShaderModule = VK_NULL_HANDLE;
		int32 m_SubpassIndex = INDEX_NONE;
	};
}
