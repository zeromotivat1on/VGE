#pragma once

#include "Core/Common.h"
#include "Core/ShaderModule.h"

namespace vge
{
class Device;
class ShaderModule;
class DescriptorSetLayout;

class PipelineLayout
{
public:
	PipelineLayout(Device& device, const std::vector<ShaderModule*>& shaderModules);

	COPY_CTOR_DEL(PipelineLayout);
	PipelineLayout(PipelineLayout&& other);

	~PipelineLayout();

	COPY_OP_DEL(PipelineLayout);
	MOVE_OP_DEL(PipelineLayout);

	inline VkPipelineLayout GetHandle() const { return _Handle; }
	inline const std::vector<ShaderModule*>& GetShaderModules() const { return _ShaderModules; }
	inline const std::unordered_map<u32, std::vector<ShaderResource>>& GetShaderSets() const { return _ShaderSets; }
	inline bool HasDescriptorSetLayout(const u32 setIndex) const { return setIndex < _DescriptorSetLayouts.size(); }

	const std::vector<ShaderResource> GetResources(const ShaderResourceType& type = ShaderResourceType::All, VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL) const;
	DescriptorSetLayout& GetDescriptorSetLayout(const u32 setIndex) const;
	VkShaderStageFlags GetPushConstantRangeStage(u32 size, u32 offset = 0) const;

private:
	Device& _Device;
	VkPipelineLayout _Handle = VK_NULL_HANDLE;
	std::vector<ShaderModule*> _ShaderModules; // shader modules that this pipeline layout uses
	std::unordered_map<std::string, ShaderResource> _ShaderResources; // shader resources that this pipeline layout uses, indexed by their name
	std::unordered_map<u32, std::vector<ShaderResource>> _ShaderSets; // map of each set and the resources it owns used by the pipeline layout
	std::vector<DescriptorSetLayout*> _DescriptorSetLayouts; // different descriptor set layouts for this pipeline layout
};
}	// namespace vge
