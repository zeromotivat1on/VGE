#pragma once

#include "Core/Subpass.h"

namespace vge
{
/**
 * RenderPipeline is a sequence of Subpass objects. Subpass holds shaders and can draw to the scene.
 * 
 * More subpasses can be added to the sequence if required.
 * For example, postprocessing can be implemented with two pipelines which share render targets.
 *
 * GeometrySubpass -> Processes Scene for Shaders, use by itself if shader requires no lighting.
 * ForwardSubpass -> Binds lights at the beginning of a GeometrySubpass to create Forward Rendering, should be used with most default shaders.
 * LightingSubpass -> Holds a Global Light uniform, Can be combined with GeometrySubpass to create Deferred Rendering.
 */
class RenderPipeline
{
public:
    RenderPipeline(std::vector<std::unique_ptr<Subpass>>&& subpasses = {});

    COPY_CTOR_DEL(RenderPipeline);
    MOVE_CTOR_DEF(RenderPipeline);

    virtual ~RenderPipeline() = default;

    COPY_OP_DEL(RenderPipeline);
    MOVE_OP_DEF(RenderPipeline);

public:
    inline std::vector<std::unique_ptr<Subpass>>& GetSubpasses() { return _Subpasses; }
    inline std::unique_ptr<Subpass>& GetActiveSubpass() { return _Subpasses[_ActiveSubpassIndex]; }
    inline const std::vector<LoadStoreInfo>& GetLoadStore() const { return _LoadStoreInfos; }
    inline const std::vector<VkClearValue>& GetClearValue() const { return _ClearValues; }
    inline void set_load_store(const std::vector<LoadStoreInfo>& ls) { _LoadStoreInfos = ls; }
    inline void set_clear_value(const std::vector<VkClearValue>& cv) { _ClearValues = cv; }

    void Prepare();
    void AddSubpass(std::unique_ptr<Subpass>&&);
    void Draw(CommandBuffer&, RenderTarget&, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

private:
    std::vector<std::unique_ptr<Subpass>> _Subpasses;
    std::vector<LoadStoreInfo> _LoadStoreInfos = std::vector<LoadStoreInfo>(2);
    std::vector<VkClearValue> _ClearValues = std::vector<VkClearValue>(2);
    size_t _ActiveSubpassIndex = 0;
};
} // namespace vge
