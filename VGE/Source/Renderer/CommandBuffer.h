#pragma once

#include "Common.h"
#include "RenderCommon.h"

namespace vge
{
	class Device;
	class RenderPass;
	class Pipeline;
	class IndexBuffer;
	class VertexBuffer;
	class Shader;
	class FrameBuffer;

	class CommandBuffer
	{
	public:
		static CommandBuffer Allocate(const Device* device);
		
		static CommandBuffer BeginOneTimeSubmit(const Device* device);
		static void EndOneTimeSubmit(CommandBuffer& cmd);

	public:
		CommandBuffer() = default;

		inline VkCommandBuffer GetHandle() const { return m_Handle; }
		inline void NextSubpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) { vkCmdNextSubpass(m_Handle, contents); }

		void BeginRecord(VkCommandBufferUsageFlags flags = 0);
		void BeginRenderPass(const RenderPass* renderPass, const FrameBuffer* framebuffer);
		void EndRenderPass();
		void EndRecord();

		void Free();

		void Bind(const Pipeline* pipeline, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
		void Bind(const IndexBuffer* idxBuffer, u32 offset = 0);
		void Bind(u32 vertBufferCount, const VertexBuffer** vertBuffers, const Shader* shader, u32 firstBinding = 0);
		void Bind(const Pipeline* pipeline, u32 descriptorSetCount, VkDescriptorSet* descriptorSets);
		void PushConstants(const Pipeline* pipeline, const Shader* shader, u32 constantSize, const void* constants, u32 offset = 0);
		void SetViewport(const glm::vec2& size, const glm::vec2& pos = { 0.0f, 0.0f });
		void SetScissor(const VkExtent2D& extent, const glm::vec<2, i32>& offset = { 0, 0 });
		void Draw(u32 vertCount, u32 instanceCount = 1, u32 firstVert = 0, u32 firstInstance = 0);
		void DrawIndexed(u32 idxCount, u32 instanceCount = 1, u32 firstIdx = 0, i32 vertOffset = 0, u32 firstInstance = 0);

	private:
		const Device* m_Device = nullptr;
		VkCommandBuffer m_Handle = VK_NULL_HANDLE;
	};

	struct ScopeCmdBuffer
	{
	public:
		ScopeCmdBuffer(const Device* device);
		~ScopeCmdBuffer();

		CommandBuffer& Get() { return m_CmdBuffer; }
		VkCommandBuffer GetHandle() const { return m_CmdBuffer.GetHandle(); }

	private:
		const Device* m_Device = nullptr;
		CommandBuffer m_CmdBuffer = {};
	};
}

