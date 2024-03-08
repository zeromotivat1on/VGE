#pragma once

#include "Platform/Application.h"
#include "Core/VkCommon.h"
#include "Core/RenderContext.h"
#include "Core/CommandBuffer.h"

namespace vge
{
class Device;
class Instance;
class CommandPool;
class SemaphorePool;
class RenderContext;
class RenderPipeline;
    
class VgeApplication final : public Application
{
public:
    ~VgeApplication() override;
    
public:
    bool Prepare(const ApplicationOptions& options) override;
    void Update(float deltaTime) override;
    void Finish() override;
    bool Resize(const u32 width, const u32 height) override;
    void ConsumeInputEvent(const InputEvent& inputEvent) override;

private:
    void CreateInstance();
    void CreateSurface();
    void CreateDevice();
    void CreateRenderContext();
    void PrepareRenderContext();
    void CreateRenderPipeline();
    
    void Draw(CommandBuffer&, RenderTarget&); // prepares the render target and draws to it
    void DrawRenderpass(CommandBuffer&, RenderTarget&); // starts the render pass, executes the render pipeline, and then ends the render pass
    void Render(CommandBuffer&); // triggers the render pipeline
    
    inline void AddDeviceExtension(const char* name, bool optional = false) { _DeviceExtensions[name] = optional; }
    inline void AddInstanceExtension(const char* name, bool optional = false) { _InstanceExtensions[name] = optional; }

    static void SetViewportAndScissor(CommandBuffer&, const VkExtent2D&);

private:
    std::unique_ptr<Instance> _Instance;
    std::unique_ptr<Device> _Device;
    std::unique_ptr<RenderContext> _RenderContext;
    std::unique_ptr<RenderPipeline> _RenderPipeline;
    std::unique_ptr<SemaphorePool> _SemaphorePool;

    VkSurfaceKHR _Surface;
    
    u32 _ApiVersion = VK_API_VERSION_1_0;
    bool _HighPriorityGraphicsQueue = false;
    std::unordered_map<const char *, bool> _DeviceExtensions;
    std::unordered_map<const char *, bool> _InstanceExtensions;
};

std::unique_ptr<vge::VgeApplication> CreateVgeApplication();
}   // namespace vge

