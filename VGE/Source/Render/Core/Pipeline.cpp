#include "Pipeline.h"
#include "Core/Device.h"

vge::Pipeline::Pipeline(Device& device) 
	: _Device(device)
{}

vge::Pipeline::Pipeline(Pipeline&& other) 
	: _Device(other._Device), _Handle(other._Handle), _State(other._State)
{
	other._Handle = VK_NULL_HANDLE;
}

vge::Pipeline::~Pipeline()
{
	if (_Handle)
	{
		vkDestroyPipeline(_Device.GetHandle(), _Handle, nullptr);
	}
}

vge::ComputePipeline::ComputePipeline(Device& device, VkPipelineCache pipelineCache, PipelineState& pipelineState) 
	: Pipeline(device)
{
	const ShaderModule* shaderModule = pipelineState.GetPipelineLayout().GetShaderModules().front();

	ENSURE_MSG(shaderModule->GetStage() == VK_SHADER_STAGE_COMPUTE_BIT, "Shader stage must be compute.");

	VkPipelineShaderStageCreateInfo stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	stage.stage = shaderModule->GetStage();
	stage.pName = shaderModule->GetEntryPoint().c_str();

	// Create the Vulkan handle
	VkShaderModuleCreateInfo moduleCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	moduleCreateInfo.codeSize = shaderModule->GetBinary().size() * sizeof(u32);
	moduleCreateInfo.pCode = shaderModule->GetBinary().data();

	VK_ENSURE(vkCreateShaderModule(device.GetHandle(), &moduleCreateInfo, nullptr, &stage.module));

	//device.get_debug_utils().set_debug_name(device.GetHandle(),
	//	VK_OBJECT_TYPE_SHADER_MODULE, reinterpret_cast<uint64_t>(stage.module),
	//	shaderModule->get_debug_name().c_str());

	// Create specialization info from tracked state.
	std::vector<u8> data;
	std::vector<VkSpecializationMapEntry> mapEntries;

	const auto specializationConstantState = pipelineState.GetSpecializationConstantState().GetSpecializationConstantState();

	for (const auto specializationConstant : specializationConstantState)
	{
		mapEntries.push_back({ specializationConstant.first, ToU32(data.size()), specializationConstant.second.size() });
		data.insert(data.end(), specializationConstant.second.begin(), specializationConstant.second.end());
	}

	VkSpecializationInfo specializationInfo = {};
	specializationInfo.mapEntryCount = ToU32(mapEntries.size());
	specializationInfo.pMapEntries = mapEntries.data();
	specializationInfo.dataSize = data.size();
	specializationInfo.pData = data.data();

	stage.pSpecializationInfo = &specializationInfo;

	VkComputePipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
	createInfo.layout = pipelineState.GetPipelineLayout().GetHandle();
	createInfo.stage = stage;

	VK_ENSURE(vkCreateComputePipelines(device.GetHandle(), pipelineCache, 1, &createInfo, nullptr, &_Handle));

	vkDestroyShaderModule(device.GetHandle(), stage.module, nullptr);
}

vge::GraphicsPipeline::GraphicsPipeline(Device& device, VkPipelineCache pipelineCache, PipelineState& pipelineState) 
	: Pipeline(device)
{
	std::vector<VkShaderModule> shaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> stageCreateInfos;

	// Create specialization info from tracked state. This is shared by all shaders.
	std::vector<u8> data;
	std::vector<VkSpecializationMapEntry> mapEntries;

	const auto specializationConstantState = pipelineState.GetSpecializationConstantState().GetSpecializationConstantState();

	for (const auto specializationConstant : specializationConstantState)
	{
		mapEntries.push_back({ specializationConstant.first, ToU32(data.size()), specializationConstant.second.size() });
		data.insert(data.end(), specializationConstant.second.begin(), specializationConstant.second.end());
	}

	VkSpecializationInfo specializationInfo = {};
	specializationInfo.mapEntryCount = ToU32(mapEntries.size());
	specializationInfo.pMapEntries = mapEntries.data();
	specializationInfo.dataSize = data.size();
	specializationInfo.pData = data.data();

	for (const ShaderModule* shaderModule : pipelineState.GetPipelineLayout().GetShaderModules())
	{
		VkPipelineShaderStageCreateInfo stageCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		stageCreateInfo.stage = shaderModule->GetStage();
		stageCreateInfo.pName = shaderModule->GetEntryPoint().c_str();

		// Create the Vulkan handle
		VkShaderModuleCreateInfo moduleCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		moduleCreateInfo.codeSize = shaderModule->GetBinary().size() * sizeof(u32);
		moduleCreateInfo.pCode = shaderModule->GetBinary().data();

		VK_ENSURE(vkCreateShaderModule(device.GetHandle(), &moduleCreateInfo, nullptr, &stageCreateInfo.module));

		//device.get_debug_utils().set_debug_name(device.GetHandle(),
		//	VK_OBJECT_TYPE_SHADER_MODULE, reinterpret_cast<uint64_t>(stageCreateInfo.module),
		//	shaderModule->get_debug_name().c_str());

		stageCreateInfo.pSpecializationInfo = &specializationInfo;

		stageCreateInfos.push_back(stageCreateInfo);
		shaderModules.push_back(stageCreateInfo.module);
	}

	VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	createInfo.stageCount = ToU32(stageCreateInfos.size());
	createInfo.pStages = stageCreateInfos.data();

	VkPipelineVertexInputStateCreateInfo vertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputState.pVertexAttributeDescriptions = pipelineState.GetVertexInputState().Attributes.data();
	vertexInputState.vertexAttributeDescriptionCount = ToU32(pipelineState.GetVertexInputState().Attributes.size());
	vertexInputState.pVertexBindingDescriptions = pipelineState.GetVertexInputState().Bindings.data();
	vertexInputState.vertexBindingDescriptionCount = ToU32(pipelineState.GetVertexInputState().Bindings.size());

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssemblyState.topology = pipelineState.GetInputAssemblyState().Topology;
	inputAssemblyState.primitiveRestartEnable = pipelineState.GetInputAssemblyState().PrimitiveRestartEnable;

	VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = pipelineState.GetViewportState().ViewportCount;
	viewportState.scissorCount = pipelineState.GetViewportState().ScissorCount;

	VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.depthClampEnable = pipelineState.GetRasterizationState().DepthClampEnable;
	rasterizationState.rasterizerDiscardEnable = pipelineState.GetRasterizationState().RasterizerDiscardEnable;
	rasterizationState.polygonMode = pipelineState.GetRasterizationState().PolygonMode;
	rasterizationState.cullMode = pipelineState.GetRasterizationState().CullMode;
	rasterizationState.frontFace = pipelineState.GetRasterizationState().FrontFace;
	rasterizationState.depthBiasEnable = pipelineState.GetRasterizationState().DepthBiasEnable;
	rasterizationState.depthBiasClamp = 1.0f;
	rasterizationState.depthBiasSlopeFactor = 1.0f;
	rasterizationState.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisample_state = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisample_state.sampleShadingEnable = pipelineState.GetMultisampleState().SampleShadingEnable;
	multisample_state.rasterizationSamples = pipelineState.GetMultisampleState().RasterizationSamples;
	multisample_state.minSampleShading = pipelineState.GetMultisampleState().MinSampleShading;
	multisample_state.alphaToCoverageEnable = pipelineState.GetMultisampleState().AlphaToCoverageEnable;
	multisample_state.alphaToOneEnable = pipelineState.GetMultisampleState().AlphaToOneEnable;

	if (pipelineState.GetMultisampleState().SampleMask)
	{
		multisample_state.pSampleMask = &pipelineState.GetMultisampleState().SampleMask;
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	depthStencilState.depthTestEnable = pipelineState.GetDepthStencilState().DepthTestEnable;
	depthStencilState.depthWriteEnable = pipelineState.GetDepthStencilState().DepthWriteEnable;
	depthStencilState.depthCompareOp = pipelineState.GetDepthStencilState().DepthCompareOp;
	depthStencilState.depthBoundsTestEnable = pipelineState.GetDepthStencilState().DepthBoundsTestEnable;
	depthStencilState.stencilTestEnable = pipelineState.GetDepthStencilState().StencilTestEnable;
	depthStencilState.front.failOp = pipelineState.GetDepthStencilState().Front.FailOp;
	depthStencilState.front.passOp = pipelineState.GetDepthStencilState().Front.PassOp;
	depthStencilState.front.depthFailOp = pipelineState.GetDepthStencilState().Front.DepthFailOp;
	depthStencilState.front.compareOp = pipelineState.GetDepthStencilState().Front.CompareOp;
	depthStencilState.front.compareMask = ~0U;
	depthStencilState.front.writeMask = ~0U;
	depthStencilState.front.reference = ~0U;
	depthStencilState.back.failOp = pipelineState.GetDepthStencilState().Back.FailOp;
	depthStencilState.back.passOp = pipelineState.GetDepthStencilState().Back.PassOp;
	depthStencilState.back.depthFailOp = pipelineState.GetDepthStencilState().Back.DepthFailOp;
	depthStencilState.back.compareOp = pipelineState.GetDepthStencilState().Back.CompareOp;
	depthStencilState.back.compareMask = ~0U;
	depthStencilState.back.writeMask = ~0U;
	depthStencilState.back.reference = ~0U;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.logicOpEnable = pipelineState.GetColorBlendState().LogicOpEnable;
	colorBlendState.logicOp = pipelineState.GetColorBlendState().LogicOp;
	colorBlendState.attachmentCount = ToU32(pipelineState.GetColorBlendState().Attachments.size());
	colorBlendState.pAttachments = reinterpret_cast<const VkPipelineColorBlendAttachmentState*>(pipelineState.GetColorBlendState().Attachments.data());
	colorBlendState.blendConstants[0] = 1.0f;
	colorBlendState.blendConstants[1] = 1.0f;
	colorBlendState.blendConstants[2] = 1.0f;
	colorBlendState.blendConstants[3] = 1.0f;

	std::array<VkDynamicState, 9> dynamicStates = 
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH,
		VK_DYNAMIC_STATE_DEPTH_BIAS,
		VK_DYNAMIC_STATE_BLEND_CONSTANTS,
		VK_DYNAMIC_STATE_DEPTH_BOUNDS,
		VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
		VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
		VK_DYNAMIC_STATE_STENCIL_REFERENCE,
	};

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.pDynamicStates = dynamicStates.data();
	dynamicState.dynamicStateCount = ToU32(dynamicStates.size());

	createInfo.pVertexInputState = &vertexInputState;
	createInfo.pInputAssemblyState = &inputAssemblyState;
	createInfo.pViewportState = &viewportState;
	createInfo.pRasterizationState = &rasterizationState;
	createInfo.pMultisampleState = &multisample_state;
	createInfo.pDepthStencilState = &depthStencilState;
	createInfo.pColorBlendState = &colorBlendState;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = pipelineState.GetPipelineLayout().GetHandle();
	createInfo.renderPass = pipelineState.GetRenderPass()->GetHandle();
	createInfo.subpass = pipelineState.GetSubpassIndex();

	VK_ENSURE(vkCreateGraphicsPipelines(device.GetHandle(), pipelineCache, 1, &createInfo, nullptr, &_Handle));

	for (auto shaderModule : shaderModules)
	{
		vkDestroyShaderModule(device.GetHandle(), shaderModule, nullptr);
	}

	_State = pipelineState;
}
