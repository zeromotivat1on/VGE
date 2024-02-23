#pragma once

#include "Core/ResourceRecord.h"

namespace vge
{
class ResourceCache;

// Reads Vulkan objects from a memory stream and creates them in the resource cache.
class ResourceReplay
{
public:
	ResourceReplay();

	void Play(ResourceCache&, ResourceRecord&);

protected:
	void CreateRenderPass(ResourceCache&, std::istringstream&);
	void CreateShaderModule(ResourceCache&, std::istringstream&);
	void CreatePipelineLayout(ResourceCache&, std::istringstream&);
	void CreateGraphicsPipeline(ResourceCache&, std::istringstream&);

private:
	using ResourceFunc = std::function<void(ResourceCache&, std::istringstream&)>;

private:
	std::unordered_map<ResourceType, ResourceFunc> _StreamResources;
	std::vector<ShaderModule*> _ShaderModules;
	std::vector<PipelineLayout*> _PipelineLayouts;
	std::vector<const RenderPass*> _RenderPasses;
	std::vector<const GraphicsPipeline*> _GraphicsPipelines;
};
}	// namespace vge
