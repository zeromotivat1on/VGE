#pragma once

#include "VkCommon.h"
#include "Core/ResourceRecord.h"
#include "Core/ResourceReplay.h"
#include "Core/DescriptorSet.h"
#include "Core/DescriptorPool.h"
#include "Core/DescriptorSetLayout.h"
#include "Core/Framebuffer.h"
#include "Core/Pipeline.h"

namespace vge
{
class Device;
class ImageView;

struct ResourceCacheState
{
	std::unordered_map<size_t, ShaderModule> ShaderModules;
	std::unordered_map<size_t, PipelineLayout> PipelineLayouts;
	std::unordered_map<size_t, DescriptorSetLayout> DescriptorSetLayouts;
	std::unordered_map<size_t, DescriptorPool> DescriptorPools;
	std::unordered_map<size_t, RenderPass> RenderPasses;
	std::unordered_map<size_t, GraphicsPipeline> GraphicsPipelines;
	std::unordered_map<size_t, ComputePipeline> ComputePipelines;
	std::unordered_map<size_t, DescriptorSet> DescriptorSets;
	std::unordered_map<size_t, Framebuffer> Framebuffers;
};

/**
* Cache all sorts of Vulkan objects specific to a Vulkan device.
* Supports serialization and deserialization of cached resources.
* There is only one cache for all these objects, with several unordered_map of hash indices
* and objects. For every object requested, there is a templated version on request_resource.
* Some objects may need building if they are not found in the cache.
*
* The resource cache is also linked with ResourceRecord and ResourceReplay. Replay can warm-up
* the cache on app startup by creating all necessary objects.
* The cache holds pointers to objects and has a mapping from such pointers to hashes.
* It can only be destroyed in bulk, single elements cannot be removed.
*/
class ResourceCache
{
public:
	ResourceCache(Device& device);

	COPY_CTOR_DEL(ResourceCache);
	MOVE_CTOR_DEL(ResourceCache);
	
	COPY_OP_DEL(ResourceCache);
	MOVE_OP_DEL(ResourceCache);

public:
	inline const ResourceCacheState& GetInternalState() const { return _State; }
	inline void SetPipelineCache(VkPipelineCache pipelineCache) { _PipelineCache = pipelineCache; }
	inline void ClearFramebuffers() { _State.Framebuffers.clear(); }

	void Warmup(const std::vector<u8>& data);

	std::vector<u8> Serialize();

	ShaderModule& RequestShaderModule(VkShaderStageFlagBits, const ShaderSource&, const ShaderVariant& = {});
	PipelineLayout& RequestPipelineLayout(const std::vector<ShaderModule*>&);
	DescriptorSetLayout& RequestDescriptorSetLayout(const u32 setIndex, const std::vector<ShaderModule*>&, const std::vector<ShaderResource>&);
	GraphicsPipeline& RequestGraphicsPipeline(PipelineState&);
	ComputePipeline& RequestComputePipeline(PipelineState&);
	DescriptorSet& RequestDescriptorSet(DescriptorSetLayout&, const BindingMap<VkDescriptorBufferInfo>&, const BindingMap<VkDescriptorImageInfo>&);
	RenderPass& RequestRenderPass(const std::vector<Attachment>&, const std::vector<LoadStoreInfo>&, const std::vector<SubpassInfo>&);
	Framebuffer& RequestFramebuffer(const RenderTarget&, const RenderPass&);
	
	// Update those descriptor sets referring to old views.
	void UpdateDescriptorSets(const std::vector<ImageView>& oldViews, const std::vector<ImageView>& newViews);

	void Clear();
	void ClearPipelines();

private:
	Device& _Device;
	ResourceRecord _Recorder;
	ResourceReplay _Replayer;
	VkPipelineCache _PipelineCache = VK_NULL_HANDLE;
	ResourceCacheState _State;

	std::mutex _DescriptorSetMutex;
	std::mutex _PipelineLayoutMutex;
	std::mutex _ShaderModuleMutex;
	std::mutex _DescriptorSetLayoutMutex;
	std::mutex _GraphicsPipelineMutex;
	std::mutex _RenderPassMutex;
	std::mutex _ComputePipelineMutex;
	std::mutex _FramebufferMutex;
};
}	// namespace vge
