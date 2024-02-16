#include "DescriptorSetLayout.h"
#include "PhysicalDevice.h"
#include "ShaderModule.h"
#include "Device.h"

namespace vge
{
namespace
{
inline VkDescriptorType FindDescriptorType(ShaderResourceType resourceType, bool dynamic)
{
	switch (resourceType)
	{
	case ShaderResourceType::InputAttachment:
		return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		break;
	case ShaderResourceType::Image:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		break;
	case ShaderResourceType::ImageSampler:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		break;
	case ShaderResourceType::ImageStorage:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		break;
	case ShaderResourceType::Sampler:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
		break;
	case ShaderResourceType::BufferUniform:
		if (dynamic)
		{
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		}
		else
		{
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
		break;
	case ShaderResourceType::BufferStorage:
		if (dynamic)
		{
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		}
		else
		{
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}
		break;
	default:
		ENSURE_MSG(false, "No conversion possible for the shader resource type.");
		break;
	}
}

inline bool ValidateBinding(const VkDescriptorSetLayoutBinding& binding, const std::vector<VkDescriptorType>& blacklist)
{
	return !(std::find_if(blacklist.begin(), blacklist.end(), [binding](const VkDescriptorType& type) { return type == binding.descriptorType; }) != blacklist.end());
}

inline bool ValidateFlags(const PhysicalDevice& gpu, const std::vector<VkDescriptorSetLayoutBinding>& bindings, const std::vector<VkDescriptorBindingFlagsEXT>& flags)
{
	// Assume bindings are valid if there are no flags.
	if (flags.empty())
	{
		return true;
	}

	// Binding count has to equal flag count as its a 1:1 mapping.
	if (bindings.size() != flags.size())
	{
		LOG(Error, "Binding count has to be equal to flag count.");
		return false;
	}

	return true;
}
}	// namespace
}	// namespace vge

vge::DescriptorSetLayout::DescriptorSetLayout(
	Device& device,
	const u32 setIndex,
	const std::vector<ShaderModule*>& shaderModules,
	const std::vector<ShaderResource>& resourceSet) 
	: _Device(device), _SetIndex(setIndex), _ShaderModules(shaderModules)
{
	// NOTE: `shaderModules` is passed in mainly for hashing their handles in `request_resource`.
	//        This way, different pipelines (with different shaders / shader variants) will get
	//        different descriptor set layouts (incl. appropriate name -> binding lookups).

	for (const auto& resource : resourceSet)
	{
		// Skip shader resources whitout a binding point.
		if (resource.Type == ShaderResourceType::Input ||
			resource.Type == ShaderResourceType::Output ||
			resource.Type == ShaderResourceType::PushConstant ||
			resource.Type == ShaderResourceType::SpecializationConstant)
		{
			continue;
		}

		// Convert from ShaderResourceType to VkDescriptorType.
		const auto descriptorType = FindDescriptorType(resource.Type, resource.Mode == ShaderResourceMode::Dynamic);

		if (resource.Mode == ShaderResourceMode::UpdateAfterBind)
		{
			_BindingFlags.push_back(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT);
		}
		else
		{
			// When creating a descriptor set layout, if we give a structure to createInfo.pNext, each binding needs to have a binding flag
			// (pBindings[i] uses the flags in pBindingFlags[i])
			// Adding 0 ensures the bindings that dont use any flags are mapped correctly.
			_BindingFlags.push_back(0);
		}

		// Convert ShaderResource to Vkvge::DescriptorSetLayoutBinding.
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = resource.Binding;
		layoutBinding.descriptorCount = resource.ArraySize;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.stageFlags = static_cast<VkShaderStageFlags>(resource.Stages);

		_Bindings.push_back(layoutBinding);

		// Store mapping between binding and the binding point.
		_BindingsLookup.emplace(resource.Binding, layoutBinding);
		_BindingFlagsLookup.emplace(resource.Binding, _BindingFlags.back());
		_ResourcesLookup.emplace(resource.Name, resource.Binding);
	}

	VkDescriptorSetLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	createInfo.flags = 0;
	createInfo.bindingCount = ToU32(_Bindings.size());
	createInfo.pBindings = _Bindings.data();

	// Handle update-after-bind extensions.
	if (std::find_if(resourceSet.begin(), resourceSet.end(),
		[](const ShaderResource& shader_resource) { return shader_resource.Mode == ShaderResourceMode::UpdateAfterBind; }) != resourceSet.end())
	{
		// Spec states you can't have ANY dynamic resources if you have one of the bindings set to update-after-bind.
		ENSURE_MSG(std::find_if(resourceSet.begin(), resourceSet.end(),
			[](const ShaderResource& shader_resource) { return shader_resource.Mode == ShaderResourceMode::Dynamic; }) == resourceSet.end(),
			"Cannot create descriptor set layout, dynamic resources are not allowed if at least one resource is update-after-bind.");

		ENSURE_MSG(ValidateFlags(device.GetGpu(), _Bindings, _BindingFlags), "Invalid binding, couldn't create descriptor set layout.");

		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlagsCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT };
		bindingFlagsCreateInfo.bindingCount = ToU32(_BindingFlags.size());
		bindingFlagsCreateInfo.pBindingFlags = _BindingFlags.data();

		createInfo.pNext = &bindingFlagsCreateInfo;
		createInfo.flags |= std::find(_BindingFlags.begin(), _BindingFlags.end(), VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT) != _BindingFlags.end() ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT : 0;
	}

	// Create the Vulkan descriptor set layout handle.
	VK_ENSURE(vkCreateDescriptorSetLayout(device.GetHandle(), &createInfo, nullptr, &_Handle));
}

vge::DescriptorSetLayout::DescriptorSetLayout(vge::DescriptorSetLayout&& other) 
	: _Device(other._Device),
	_ShaderModules(other._ShaderModules),
	_Handle(other._Handle),
	_SetIndex(other._SetIndex),
	_Bindings(std::move(other._Bindings)),
	_BindingFlags(std::move(other._BindingFlags)),
	_BindingsLookup(std::move(other._BindingsLookup)),
	_BindingFlagsLookup(std::move(other._BindingFlagsLookup)),
	_ResourcesLookup(std::move(other._ResourcesLookup))
{
	other._Handle = VK_NULL_HANDLE;
}

vge::DescriptorSetLayout::~DescriptorSetLayout()
{
	if (_Handle)
	{
		vkDestroyDescriptorSetLayout(_Device.GetHandle(), _Handle, nullptr);
	}
}

std::unique_ptr<VkDescriptorSetLayoutBinding> vge::DescriptorSetLayout::GetLayoutBinding(u32 bindingIndex) const
{
	auto it = _BindingsLookup.find(bindingIndex);

	if (it == _BindingsLookup.end())
	{
		return nullptr;
	}

	return std::make_unique<VkDescriptorSetLayoutBinding>(it->second);
}

std::unique_ptr<VkDescriptorSetLayoutBinding> vge::DescriptorSetLayout::GetLayoutBinding(const std::string& name) const
{
	auto it = _ResourcesLookup.find(name);

	if (it == _ResourcesLookup.end())
	{
		return nullptr;
	}

	return GetLayoutBinding(it->second);
}

VkDescriptorBindingFlagsEXT vge::DescriptorSetLayout::GetLayoutBindingFlag(const u32 bindingIndex) const
{
	auto it = _BindingFlagsLookup.find(bindingIndex);

	if (it == _BindingFlagsLookup.end())
	{
		return 0;
	}

	return it->second;
}
