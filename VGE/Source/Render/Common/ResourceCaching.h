#pragma once

#include "..\Core\VkCommon.h"
#include "Core/Helpers.h"
#include "Core/Pipeline.h"
#include "Core/Framebuffer.h"
#include "Core/RenderTarget.h"
#include "Core/PipelineState.h"
#include "Core/ResourceRecord.h"
#include "Core/DescriptorPool.h"
#include "Core/DescriptorSetLayout.h"

namespace std
{
template <>
struct hash<vge::ShaderSource>
{
	size_t operator()(const vge::ShaderSource& shaderSource) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, shaderSource.GetId());
		return result;
	}
};

template <>
struct hash<vge::ShaderVariant>
{
	size_t operator()(const vge::ShaderVariant& ShaderVariant) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, ShaderVariant.GetId());
		return result;
	}
};

template <>
struct hash<vge::ShaderModule>
{
	size_t operator()(const vge::ShaderModule& shaderModule) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, shaderModule.GetId());
		return result;
	}
};

template <>
struct hash<vge::DescriptorSetLayout>
{
	size_t operator()(const vge::DescriptorSetLayout& descriptorSetLayout) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, descriptorSetLayout.GetHandle());
		return result;
	}
};

template <>
struct hash<vge::DescriptorPool>
{
	size_t operator()(const vge::DescriptorPool& descriptorPool) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, descriptorPool.GetDescriptorSetLayout());
		return result;
	}
};

template <>
struct hash<vge::PipelineLayout>
{
	size_t operator()(const vge::PipelineLayout& pipelineLayout) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, pipelineLayout.GetHandle());
		return result;
	}
};

template <>
struct hash<vge::RenderPass>
{
	size_t operator()(const vge::RenderPass& renderPass) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, renderPass.GetHandle());
		return result;
	}
};

template <>
struct hash<vge::Attachment>
{
	size_t operator()(const vge::Attachment& attachment) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkFormat>>(attachment.Format));
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkSampleCountFlagBits>>(attachment.Samples));
		vge::HashCombine(result, attachment.Usage);
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkImageLayout>>(attachment.InitialLayout));
		return result;
	}
};

template <>
struct hash<vge::LoadStoreInfo>
{
	size_t operator()(const vge::LoadStoreInfo& loadStoreInfo) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkAttachmentLoadOp>>(loadStoreInfo.LoadOp));
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkAttachmentStoreOp>>(loadStoreInfo.StoreOp));
		return result;
	}
};

template <>
struct hash<vge::SubpassInfo>
{
	size_t operator()(const vge::SubpassInfo& subpassInfo) const noexcept
	{
		size_t result = 0;

		for (uint32_t output_attachment : subpassInfo.OutputAttachments)
		{
			vge::HashCombine(result, output_attachment);
		}

		for (uint32_t input_attachment : subpassInfo.InputAttachments)
		{
			vge::HashCombine(result, input_attachment);
		}

		for (uint32_t resolve_attachment : subpassInfo.ColorResolveAttachments)
		{
			vge::HashCombine(result, resolve_attachment);
		}

		vge::HashCombine(result, subpassInfo.DisableDepthStencilAttachment);
		vge::HashCombine(result, subpassInfo.DepthStencilResolveAttachment);
		vge::HashCombine(result, subpassInfo.DepthStencilResolveMode);

		return result;
	}
};

template <>
struct hash<vge::SpecializationConstantState>
{
	size_t operator()(const vge::SpecializationConstantState& specializationConstantState) const noexcept
	{
		size_t result = 0;

		for (const auto& [constantId, data] : specializationConstantState.GetSpecializationConstantState())
		{
			vge::HashCombine(result, constantId);
			for (const auto byte : data)
			{
				vge::HashCombine(result, byte);
			}
		}

		return result;
	}
};

template <>
struct hash<vge::ShaderResource>
{
	size_t operator()(const vge::ShaderResource& shaderResource) const noexcept
	{
		size_t result = 0;

		if (shaderResource.Type == vge::ShaderResourceType::Input ||
			shaderResource.Type == vge::ShaderResourceType::Output ||
			shaderResource.Type == vge::ShaderResourceType::PushConstant ||
			shaderResource.Type == vge::ShaderResourceType::SpecializationConstant)
		{
			return result;
		}

		vge::HashCombine(result, shaderResource.Set);
		vge::HashCombine(result, shaderResource.Binding);
		vge::HashCombine(result, static_cast<std::underlying_type_t<vge::ShaderResourceType>>(shaderResource.Type));
		vge::HashCombine(result, shaderResource.Mode);

		return result;
	}
};

template <>
struct hash<VkDescriptorBufferInfo>
{
	size_t operator()(const VkDescriptorBufferInfo& descriptorBufferInfo) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, descriptorBufferInfo.buffer);
		vge::HashCombine(result, descriptorBufferInfo.range);
		vge::HashCombine(result, descriptorBufferInfo.offset);
		return result;
	}
};

template <>
struct hash<VkDescriptorImageInfo>
{
	size_t operator()(const VkDescriptorImageInfo& descriptorImageInfo) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, descriptorImageInfo.imageView);
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkImageLayout>>(descriptorImageInfo.imageLayout));
		vge::HashCombine(result, descriptorImageInfo.sampler);
		return result;
	}
};

template <>
struct hash<VkWriteDescriptorSet>
{
	size_t operator()(const VkWriteDescriptorSet& writeDescriptorSet) const noexcept
	{
		size_t result = 0;

		vge::HashCombine(result, writeDescriptorSet.dstSet);
		vge::HashCombine(result, writeDescriptorSet.dstBinding);
		vge::HashCombine(result, writeDescriptorSet.dstArrayElement);
		vge::HashCombine(result, writeDescriptorSet.descriptorCount);
		vge::HashCombine(result, writeDescriptorSet.descriptorType);

		switch (writeDescriptorSet.descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			for (uint32_t i = 0; i < writeDescriptorSet.descriptorCount; i++)
			{
				vge::HashCombine(result, writeDescriptorSet.pImageInfo[i]);
			}
			break;

		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			for (uint32_t i = 0; i < writeDescriptorSet.descriptorCount; i++)
			{
				vge::HashCombine(result, writeDescriptorSet.pTexelBufferView[i]);
			}
			break;

		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			for (uint32_t i = 0; i < writeDescriptorSet.descriptorCount; i++)
			{
				vge::HashCombine(result, writeDescriptorSet.pBufferInfo[i]);
			}
			break;

		default:
			// Not implemented
			break;
		}

		return result;
	}
};

template <>
struct hash<VkVertexInputAttributeDescription>
{
	size_t operator()(const VkVertexInputAttributeDescription& vertexAttribute) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, vertexAttribute.binding);
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkFormat>>(vertexAttribute.format));
		vge::HashCombine(result, vertexAttribute.location);
		vge::HashCombine(result, vertexAttribute.offset);
		return result;
	}
};

template <>
struct hash<VkVertexInputBindingDescription>
{
	size_t operator()(const VkVertexInputBindingDescription& vertexBinding) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, vertexBinding.binding);
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkVertexInputRate>>(vertexBinding.inputRate));
		vge::HashCombine(result, vertexBinding.stride);
		return result;
	}
};

template <>
struct hash<vge::StencilOpState>
{
	size_t operator()(const vge::StencilOpState& stencil) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkCompareOp>>(stencil.CompareOp));
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkStencilOp>>(stencil.DepthFailOp));
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkStencilOp>>(stencil.FailOp));
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkStencilOp>>(stencil.PassOp));
		return result;
	}
};

template <>
struct hash<VkExtent2D>
{
	size_t operator()(const VkExtent2D& extent) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, extent.width);
		vge::HashCombine(result, extent.height);
		return result;
	}
};

template <>
struct hash<VkOffset2D>
{
	size_t operator()(const VkOffset2D& offset) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, offset.x);
		vge::HashCombine(result, offset.y);
		return result;
	}
};

template <>
struct hash<VkRect2D>
{
	size_t operator()(const VkRect2D& rect) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, rect.extent);
		vge::HashCombine(result, rect.offset);
		return result;
	}
};

template <>
struct hash<VkViewport>
{
	size_t operator()(const VkViewport& viewport) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, viewport.width);
		vge::HashCombine(result, viewport.height);
		vge::HashCombine(result, viewport.maxDepth);
		vge::HashCombine(result, viewport.minDepth);
		vge::HashCombine(result, viewport.x);
		vge::HashCombine(result, viewport.y);
		return result;
	}
};

template <>
struct hash<vge::ColorBlendAttachmentState>
{
	size_t operator()(const vge::ColorBlendAttachmentState& colorBlendAttachment) const noexcept
	{
		size_t result = 0;
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkBlendOp>>(colorBlendAttachment.AlphaBlendOp));
		vge::HashCombine(result, colorBlendAttachment.BlendEnable);
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkBlendOp>>(colorBlendAttachment.ColorBlendOp));
		vge::HashCombine(result, colorBlendAttachment.ColorWriteMask);
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkBlendFactor>>(colorBlendAttachment.DstAlphaBlendFactor));
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkBlendFactor>>(colorBlendAttachment.DstColorBlendFactor));
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkBlendFactor>>(colorBlendAttachment.SrcAlphaBlendFactor));
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkBlendFactor>>(colorBlendAttachment.SrcColorBlendFactor));
		return result;
	}
};

template <>
struct hash<vge::RenderTarget>
{
	size_t operator()(const vge::RenderTarget& renderTarget) const noexcept
	{
		size_t result = 0;

		for (auto& view : renderTarget.GetViews())
		{
			vge::HashCombine(result, view.GetHandle());
			vge::HashCombine(result, view.GetImage().GetHandle());
		}

		return result;
	}
};

template <>
struct hash<vge::PipelineState>
{
	size_t operator()(const vge::PipelineState& pipelineState) const noexcept
	{
		size_t result = 0;

		vge::HashCombine(result, pipelineState.GetPipelineLayout().GetHandle());

		// For graphics only.
		if (const auto renderPass = pipelineState.GetRenderPass())
		{
			vge::HashCombine(result, renderPass->GetHandle());
		}

		vge::HashCombine(result, pipelineState.GetSpecializationConstantState());
		vge::HashCombine(result, pipelineState.GetSubpassIndex());

		for (const auto shaderModule : pipelineState.GetPipelineLayout().GetShaderModules())
		{
			vge::HashCombine(result, shaderModule->GetId());
		}

		// VkPipelineVertexInputStateCreateInfo
		for (auto& attribute : pipelineState.GetVertexInputState().Attributes)
		{
			vge::HashCombine(result, attribute);
		}

		for (auto& binding : pipelineState.GetVertexInputState().Bindings)
		{
			vge::HashCombine(result, binding);
		}

		// VkPipelineInputAssemblyStateCreateInfo
		vge::HashCombine(result, pipelineState.GetInputAssemblyState().PrimitiveRestartEnable);
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkPrimitiveTopology>>(pipelineState.GetInputAssemblyState().Topology));

		//VkPipelineViewportStateCreateInfo
		vge::HashCombine(result, pipelineState.GetViewportState().ViewportCount);
		vge::HashCombine(result, pipelineState.GetViewportState().ScissorCount);

		// VkPipelineRasterizationStateCreateInfo
		vge::HashCombine(result, pipelineState.GetRasterizationState().CullMode);
		vge::HashCombine(result, pipelineState.GetRasterizationState().DepthBiasEnable);
		vge::HashCombine(result, pipelineState.GetRasterizationState().DepthClampEnable);
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkFrontFace>>(pipelineState.GetRasterizationState().FrontFace));
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkPolygonMode>>(pipelineState.GetRasterizationState().PolygonMode));
		vge::HashCombine(result, pipelineState.GetRasterizationState().RasterizerDiscardEnable);

		// VkPipelineMultisampleStateCreateInfo
		vge::HashCombine(result, pipelineState.GetMultisampleState().AlphaToCoverageEnable);
		vge::HashCombine(result, pipelineState.GetMultisampleState().AlphaToOneEnable);
		vge::HashCombine(result, pipelineState.GetMultisampleState().MinSampleShading);
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkSampleCountFlagBits>>(pipelineState.GetMultisampleState().RasterizationSamples));
		vge::HashCombine(result, pipelineState.GetMultisampleState().SampleShadingEnable);
		vge::HashCombine(result, pipelineState.GetMultisampleState().SampleMask);

		// VkPipelineDepthStencilStateCreateInfo
		vge::HashCombine(result, pipelineState.GetDepthStencilState().Back);
		vge::HashCombine(result, pipelineState.GetDepthStencilState().DepthBoundsTestEnable);
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkCompareOp>>(pipelineState.GetDepthStencilState().DepthCompareOp));
		vge::HashCombine(result, pipelineState.GetDepthStencilState().DepthTestEnable);
		vge::HashCombine(result, pipelineState.GetDepthStencilState().DepthWriteEnable);
		vge::HashCombine(result, pipelineState.GetDepthStencilState().Front);
		vge::HashCombine(result, pipelineState.GetDepthStencilState().StencilTestEnable);

		// VkPipelineColorBlendStateCreateInfo
		vge::HashCombine(result, static_cast<std::underlying_type_t<VkLogicOp>>(pipelineState.GetColorBlendState().LogicOp));
		vge::HashCombine(result, pipelineState.GetColorBlendState().LogicOpEnable);

		for (auto& attachment : pipelineState.GetColorBlendState().Attachments)
		{
			vge::HashCombine(result, attachment);
		}

		return result;
	}
};
}	// namespace std

namespace vge
{
namespace
{
template <typename T>
inline void HashParam(size_t& seed, const T& value)
{
	HashCombine(seed, value);
}

template <>
inline void HashParam(size_t& /*seed*/, const VkPipelineCache& /*value*/)
{
}

template <>
inline void HashParam<std::vector<uint8_t>>(size_t& seed, const std::vector<uint8_t>& value)
{
	HashCombine(seed, std::string(value.begin(), value.end()));
}

template <>
inline void HashParam<std::vector<Attachment>>(size_t& seed, const std::vector<Attachment>& value)
{
	for (auto& attachment : value)
	{
		HashCombine(seed, attachment);
	}
}

template <>
inline void HashParam<std::vector<LoadStoreInfo>>(size_t& seed, const std::vector<LoadStoreInfo>& value)
{
	for (auto& loadStoreInfo : value)
	{
		HashCombine(seed, loadStoreInfo);
	}
}

template <>
inline void HashParam<std::vector<SubpassInfo>>(size_t& seed, const std::vector<SubpassInfo>& value)
{
	for (auto& subpassInfo : value)
	{
		HashCombine(seed, subpassInfo);
	}
}

template <>
inline void HashParam<std::vector<ShaderModule*>>(size_t& seed, const std::vector<ShaderModule*>& value)
{
	for (auto& shaderModule : value)
	{
		HashCombine(seed, shaderModule->GetId());
	}
}

template <>
inline void HashParam<std::vector<ShaderResource>>(size_t& seed, const std::vector<ShaderResource>& value)
{
	for (auto& resource : value)
	{
		HashCombine(seed, resource);
	}
}

template <>
inline void HashParam<std::map<uint32_t, std::map<uint32_t, VkDescriptorBufferInfo>>>(size_t& seed, const std::map<uint32_t, std::map<uint32_t, VkDescriptorBufferInfo>>& value)
{
	for (auto& bindingSet : value)
	{
		HashCombine(seed, bindingSet.first);

		for (auto& binding_element : bindingSet.second)
		{
			HashCombine(seed, binding_element.first);
			HashCombine(seed, binding_element.second);
		}
	}
}

template <>
inline void HashParam<std::map<uint32_t, std::map<uint32_t, VkDescriptorImageInfo>>>(size_t& seed, const std::map<uint32_t, std::map<uint32_t, VkDescriptorImageInfo>>& value)
{
	for (auto& bindingSet : value)
	{
		HashCombine(seed, bindingSet.first);

		for (auto& bindingElement : bindingSet.second)
		{
			HashCombine(seed, bindingElement.first);
			HashCombine(seed, bindingElement.second);
		}
	}
}

template <typename T, typename... Args>
inline void HashParam(size_t& seed, const T& firstArg, const Args &... args)
{
	HashParam(seed, firstArg);
	HashParam(seed, args...);
}

template <class T, class... Args>
struct RecordHelper
{
	size_t Record(ResourceRecord& /*recorder*/, Args &... /*args*/)
	{
		return 0;
	}

	void Index(ResourceRecord& /*recorder*/, size_t /*index*/, T& /*resource*/)
	{
	}
};

template <class... Args>
struct RecordHelper<ShaderModule, Args...>
{
	size_t Record(ResourceRecord& recorder, Args &... args)
	{
		return recorder.RegisterShaderModule(args...);
	}

	void Index(ResourceRecord& recorder, size_t index, ShaderModule& shaderModule)
	{
		recorder.SetShaderModule(index, shaderModule);
	}
};

template <class... Args>
struct RecordHelper<PipelineLayout, Args...>
{
	size_t Record(ResourceRecord& recorder, Args &... args)
	{
		return recorder.RegisterPipelineLayout(args...);
	}

	void Index(ResourceRecord& recorder, size_t index, PipelineLayout& pipelineLayout)
	{
		recorder.SetPipelineLayout(index, pipelineLayout);
	}
};

template <class... Args>
struct RecordHelper<RenderPass, Args...>
{
	size_t Record(ResourceRecord& recorder, Args &... args)
	{
		return recorder.RegisterRenderPass(args...);
	}

	void Index(ResourceRecord& recorder, size_t index, RenderPass& renderPass)
	{
		recorder.SetRenderPass(index, renderPass);
	}
};

template <class... Args>
struct RecordHelper<GraphicsPipeline, Args...>
{
	size_t Record(ResourceRecord& recorder, Args &... args)
	{
		return recorder.RegisterGraphicsPipeline(args...);
	}

	void Index(ResourceRecord& recorder, size_t index, GraphicsPipeline& graphicsPipeline)
	{
		recorder.SetGraphicsPipeline(index, graphicsPipeline);
	}
};
}	// namespace

template <class T, class... Args>
T& RequestResource(Device& device, ResourceRecord* recorder, std::unordered_map<size_t, T>& resources, Args&... args)
{
	size_t hash = 0;
	HashParam(hash, args...);

	auto resIt = resources.find(hash);

	if (resIt != resources.end())
	{
		return resIt->second;
	}

	// If we do not have it already, create and cache it.
	const char* resType = typeid(T).name();
	const size_t resId = resources.size();

	LOG(Log, "Building %d cache object (%s).", resId, resType);

	{
		T resource(device, args...);

		auto insertedIt = resources.emplace(hash, std::move(resource));
		ENSURE_MSG(insertedIt.second, "Insertion error for %d cache object (%s).", resId, resType);

		resIt = insertedIt.first;

		if (recorder)
		{
			RecordHelper<T, Args...> recordHelper;
			size_t index = recordHelper.Record(*recorder, args...);
			recordHelper.Index(*recorder, index, resIt->second);
		}
	}

	return resIt->second;
}
}	// namespace vge