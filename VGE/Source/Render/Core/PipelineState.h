#pragma once

#include "VkCommon.h"
#include "Core/Helpers.h"
#include "Core/RenderPass.h"
#include "Core/PipelineLayout.h"

namespace vge
{
struct VertexInputState
{
	std::vector<VkVertexInputBindingDescription> Bindings;
	std::vector<VkVertexInputAttributeDescription> Attributes;
};

struct InputAssemblyState
{
	VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	VkBool32 PrimitiveRestartEnable = VK_FALSE;
};

struct RasterizationState
{
	VkBool32 DepthClampEnable = VK_FALSE;
	VkBool32 RasterizerDiscardEnable = VK_FALSE;
	VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
	VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
	VkFrontFace FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	VkBool32 DepthBiasEnable = VK_FALSE;
};

struct ViewportState
{
	u32 ViewportCount = 1;
	u32 ScissorCount = 1;
};

struct MultisampleState
{
	VkSampleCountFlagBits RasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	VkBool32 SampleShadingEnable = VK_FALSE;
	float MinSampleShading = 0.0f;
	VkSampleMask SampleMask = 0;
	VkBool32 AlphaToCoverageEnable = VK_FALSE;
	VkBool32 AlphaToOneEnable = VK_FALSE;
};

struct StencilOpState
{
	VkStencilOp FailOp = VK_STENCIL_OP_REPLACE;
	VkStencilOp PassOp = VK_STENCIL_OP_REPLACE;
	VkStencilOp DepthFailOp = VK_STENCIL_OP_REPLACE;
	VkCompareOp CompareOp = VK_COMPARE_OP_NEVER;
};

struct DepthStencilState
{
	VkBool32 DepthTestEnable = VK_TRUE;
	VkBool32 DepthWriteEnable = VK_TRUE;
	VkCompareOp DepthCompareOp = VK_COMPARE_OP_GREATER; // using reversed depth-buffer for increased precision, so greater depth values are kept
	VkBool32 DepthBoundsTestEnable = VK_FALSE;
	VkBool32 StencilTestEnable = VK_FALSE;
	StencilOpState Front = {};	
	StencilOpState Back = {};
};

struct ColorBlendAttachmentState
{
	VkBool32 BlendEnable = VK_FALSE;
	VkBlendFactor SrcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	VkBlendFactor DstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	VkBlendOp ColorBlendOp = VK_BLEND_OP_ADD;
	VkBlendFactor SrcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	VkBlendFactor DstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	VkBlendOp AlphaBlendOp = VK_BLEND_OP_ADD;
	VkColorComponentFlags ColorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
};

struct ColorBlendState
{
	VkBool32 LogicOpEnable = VK_FALSE;
	VkLogicOp LogicOp = VK_LOGIC_OP_CLEAR;
	std::vector<ColorBlendAttachmentState> Attachments;
};

// Helper class to create specialization constants for a Vulkan pipeline. 
// The state tracks a pipeline globally, and not per shader. 
// Two shaders using the same constantId will have the same data.
class SpecializationConstantState
{
public:
	inline const std::map<u32, std::vector<uint8_t>>& GetSpecializationConstantState() const { return _SpecializationConstantState; }
	inline void SetSpecializationConstantState(const std::map<u32, std::vector<uint8_t>>& state) { _SpecializationConstantState = state; }
	inline bool IsDirty() const { return _Dirty; }
	inline void ClearDirty() { _Dirty = false; }

	void Reset();

	template <class T>
	void SetConstant(u32 constantId, const T& data);
	void SetConstant(u32 constantId, const std::vector<uint8_t>& data);

private:
	bool _Dirty = false;
	std::map<u32, std::vector<uint8_t>> _SpecializationConstantState;
};

template <class T>
inline void SpecializationConstantState::SetConstant(u32 constantId, const T& data) { SetConstant(constantId, ToBytes(ToU32(data))); }

template <>
inline void SpecializationConstantState::SetConstant<bool>(u32 constantId, const bool& data) { SetConstant(constantId, ToBytes(ToU32(data))); }

class PipelineState
{
public:
	inline const RenderPass* GetRenderPass() const { return _RenderPass; }
	inline const SpecializationConstantState& GetSpecializationConstantState() const { return _SpecializationConstantState; }
	inline const VertexInputState& GetVertexInputState() const { return _VertexInputState; }
	inline const InputAssemblyState& GetInputAssemblyState() const { return _InputAssemblyState; }
	inline const RasterizationState& GetRasterizationState() const { return _RasterizationState; }
	inline const ViewportState& GetViewportState() const { return _ViewportState; }
	inline const MultisampleState& GetMultisampleState() const { return _MultisampleState; }
	inline const DepthStencilState& GetDepthStencilState() const { return _DepthStencilState; }
	inline const ColorBlendState& GetColorBlendState() const { return _ColorBlendState; }
	inline u32 GetSubpassIndex() const { return _SubpassIndex; }
	inline bool IsDirty() const { return _Dirty || _SpecializationConstantState.IsDirty(); }

	void Reset();

	void SetPipelineLayout(PipelineLayout& pipelineLayout);
	void SetRenderPass(const RenderPass& renderPass);
	void SetSpecializationConstant(u32 constantId, const std::vector<uint8_t>& data);
	void SetVertexInputState(const VertexInputState& vertexInputState);
	void SetInputAssemblyState(const InputAssemblyState& inputAssemblyState);
	void SetRasterizationState(const RasterizationState& rasterizationState);
	void SetViewportState(const ViewportState& viewportState);
	void SetMultisampleState(const MultisampleState& multisampleState);
	void SetDepthStencilState(const DepthStencilState& depthStencilState);
	void SetColorBlendState(const ColorBlendState& colorBlendState);
	void SetSubpassIndex(u32 subpassIndex);

	const PipelineLayout& GetPipelineLayout() const;
	void ClearDirty();

private:
	PipelineLayout* _PipelineLayout = nullptr;
	const RenderPass* _RenderPass = nullptr;
	u32 _SubpassIndex = 0U;
	bool _Dirty = false;

	SpecializationConstantState _SpecializationConstantState = {};
	VertexInputState _VertexInputState = {};
	InputAssemblyState _InputAssemblyState = {};
	RasterizationState _RasterizationState = {};
	ViewportState _ViewportState = {};
	MultisampleState _MultisampleState = {};
	DepthStencilState _DepthStencilState = {};
	ColorBlendState _ColorBlendState = {};
};
}	// namespace vge