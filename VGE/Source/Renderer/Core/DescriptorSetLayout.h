#pragma once

#include "Core/Common.h"

namespace vge
{
class Device;
class ShaderModule;

struct ShaderResource;

 // Caches DescriptorSet objects for the shader's set index.
class DescriptorSetLayout
{
public:
	DescriptorSetLayout(
		Device& device,
		const u32 setIndex,
		const std::vector<ShaderModule*>& shaderModules,
		const std::vector<ShaderResource>& resourceSet);

	COPY_CTOR_DEL(DescriptorSetLayout);
	DescriptorSetLayout(DescriptorSetLayout&& other);

	~DescriptorSetLayout();

	COPY_OP_DEL(DescriptorSetLayout);
	MOVE_OP_DEL(DescriptorSetLayout);

public:
	inline VkDescriptorSetLayout GetHandle() const { return _Handle; }
	inline const u32 GetIndex() const { return _SetIndex; }
	inline const std::vector<VkDescriptorSetLayoutBinding>& GetBindings() const { return _Bindings; }
	inline const std::vector<ShaderModule*>& GetShaderModules() const { return _ShaderModules; }
	inline const std::vector<VkDescriptorBindingFlagsEXT>& GetBindingFlags() const { return _BindingFlags; }

	std::unique_ptr<VkDescriptorSetLayoutBinding> GetLayoutBinding(const u32 binding_index) const;
	std::unique_ptr<VkDescriptorSetLayoutBinding> GetLayoutBinding(const std::string& name) const;
	VkDescriptorBindingFlagsEXT GetLayoutBindingFlag(const u32 binding_index) const;

private:
	Device& _Device;
	VkDescriptorSetLayout _Handle = VK_NULL_HANDLE;
	const u32 _SetIndex;
	std::vector<VkDescriptorSetLayoutBinding> _Bindings;
	std::vector<VkDescriptorBindingFlagsEXT> _BindingFlags;
	std::unordered_map<u32, VkDescriptorSetLayoutBinding> _BindingsLookup;
	std::unordered_map<u32, VkDescriptorBindingFlagsEXT> _BindingFlagsLookup;
	std::unordered_map<std::string, u32> _ResourcesLookup;
	std::vector<ShaderModule*> _ShaderModules;
};
}	// namespace vge
