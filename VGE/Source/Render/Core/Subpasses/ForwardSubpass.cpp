#include "Core/Subpasses/ForwardSubpass.h"

#include "Core/Device.h"
#include "Core/VkCommon.h"
#include "Core/RenderContext.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/ModelComponent.h"
//#include "scene_graph/components/camera.h"
//#include "scene_graph/components/image.h"
//#include "scene_graph/components/material.h"
//#include "scene_graph/components/mesh.h"
//#include "scene_graph/components/pbr_material.h"
//#include "scene_graph/components/sub_mesh.h"
//#include "scene_graph/components/texture.h"
//#include "scene_graph/node.h"
//#include "scene_graph/scene.h"

vge::ForwardSubpass::ForwardSubpass(RenderContext& renderContext, ShaderSource&& vertex, ShaderSource&& fragment, Scene& scene, CameraComponent& camera)
	: GeometrySubpass(renderContext, std::move(vertex), std::move(fragment), scene, camera)
{
}

void vge::ForwardSubpass::Prepare()
{
	auto& device = _RenderContext.GetDevice();
	for (auto& model : _Models)
	{
		for (auto& mesh : model->Meshes)
		{
			auto& variant = mesh->ShaderVariant;

			// Same as Geometry except adds lighting definitions to sub mesh variants.
			variant.AddDefinitions({"MAX_LIGHT_COUNT " + std::to_string(MAX_FORWARD_LIGHT_COUNT)});
			variant.AddDefinitions(GLightTypeDefinitions);

			const auto& vertModule = device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_VERTEX_BIT, GetVertexShader(), variant);
			const auto& fragModule = device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, GetFragmentShader(), variant);
		}
	}
}

void vge::ForwardSubpass::Draw(CommandBuffer& cmd)
{
	//AllocateLights<ForwardLights>(scene.get_components<Light>(), MAX_FORWARD_LIGHT_COUNT);
	//cmd.BindLighting(GetLightingState(), 0, 4);

	GeometrySubpass::Draw(cmd);
}
