#include "LightingSubpass.h"

#include "Camera.h"
#include "Core/Device.h"
#include "Core/RenderContext.h"
#include "ECS/Components/CameraComponent.h"
//#include "scene_graph/components/camera.h"
//#include "scene_graph/scene.h"

vge::LightingSubpass::LightingSubpass(RenderContext& renderContext, ShaderSource&& vertex, ShaderSource&& fragment, CameraComponent& camera, Scene& scene)
	: Subpass(renderContext, std::move(vertex), std::move(fragment)), _Camera(camera), _Scene(scene)
{
}

void vge::LightingSubpass::Prepare()
{
	_LightingVariant.AddDefinitions({"MAX_LIGHT_COUNT " + std::to_string(MAX_DEFERRED_LIGHT_COUNT)});
	_LightingVariant.AddDefinitions(GLightTypeDefinitions);
	
	// Build all shaders upfront.
	auto& resourceCache = _RenderContext.GetDevice().GetResourceCache();
	resourceCache.RequestShaderModule(VK_SHADER_STAGE_VERTEX_BIT, GetVertexShader(), _LightingVariant);
	resourceCache.RequestShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, GetFragmentShader(), _LightingVariant);
}

void vge::LightingSubpass::Draw(CommandBuffer& cmd)
{
	//AllocateLights<DeferredLights>(_Scene.get_components<Light>(), MAX_DEFERRED_LIGHT_COUNT);
	//cmd.BindLighting(GetLightingState(), 0, 4);

	// Get shaders from cache.
	auto& resourceCache = cmd.GetDevice().GetResourceCache();
	auto& vertShaderModule = resourceCache.RequestShaderModule(VK_SHADER_STAGE_VERTEX_BIT, GetVertexShader(), _LightingVariant);
	auto& fragShaderModule = resourceCache.RequestShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, GetFragmentShader(), _LightingVariant);

	const std::vector<ShaderModule*> shaderModules = { &vertShaderModule, &fragShaderModule };

	// Create pipeline layout and bind it.
	auto& pipelineLayout = resourceCache.RequestPipelineLayout(shaderModules);
	cmd.BindPipelineLayout(pipelineLayout);

	// We know, that the lighting subpass does not have any vertex stage input -> reset the vertex input state.
	ASSERT(pipelineLayout.GetResources(ShaderResourceType::Input, VK_SHADER_STAGE_VERTEX_BIT).empty());
	cmd.SetVertexInputState({});

	// Get image views of the attachments.
	const auto& renderTarget = _RenderContext.GetActiveFrame().GetRenderTarget();
	auto& targetViews  = renderTarget.GetViews();
	ASSERT(targetViews.size() > 3);

	// Bind depth, albedo, and normal as input attachments.
	auto& depthView = targetViews[1];
	cmd.BindInput(depthView, 0, 0, 0);

	auto& albedoView = targetViews[2];
	cmd.BindInput(albedoView, 0, 1, 0);

	auto& normalView = targetViews[3];
	cmd.BindInput(normalView, 0, 2, 0);

	// Set cull mode to front as full screen triangle is clock-wise.
	RasterizationState rasterizationState;
	rasterizationState.CullMode = VK_CULL_MODE_FRONT_BIT;
	cmd.SetRasterizationState(rasterizationState);

	// Populate uniform values.
	LightUniform lightUniform;

	// Inverse resolution.
	lightUniform.InvResolution.x = 1.0f / static_cast<float>(renderTarget.GetExtent().width);
	lightUniform.InvResolution.y = 1.0f / static_cast<float>(renderTarget.GetExtent().height);

	// Inverse view projection.
	lightUniform.InvViewProj = glm::inverse(VulkanStyleProjection(_Camera.GetProjMat4()) * _Camera.GetViewMat4());

	// Allocate a buffer using the buffer pool from the active frame to store uniform values and bind it.
	auto& renderFrame = _RenderContext.GetActiveFrame();
	auto allocation = renderFrame.AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(LightUniform));
	allocation.Update(lightUniform);
	cmd.BindBuffer(allocation.GetBuffer(), allocation.GetOffset(), allocation.GetSize(), 0, 3, 0);

	// Draw full screen triangle.
	cmd.Draw(3, 1, 0, 0);
}
