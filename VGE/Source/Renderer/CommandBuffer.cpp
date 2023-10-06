#include "CommandBuffer.h"
#include "Device.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "Shader.h"
#include "Buffer.h"
#include "Mesh.h"

vge::ScopeCmdBuffer::ScopeCmdBuffer(const Device* device)
	: m_Device(device)
{
	m_CmdBuffer = CommandBuffer::BeginOneTimeSubmit(device);
}

vge::ScopeCmdBuffer::~ScopeCmdBuffer()
{
	CommandBuffer::EndOneTimeSubmit(m_CmdBuffer);
}

vge::CommandBuffer vge::CommandBuffer::Allocate(const Device* device)
{
	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandPool = device->GetCommandPool();
	cmdBufferAllocInfo.commandBufferCount = 1;
	
	CommandBuffer cmd = {};
	cmd.m_Device = device;
	VK_ENSURE(vkAllocateCommandBuffers(device->GetHandle(), &cmdBufferAllocInfo, &cmd.m_Handle));

	return cmd;
}

vge::CommandBuffer vge::CommandBuffer::BeginOneTimeSubmit(const Device* device)
{
	CommandBuffer cmd = CommandBuffer::Allocate(device);
	cmd.BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	return cmd;
}

void vge::CommandBuffer::EndOneTimeSubmit(CommandBuffer& cmd)
{
	cmd.EndRecord();
	cmd.Free();
}

void vge::CommandBuffer::BeginRecord(VkCommandBufferUsageFlags flags /*= 0*/)
{
	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.flags = flags;

	vkBeginCommandBuffer(m_Handle, &cmdBufferBeginInfo);
}

void vge::CommandBuffer::BeginRenderPass(const RenderPass* renderPass, const FrameBuffer* framebuffer)
{
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass->GetHandle();
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = framebuffer->GetExtent();
	renderPassBeginInfo.clearValueCount = renderPass->GetClearValueCount();
	renderPassBeginInfo.pClearValues = renderPass->GetClearValues();
	renderPassBeginInfo.framebuffer = framebuffer->GetHandle();

	vkCmdBeginRenderPass(m_Handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void vge::CommandBuffer::EndRenderPass()
{
	vkCmdEndRenderPass(m_Handle);
}

void vge::CommandBuffer::EndRecord()
{
	vkEndCommandBuffer(m_Handle);
}

void vge::CommandBuffer::Free()
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_Handle;

	vkQueueSubmit(m_Device->GetGfxQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_Device->GetGfxQueue()); // not good if we have a lot of calls to this

	vkFreeCommandBuffers(m_Device->GetHandle(), m_Device->GetCommandPool(), 1, &m_Handle);
}

void vge::CommandBuffer::Bind(const Pipeline* pipeline, VkPipelineBindPoint bindPoint /*= VK_PIPELINE_BIND_POINT_GRAPHICS*/)
{
	vkCmdBindPipeline(m_Handle, bindPoint, pipeline->GetHandle());
}

void vge::CommandBuffer::Bind(const Pipeline* pipeline, uint32 descriptorSetCount, VkDescriptorSet* descriptorSets)
{
	vkCmdBindDescriptorSets(m_Handle, pipeline->GetBindPoint(), pipeline->GetLayout(), 0, descriptorSetCount, descriptorSets, 0, nullptr);
}

void vge::CommandBuffer::Bind(uint32 vertBufferCount, const VertexBuffer** vertBuffers, const Shader* shader, uint32 firstBinding /*= 0*/)
{
	std::vector<VkDeviceSize> offsets(vertBufferCount, 0);
	const uint32 bindingCount = static_cast<uint32>(shader->GetDescriptorSetLayout().Bindings.size());

	std::vector<VkBuffer> vkVertBuffers;
	vkVertBuffers.reserve(vertBufferCount);
	for(size_t i = 0; i < vertBufferCount; ++i)
	{
		vkVertBuffers.push_back(vertBuffers[i]->Get().Handle);
	}

	vkCmdBindVertexBuffers(m_Handle, firstBinding, bindingCount, vkVertBuffers.data(), offsets.data());
}

void vge::CommandBuffer::Bind(const IndexBuffer* idxBuffer, uint32 offset /*= 0*/)
{
	vkCmdBindIndexBuffer(m_Handle, idxBuffer->Get().Handle, offset, idxBuffer->GetIndexType());
}

void vge::CommandBuffer::PushConstants(const Pipeline* pipeline, const Shader* shader, uint32 constantSize, const void* constants, uint32 offset /*= 0*/)
{
	vkCmdPushConstants(m_Handle, pipeline->GetLayout(), shader->GetStageFlags(), offset, constantSize, constants);
}

void vge::CommandBuffer::SetViewport(const glm::vec2& size, const glm::vec2& pos /*= { 0.0f, 0.0f }*/)
{
	VkViewport viewport = {};
	viewport.x = pos.x;
	viewport.y = pos.y;
	viewport.width = size.x;
	viewport.height = size.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(m_Handle, 0, 1, &viewport);
}

void vge::CommandBuffer::SetScissor(const VkExtent2D& extent, const glm::vec<2, int32>& offset /*= { 0.0f, 0.0f }*/)
{
	VkRect2D scissor = {};
	scissor.offset = { offset.x, offset.y };
	scissor.extent = extent;
	vkCmdSetScissor(m_Handle, 0, 1, &scissor);
}

void vge::CommandBuffer::Draw(uint32 vertCount, uint32 instanceCount /*= 1*/, uint32 firstVert /*= 0*/, uint32 firstInstance /*= 0*/)
{
	vkCmdDraw(m_Handle, vertCount, instanceCount, firstVert, firstInstance);
}

void vge::CommandBuffer::DrawIndexed(uint32 idxCount, uint32 instanceCount /*= 1*/, uint32 firstIdx /*= 0*/, int32 vertOffset /*= 0*/, uint32 firstInstance /*= 0*/)
{
	vkCmdDrawIndexed(m_Handle, idxCount, instanceCount, firstIdx, vertOffset, firstInstance);
}
