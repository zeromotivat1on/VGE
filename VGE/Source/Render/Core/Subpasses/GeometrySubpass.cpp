#include "Core/Subpasses/GeometrySubpass.h"

#include "Core/Device.h"
#include "Core/VkCommon.h"
#include "Core/RenderContext.h"
#include "ECS/Components/ImageComponent.h"
#include "ECS/Components/MaterialComponent.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/ModelComponent.h"
#include "ECS/Components/NodeComponent.h"
#include "ECS/Components/SamplerComponent.h"
#include "ECS/Components/TextureComponent.h"
//#include "scene_graph/components/camera.h"
//#include "scene_graph/components/image.h"
//#include "scene_graph/components/material.h"
//#include "scene_graph/components/mesh.h"
//#include "scene_graph/components/pbr_material.h"
//#include "scene_graph/components/texture.h"
//#include "scene_graph/node.h"
//#include "scene_graph/scene.h"

vge::GeometrySubpass::GeometrySubpass(RenderContext& renderContext, ShaderSource&& vertex, ShaderSource&& fragment, Scene& scene, CameraComponent& camera)
    : Subpass(renderContext, std::move(vertex), std::move(fragment)),
      _Scene(scene), _Camera(camera), _Models(scene.get_components<ModelComponent>())
{
}

void vge::GeometrySubpass::Prepare()
{
    // Build all shader variance upfront.
    auto& device = _RenderContext.GetDevice();
    for (const auto& model : _Models)
    {
        for (const auto& mesh : model->Meshes)
        {
            const auto& variant = mesh->ShaderVariant;
            const auto& vertModule = device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_VERTEX_BIT, GetVertexShader(), variant);
            const auto& fragModule = device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, GetFragmentShader(), variant);
        }
    }
}

void vge::GeometrySubpass::GetSortedNodes(
    std::multimap<float, std::pair<NodeComponent*, MeshComponent*>>& opaqueNodes,
    std::multimap<float, std::pair<NodeComponent*, MeshComponent*>>& transparentNodes)
{
    const auto cameraTransform = GetWorldMat4(_Camera.Node->Transform);

    for (const auto& model : _Models)
    {
        for (const auto& node : model->Nodes)
        {
            const auto nodeTransform = GetWorldMat4(node->Transform);

            const AABBComponent& meshBounds = model->Bounds;

            AABBComponent worldBounds = { meshBounds.Min, meshBounds.Max };
            Transform(worldBounds, nodeTransform);

            const float distance = glm::length(glm::vec3(cameraTransform[3]) - GetCenter(worldBounds));

            for (auto& mesh : model->Meshes)
            {
                if (mesh->Material->AlphaMode == AlphaMode::Blend)
                {
                    transparentNodes.emplace(distance, std::make_pair(node, mesh));
                }
                else
                {
                    opaqueNodes.emplace(distance, std::make_pair(node, mesh));
                }
            }
        }
    }
}

void vge::GeometrySubpass::Draw(CommandBuffer& cmd)
{
    std::multimap<float, std::pair<NodeComponent*, MeshComponent*>> opaqueNodes;
    std::multimap<float, std::pair<NodeComponent*, MeshComponent*>> transparentNodes;

    GetSortedNodes(opaqueNodes, transparentNodes);

    // Draw opaque objects in front-to-back order.
    {
        //ScopedDebugLabel opaque_debug_label{cmd, "Opaque objects"};

        for (auto nodeIt = opaqueNodes.begin(); nodeIt != opaqueNodes.end(); ++nodeIt)
        {
            const auto& [node, mesh] = nodeIt->second;
            UpdateUniform(cmd, *node, _ThreadIndex);

            // Invert the front face if the mesh was flipped.
            const auto& scale = node->Transform.Scale;
            const bool flipped = scale.x * scale.y * scale.z < 0;
            const VkFrontFace frontFace = flipped ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;

            DrawMesh(cmd, *mesh, frontFace);
        }
    }

    // Enable alpha blending.
    ColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.BlendEnable = VK_TRUE;
    colorBlendAttachment.SrcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.DstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.SrcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    ColorBlendState colorBlendState = {};
    colorBlendState.Attachments.resize(GetOutputAttachments().size());
    for (auto& it : colorBlendState.Attachments)
    {
        it = colorBlendAttachment;
    }

    cmd.SetColorBlendState(colorBlendState);
    cmd.SetDepthStencilState(GetDepthStencilState());

    // Draw transparent objects in back-to-front order.
    {
        //ScopedDebugLabel transparent_debug_label{cmd, "Transparent objects"};
        for (auto nodeIt = transparentNodes.rbegin(); nodeIt != transparentNodes.rend(); ++nodeIt)
        {
            const auto& [node, mesh] = nodeIt->second;
            UpdateUniform(cmd, *node, _ThreadIndex);
            DrawMesh(cmd, *mesh);
        }
    }
}

void vge::GeometrySubpass::UpdateUniform(CommandBuffer& cmd, NodeComponent& node, size_t threadIndex)
{
    GlobalUniform globalUniform;

    globalUniform.CameraViewProj = _Camera.PreRotation * VulkanStyleProjection(GetProjMat4(_Camera)) * GetViewMat4(_Camera);

    auto& renderFrame = _RenderContext.GetActiveFrame();
    auto& transform = node.Transform;
    auto allocation = renderFrame.AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalUniform), threadIndex);

    globalUniform.Model = GetWorldMat4(transform);
    globalUniform.CameraPosition = glm::vec3(glm::inverse(GetViewMat4(_Camera))[3]);

    allocation.Update(globalUniform);

    cmd.BindBuffer(allocation.GetBuffer(), allocation.GetOffset(), allocation.GetSize(), 0, 1, 0);
}

void vge::GeometrySubpass::DrawMesh(CommandBuffer& cmd, MeshComponent& mesh, VkFrontFace frontFace)
{
    auto& device = cmd.GetDevice();

    //ScopedDebugLabel submesh_debug_label{cmd, subMesh.get_name().c_str()};

    PreparePipelineState(cmd, frontFace, mesh.Material->DoubleSided);

    MultisampleState multisampleState = {};
    multisampleState.RasterizationSamples = _SampleCount;
    cmd.SetMultisampleState(multisampleState);

    auto& vertShaderModule = device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_VERTEX_BIT, GetVertexShader(), mesh.ShaderVariant);
    auto& fragShaderModule = device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, GetFragmentShader(), mesh.ShaderVariant);

    std::vector<ShaderModule*> shaderModules = { &vertShaderModule, &fragShaderModule };

    auto& pipelineLayout = PreparePipelineLayout(cmd, shaderModules);

    cmd.BindPipelineLayout(pipelineLayout);

    if (pipelineLayout.GetPushConstantRangeStage(sizeof(PBRMaterialUniform)) != 0)
    {
        PreparePushConstants(cmd, mesh);
    }

    DescriptorSetLayout& descriptorSetLayout = pipelineLayout.GetDescriptorSetLayout(0);

    for (auto& texture : mesh.Material->Textures)
    {
        if (auto layoutBinding = descriptorSetLayout.GetLayoutBinding(texture.first))
        {
            cmd.BindImage(
                *texture.second->Image->VkImageView,
                texture.second->Sampler->VkSampler,
                0, layoutBinding->binding, 0);
        }
    }

    auto vertexInputResources = pipelineLayout.GetResources(ShaderResourceType::Input, VK_SHADER_STAGE_VERTEX_BIT);

    VertexInputState vertexInputState;

    for (auto& inputResource : vertexInputResources)
    {
        VertexAttribute attribute;

        if (!GetAttribute(mesh, inputResource.Name, attribute))
        {
            continue;
        }

        VkVertexInputAttributeDescription vertexAttribute = {};
        vertexAttribute.binding = inputResource.Location;
        vertexAttribute.format = attribute.Format;
        vertexAttribute.location = inputResource.Location;
        vertexAttribute.offset = attribute.Offset;

        vertexInputState.Attributes.push_back(vertexAttribute);

        VkVertexInputBindingDescription vertexBinding = {};
        vertexBinding.binding = inputResource.Location;
        vertexBinding.stride = attribute.Stride;

        vertexInputState.Bindings.push_back(vertexBinding);
    }

    cmd.SetVertexInputState(vertexInputState);

    // Find submesh vertex buffers matching the shader input attribute names.
    for (auto& inputResource : vertexInputResources)
    {
        const auto& bufferIt = mesh.VertexBuffers.find(inputResource.Name);

        if (bufferIt != mesh.VertexBuffers.end())
        {
            std::vector<std::reference_wrapper<const Buffer>> buffers;
            buffers.emplace_back(std::ref(bufferIt->second));

            // Bind vertex buffers only for the attribute locations defined.
            cmd.BindVertexBuffers(inputResource.Location, std::move(buffers), {0});
        }
    }

    DrawMeshCommand(cmd, mesh);
}

void vge::GeometrySubpass::PreparePipelineState(CommandBuffer& cmd, VkFrontFace frontFace, bool doubleSidedMaterial)
{
    RasterizationState rasterizationState = _BaseRasterizationState;
    rasterizationState.FrontFace = frontFace;

    if (doubleSidedMaterial)
    {
        rasterizationState.CullMode = VK_CULL_MODE_NONE;
    }

    cmd.SetRasterizationState(rasterizationState);

    MultisampleState multisampleState = {};
    multisampleState.RasterizationSamples = _SampleCount;
    cmd.SetMultisampleState(multisampleState);
}

vge::PipelineLayout& vge::GeometrySubpass::PreparePipelineLayout(CommandBuffer& cmd, const std::vector<ShaderModule*>& shaderModules)
{
    // Sets any specified resource modes
    for (auto& shaderModule : shaderModules)
    {
        for (auto& resource_mode : _ResourceModeMap)
        {
            shaderModule->SetResourceMode(resource_mode.first, resource_mode.second);
        }
    }

    return cmd.GetDevice().GetResourceCache().RequestPipelineLayout(shaderModules);
}

void vge::GeometrySubpass::PreparePushConstants(CommandBuffer& cmd, MeshComponent& mesh)
{
    PBRMaterialUniform pbrMaterialUniform = {};
    pbrMaterialUniform.BaseColorFactor = mesh.Material->BaseColorFactor;
    pbrMaterialUniform.MetallicFactor = mesh.Material->MetallicFactor;
    pbrMaterialUniform.RoughnessFactor = mesh.Material->RoughnessFactor;

    if (const auto data = ToBytes(pbrMaterialUniform); !data.empty())
    {
        cmd.PushConstants(data);
    }
}

void vge::GeometrySubpass::DrawMeshCommand(CommandBuffer& cmd, MeshComponent& mesh)
{
    // Draw submesh indexed if indices exists.
    if (mesh.VertexIndices != 0)
    {
        cmd.BindIndexBuffer(*mesh.IndexBuffer, mesh.IndexOffset, mesh.IndexType);
        cmd.DrawIndexed(mesh.VertexIndices, 1, 0, 0, 0);
    }
    else
    {
        cmd.Draw(mesh.VerticesCount, 1, 0, 0);
    }
}
