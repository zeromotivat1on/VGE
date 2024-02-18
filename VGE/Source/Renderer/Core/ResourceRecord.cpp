#include "ResourceRecord.h"

namespace vge
{
namespace
{
inline void WriteSubpassInfo(std::ostringstream& os, const std::vector<SubpassInfo>& value)
{
	Write(os, value.size());
	for (const SubpassInfo& item : value)
	{
		Write(os, item.InputAttachments);
		Write(os, item.OutputAttachments);
	}
}

inline void WriteProcesses(std::ostringstream& os, const std::vector<std::string>& value)
{
	Write(os, value.size());
	for (const std::string& item : value)
	{
		Write(os, item);
	}
}
}	// namespace
}	// namespace vge

size_t vge::ResourceRecord::RegisterShaderModule(VkShaderStageFlagBits stage, const ShaderSource& glslSource, const std::string& entryPoint, const ShaderVariant& shaderVariant)
{
	_ShaderModuleIndices.push_back(_ShaderModuleIndices.size());

	Write(_Stream, ResourceType::ShaderModule, stage, glslSource.GetSource(), entryPoint, shaderVariant.GetPreamble());
	WriteProcesses(_Stream, shaderVariant.GetProcesses());

	return _ShaderModuleIndices.back();
}

size_t vge::ResourceRecord::RegisterPipelineLayout(const std::vector<ShaderModule*>& shaderModules)
{
	_PipelineLayoutIndices.push_back(_PipelineLayoutIndices.size());

	std::vector<size_t> shaderIndices(shaderModules.size());
	std::transform(shaderModules.begin(), shaderModules.end(), shaderIndices.begin(),
		[this](ShaderModule* shader_module) { return _ShaderModuleToIndex.at(shader_module); });

	Write(_Stream, ResourceType::PipelineLayout, shaderIndices);

	return _PipelineLayoutIndices.back();
}

size_t vge::ResourceRecord::RegisterRenderPass(const std::vector<Attachment>& attachments, const std::vector<LoadStoreInfo>& loadStoreInfos, const std::vector<SubpassInfo>& subpasses)
{
	_RenderPassIndices.push_back(_RenderPassIndices.size());

	Write(_Stream, ResourceType::RenderPass, attachments, loadStoreInfos);
	WriteSubpassInfo(_Stream, subpasses);

	return _RenderPassIndices.back();
}

size_t vge::ResourceRecord::RegisterGraphicsPipeline(VkPipelineCache /*pipeline_cache*/, PipelineState& pipelineState)
{
	_GraphicsPipelineIndices.push_back(_GraphicsPipelineIndices.size());

	auto& pipelineLayout = pipelineState.GetPipelineLayout();
	auto renderPass = pipelineState.GetRenderPass();
	Write(
		_Stream, 
		ResourceType::GraphicsPipeline, 
		_PipelineLayoutToIndex.at(&pipelineLayout), 
		_RenderPassToIndex.at(renderPass), 
		pipelineState.GetSubpassIndex());

	auto& specializationConstantState = pipelineState.GetSpecializationConstantState().GetSpecializationConstantState();
	Write(_Stream, specializationConstantState);

	auto& vertexInputState = pipelineState.GetVertexInputState();
	Write(_Stream, vertexInputState.Attributes, vertexInputState.Bindings);

	Write(
		_Stream,
		pipelineState.GetInputAssemblyState(),
		pipelineState.GetRasterizationState(),
		pipelineState.GetViewportState(),
		pipelineState.GetMultisampleState(),
		pipelineState.GetDepthStencilState());

	auto& colorBlendState = pipelineState.GetColorBlendState();
	Write(_Stream, colorBlendState.LogicOp, colorBlendState.LogicOpEnable, colorBlendState.Attachments);

	return _GraphicsPipelineIndices.back();
}