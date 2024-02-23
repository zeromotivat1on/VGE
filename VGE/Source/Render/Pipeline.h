#pragma once

#include "Common.h"
#include "Shader.h"

namespace vge
{
	class Device;
	class RenderPass;

	struct PipelineCreateInfo
	{
		vge::Device* Device = nullptr;
		std::vector<const char*> ShaderFilenames = {};
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> DescriptorSetLayoutBindings = {};
		std::vector<VkDynamicState> DynamicStates = {};
		std::vector<VkPushConstantRange> PushConstants = {};
		u32 SubpassIndex = 0;
		vge::RenderPass* RenderPass = nullptr;
		VkPipelineBindPoint BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		// Can be set by DefaultCreateInfo.
		VkPipelineViewportStateCreateInfo ViewportInfo = {};
		VkPipelineVertexInputStateCreateInfo VertexInfo = {};
		VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo = {};
		VkPipelineRasterizationStateCreateInfo RasterizationInfo = {};
		VkPipelineMultisampleStateCreateInfo MultisampleInfo = {};
		VkPipelineColorBlendAttachmentState ColorBlendAttachment = {};
		VkPipelineColorBlendStateCreateInfo ColorBlendInfo = {};
		VkPipelineDepthStencilStateCreateInfo DepthStencilInfo = {};
		VkPipelineDynamicStateCreateInfo DynamicStateInfo = {};
	};

	class Pipeline
	{
	public:
		// NOTE: If you want to use DynamicStates, ensure they are valid before calling this or update dynamic state info manually.
		static void DefaultCreateInfo(PipelineCreateInfo& createInfo);

	public:
		Pipeline() = default;

	public:
		void Initialize(const PipelineCreateInfo& data);
		void Destroy();

		inline VkPipeline GetHandle() const { return m_Handle; }
		inline VkPipelineLayout GetLayout() const { return m_Layout; }
		inline const Shader* GetShader(ShaderStage stage) const { return &m_Shaders[(size_t)stage]; }
		inline VkPipelineBindPoint GetBindPoint() const { return m_BindPoint; }

	private:
		void GetShaderStageInfos(std::vector<VkPipelineShaderStageCreateInfo>& outStageInfos);

	private:
		Device* m_Device = nullptr;
		RenderPass* m_RenderPass = nullptr;
		VkPipeline m_Handle = VK_NULL_HANDLE;
		VkPipelineLayout m_Layout = VK_NULL_HANDLE;
		i32 m_SubpassIndex = INDEX_NONE;
		Shader m_Shaders[(size_t)ShaderStage::Count];
		VkPipelineBindPoint m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	};
}
