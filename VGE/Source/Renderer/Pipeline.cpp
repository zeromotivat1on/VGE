#include "Pipeline.h"
#include "File.h"
#include "Shader.h"
#include "VulkanUtils.h"
#include "VulkanContext.h"

vge::PipelineCreateInfo vge::Pipeline::DefaultCreateInfo(VkExtent2D extent)
{
#pragma region DynamicStateExample
	{
		std::vector<VkDynamicState> dynamicStates;
		dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT); // can be set from vkCmdSetViewport(cmdBuffer, pos, amount, newViewport)
		dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);	// can be set from vkCmdSetScissor(cmdBuffer, pos, amount, newScissor)

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
		dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32>(dynamicStates.size());
		dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
	}
#pragma endregion DynamicStateExample

	PipelineCreateInfo createInfo = {};

	createInfo.InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	createInfo.InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	createInfo.Viewport.x = 0.0f;
	createInfo.Viewport.y = 0.0f;
	createInfo.Viewport.width = static_cast<float>(extent.width);
	createInfo.Viewport.height = static_cast<float>(extent.height);
	createInfo.Viewport.minDepth = 0.0f;
	createInfo.Viewport.maxDepth = 1.0f;

	createInfo.Scissor.offset = { 0, 0 };
	createInfo.Scissor.extent = extent;

	createInfo.ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	createInfo.ViewportInfo.viewportCount = 1;
	createInfo.ViewportInfo.pViewports = &createInfo.Viewport;
	createInfo.ViewportInfo.scissorCount = 1;
	createInfo.ViewportInfo.pScissors = &createInfo.Scissor;

	createInfo.VertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	createInfo.VertexInfo.vertexBindingDescriptionCount = 0;
	createInfo.VertexInfo.pVertexBindingDescriptions = nullptr;
	createInfo.VertexInfo.vertexAttributeDescriptionCount = 0;
	createInfo.VertexInfo.pVertexAttributeDescriptions = nullptr;

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

	return createInfo;
}

void vge::Pipeline::Initialize(const char* vertexShader, const char* fragmentShader, const PipelineCreateInfo& data)
{
	ENSURE(data.PipelineLayout != VK_NULL_HANDLE);
	ENSURE(data.RenderPass != VK_NULL_HANDLE);
	ENSURE(data.SubpassIndex != INDEX_NONE);

	m_Layout = data.PipelineLayout;
	m_SubpassIndex = data.SubpassIndex;

	std::vector<char> vertexShaderCode = file::ReadShader(vertexShader);
	std::vector<char> fragmentShaderCode = file::ReadShader(fragmentShader);

	m_VertexShaderModule = CreateShaderModule(VulkanContext::Device, vertexShaderCode);
	m_FragmentShaderModule = CreateShaderModule(VulkanContext::Device, fragmentShaderCode);

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {};
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = m_VertexShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = m_FragmentShaderModule;
	shaderStages[1].pName = "main";

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = static_cast<uint32>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &data.VertexInfo;
	pipelineCreateInfo.pInputAssemblyState = &data.InputAssemblyInfo;
	pipelineCreateInfo.pViewportState = &data.ViewportInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.pRasterizationState = &data.RasterizationInfo;
	pipelineCreateInfo.pMultisampleState = &data.MultisampleInfo;
	pipelineCreateInfo.pColorBlendState = &data.ColorBlendInfo;
	pipelineCreateInfo.pDepthStencilState = &data.DepthStencilInfo;
	pipelineCreateInfo.layout = data.PipelineLayout;
	pipelineCreateInfo.renderPass = data.RenderPass;
	pipelineCreateInfo.subpass = data.SubpassIndex;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = INDEX_NONE;

	VK_ENSURE(vkCreateGraphicsPipelines(VulkanContext::Device, nullptr, 1, &pipelineCreateInfo, nullptr, &m_Handle));
}

void vge::Pipeline::Destroy()
{
	vkDestroyShaderModule(VulkanContext::Device, m_FragmentShaderModule, nullptr);
	vkDestroyShaderModule(VulkanContext::Device, m_VertexShaderModule, nullptr);
	vkDestroyPipeline(VulkanContext::Device, m_Handle, nullptr);
	vkDestroyPipelineLayout(VulkanContext::Device, m_Layout, nullptr);
}
