#pragma once

#include "Core/ResourceRecord.h"
#include "Core/ResourceCache.h"

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
	void CreateShaderModule(ResourceCache&, std::istringstream&);
	void CreatePipelineLayout(ResourceCache&, std::istringstream&);
	void CreateRenderPass(ResourceCache&, std::istringstream&);
	void CreateGraphicsPipeline(ResourceCache&, std::istringstream&);

private:
	using ResourceFunc = std::function<void(ResourceCache&, std::istringstream&)>;

private:
	std::unordered_map<ResourceType, ResourceFunc> stream_resources;
	std::vector<ShaderModule*> shader_modules;
	std::vector<PipelineLayout*> pipeline_layouts;
	std::vector<const RenderPass*> render_passes;
	std::vector<const GraphicsPipeline*> graphics_pipelines;
};
}	// namespace vge
