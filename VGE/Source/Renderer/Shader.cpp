#include "Shader.h"
#include "Device.h"

const vge::c8* const vge::Shader::DefaultEntryName = "main";

VkShaderStageFlagBits vge::Shader::GetFlagsFromStage(ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::Vertex:
		return VK_SHADER_STAGE_VERTEX_BIT;

	case ShaderStage::Fragment:
		return VK_SHADER_STAGE_FRAGMENT_BIT;

	default:
		return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}
}

vge::ShaderStage vge::Shader::GetStageFromFlags(VkShaderStageFlagBits flags)
{
	if (flags & VK_SHADER_STAGE_VERTEX_BIT)
	{
		return ShaderStage::Vertex;
	}
	else if (flags & VK_SHADER_STAGE_FRAGMENT_BIT)
	{
		return ShaderStage::Fragment;
	}
	else
	{
		return ShaderStage::None;
	}
}

void vge::Shader::Initialize(const ShaderCreateInfo& data)
{
	m_Device = data.Device;
	m_StageFlags = data.StageFlags;
	m_Stage = GetStageFromFlags(data.StageFlags);
	
	CreateModule(data.SpirvChar);

	if (data.DescriptorSetLayoutBindings.size() > 0)
	{
#if DEBUG
		VerifyBindingIndices(data.DescriptorSetLayoutBindings);
#endif

		CreateDescriptorSetLayout(data.DescriptorSetLayoutBindings);
	}
}

void vge::Shader::Destroy()
{
	vkDestroyShaderModule(m_Device->GetHandle(), m_Module, nullptr);
	vkDestroyDescriptorSetLayout(m_Device->GetHandle(), m_DescriptorLayout.Handle, nullptr);
}

VkPipelineShaderStageCreateInfo vge::Shader::GetStageCreateInfo() const
{
	VkPipelineShaderStageCreateInfo stageCreateInfo = {};
	stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageCreateInfo.stage = m_StageFlags;
	stageCreateInfo.module = m_Module;
	stageCreateInfo.pName = DefaultEntryName;

	return stageCreateInfo;
}

void vge::Shader::CreateModule(const std::vector<c8>* SpirvInt8)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = SpirvInt8->size();
	createInfo.pCode = reinterpret_cast<const u32*>(SpirvInt8->data());

	VK_ENSURE(vkCreateShaderModule(m_Device->GetHandle(), &createInfo, nullptr, &m_Module));
}

void vge::Shader::CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	m_DescriptorLayout.Bindings = bindings;

	VkDescriptorSetLayoutCreateInfo uniformLayoutCreateInfo = {};
	uniformLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	uniformLayoutCreateInfo.bindingCount = static_cast<u32>(bindings.size());
	uniformLayoutCreateInfo.pBindings = bindings.data();

	VK_ENSURE(vkCreateDescriptorSetLayout(m_Device->GetHandle(), &uniformLayoutCreateInfo, nullptr, &m_DescriptorLayout.Handle))
}

#if DEBUG
void vge::Shader::VerifyBindingIndices(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	std::vector<u32> bindingIndices;
	for (const VkDescriptorSetLayoutBinding& binding : bindings)
	{
		bindingIndices.push_back(binding.binding);
	}

	std::unordered_set<u32> uniqueBindingIndices(std::cbegin(bindingIndices), std::cend(bindingIndices));
	ENSURE_MSG(uniqueBindingIndices.size() == bindingIndices.size(), "Descriptor set layout bindings must have unique indices.");
}
#endif
