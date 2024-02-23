#include "ResourceReplay.h"
#include "Core/Helpers.h"
#include "Core/ResourceCache.h"

namespace vge
{
namespace
{
inline void ReadSubpassInfo(std::istringstream& is, std::vector<SubpassInfo>& value)
{
	std::size_t size;
	Read(is, size);
	value.resize(size);
	for (SubpassInfo& subpass : value)
	{
		Read(is, subpass.InputAttachments);
		Read(is, subpass.OutputAttachments);
	}
}

inline void ReadProcesses(std::istringstream& is, std::vector<std::string>& value)
{
	std::size_t size;
	Read(is, size);
	value.resize(size);
	for (std::string& item : value)
	{
		Read(is, item);
	}
}
}	// namespace
}	// namespace vge

vge::ResourceReplay::ResourceReplay()
{
	_StreamResources[ResourceType::ShaderModule] = std::bind(&ResourceReplay::CreateShaderModule, this, std::placeholders::_1, std::placeholders::_2);
	_StreamResources[ResourceType::PipelineLayout] = std::bind(&ResourceReplay::CreatePipelineLayout, this, std::placeholders::_1, std::placeholders::_2);
	_StreamResources[ResourceType::RenderPass] = std::bind(&ResourceReplay::CreateRenderPass, this, std::placeholders::_1, std::placeholders::_2);
	_StreamResources[ResourceType::GraphicsPipeline] = std::bind(&ResourceReplay::CreateGraphicsPipeline, this, std::placeholders::_1, std::placeholders::_2);
}

void vge::ResourceReplay::Play(ResourceCache& resourceCache, ResourceRecord& recorder)
{
	std::istringstream stream = std::istringstream(recorder.GetStream().str());

	while (true)
	{
		// Read command id.
		ResourceType resourceType;
		Read(stream, resourceType);

		if (stream.eof())
		{
			break;
		}

		// Find command function for the given command id.
		auto cmdIt = _StreamResources.find(resourceType);

		// Check if command replayer supports the given command.
		if (cmdIt != _StreamResources.end())
		{
			// Run command function.
			cmdIt->second(resourceCache, stream);
		}
		else
		{
			LOG(Error, "Replay command not supported.");
		}
	}
}

void vge::ResourceReplay::CreateShaderModule(ResourceCache& resourceCache, std::istringstream& stream)
{
	VkShaderStageFlagBits stage;
	std::string glslSource;
	std::string entryPoint;
	std::string preamble;
	std::vector<std::string> processes;

	Read(stream, stage, glslSource, entryPoint, preamble);

	ReadProcesses(stream, processes);

	ShaderSource shaderSource;
	shaderSource.SetSource(std::move(glslSource));
	
	ShaderVariant shaderVariant(std::move(preamble), std::move(processes));

	auto& shaderModule = resourceCache.RequestShaderModule(stage, shaderSource, shaderVariant);

	_ShaderModules.push_back(&shaderModule);
}

void vge::ResourceReplay::CreatePipelineLayout(ResourceCache& resourceCache, std::istringstream& stream)
{
	std::vector<size_t> shaderIndices;

	Read(stream, shaderIndices);

	std::vector<ShaderModule*> shaderStages(shaderIndices.size());
	std::transform(shaderIndices.begin(), shaderIndices.end(), shaderStages.begin(),
		[&](size_t shaderIndex) 
		{
			ASSERT(shaderIndex < _ShaderModules.size());
			return _ShaderModules[shaderIndex];
		});

	auto& pipelineLayout = resourceCache.RequestPipelineLayout(shaderStages);

	_PipelineLayouts.push_back(&pipelineLayout);
}

void vge::ResourceReplay::CreateRenderPass(ResourceCache& resourceCache, std::istringstream& stream)
{
	std::vector<Attachment> attachments;
	std::vector<LoadStoreInfo> loadStoreInfos;
	std::vector<SubpassInfo> subpasses;

	Read(stream, attachments, loadStoreInfos);
	ReadSubpassInfo(stream, subpasses);

	auto& renderPass = resourceCache.RequestRenderPass(attachments, loadStoreInfos, subpasses);

	_RenderPasses.push_back(&renderPass);
}

void vge::ResourceReplay::CreateGraphicsPipeline(ResourceCache& resourceCache, std::istringstream& stream)
{
	size_t pipelineLayoutIndex = 0;
	size_t renderPassIndex = 0;
	u32 subpassIndex = 0;
	Read(stream, pipelineLayoutIndex, renderPassIndex, subpassIndex);

	std::map<u32, std::vector<uint8_t>> specializationConstantState;
	Read(stream, specializationConstantState);

	VertexInputState vertexInputState;
	Read(stream, vertexInputState.Attributes, vertexInputState.Bindings);

	InputAssemblyState inputAssemblyState;
	RasterizationState rasterizationState;
	ViewportState viewportState;
	MultisampleState multisampleState;
	DepthStencilState depthStencilState;
	Read(stream, inputAssemblyState, rasterizationState, viewportState, multisampleState, depthStencilState);

	ColorBlendState colorBlendState;
	Read(stream, colorBlendState.LogicOp, colorBlendState.LogicOpEnable, colorBlendState.Attachments);

	PipelineState pipelineState;
	ASSERT(pipelineLayoutIndex < _PipelineLayouts.size());
	pipelineState.SetPipelineLayout(*_PipelineLayouts[pipelineLayoutIndex]);
	ASSERT(renderPassIndex < _RenderPasses.size());
	pipelineState.SetRenderPass(*_RenderPasses[renderPassIndex]);

	for (auto& item : specializationConstantState)
	{
		pipelineState.SetSpecializationConstant(item.first, item.second);
	}

	pipelineState.SetSubpassIndex(subpassIndex);
	pipelineState.SetVertexInputState(vertexInputState);
	pipelineState.SetInputAssemblyState(inputAssemblyState);
	pipelineState.SetRasterizationState(rasterizationState);
	pipelineState.SetViewportState(viewportState);
	pipelineState.SetMultisampleState(multisampleState);
	pipelineState.SetDepthStencilState(depthStencilState);
	pipelineState.SetColorBlendState(colorBlendState);

	auto& graphics_pipeline = resourceCache.RequestGraphicsPipeline(pipelineState);

	_GraphicsPipelines.push_back(&graphics_pipeline);
}
