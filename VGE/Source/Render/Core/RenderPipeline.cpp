#include "RenderPipeline.h"

vge::RenderPipeline::RenderPipeline(std::vector<std::unique_ptr<Subpass>>&& subpasses)
    : _Subpasses(std::move(subpasses))
{
    Prepare();

    // Default clear value
    _ClearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    _ClearValues[1].depthStencil = { 0.0f, ~0U };
}

void vge::RenderPipeline::Prepare()
{
    for (auto &subpass : _Subpasses)
    {
        subpass->Prepare();
    }
}

void vge::RenderPipeline::AddSubpass(std::unique_ptr<Subpass>&& subpass)
{
    subpass->Prepare();
    _Subpasses.emplace_back(std::move(subpass));
}

void vge::RenderPipeline::Draw(CommandBuffer& cmd, RenderTarget& target, VkSubpassContents contents)
{
    ASSERT_MSG(!_Subpasses.empty(), "Render pipeline should contain at least one sub-pass");

    // Pad clear values if they're less than render target attachments.
    while (_ClearValues.size() < target.GetAttachments().size())
    {
        _ClearValues.push_back({0.0f, 0.0f, 0.0f, 1.0f});
    }

    for (size_t i = 0; i < _Subpasses.size(); ++i)
    {
        _ActiveSubpassIndex = i;

        auto& subpass = _Subpasses[i];

        subpass->UpdateRenderTargetAttachments(target);

        if (i == 0)
        {
            cmd.BeginRenderPass(target, _LoadStoreInfos, _ClearValues, _Subpasses, contents);
        }
        else
        {
            cmd.NextSubpass();
        }

        //if (subpass->get_debug_name().empty())
        //{
        //    subpass->set_debug_name(fmt::format("RP subpass #{}", i));
        //}
        //ScopedDebugLabel subpass_debug_label{commandBuffer, subpass->get_debug_name().c_str()};

        subpass->Draw(cmd);
    }

    _ActiveSubpassIndex = 0;
}