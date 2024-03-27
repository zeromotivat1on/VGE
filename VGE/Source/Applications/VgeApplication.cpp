#include "VgeApplication.h"

#include "Platform/FileSystem.h"
#include "Scene.h"
#include "Core/Error.h"
#include "Core/Device.h"
#include "Core/Instance.h"
#include "Core/RenderContext.h"
#include "Core/RenderPipeline.h"
#include "Core/Subpasses/GeometrySubpass.h"
#include "Platform/Window.h"
#include "ECS/Coordinator.h"
#include "ECS/Components/AABBComponent.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/ImageComponent.h"
#include "ECS/Components/InputComponent.h"
#include "ECS/Components/MaterialComponent.h"
#include "ECS/Components/TextureComponent.h"
#include "ECS/Components/ModelComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/SamplerComponent.h"

namespace vge
{
// TODO: finalize mesh component creation.
MeshComponent ReadMesh(const aiMesh* mesh)
{
    constexpr glm::vec3 DontCareColor = glm::vec3(0.0f);

    std::vector<Vertex> vertices(mesh->mNumVertices);
    std::vector<u32> indices = {};

    for (u32 i = 0; i < mesh->mNumVertices; ++i)
    {
        vertices[i].Color = DontCareColor;

        vertices[i].Position.x = mesh->mVertices[i].x;
        vertices[i].Position.y = mesh->mVertices[i].y;
        vertices[i].Position.z = mesh->mVertices[i].z;

        if (mesh->mTextureCoords[0])
        {
            vertices[i].UV.x = mesh->mTextureCoords[0][i].x;
            vertices[i].UV.y = mesh->mTextureCoords[0][i].y;
        }
        else
        {
            vertices[i].UV.x = 0.0f;
            vertices[i].UV.x = 0.0f;
        }
    }

    for (u32 i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace face = mesh->mFaces[i];
        for (u32 j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    MeshComponent meshComponent = {};
    meshComponent.VerticesCount = ToU32(vertices.size());
    meshComponent.IndicesCount = ToU32(indices.size());

    return meshComponent;
}
}

std::unique_ptr<vge::VgeApplication> vge::CreateVgeApplication()
{
    return std::make_unique<VgeApplication>();
}

vge::VgeApplication::~VgeApplication()
{
    if (_Device)
    {
        _Device->WaitIdle();
    }

    //scene.reset();
    //stats.reset();
    //gui.reset();

    _RenderContext.reset();
    _Device.reset();

    if (_Surface)
    {
        vkDestroySurfaceKHR(_Instance->GetHandle(), _Surface, nullptr);
    }

    _Instance.reset();
}

bool vge::VgeApplication::Prepare(const ApplicationOptions& options)
{
    if (!Application::Prepare(options))
    {
        return false;
    }

    CreateInstance();
    CreateSurface();
    CreateDevice();

    CreateRenderContext();
    PrepareRenderContext();

    PrepareEcs();
    PrepareScene();
    
    CreateRenderPipeline();
    
    return true;
}

void vge::VgeApplication::Update(float deltaTime)
{
    Application::Update(deltaTime);

    CommandBuffer& cmd = _RenderContext->Begin();
    cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    Draw(cmd, _RenderContext->GetActiveFrame().GetRenderTarget());
    cmd.End();
    _RenderContext->Submit(cmd);
}

void vge::VgeApplication::Finish()
{
    Application::Finish();

    if (_Device)
    {
        _Device->WaitIdle();
    }
}

bool vge::VgeApplication::Resize(const u32 width, const u32 height)
{
    if (!Application::Resize(width, height))
    {
        return false;
    }


    return true;
}

void vge::VgeApplication::ConsumeInputEvent(const InputEvent& inputEvent)
{
    Application::ConsumeInputEvent(inputEvent);

    
}

void vge::VgeApplication::CreateInstance()
{
    for (const char* extension : _Window->GetRequiredSurfaceExtensions())
    {
        AddInstanceExtension(extension);
    }

    const std::vector<const char*>& validationLayers = {};
    _Instance = std::make_unique<Instance>(GetName(), _InstanceExtensions, validationLayers, _Window->IsHeadless(), _ApiVersion);
}

void vge::VgeApplication::CreateSurface()
{
    _Surface = _Window->CreateSurface(*_Instance);
}

void vge::VgeApplication::CreateDevice()
{
    if (!_Window->IsHeadless() || _Instance->IsExtensionEnabled(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
    {
        AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        if (_InstanceExtensions.find(VK_KHR_DISPLAY_EXTENSION_NAME) != _InstanceExtensions.end())
        {
            AddDeviceExtension(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME, /*optional=*/true);
        }
    }

    _Device = std::make_unique<Device>(_Instance->GetSuitableGpu(_Surface), _Surface, _DeviceExtensions);
}

void vge::VgeApplication::CreateRenderContext()
{
    VkPresentModeKHR presentMode = (_Window->GetProperties().Vsync == Window::Vsync::On)
                                       ? VK_PRESENT_MODE_FIFO_KHR
                                       : VK_PRESENT_MODE_MAILBOX_KHR;
    _RenderContext = std::make_unique<RenderContext>(*_Device, _Surface, *_Window, presentMode);
}

void vge::VgeApplication::PrepareRenderContext()
{
    _RenderContext->Prepare();
}

void vge::VgeApplication::PrepareEcs()
{
    ecs::RegisterComponent<NodeComponent>();
    ecs::RegisterComponent<CameraComponent>();
    ecs::RegisterComponent<TransformComponent>();
    ecs::RegisterComponent<ModelComponent>();
    ecs::RegisterComponent<MeshComponent>();
    ecs::RegisterComponent<ImageComponent>();
    ecs::RegisterComponent<TextureComponent>();
    ecs::RegisterComponent<SamplerComponent>();
    ecs::RegisterComponent<AABBComponent>();
    ecs::RegisterComponent<InputComponent>();
    ecs::RegisterComponent<MaterialComponent>();
}

void vge::VgeApplication::PrepareScene()
{
    static const char* DefaultSceneName = "default_scene";
    
    _Scene = std::make_unique<Scene>(DefaultSceneName);

    const Entity rootEntity = ecs::CreateEntity();
    ENSURE_MSG(rootEntity == 0, "Root Entity should be the first created entity.");
    
    NodeComponent rootNode = {};
    rootNode.Id = 0;
    rootNode.Name = DefaultSceneName;
    
    ecs::AddComponent<NodeComponent>(rootEntity, std::move(rootNode));
    
    _Scene->SetRoot(ecs::GetComponent<NodeComponent>(rootEntity));

    const Entity cameraEntity = ecs::CreateEntity();

    NodeComponent cameraNode = {};
    cameraNode.Id = -1;
    cameraNode.Name = "default_camera";

    ecs::AddComponent<NodeComponent>(cameraEntity, std::move(cameraNode));
    auto& defaultCameraNode = ecs::GetComponent<NodeComponent>(cameraEntity);
    
    CameraComponent defaultCamera = CameraComponent::CreateDefault();
    defaultCamera.Node = &defaultCameraNode;

    ecs::AddComponent<CameraComponent>(cameraEntity, std::move(defaultCamera));
    auto& defaultCameraComponent = ecs::GetComponent<CameraComponent>(cameraEntity);
    
    defaultCameraNode.Components.insert_or_assign(ecs::GetComponentType<NodeComponent>(), &defaultCameraComponent);

    _Scene->AddChild(defaultCameraNode);
    _Scene->AddComponent<CameraComponent>(defaultCameraComponent);

    Assimp::Importer importer;
    const auto assimpScene = fs::assimp::ReadAsset("Models/male.obj", importer);
    //_Scene->AddComponent<ModelComponent>();
}

void vge::VgeApplication::CreateRenderPipeline()
{
    ShaderSource vertex("Shaders/first.vert");
    ShaderSource fragment("Shaders/first.frag");
    auto subpass = std::make_unique<GeometrySubpass>(
        *_RenderContext, std::move(vertex), std::move(fragment), *_Scene, ecs::GetComponent<CameraComponent>(1));

    auto renderPipeline = RenderPipeline();
    renderPipeline.AddSubpass(std::move(subpass));
    
    _RenderPipeline = std::make_unique<RenderPipeline>(std::move(renderPipeline));
}

void vge::VgeApplication::Draw(CommandBuffer& cmd, RenderTarget& target)
{
    auto& views = target.GetViews();
    ASSERT(views.size() > 1);

    {
        // Image 0 is the swapchain.
        ImageMemoryBarrier barrier = {};
        barrier.OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.NewLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.SrcAccessMask = 0;
        barrier.DstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.SrcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        barrier.DstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        cmd.ImageMemoryBarrier(views[0], barrier);

        // Skip 1 as it is handled later as a depth-stencil attachment.
        for (size_t i = 2; i < views.size(); ++i)
        {
            cmd.ImageMemoryBarrier(views[i], barrier);
        }
    }

    {
        ImageMemoryBarrier barrier = {};
        barrier.OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.NewLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.SrcAccessMask = 0;
        barrier.DstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.SrcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        barrier.DstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        cmd.ImageMemoryBarrier(views[1], barrier);
    }

    DrawRenderpass(cmd, target);

    {
        ImageMemoryBarrier barrier = {};
        barrier.OldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.NewLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.SrcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.SrcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        barrier.DstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        cmd.ImageMemoryBarrier(views[0], barrier);
    }
}

void vge::VgeApplication::DrawRenderpass(CommandBuffer& cmd, RenderTarget& target)
{
    SetViewportAndScissor(cmd, target.GetExtent());

    Render(cmd);

    //if (gui)
    //{
    //    gui->draw(cmd);
    //}

    cmd.EndRenderPass();
}

void vge::VgeApplication::Render(CommandBuffer& cmd)
{
    if (_RenderPipeline)
    {
        _RenderPipeline->Draw(cmd, _RenderContext->GetActiveFrame().GetRenderTarget());
    }
}

void vge::VgeApplication::SetViewportAndScissor(CommandBuffer& cmd, const VkExtent2D& extent)
{
    VkViewport viewport = {};
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmd.SetViewport(0, {viewport});

    VkRect2D scissor = {};
    scissor.extent = extent;
    cmd.SetScissor(0, {scissor});
}
