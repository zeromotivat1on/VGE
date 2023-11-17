#include "Pipeline.h"
#include "File.h"
#include "Device.h"
#include "RenderPass.h"

void vge::Pipeline::DefaultCreateInfo(PipelineCreateInfo& createInfo)
{
	createInfo.ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	createInfo.ViewportInfo.viewportCount = 1;
	createInfo.ViewportInfo.pViewports = nullptr;
	createInfo.ViewportInfo.scissorCount = 1;
	createInfo.ViewportInfo.pScissors = nullptr;

	createInfo.VertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	createInfo.VertexInfo.vertexBindingDescriptionCount = 0;
	createInfo.VertexInfo.pVertexBindingDescriptions = nullptr;
	createInfo.VertexInfo.vertexAttributeDescriptionCount = 0;
	createInfo.VertexInfo.pVertexAttributeDescriptions = nullptr;

	createInfo.InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	createInfo.InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	createInfo.RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	createInfo.RasterizationInfo.depthClampEnable = VK_FALSE;
	createInfo.RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	createInfo.RasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	createInfo.RasterizationInfo.lineWidth = 1.0f;
	createInfo.RasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	createInfo.RasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // projection matrix y-axis is inverted, so use counter-clockwise
	createInfo.RasterizationInfo.depthBiasEnable = VK_FALSE;

	createInfo.MultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	createInfo.MultisampleInfo.sampleShadingEnable = VK_FALSE;
	createInfo.MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	createInfo.ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	createInfo.ColorBlendAttachment.blendEnable = VK_TRUE;
	createInfo.ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	createInfo.ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	createInfo.ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	createInfo.ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	createInfo.ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	createInfo.ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	createInfo.ColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	createInfo.ColorBlendInfo.logicOpEnable = VK_FALSE;
	createInfo.ColorBlendInfo.attachmentCount = 1;
	createInfo.ColorBlendInfo.pAttachments = &createInfo.ColorBlendAttachment;

	createInfo.DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	createInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;			// enable depth testing to determine fragment write
	createInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;			// enable writing to depth buffer (to replace old values)
	createInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	createInfo.DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;	// enable check between min and max given depth values
	createInfo.DepthStencilInfo.stencilTestEnable = VK_FALSE;

	createInfo.DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	createInfo.DynamicStateInfo.dynamicStateCount = static_cast<u32>(createInfo.DynamicStates.size());
	createInfo.DynamicStateInfo.pDynamicStates = createInfo.DynamicStates.data();
}

void vge::Pipeline::Initialize(const PipelineCreateInfo& data)
{
	ENSURE(data.Device);
	ENSURE(data.RenderPass);
	ENSURE(data.SubpassIndex != INDEX_NONE);
	ENSURE(data.SubpassIndex < data.RenderPass->GeSubpassCount());

	m_Device = data.Device;
	m_RenderPass = data.RenderPass;
	m_SubpassIndex = data.SubpassIndex;
	m_BindPoint = data.BindPoint;

	// Initialize pipeline shaders.
	{
		const size_t shaderFilenameCount = data.ShaderFilenames.size();
		const size_t bindingsCount = data.DescriptorSetLayoutBindings.size();

		ENSURE(shaderFilenameCount <= (size_t)ShaderStage::Count);
		ENSURE(shaderFilenameCount == bindingsCount);

		for (size_t i = 0; i < shaderFilenameCount; ++i)
		{
			std::vector<c8> shaderCode = file::ReadShader(data.ShaderFilenames[i]);

			ShaderCreateInfo createInfo = {};
			createInfo.Device = m_Device;
			createInfo.SpirvChar = &shaderCode;
			createInfo.StageFlags = Shader::GetFlagsFromStage((ShaderStage)i);
			createInfo.DescriptorSetLayoutBindings = data.DescriptorSetLayoutBindings[i];

			m_Shaders[i].Initialize(createInfo);
		}
	}

	// Create pipeline layout.
	{
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		for (size_t i = 0; i < (size_t)ShaderStage::Count; ++i)
		{
			if (!m_Shaders[i].IsValid())
			{
				LOG(Warning, "Shader with index %d is not valid.", i);
				continue;
			}

			if (m_Shaders[i].GetDescriptorSetLayout().Handle != VK_NULL_HANDLE)
			{
				descriptorSetLayouts.push_back(m_Shaders[i].GetDescriptorSetLayout().Handle);
			}
		}

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = static_cast<u32>(descriptorSetLayouts.size());
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<u32>(data.PushConstants.size());
		pipelineLayoutCreateInfo.pPushConstantRanges = data.PushConstants.data();

		VK_ENSURE(vkCreatePipelineLayout(m_Device->GetHandle(), &pipelineLayoutCreateInfo, nullptr, &m_Layout));
	}

	// Create pipeline.
	{
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		GetShaderStageInfos(shaderStages);

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.stageCount = static_cast<u32>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.pVertexInputState = &data.VertexInfo;
		pipelineCreateInfo.pInputAssemblyState = &data.InputAssemblyInfo;
		pipelineCreateInfo.pViewportState = &data.ViewportInfo;
		pipelineCreateInfo.pDynamicState = &data.DynamicStateInfo;
		pipelineCreateInfo.pRasterizationState = &data.RasterizationInfo;
		pipelineCreateInfo.pMultisampleState = &data.MultisampleInfo;
		pipelineCreateInfo.pColorBlendState = &data.ColorBlendInfo;
		pipelineCreateInfo.pDepthStencilState = &data.DepthStencilInfo;
		pipelineCreateInfo.layout = m_Layout;
		pipelineCreateInfo.renderPass = data.RenderPass->GetHandle();
		pipelineCreateInfo.subpass = data.SubpassIndex;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.basePipelineIndex = INDEX_NONE;

		VK_ENSURE(vkCreateGraphicsPipelines(m_Device->GetHandle(), nullptr, 1, &pipelineCreateInfo, nullptr, &m_Handle));
	}
}

void vge::Pipeline::Destroy()
{
	for (Shader& shader : m_Shaders)
	{
		shader.Destroy();
	}

	vkDestroyPipeline(m_Device->GetHandle(), m_Handle, nullptr);
	vkDestroyPipelineLayout(m_Device->GetHandle(), m_Layout, nullptr);
}

void vge::Pipeline::GetShaderStageInfos(std::vector<VkPipelineShaderStageCreateInfo>& outStageInfos)
{
	for (size_t i = 0; i < (size_t)ShaderStage::Count; ++i)
	{
		if (!m_Shaders[i].IsValid())
		{
			LOG(Warning, "Shader with index %d is not valid.", i);
			continue;
		}

		outStageInfos.push_back(m_Shaders[i].GetStageCreateInfo());
	}
}
