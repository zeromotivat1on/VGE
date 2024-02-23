#include "PipelineState.h"


bool operator==(const VkVertexInputAttributeDescription& lhs, const VkVertexInputAttributeDescription& rhs)
{
	return std::tie(lhs.binding, lhs.format, lhs.location, lhs.offset) == std::tie(rhs.binding, rhs.format, rhs.location, rhs.offset);
}

bool operator==(const VkVertexInputBindingDescription& lhs, const VkVertexInputBindingDescription& rhs)
{
	return std::tie(lhs.binding, lhs.inputRate, lhs.stride) == std::tie(rhs.binding, rhs.inputRate, rhs.stride);
}

bool operator==(const vge::ColorBlendAttachmentState& lhs, const vge::ColorBlendAttachmentState& rhs)
{
	return std::tie(lhs.AlphaBlendOp, lhs.BlendEnable, lhs.ColorBlendOp, lhs.ColorWriteMask, lhs.DstAlphaBlendFactor, lhs.DstColorBlendFactor, lhs.SrcAlphaBlendFactor, lhs.SrcColorBlendFactor) ==
		std::tie(rhs.AlphaBlendOp, rhs.BlendEnable, rhs.ColorBlendOp, rhs.ColorWriteMask, rhs.DstAlphaBlendFactor, rhs.DstColorBlendFactor, rhs.SrcAlphaBlendFactor, rhs.SrcColorBlendFactor);
}

bool operator!=(const vge::StencilOpState& lhs, const vge::StencilOpState& rhs)
{
	return std::tie(lhs.CompareOp, lhs.DepthFailOp, lhs.FailOp, lhs.PassOp) != std::tie(rhs.CompareOp, rhs.DepthFailOp, rhs.FailOp, rhs.PassOp);
}

bool operator!=(const vge::VertexInputState& lhs, const vge::VertexInputState& rhs)
{
	return lhs.Attributes != rhs.Attributes || lhs.Bindings != rhs.Bindings;
}

bool operator!=(const vge::InputAssemblyState& lhs, const vge::InputAssemblyState& rhs)
{
	return std::tie(lhs.PrimitiveRestartEnable, lhs.Topology) != std::tie(rhs.PrimitiveRestartEnable, rhs.Topology);
}

bool operator!=(const vge::RasterizationState& lhs, const vge::RasterizationState& rhs)
{
	return std::tie(lhs.CullMode, lhs.DepthBiasEnable, lhs.DepthClampEnable, lhs.FrontFace, lhs.FrontFace, lhs.PolygonMode, lhs.RasterizerDiscardEnable) !=
		std::tie(rhs.CullMode, rhs.DepthBiasEnable, rhs.DepthClampEnable, rhs.FrontFace, rhs.FrontFace, rhs.PolygonMode, rhs.RasterizerDiscardEnable);
}

bool operator!=(const vge::ViewportState& lhs, const vge::ViewportState& rhs)
{
	return lhs.ViewportCount != rhs.ViewportCount || lhs.ScissorCount != rhs.ScissorCount;
}

bool operator!=(const vge::MultisampleState& lhs, const vge::MultisampleState& rhs)
{
	return std::tie(lhs.AlphaToCoverageEnable, lhs.AlphaToOneEnable, lhs.MinSampleShading, lhs.RasterizationSamples, lhs.SampleMask, lhs.SampleShadingEnable) !=
		std::tie(rhs.AlphaToCoverageEnable, rhs.AlphaToOneEnable, rhs.MinSampleShading, rhs.RasterizationSamples, rhs.SampleMask, rhs.SampleShadingEnable);
}

bool operator!=(const vge::DepthStencilState& lhs, const vge::DepthStencilState& rhs)
{
	return std::tie(lhs.DepthBoundsTestEnable, lhs.DepthCompareOp, lhs.DepthTestEnable, lhs.DepthWriteEnable, lhs.StencilTestEnable) !=
		std::tie(rhs.DepthBoundsTestEnable, rhs.DepthCompareOp, rhs.DepthTestEnable, rhs.DepthWriteEnable, rhs.StencilTestEnable) ||
		lhs.Back != rhs.Back || lhs.Front != rhs.Front;
}

bool operator!=(const vge::ColorBlendState& lhs, const vge::ColorBlendState& rhs)
{
	return std::tie(lhs.LogicOp, lhs.LogicOpEnable) != std::tie(rhs.LogicOp, rhs.LogicOpEnable) ||
		lhs.Attachments.size() != rhs.Attachments.size() ||
		!std::equal(lhs.Attachments.begin(), lhs.Attachments.end(), rhs.Attachments.begin(),
			[](const vge::ColorBlendAttachmentState& lhs, const vge::ColorBlendAttachmentState& rhs) {
				return lhs == rhs;
			});
}

void vge::SpecializationConstantState::Reset()
{
	if (_Dirty)
	{
		_SpecializationConstantState.clear();
	}

	_Dirty = false;
}

void vge::SpecializationConstantState::SetConstant(u32 constantId, const std::vector<u8>& value)
{
	auto data = _SpecializationConstantState.find(constantId);

	if (data != _SpecializationConstantState.end() && data->second == value)
	{
		return;
	}

	_Dirty = true;

	_SpecializationConstantState[constantId] = value;
}

void vge::PipelineState::Reset()
{
	ClearDirty();

	_PipelineLayout = nullptr;
	_RenderPass = nullptr;
	_SpecializationConstantState.Reset();
	_VertexInputState = {};
	_InputAssemblyState = {};
	_RasterizationState = {};
	_MultisampleState = {};
	_DepthStencilState = {};
	_ColorBlendState = {};
	_SubpassIndex = 0U;
}

void vge::PipelineState::SetPipelineLayout(PipelineLayout& pipelineLayout)
{
	if (_PipelineLayout)
	{
		if (_PipelineLayout->GetHandle() != pipelineLayout.GetHandle())
		{
			_PipelineLayout = &pipelineLayout;

			_Dirty = true;
		}
	}
	else
	{
		_PipelineLayout = &pipelineLayout;

		_Dirty = true;
	}
}

void vge::PipelineState::SetRenderPass(const RenderPass& renderPass)
{
	if (_RenderPass)
	{
		if (_RenderPass->GetHandle() != renderPass.GetHandle())
		{
			_RenderPass = &renderPass;

			_Dirty = true;
		}
	}
	else
	{
		_RenderPass = &renderPass;

		_Dirty = true;
	}
}

void vge::PipelineState::SetSpecializationConstant(u32 constantId, const std::vector<u8>& data)
{
	_SpecializationConstantState.SetConstant(constantId, data);

	if (_SpecializationConstantState.IsDirty())
	{
		_Dirty = true;
	}
}

void vge::PipelineState::SetVertexInputState(const VertexInputState& vertexInputState)
{
	if (_VertexInputState != vertexInputState)
	{
		_VertexInputState = vertexInputState;

		_Dirty = true;
	}
}

void vge::PipelineState::SetInputAssemblyState(const InputAssemblyState& inputAssemblyState)
{
	if (_InputAssemblyState != inputAssemblyState)
	{
		_InputAssemblyState = inputAssemblyState;

		_Dirty = true;
	}
}

void vge::PipelineState::SetRasterizationState(const RasterizationState& rasterizationState)
{
	if (_RasterizationState != rasterizationState)
	{
		_RasterizationState = rasterizationState;

		_Dirty = true;
	}
}

void vge::PipelineState::SetViewportState(const ViewportState& viewportState)
{
	if (_ViewportState != viewportState)
	{
		_ViewportState = viewportState;

		_Dirty = true;
	}
}

void vge::PipelineState::SetMultisampleState(const MultisampleState& multisampleState)
{
	if (_MultisampleState != multisampleState)
	{
		_MultisampleState = multisampleState;

		_Dirty = true;
	}
}

void vge::PipelineState::SetDepthStencilState(const DepthStencilState& depthStencilState)
{
	if (_DepthStencilState != depthStencilState)
	{
		_DepthStencilState = depthStencilState;

		_Dirty = true;
	}
}

void vge::PipelineState::SetColorBlendState(const ColorBlendState& colorBlendState)
{
	if (_ColorBlendState != colorBlendState)
	{
		_ColorBlendState = colorBlendState;

		_Dirty = true;
	}
}

void vge::PipelineState::SetSubpassIndex(u32 subpassIndex)
{
	if (_SubpassIndex != subpassIndex)
	{
		_SubpassIndex = subpassIndex;

		_Dirty = true;
	}
}

const vge::PipelineLayout& vge::PipelineState::GetPipelineLayout() const
{
	ASSERT_MSG(_PipelineLayout, "Graphics state Pipeline layout is not set.");
	return *_PipelineLayout;
}

void vge::PipelineState::ClearDirty()
{
	_Dirty = false;
	_SpecializationConstantState.ClearDirty();
}