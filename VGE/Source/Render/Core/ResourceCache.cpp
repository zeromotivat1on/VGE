#include "ResourceCache.h"
#include "Common/ResourceCaching.h"
#include "Core/VkCommon.h"
#include "Core/Device.h"

namespace vge
{
namespace
{
template <class T, class... Args>
T& RequestResource(Device& device, ResourceRecord& recorder, std::mutex& resourceMutex, std::unordered_map<std::size_t, T>& resources, Args&... args)
{
	std::lock_guard<std::mutex> guard(resourceMutex);
	auto& res = RequestResource(device, &recorder, resources, args...);
	return res;
}
}   // namespace
}	// namespace vge

vge::ResourceCache::ResourceCache(Device& device) 
	: _Device(device)
{
}

void vge::ResourceCache::Warmup(const std::vector<u8>& data)
{
	_Recorder.SetData(data);
	_Replayer.Play(*this, _Recorder);
}

std::vector<vge::u8> vge::ResourceCache::Serialize()
{
	return _Recorder.GetData();
}

vge::ShaderModule& vge::ResourceCache::RequestShaderModule(VkShaderStageFlagBits stage, const ShaderSource& glslSource, const ShaderVariant& shaderVariant)
{
	std::string entryPoint = "main";
	return RequestResource(_Device, _Recorder, _ShaderModuleMutex, _State.ShaderModules, stage, glslSource, entryPoint, shaderVariant);
}

vge::PipelineLayout& vge::ResourceCache::RequestPipelineLayout(const std::vector<ShaderModule*>& shaderModules)
{
	return RequestResource(_Device, _Recorder, _PipelineLayoutMutex, _State.PipelineLayouts, shaderModules);
}

vge::DescriptorSetLayout& vge::ResourceCache::RequestDescriptorSetLayout(
	const u32 setIndex,
	const std::vector<ShaderModule*>& shader_modules,
	const std::vector<ShaderResource>& set_resources)
{
	return RequestResource(_Device, _Recorder, _DescriptorSetLayoutMutex, _State.DescriptorSetLayouts, setIndex, shader_modules, set_resources);
}

vge::GraphicsPipeline& vge::ResourceCache::RequestGraphicsPipeline(PipelineState& pipelineState)
{
	return RequestResource(_Device, _Recorder, _GraphicsPipelineMutex, _State.GraphicsPipelines, _PipelineCache, pipelineState);
}

vge::ComputePipeline& vge::ResourceCache::RequestComputePipeline(PipelineState& pipelineSate)
{
	return RequestResource(_Device, _Recorder, _ComputePipelineMutex, _State.ComputePipelines, _PipelineCache, pipelineSate);
}

vge::DescriptorSet& vge::ResourceCache::RequestDescriptorSet(
	DescriptorSetLayout& descriptorSetLayout, 
	const BindingMap<VkDescriptorBufferInfo>& bufferInfos, 
	const BindingMap<VkDescriptorImageInfo>& imageInfos)
{
	auto& descriptorPool = RequestResource(_Device, _Recorder, _DescriptorSetMutex, _State.DescriptorPools, descriptorSetLayout);
	return RequestResource(_Device, _Recorder, _DescriptorSetMutex, _State.DescriptorSets, descriptorSetLayout, descriptorPool, bufferInfos, imageInfos);
}

vge::RenderPass& vge::ResourceCache::RequestRenderPass(
	const std::vector<Attachment>& attachments, 
	const std::vector<LoadStoreInfo>& loadStoreInfos, 
	const std::vector<SubpassInfo>& subpasses)
{
	return RequestResource(_Device, _Recorder, _RenderPassMutex, _State.RenderPasses, attachments, loadStoreInfos, subpasses);
}

vge::Framebuffer& vge::ResourceCache::RequestFramebuffer(const RenderTarget& renderTarget, const RenderPass& renderPass)
{
	return RequestResource(_Device, _Recorder, _FramebufferMutex, _State.Framebuffers, renderTarget, renderPass);
}

void vge::ResourceCache::ClearPipelines()
{
	_State.GraphicsPipelines.clear();
	_State.ComputePipelines.clear();
}

void vge::ResourceCache::UpdateDescriptorSets(const std::vector<ImageView>& oldViews, const std::vector<ImageView>& newViews)
{
	// Find descriptor sets referring to the old image view.
	std::vector<VkWriteDescriptorSet> setUpdates;
	std::set<size_t> matches;

	for (size_t i = 0; i < oldViews.size(); ++i)
	{
		auto& oldView = oldViews[i];
		auto& newView = newViews[i];

		for (auto& kdPair : _State.DescriptorSets)
		{
			auto& key = kdPair.first;
			auto& descriptorSet = kdPair.second;

			auto& imageInfos = descriptorSet.GetImageInfos();

			for (auto& baPair : imageInfos)
			{
				auto& binding = baPair.first;
				auto& array = baPair.second;

				for (auto& aiPair : array)
				{
					auto& arrayElement = aiPair.first;
					auto& imageInfo = aiPair.second;

					if (imageInfo.imageView == oldView.GetHandle())
					{
						// Save key to remove old descriptor set.
						matches.insert(key);

						// Update image info with new view
						imageInfo.imageView = newView.GetHandle();

						// Save struct for writing the update later.
						VkWriteDescriptorSet writeDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

						if (auto bindingInfo = descriptorSet.GetLayout().GetLayoutBinding(binding))
						{
							writeDescriptorSet.dstBinding = binding;
							writeDescriptorSet.descriptorType = bindingInfo->descriptorType;
							writeDescriptorSet.pImageInfo = &imageInfo;
							writeDescriptorSet.dstSet = descriptorSet.GetHandle();
							writeDescriptorSet.dstArrayElement = arrayElement;
							writeDescriptorSet.descriptorCount = 1;

							setUpdates.push_back(writeDescriptorSet);
						}
						else
						{
							LOG(Error, "Shader layout set does not use image binding at %d.", binding);
						}
					}
				}
			}
		}
	}

	if (!setUpdates.empty())
	{
		vkUpdateDescriptorSets(_Device.GetHandle(), ToU32(setUpdates.size()), setUpdates.data(), 0, nullptr);
	}

	// Delete old entries (moved out descriptor sets).
	for (auto& match : matches)
	{
		// Move out of the map.
		auto it = _State.DescriptorSets.find(match);
		auto descriptorSet = std::move(it->second);
		_State.DescriptorSets.erase(match);

		// Generate new key.
		size_t newKey = 0;
		HashParam(newKey, descriptorSet.GetLayout(), descriptorSet.GetBufferInfos(), descriptorSet.GetImageInfos());

		// Add (key, resource) to the cache.
		_State.DescriptorSets.emplace(newKey, std::move(descriptorSet));
	}
}

void vge::ResourceCache::Clear()
{
	_State.ShaderModules.clear();
	_State.PipelineLayouts.clear();
	_State.DescriptorSets.clear();
	_State.DescriptorSetLayouts.clear();
	_State.RenderPasses.clear();
	ClearPipelines();
	ClearFramebuffers();
}
