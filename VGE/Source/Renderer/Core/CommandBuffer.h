#pragma once

#include "Core/VulkanResource.h"
#include "Core/ResourceBindingState.h"
#include "Core/PipelineState.h"
#include "Core/ImageView.h"
#include "Core/Sampler.h"
#include "Core/Buffer.h"

namespace vge
{
class CommandPool;
class DescriptorSet;
class Framebuffer;
class Pipeline;
class PipelineLayout;
class PipelineState;
class RenderTarget;
class Subpass;
struct LightingState;

class CommandBuffer : public VulkanResource<VkCommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER>
{
public:
	enum class ResetMode
	{
		ResetPool,
		ResetIndividually,
		AlwaysAllocate,
	};

	struct RenderPassBinding
	{
		const RenderPass* RenderPass;
		const Framebuffer* Framebuffer;
	};

public:
	CommandBuffer(CommandPool& commandPool, VkCommandBufferLevel level);

	COPY_CTOR_DEL(CommandBuffer);
	CommandBuffer(CommandBuffer&& other);

	~CommandBuffer();

	COPY_OP_DEL(CommandBuffer);
	MOVE_OP_DEL(CommandBuffer);

public:
	void Flush(VkPipelineBindPoint pipelineBindPoint);
	
	VkResult Begin(VkCommandBufferUsageFlags flags, CommandBuffer* primaryCmdBuf = nullptr);
	VkResult Begin(VkCommandBufferUsageFlags flags, const RenderPass* renderPass, const Framebuffer* framebuffer, u32 subpassIndex);
	
	VkResult End();

	void Clear(VkClearAttachment attachment, VkClearRect rect);

	void BeginRenderPass(
		const RenderTarget& renderTarget, 
		const std::vector<LoadStoreInfo>& loadStoreInfos, 
		const std::vector<VkClearValue>& clearValues, 
		const std::vector<std::unique_ptr<Subpass>>& subpasses, 
		VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

	void BeginRenderPass(
		const RenderTarget& renderTarget, 
		const RenderPass& renderPass, 
		const Framebuffer& framebuffer, 
		const std::vector<VkClearValue>& clearValues, 
		VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

	void NextSubpass();

	void ExecuteCommands(CommandBuffer& secondaryCommandBuffer);
	void ExecuteCommands(std::vector<CommandBuffer*>& secondaryCommandBuffers);

	void EndRenderPass();

	void BindPipelineLayout(PipelineLayout& pipeline_layout);

	template <class T>
	inline void SetSpecializationConstant(u32 constantId, const T& data) { SetSpecializationConstant(constantId, ToBytes(data)); }
	template <>
	inline void SetSpecializationConstant(u32 constantId, const bool& data) { SetSpecializationConstant(constantId, ToBytes(ToU32(data))); }
	void SetSpecializationConstant(u32 constantId, const std::vector<u8>& data);

	template <typename T>
	inline void PushConstants(const T& value) { PushConstants(ToBytes(value)); }
	void PushConstants(const std::vector<u8>& values);

	void BindBuffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, u32 set, u32 binding, u32 arrayElement);
	void BindImage(const ImageView& imageView, const Sampler& sampler, u32 set, u32 binding, u32 arrayElement);
	void BindImage(const ImageView& imageView, u32 set, u32 binding, u32 arrayElement);
	void BindInput(const ImageView& imageView, u32 set, u32 binding, u32 arrayElement);
	void BindVertexBuffers(u32 firstBinding, const std::vector<std::reference_wrapper<const Buffer>>& buffers, const std::vector<VkDeviceSize>& offsets);
	void BindIndexBuffer(const Buffer& buffer, VkDeviceSize offset, VkIndexType indexType);
	void BindLighting(LightingState& lightingState, u32 set, u32 binding);

	inline void SetViewportState(const ViewportState& stateInfo) { _PipelineState.SetViewportState(stateInfo); }
	inline void SetVertexInputState(const VertexInputState& stateInfo) { _PipelineState.SetVertexInputState(stateInfo); }
	inline void SetInputAssemblyState(const InputAssemblyState& stateInfo) { _PipelineState.SetInputAssemblyState(stateInfo); }
	inline void SetRasterizationState(const RasterizationState& stateInfo) { _PipelineState.SetRasterizationState(stateInfo); }
	inline void SetMultisampleState(const MultisampleState& stateInfo) { _PipelineState.SetMultisampleState(stateInfo); }
	inline void SetDepthStencilState(const DepthStencilState& stateInfo) { _PipelineState.SetDepthStencilState(stateInfo); }
	inline void SetColorBlendState(const ColorBlendState& stateInfo) { _PipelineState.SetColorBlendState(stateInfo); }

	void SetViewport(u32 firstViewport, const std::vector<VkViewport>& viewports);
	void SetScissor(u32 firstScissor, const std::vector<VkRect2D>& scissors);
	void SetLineWidth(float lineWidth);
	void SetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
	void SetBlendConstants(const std::array<float, 4>& blendConstants);
	void SetDepthBounds(float minDepthBounds, float maxDepthBounds);

	void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);
	void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance);
	void DrawIndexedIndirect(const Buffer& buffer, VkDeviceSize offset, u32 drawCount, u32 stride);

	void Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ);
	void DispatchIndirect(const Buffer& buffer, VkDeviceSize offset);

	void UpdateBuffer(const Buffer& buffer, VkDeviceSize offset, const std::vector<u8>& data);

	void BlitImage(const Image& srcImg, const Image& dstImg, const std::vector<VkImageBlit>& regions);
	void ResolveImage(const Image& srcImg, const Image& dstImg, const std::vector<VkImageResolve>& regions);

	void CopyBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, VkDeviceSize size);
	void CopyImage(const Image& srcImg, const Image& dstImg, const std::vector<VkImageCopy>& regions);
	void CopyBufferToImage(const Buffer& buffer, const Image& image, const std::vector<VkBufferImageCopy>& regions);
	void CopyImageToBuffer(const Image& image, VkImageLayout imageLayout, const Buffer& buffer, const std::vector<VkBufferImageCopy>& regions);

	void BufferMemoryBarrier(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize size, const vge::BufferMemoryBarrier& memoryBarrier);
	void ImageMemoryBarrier(const ImageView& imageView, const vge::ImageMemoryBarrier& memoryBarrier) const;

	inline void SetUpdateAfterBind(bool updateAfterBind) { _UpdateAfterBind = updateAfterBind; }

	void ResetQueryPool(const QueryPool& queryPool, u32 firstQuery, u32 queryCount);
	void BeginQuery(const QueryPool& queryPool, u32 query, VkQueryControlFlags flags);
	void EndQuery(const QueryPool& queryPool, u32 query);

	void WriteTimestamp(VkPipelineStageFlagBits pipelineStage, const QueryPool& queryPool, u32 query);

	VkResult Reset(ResetMode resetMode);

	RenderPass& GetRenderPass(const RenderTarget& renderTarget, const std::vector<LoadStoreInfo>& loadStoreInfos, const std::vector<std::unique_ptr<Subpass>>& subpasses);

public:
	const VkCommandBufferLevel Level;

private:
	inline const RenderPassBinding& GetCurrentRenderPass() const { return _CurrentRenderPass; }
	inline const u32 GetCurrentSubpassIndex() const { return _PipelineState.GetSubpassIndex(); }

	const bool IsRenderSizeOptimal(const VkExtent2D& extent, const VkRect2D& renderArea);

	void FlushPipelineState(VkPipelineBindPoint pipelineBindPoint);
	void FlushDescriptorState(VkPipelineBindPoint pipelineBindPoint);
	void FlushPushConstants();

private:
	CommandPool& _CommandPool;
	RenderPassBinding _CurrentRenderPass;
	PipelineState _PipelineState;
	ResourceBindingState _ResourceBindingState;

	std::vector<u8> _StoredPushConstants;
	u32 _MaxPushConstantsSize;

	VkExtent2D _LastFramebufferExtent = {};
	VkExtent2D _LastRenderAreaExtent = {};

	// If true, it becomes the responsibility of the caller to update ANY descriptor bindings
	// that contain update after bind, as they wont be implicitly updated.
	bool _UpdateAfterBind = false;

	std::unordered_map<u32, DescriptorSetLayout*> _DescriptorSetLayoutBindingState;
};
}	// namespace vge
