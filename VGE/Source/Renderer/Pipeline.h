#pragma once

#include "Common.h"
#include "Device.h"

namespace vge
{
	struct PipelineCreateInfo
	{
		NOT_COPYABLE(PipelineCreateInfo);

		VkPipelineViewportStateCreateInfo ViewportInfo = {};
		VkPipelineVertexInputStateCreateInfo VertexInfo = {};
		VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo = {};
		VkPipelineRasterizationStateCreateInfo RasterizationInfo = {};
		VkPipelineMultisampleStateCreateInfo MultisampleInfo = {};
		VkPipelineColorBlendAttachmentState ColorBlendAttachment = {};
		VkPipelineColorBlendStateCreateInfo ColorBlendInfo = {};
		VkPipelineDepthStencilStateCreateInfo DepthStencilInfo = {};
		std::vector<VkDynamicState> DynamicStates = {};
		VkPipelineDynamicStateCreateInfo DynamicStateInfo = {};
		VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
		VkRenderPass RenderPass = VK_NULL_HANDLE;
		int32 SubpassIndex = INDEX_NONE;
	};

	class Pipeline
	{
	public:
		static void DefaultCreateInfo(PipelineCreateInfo& createInfo);

	public:
		Pipeline() = default;
		Pipeline(Device& device);
		NOT_COPYABLE(Pipeline);

	public:
		void Initialize(const char* vertexShader, const char* fragmentShader, const PipelineCreateInfo& data);
		void Destroy();

		inline VkPipeline GetHandle() const { return m_Handle; }
		inline VkPipelineLayout GetLayout() const { return m_Layout; }
		
		inline void Bind(VkCommandBuffer cmdBuffer, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) { vkCmdBindPipeline(cmdBuffer, bindPoint, m_Handle); }

	private:
		Device& m_Device;
		VkPipeline m_Handle = VK_NULL_HANDLE;
		VkPipelineLayout m_Layout = VK_NULL_HANDLE;
		VkShaderModule m_VertexShaderModule = VK_NULL_HANDLE;
		VkShaderModule m_FragmentShaderModule = VK_NULL_HANDLE;
		int32 m_SubpassIndex = INDEX_NONE;
	};
}
