#include "PipelineLayout.h"
#include "DescriptorSetLayout.h"
#include "Device.h"

vge::PipelineLayout::PipelineLayout(Device& device, const std::vector<ShaderModule*>& shaderModules)
	: _Device(device), _ShaderModules(shaderModules)
{
	// Collect and combine all the shader resources from each of the shader modules.
	// Collate them all into a map that is indexed by the name of the resource.
	for (auto* shaderModule : shaderModules)
	{
		for (const auto& shaderResource : shaderModule->GetResources())
		{
			std::string key = shaderResource.Name;

			// Since 'Input' and 'Output' resources can have the same name, we modify the key string.
			if (shaderResource.Type == ShaderResourceType::Input || shaderResource.Type == ShaderResourceType::Output)
			{
				key = std::to_string(shaderResource.Stages) + "_" + key;
			}

			auto it = _ShaderResources.find(key);

			if (it != _ShaderResources.end())
			{
				// Append stage flags if resource already exists.
				it->second.Stages |= shaderResource.Stages;
			}
			else
			{
				// Create a new entry in the map.
				_ShaderResources.emplace(key, shaderResource);
			}
		}
	}

	// Sift through the map of name indexed shader resources.
	// Separate them into their respective sets.
	for (auto& it : _ShaderResources)
	{
		auto& shaderResource = it.second;

		// Find binding by set index in the map.
		auto it2 = _ShaderSets.find(shaderResource.Set);

		if (it2 != _ShaderSets.end())
		{
			// Add resource to the found set index.
			it2->second.push_back(shaderResource);
		}
		else
		{
			// Create a new set index and with the first resource.
			_ShaderSets.emplace(shaderResource.Set, std::vector<ShaderResource>{shaderResource});
		}
	}

	// Create a descriptor set layout for each shader set in the shader modules.
	for (auto& shaderSetIt : _ShaderSets)
	{
		_DescriptorSetLayouts.emplace_back(&device.GetResourceCache().RequestDescriptorSetLayout(shaderSetIt.first, _ShaderModules, shaderSetIt.second));
	}

	// Collect all the descriptor set layout handles, maintaining set order.
	std::vector<VkDescriptorSetLayout> descriptorSetLayoutHandles;
	for (u32 i = 0; i < _DescriptorSetLayouts.size(); ++i)
	{
		if (_DescriptorSetLayouts[i])
		{
			descriptorSetLayoutHandles.push_back(_DescriptorSetLayouts[i]->GetHandle());
		}
		else
		{
			descriptorSetLayoutHandles.push_back(VK_NULL_HANDLE);
		}
	}

	// Collect all the push constant shader resources.
	std::vector<VkPushConstantRange> pushConstantRanges;
	for (auto& pushConstantResource : GetResources(ShaderResourceType::PushConstant))
	{
		pushConstantRanges.push_back({ pushConstantResource.Stages, pushConstantResource.Offset, pushConstantResource.Size });
	}

	VkPipelineLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	createInfo.setLayoutCount = ToU32(descriptorSetLayoutHandles.size());
	createInfo.pSetLayouts = descriptorSetLayoutHandles.data();
	createInfo.pushConstantRangeCount = ToU32(pushConstantRanges.size());
	createInfo.pPushConstantRanges = pushConstantRanges.data();

	// Create the Vulkan pipeline layout handle.
	VK_ENSURE(vkCreatePipelineLayout(device.GetHandle(), &createInfo, nullptr, &_Handle));
}

vge::PipelineLayout::PipelineLayout(vge::PipelineLayout&& other) 
	: _Device(other._Device),
	_Handle(other._Handle),
	_ShaderModules(std::move(other._ShaderModules)),
	_ShaderResources(std::move(other._ShaderResources)),
	_ShaderSets(std::move(other._ShaderSets)),
	_DescriptorSetLayouts(std::move(other._DescriptorSetLayouts))
{
	other._Handle = VK_NULL_HANDLE;
}

vge::PipelineLayout::~PipelineLayout()
{
	if (_Handle)
	{
		vkDestroyPipelineLayout(_Device.GetHandle(), _Handle, nullptr);
	}
}

const std::vector<vge::ShaderResource> vge::PipelineLayout::GetResources(const ShaderResourceType& type, VkShaderStageFlagBits stage) const
{
	std::vector<ShaderResource> foundResources;

	for (auto& it : _ShaderResources)
	{
		auto& shaderResource = it.second;

		if (shaderResource.Type == type || type == ShaderResourceType::All)
		{
			if (shaderResource.Stages == stage || stage == VK_SHADER_STAGE_ALL)
			{
				foundResources.push_back(shaderResource);
			}
		}
	}

	return foundResources;
}

vge::DescriptorSetLayout& vge::PipelineLayout::GetDescriptorSetLayout(const u32 setIndex) const
{
	for (auto& descriptorSetLayout : _DescriptorSetLayouts)
	{
		if (descriptorSetLayout->GetIndex() == setIndex)
		{
			return *descriptorSetLayout;
		}
	}
	ENSURE_MSG(false, "Couldn't find descriptor set layout at set index %d.", setIndex);
}

VkShaderStageFlags vge::PipelineLayout::GetPushConstantRangeStage(u32 size, u32 offset) const
{
	VkShaderStageFlags stages = 0;

	for (auto& pushConstantResource : GetResources(ShaderResourceType::PushConstant))
	{
		if (offset >= pushConstantResource.Offset && offset + size <= pushConstantResource.Offset + pushConstantResource.Size)
		{
			stages |= pushConstantResource.Stages;
		}
	}
	return stages;
}