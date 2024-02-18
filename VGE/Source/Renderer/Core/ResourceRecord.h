#pragma once

#include "Core/Common.h"
#include "Core/PipelineState.h"

namespace vge
{
class GraphicsPipeline;
class PipelineLayout;
class RenderPass;
class ShaderModule;

enum class ResourceType : u8
{
	ShaderModule,
	PipelineLayout,
	RenderPass,
	GraphicsPipeline
};

// Writes Vulkan objects in a memory stream.
class ResourceRecord
{
public:
	inline std::vector<u8> GetData() { std::string str = _Stream.str(); return std::vector<u8>(str.begin(), str.end()); }
	inline const std::ostringstream& GetStream() { return _Stream; }

	inline void SetData(const std::vector<u8>& data) { _Stream.str(std::string(data.begin(), data.end())); }
	inline void SetShaderModule(size_t index, const ShaderModule& shaderModule) { _ShaderModuleToIndex[&shaderModule] = index; }
	inline void SetPipelineLayout(size_t index, const PipelineLayout& pipelineLayout) { _PipelineLayoutToIndex[&pipelineLayout] = index; }
	inline void SetRenderPass(size_t index, const RenderPass& renderPass) { _RenderPassToIndex[&renderPass] = index; }
	inline void SetGraphicsPipeline(size_t index, const GraphicsPipeline& graphicsPipeline) { _GraphicsPipelineToIndex[&graphicsPipeline] = index; }

	size_t RegisterShaderModule(
		VkShaderStageFlagBits stage,
		const ShaderSource& glslSource,
		const std::string& entryPoint,
		const ShaderVariant& shaderVariant);

	size_t RegisterPipelineLayout(const std::vector<ShaderModule*>& shaderModules);

	size_t RegisterRenderPass(
		const std::vector<Attachment>& attachments,
		const std::vector<LoadStoreInfo>& loadStoreInfos,
		const std::vector<SubpassInfo>& subpasses);

	size_t RegisterGraphicsPipeline(VkPipelineCache pipelineCache, PipelineState& pipelineState);

private:
	std::ostringstream _Stream;
	std::vector<size_t> _ShaderModuleIndices;
	std::vector<size_t> _PipelineLayoutIndices;
	std::vector<size_t> _RenderPassIndices;
	std::vector<size_t> _GraphicsPipelineIndices;
	std::unordered_map<const ShaderModule*, size_t> _ShaderModuleToIndex;
	std::unordered_map<const PipelineLayout*, size_t> _PipelineLayoutToIndex;
	std::unordered_map<const RenderPass*, size_t> _RenderPassToIndex;
	std::unordered_map<const GraphicsPipeline*, size_t> _GraphicsPipelineToIndex;
};
}	// namespace vge
