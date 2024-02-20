#include "CommandBuffer.h"
#include "CommandPool.h"
#include "DescriptorSetLayout.h"
#include "Framebuffer.h"
#include "RenderPass.h"
#include "Subpass.h"
#include "Device.h"

vge::CommandBuffer::CommandBuffer(CommandPool& commandPool, VkCommandBufferLevel level)
	: VulkanResource(VK_NULL_HANDLE, &commandPool.GetDevice()),
	_CommandPool(commandPool),
	_MaxPushConstantsSize(_Device->GetGpu().GetProperties().limits.maxPushConstantsSize),
	Level(level)
{
	VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocateInfo.commandPool = commandPool.GetHandle();
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = level;

	VK_ENSURE(vkAllocateCommandBuffers(_Device->GetHandle(), &allocateInfo, &_Handle));
}

vge::CommandBuffer::CommandBuffer(CommandBuffer&& other)
	: VulkanResource(std::move(other)),
	Level(other.Level),
	_CommandPool(other._CommandPool),
	_CurrentRenderPass(std::exchange(other._CurrentRenderPass, {})),
	_PipelineState(std::exchange(other._PipelineState, {})),
	_ResourceBindingState(std::exchange(other._ResourceBindingState, {})),
	_StoredPushConstants(std::exchange(other._StoredPushConstants, {})),
	_MaxPushConstantsSize(std::exchange(other._MaxPushConstantsSize, {})),
	_LastFramebufferExtent(std::exchange(other._LastFramebufferExtent, {})),
	_LastRenderAreaExtent(std::exchange(other._LastRenderAreaExtent, {})),
	_UpdateAfterBind(std::exchange(other._UpdateAfterBind, {})),
	_DescriptorSetLayoutBindingState(std::exchange(other._DescriptorSetLayoutBindingState, {}))
{
}

vge::CommandBuffer::~CommandBuffer()
{
	if (_Handle)
	{
		vkFreeCommandBuffers(GetDevice().GetHandle(), _CommandPool.GetHandle(), 1, &_Handle);
	}
}

void vge::CommandBuffer::Flush(VkPipelineBindPoint pipelineBindPoint)
{
	FlushPipelineState(pipelineBindPoint);
	FlushPushConstants();
	FlushDescriptorState(pipelineBindPoint);
}

VkResult vge::CommandBuffer::Begin(VkCommandBufferUsageFlags flags, CommandBuffer* primaryCmdBuf /*= nullptr*/)
{
	if (Level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
	{
		ASSERT_MSG(primaryCmdBuf, "A primary command buffer pointer must be provided when calling begin from a secondary one");
		const auto renderPassBinding = primaryCmdBuf->GetCurrentRenderPass();

		return Begin(flags, renderPassBinding.RenderPass, renderPassBinding.Framebuffer, primaryCmdBuf->GetCurrentSubpassIndex());
	}

	return Begin(flags, nullptr, nullptr, 0);
}

VkResult vge::CommandBuffer::Begin(VkCommandBufferUsageFlags flags, const RenderPass* renderPass, const Framebuffer* framebuffer, u32 subpassIndex)
{
	_PipelineState.Reset();
	_ResourceBindingState.Reset();
	_DescriptorSetLayoutBindingState.clear();
	_StoredPushConstants.clear();

	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = flags;

	if (Level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
	{
		ASSERT_MSG((renderPass && framebuffer), "Render pass and framebuffer must be provided when calling begin from a secondary one");

		_CurrentRenderPass.RenderPass = renderPass;
		_CurrentRenderPass.Framebuffer = framebuffer;

		VkCommandBufferInheritanceInfo inheritance = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
		inheritance.renderPass = _CurrentRenderPass.RenderPass->GetHandle();
		inheritance.framebuffer = _CurrentRenderPass.Framebuffer->GetHandle();
		inheritance.subpass = subpassIndex;

		beginInfo.pInheritanceInfo = &inheritance;
	}

	return vkBeginCommandBuffer(GetHandle(), &beginInfo);
}

VkResult vge::CommandBuffer::End()
{
	return vkEndCommandBuffer(GetHandle());
}

void vge::CommandBuffer::Clear(VkClearAttachment attachment, VkClearRect rect)
{
	vkCmdClearAttachments(_Handle, 1, &attachment, 1, &rect);
}

void vge::CommandBuffer::BeginRenderPass(
	const RenderTarget& renderTarget, 
	const std::vector<LoadStoreInfo>& loadStoreInfos, 
	const std::vector<VkClearValue>& clearValues, 
	const std::vector<std::unique_ptr<Subpass>>& subpasses, 
	VkSubpassContents contents /*= VK_SUBPASS_CONTENTS_INLINE*/)
{
	_PipelineState.Reset();
	_ResourceBindingState.Reset();
	_DescriptorSetLayoutBindingState.clear();

	const auto& renderPass = GetRenderPass(renderTarget, loadStoreInfos, subpasses);
	const auto& framebuffer = GetDevice().GetResourceCache().RequestFramebuffer(renderTarget, renderPass);

	BeginRenderPass(renderTarget, renderPass, framebuffer, clearValues, contents);
}

void vge::CommandBuffer::BeginRenderPass(
	const RenderTarget& renderTarget, 
	const RenderPass& renderPass, 
	const Framebuffer& framebuffer, 
	const std::vector<VkClearValue>& clearValues, 
	VkSubpassContents contents /*= VK_SUBPASS_CONTENTS_INLINE*/)
{
	_CurrentRenderPass.RenderPass = &renderPass;
	_CurrentRenderPass.Framebuffer = &framebuffer;

	// Begin render pass
	VkRenderPassBeginInfo beginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	beginInfo.renderPass = _CurrentRenderPass.RenderPass->GetHandle();
	beginInfo.framebuffer = _CurrentRenderPass.Framebuffer->GetHandle();
	beginInfo.renderArea.extent = renderTarget.GetExtent();
	beginInfo.clearValueCount = ToU32(clearValues.size());
	beginInfo.pClearValues = clearValues.data();

	const auto& framebuffer_extent = _CurrentRenderPass.Framebuffer->GetExtent();

	// Test the requested render area to confirm that it is optimal and could not cause a performance reduction.
	if (!IsRenderSizeOptimal(framebuffer_extent, beginInfo.renderArea))
	{
		// Only prints the warning if the framebuffer or render area are different since the last time the render size was not optimal.
		if (framebuffer_extent.width != _LastFramebufferExtent.width || framebuffer_extent.height != _LastFramebufferExtent.height ||
			beginInfo.renderArea.extent.width != _LastRenderAreaExtent.width || beginInfo.renderArea.extent.height != _LastRenderAreaExtent.height)
		{
			LOG(Warning, "Render target extent is not an optimal size, this may result in reduced performance.");
		}

		_LastFramebufferExtent = _CurrentRenderPass.Framebuffer->GetExtent();
		_LastRenderAreaExtent = beginInfo.renderArea.extent;
	}

	vkCmdBeginRenderPass(GetHandle(), &beginInfo, contents);

	// Update blend state attachments for first subpass.
	auto blendState = _PipelineState.GetColorBlendState();
	blendState.Attachments.resize(_CurrentRenderPass.RenderPass->GetColorOutputCount(_PipelineState.GetSubpassIndex()));
	_PipelineState.SetColorBlendState(blendState);
}

void vge::CommandBuffer::NextSubpass()
{
	// Increment subpass index.
	_PipelineState.SetSubpassIndex(_PipelineState.GetSubpassIndex() + 1);

	// Update blend state attachments.
	auto blendState = _PipelineState.GetColorBlendState();
	blendState.Attachments.resize(_CurrentRenderPass.RenderPass->GetColorOutputCount(_PipelineState.GetSubpassIndex()));
	_PipelineState.SetColorBlendState(blendState);

	// Reset descriptor sets.
	_ResourceBindingState.Reset();
	_DescriptorSetLayoutBindingState.clear();

	// Clear stored push constants.
	_StoredPushConstants.clear();

	vkCmdNextSubpass(GetHandle(), VK_SUBPASS_CONTENTS_INLINE);
}

void vge::CommandBuffer::ExecuteCommands(CommandBuffer& secondaryCommandBuffer)
{
	vkCmdExecuteCommands(GetHandle(), 1, &secondaryCommandBuffer.GetHandle());
}

void vge::CommandBuffer::ExecuteCommands(std::vector<CommandBuffer*>& secondaryCommandBuffers)
{
	std::vector<VkCommandBuffer> secCmdBufHandles(secondaryCommandBuffers.size(), VK_NULL_HANDLE);
	std::transform(secondaryCommandBuffers.begin(), secondaryCommandBuffers.end(), secCmdBufHandles.begin(),
		[](const CommandBuffer* secCmdBuf) { return secCmdBuf->GetHandle(); });
	
	vkCmdExecuteCommands(GetHandle(), ToU32(secCmdBufHandles.size()), secCmdBufHandles.data());
}

void vge::CommandBuffer::EndRenderPass()
{
	vkCmdEndRenderPass(GetHandle());
}

void vge::CommandBuffer::BindPipelineLayout(PipelineLayout& pipeline_layout)
{
	_PipelineState.SetPipelineLayout(pipeline_layout);
}

void vge::CommandBuffer::SetSpecializationConstant(u32 constantId, const std::vector<u8>& data)
{
	_PipelineState.SetSpecializationConstant(constantId, data);
}

void vge::CommandBuffer::PushConstants(const std::vector<u8>& values)
{
	const u32 pushConstantSize = ToU32(_StoredPushConstants.size() + values.size());

	if (pushConstantSize > _MaxPushConstantsSize)
	{
		ENSURE_MSG(false, "Push constant limit of %d exceeded (pushing %d bytes for a total of %d bytes).", _MaxPushConstantsSize, values.size(), pushConstantSize);
	}
	else
	{
		_StoredPushConstants.insert(_StoredPushConstants.end(), values.begin(), values.end());
	}
}

void vge::CommandBuffer::BindBuffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, u32 set, u32 binding, u32 arrayElement)
{
	_ResourceBindingState.BindBuffer(buffer, offset, range, set, binding, arrayElement);
}

void vge::CommandBuffer::BindImage(const ImageView& imageView, const Sampler& sampler, u32 set, u32 binding, u32 arrayElement)
{
	_ResourceBindingState.BindImage(imageView, sampler, set, binding, arrayElement);
}

void vge::CommandBuffer::BindImage(const ImageView& imageView, u32 set, u32 binding, u32 arrayElement)
{
	_ResourceBindingState.BindImage(imageView, set, binding, arrayElement);
}

void vge::CommandBuffer::BindInput(const ImageView& imageView, u32 set, u32 binding, u32 arrayElement)
{
	_ResourceBindingState.BindInput(imageView, set, binding, arrayElement);
}

void vge::CommandBuffer::BindVertexBuffers(u32 firstBinding, const std::vector<std::reference_wrapper<const Buffer>>& buffers, const std::vector<VkDeviceSize>& offsets)
{
	std::vector<VkBuffer> bufferHandles(buffers.size(), VK_NULL_HANDLE);
	std::transform(buffers.begin(), buffers.end(), bufferHandles.begin(),
		[](const Buffer& buffer) { return buffer.GetHandle(); });
	vkCmdBindVertexBuffers(GetHandle(), firstBinding, ToU32(bufferHandles.size()), bufferHandles.data(), offsets.data());
}

void vge::CommandBuffer::BindIndexBuffer(const Buffer& buffer, VkDeviceSize offset, VkIndexType indexType)
{
	vkCmdBindIndexBuffer(GetHandle(), buffer.GetHandle(), offset, indexType);
}

void vge::CommandBuffer::BindLighting(LightingState& lightingState, u32 set, u32 binding)
{
	BindBuffer(lightingState.light_buffer.get_buffer(), lightingState.light_buffer.get_offset(), lightingState.light_buffer.get_size(), set, binding, 0);

	SetSpecializationConstant(0, ToU32(lightingState.directional_lights.size()));
	SetSpecializationConstant(1, ToU32(lightingState.point_lights.size()));
	SetSpecializationConstant(2, ToU32(lightingState.spot_lights.size()));
}

void vge::CommandBuffer::SetViewport(u32 firstViewport, const std::vector<VkViewport>& viewports)
{
	vkCmdSetViewport(GetHandle(), firstViewport, ToU32(viewports.size()), viewports.data());
}

void vge::CommandBuffer::SetScissor(u32 firstScissor, const std::vector<VkRect2D>& scissors)
{
	vkCmdSetScissor(GetHandle(), firstScissor, ToU32(scissors.size()), scissors.data());
}

void vge::CommandBuffer::SetLineWidth(float lineWidth)
{
	vkCmdSetLineWidth(GetHandle(), lineWidth);
}

void vge::CommandBuffer::SetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
	vkCmdSetDepthBias(GetHandle(), depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

void vge::CommandBuffer::SetBlendConstants(const std::array<float, 4>& blendConstants)
{
	vkCmdSetBlendConstants(GetHandle(), blendConstants.data());
}

void vge::CommandBuffer::SetDepthBounds(float minDepthBounds, float maxDepthBounds)
{
	vkCmdSetDepthBounds(GetHandle(), minDepthBounds, maxDepthBounds);
}

void vge::CommandBuffer::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
{
	Flush(VK_PIPELINE_BIND_POINT_GRAPHICS);
	vkCmdDraw(GetHandle(), vertexCount, instanceCount, firstVertex, firstInstance);
}

void vge::CommandBuffer::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance)
{
	Flush(VK_PIPELINE_BIND_POINT_GRAPHICS);
	vkCmdDrawIndexed(GetHandle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void vge::CommandBuffer::DrawIndexedIndirect(const Buffer& buffer, VkDeviceSize offset, u32 drawCount, u32 stride)
{
	Flush(VK_PIPELINE_BIND_POINT_GRAPHICS);
	vkCmdDrawIndexedIndirect(GetHandle(), buffer.GetHandle(), offset, drawCount, stride);
}

void vge::CommandBuffer::Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
{
	Flush(VK_PIPELINE_BIND_POINT_COMPUTE);
	vkCmdDispatch(GetHandle(), groupCountX, groupCountY, groupCountZ);
}

void vge::CommandBuffer::DispatchIndirect(const Buffer& buffer, VkDeviceSize offset)
{
	Flush(VK_PIPELINE_BIND_POINT_COMPUTE);
	vkCmdDispatchIndirect(GetHandle(), buffer.GetHandle(), offset);
}

void vge::CommandBuffer::UpdateBuffer(const Buffer& buffer, VkDeviceSize offset, const std::vector<uint8_t>& data)
{
	vkCmdUpdateBuffer(GetHandle(), buffer.GetHandle(), offset, data.size(), data.data());
}

void vge::CommandBuffer::BlitImage(const Image& srcImg, const Image& dstImg, const std::vector<VkImageBlit>& regions)
{
	vkCmdBlitImage(GetHandle(), srcImg.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dstImg.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		ToU32(regions.size()), regions.data(), VK_FILTER_NEAREST);
}

void vge::CommandBuffer::ResolveImage(const Image& srcImg, const Image& dstImg, const std::vector<VkImageResolve>& regions)
{
	vkCmdResolveImage(GetHandle(), srcImg.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dstImg.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		ToU32(regions.size()), regions.data());
}

void vge::CommandBuffer::CopyBuffer(const Buffer& src_buffer, const Buffer& dst_buffer, VkDeviceSize size)
{
	VkBufferCopy copy_region = {};
	copy_region.size = size;
	vkCmdCopyBuffer(GetHandle(), src_buffer.GetHandle(), dst_buffer.GetHandle(), 1, &copy_region);
}

void vge::CommandBuffer::CopyImage(const Image& srcImg, const Image& dstImg, const std::vector<VkImageCopy>& regions)
{
	vkCmdCopyImage(GetHandle(), srcImg.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dstImg.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		ToU32(regions.size()), regions.data());
}

void vge::CommandBuffer::CopyBufferToImage(const Buffer& buffer, const Image& image, const std::vector<VkBufferImageCopy>& regions)
{
	vkCmdCopyBufferToImage(GetHandle(), buffer.GetHandle(),
		image.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		ToU32(regions.size()), regions.data());
}

void vge::CommandBuffer::CopyImageToBuffer(const Image& image, VkImageLayout imageLayout, const Buffer& buffer, const std::vector<VkBufferImageCopy>& regions)
{
	vkCmdCopyImageToBuffer(GetHandle(), image.GetHandle(), imageLayout,
		buffer.GetHandle(), ToU32(regions.size()), regions.data());
}

void vge::CommandBuffer::ImageMemoryBarrier(const ImageView& imageView, const vge::ImageMemoryBarrier& memoryBarrier) const
{
	// Adjust barrier's subresource range for depth images.
	auto subresourceRange = imageView.GetSubresourceRange();
	auto format = imageView.GetFormat();

	if (IsDepthOnlyFormat(format))
	{
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else if (IsDepthStencilFormat(format))
	{
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	ImageLayoutTransition(GetHandle(),
		imageView.GetImage().GetHandle(),
		memoryBarrier.SrcStageMask,
		memoryBarrier.DstStageMask,
		memoryBarrier.SrcAccessMask,
		memoryBarrier.DstAccessMask,
		memoryBarrier.OldLayout,
		memoryBarrier.NewLayout,
		subresourceRange);
}

void vge::CommandBuffer::BufferMemoryBarrier(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize size, const vge::BufferMemoryBarrier& memoryBarrier)
{
	VkBufferMemoryBarrier bufferMemoryBarrier = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
	bufferMemoryBarrier.srcAccessMask = memoryBarrier.SrcAccessMask;
	bufferMemoryBarrier.dstAccessMask = memoryBarrier.DstAccessMask;
	bufferMemoryBarrier.buffer = buffer.GetHandle();
	bufferMemoryBarrier.offset = offset;
	bufferMemoryBarrier.size = size;

	VkPipelineStageFlags srcStageMask = memoryBarrier.SrcStageMask;
	VkPipelineStageFlags dstStageMask = memoryBarrier.DstStageMask;

	vkCmdPipelineBarrier(
		GetHandle(),
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		1, &bufferMemoryBarrier,
		0, nullptr);
}

void vge::CommandBuffer::FlushPipelineState(VkPipelineBindPoint pipelineBindPoint)
{
	// Create a new pipeline only if the graphics state changed
	if (!_PipelineState.IsDirty())
	{
		return;
	}

	_PipelineState.ClearDirty();

	// Create and bind pipeline
	if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
	{
		_PipelineState.SetRenderPass(*_CurrentRenderPass.RenderPass);
		auto& pipeline = _Device.GetResourceCache().request_graphics_pipeline(_PipelineState);

		vkCmdBindPipeline(GetHandle(),
			pipelineBindPoint,
			pipeline.GetHandle());
	}
	else if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_COMPUTE)
	{
		auto& pipeline = _Device().GetResourceCache().request_compute_pipeline(_PipelineState);

		vkCmdBindPipeline(GetHandle(),
			pipelineBindPoint,
			pipeline.GetHandle());
	}
	else
	{
		ENSURE_MSG(false, "Only graphics and compute pipeline bind points are supported now");
	}
}

void vge::CommandBuffer::FlushDescriptorState(VkPipelineBindPoint pipeline_bind_point)
{
	ASSERT_MSG(_CommandPool.GetRenderFrame(), "The command pool must be associated to a render frame");

	const auto& pipelineLayout = _PipelineState.GetPipelineLayout();

	std::unordered_set<u32> updateDescriptorSets;

	// Iterate over the shader sets to check if they have already been bound.
	// If they have, add the set so that the command buffer later updates it.
	for (auto& setIt : pipelineLayout.GetShaderSets())
	{
		u32 descriptorSetId = setIt.first;
		auto descriptorSetLayoutIt = _DescriptorSetLayoutBindingState.find(descriptorSetId);

		if (descriptorSetLayoutIt != _DescriptorSetLayoutBindingState.end())
		{
			if (descriptorSetLayoutIt->second->GetHandle() != pipelineLayout.GetDescriptorSetLayout(descriptorSetId).GetHandle())
			{
				updateDescriptorSets.emplace(descriptorSetId);
			}
		}
	}

	// Validate that the bound descriptor set layouts exist in the pipeline layout.
	for (auto setIt = _DescriptorSetLayoutBindingState.begin(); setIt != _DescriptorSetLayoutBindingState.end();)
	{
		if (!pipelineLayout.HasDescriptorSetLayout(setIt->first))
		{
			setIt = _DescriptorSetLayoutBindingState.erase(setIt);
		}
		else
		{
			++setIt;
		}
	}

	// Check if a descriptor set needs to be created.
	if (_ResourceBindingState.IsDirty() || !updateDescriptorSets.empty())
	{
		_ResourceBindingState.ClearDirty();

		// Iterate over all of the resource sets bound by the command buffer.
		for (const auto& resourceSetIt : _ResourceBindingState.GetResourceSets())
		{
			const u32 descriptorSetId = resourceSetIt.first;
			const ResourceSet& resourceSet = resourceSetIt.second;

			// Don't update resource set if it's not in the update list OR its state hasn't changed.
			if (!resourceSet.IsDirty() && (updateDescriptorSets.find(descriptorSetId) == updateDescriptorSets.end()))
			{
				continue;
			}

			// Clear dirty flag for resource set.
			_ResourceBindingState.ClearDirty(descriptorSetId);

			// Skip resource set if a descriptor set layout doesn't exist for it.
			if (!pipelineLayout.HasDescriptorSetLayout(descriptorSetId))
			{
				continue;
			}

			DescriptorSetLayout& descriptorSetLayout = pipelineLayout.GetDescriptorSetLayout(descriptorSetId);

			// Make descriptor set layout bound for current set.
			_DescriptorSetLayoutBindingState[descriptorSetId] = &descriptorSetLayout;

			BindingMap<VkDescriptorBufferInfo> bufferInfos;
			BindingMap<VkDescriptorImageInfo> imageInfos;

			std::vector<u32> dynamicOffsets;

			// Iterate over all resource bindings.
			for (auto& bindingIt : resourceSet.GetResourceBindings())
			{
				const u32 bindingIndex = bindingIt.first;
				const auto& bindingResources = bindingIt.second;

				// Check if binding exists in the pipeline layout.
				if (auto bindingInfo = descriptorSetLayout.GetLayoutBinding(bindingIndex))
				{
					// Iterate over all binding resources
					for (const auto& elementIt : bindingResources)
					{
						const u32 arrayElement = elementIt.first;
						const ResourceInfo& resourceInfo = elementIt.second;

						// Pointer references.
						auto& buffer = resourceInfo.Buffer;
						auto& sampler = resourceInfo.Sampler;
						auto& imageView = resourceInfo.ImageView;

						// Get buffer info.
						if (buffer && IsBufferDescriptorType(bindingInfo->descriptorType))
						{
							VkDescriptorBufferInfo bufferInfo = {};
							bufferInfo.buffer = resourceInfo.Buffer->GetHandle();
							bufferInfo.offset = resourceInfo.Offset;
							bufferInfo.range = resourceInfo.Range;

							if (IsDynamicBufferDescriptorType(bindingInfo->descriptorType))
							{
								dynamicOffsets.push_back(ToU32(bufferInfo.offset));
								bufferInfo.offset = 0;
							}

							bufferInfos[bindingIndex][arrayElement] = bufferInfo;
						}

						// Get image info.
						else if (imageView || sampler)
						{
							// Can be null for input attachments.
							VkDescriptorImageInfo imageInfo = {};
							imageInfo.sampler = sampler ? sampler->GetHandle() : VK_NULL_HANDLE;
							imageInfo.imageView = imageView->GetHandle();

							if (imageView)
							{
								// Add image layout info based on descriptor type.
								switch (bindingInfo->descriptorType)
								{
								case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
									imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
									break;
								case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
									if (IsDepthFormat(imageView->GetFormat()))
									{
										imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
									}
									else
									{
										imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
									}
									break;
								case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
									imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
									break;

								default:
									continue;
								}
							}

							imageInfos[bindingIndex][arrayElement] = imageInfo;
						}
					}

					ASSERT_MSG((!_UpdateAfterBind || (bufferInfos.count(bindingIndex) > 0 || (imageInfos.count(bindingIndex) > 0))),
						"Binding index with no buffer or image infos can't be checked for adding to bindings_to_update.");
				}
			}

			VkDescriptorSet descriptorSetHandle =
				_CommandPool.GetRenderFrame()->RequestDescriptorSet(descriptorSetLayout,
					bufferInfos,
					imageInfos,
					_UpdateAfterBind,
					_CommandPool.GetThreadIndex());

			// Bind descriptor set
			vkCmdBindDescriptorSets(GetHandle(),
				pipeline_bind_point,
				pipelineLayout.GetHandle(),
				descriptorSetId,
				1, &descriptorSetHandle,
				ToU32(dynamicOffsets.size()),
				dynamicOffsets.data());
		}
	}
}

void vge::CommandBuffer::FlushPushConstants()
{
	if (_StoredPushConstants.empty())
	{
		return;
	}

	const PipelineLayout& pipelineLayout = _PipelineState.GetPipelineLayout();
	VkShaderStageFlags shaderStage = pipelineLayout.GetPushConstantRangeStage(ToU32(_StoredPushConstants.size()));

	if (shaderStage)
	{
		vkCmdPushConstants(GetHandle(), pipelineLayout.GetHandle(), shaderStage, 0, ToU32(_StoredPushConstants.size()), _StoredPushConstants.data());
	}
	else
	{
		LOG(Warning, "Push constant range [%d, %d] not found.", 0, _StoredPushConstants.size());
	}

	_StoredPushConstants.clear();
}


const bool vge::CommandBuffer::IsRenderSizeOptimal(const VkExtent2D& framebufferExtent, const VkRect2D& renderArea)
{
	const VkExtent2D renderAreaGranularity = _CurrentRenderPass.RenderPass->GetRenderAreaGranularity();

	return ((renderArea.offset.x % renderAreaGranularity.width == 0) && (renderArea.offset.y % renderAreaGranularity.height == 0) 
		&& ((renderArea.extent.width % renderAreaGranularity.width == 0) || (renderArea.offset.x + renderArea.extent.width == framebufferExtent.width)) 
		&& ((renderArea.extent.height % renderAreaGranularity.height == 0) || (renderArea.offset.y + renderArea.extent.height == framebufferExtent.height)));
}

//void vge::CommandBuffer::ResetQueryPool(const QueryPool& queryPool, u32 firstQuery, u32 queryCount)
//{
//	vkCmdResetQueryPool(GetHandle(), queryPool.GetHandle(), firstQuery, queryCount);
//}
//
//void vge::CommandBuffer::BeginQuery(const QueryPool& queryPool, u32 query, VkQueryControlFlags flags)
//{
//	vkCmdBeginQuery(GetHandle(), queryPool.GetHandle(), query, flags);
//}
//
//void vge::CommandBuffer::EndQuery(const QueryPool& queryPool, u32 query)
//{
//	vkCmdEndQuery(GetHandle(), queryPool.GetHandle(), query);
//}
//
//void vge::CommandBuffer::WriteTimestamp(VkPipelineStageFlagBits pipelineStage, const QueryPool& queryPool, u32 query)
//{
//	vkCmdWriteTimestamp(GetHandle(), pipelineStage, queryPool.GetHandle(), query);
//}

VkResult vge::CommandBuffer::Reset(ResetMode resetMode)
{
	VkResult result = VK_SUCCESS;

	ASSERT_MSG(resetMode == _CommandPool.GetResetMode(), "Command buffer reset mode must match the one used by the pool to allocate it");

	if (resetMode == ResetMode::ResetIndividually)
	{
		result = vkResetCommandBuffer(_Handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}

	return result;
}

vge::RenderPass& vge::CommandBuffer::GetRenderPass(const RenderTarget& renderTarget, const std::vector<LoadStoreInfo>& loadStoreInfos, const std::vector<std::unique_ptr<Subpass>>& subpasses)
{
	// Create render pass
	ASSERT_MSG(subpasses.size() > 0, "Cannot create a render pass without any subpass");

	std::vector<SubpassInfo> subpassInfos(subpasses.size());
	auto subpassInfoIt = subpassInfos.begin();
	for (auto& subpass : subpasses)
	{
		subpassInfoIt->InputAttachments = subpass->GetInputAttachments();
		subpassInfoIt->OutputAttachments = subpass->GetOutputAttachments();
		subpassInfoIt->ColorResolveAttachments = subpass->GetColorResolveAttachments();
		subpassInfoIt->DisableDepthStencilAttachment = subpass->GetDisableDepthStencilAttachment();
		subpassInfoIt->DepthStencilResolveMode = subpass->GetDepthStencilResolveMode();
		subpassInfoIt->DepthStencilResolveAttachment = subpass->GetDepthStencilResolveAttachment();
		subpassInfoIt->DebugName = subpass->GetDebugName();

		++subpassInfoIt;
	}

	return _Device.GetResourceCache().RequestRenderPass(renderTarget.GetAttachments(), loadStoreInfos, subpassInfos);
}