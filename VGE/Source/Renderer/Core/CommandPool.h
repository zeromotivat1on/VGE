#pragma once

#include "Core/Common.h"
#include "Core/CommandBuffer.h"

namespace vge
{
class Device;
class RenderFrame;

class CommandPool
{
public:
	CommandPool(
		Device& device, u32 queueFamilyIndex, RenderFrame* renderFrame = nullptr,
		size_t threadIndex = 0, CommandBuffer::ResetMode resetMode = CommandBuffer::ResetMode::ResetPool);

	COPY_CTOR_DEL(CommandPool);
	CommandPool(CommandPool&& other);

	~CommandPool();

	COPY_OP_DEL(CommandPool);
	MOVE_OP_DEL(CommandPool);

public:
	inline Device& GetDevice() { return _Device; }
	inline VkCommandPool GetHandle() const { return _Handle; }
	inline u32 GetQueueFamilyIndex() const { return _QueueFamilyIndex; }
	inline RenderFrame* GetRenderFrame() { return _RenderFrame; }
	inline size_t GetThreadIndex() const { return _ThreadIndex; }
	inline const CommandBuffer::ResetMode GetResetMode() const { return _ResetMode; }

	VkResult ResetPool();
	CommandBuffer& RequestCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

private:
	VkResult ResetCommandBuffers();

private:
	Device& _Device;
	VkCommandPool _Handle = VK_NULL_HANDLE;
	RenderFrame* _RenderFrame = nullptr;
	size_t _ThreadIndex = 0;
	u32 _QueueFamilyIndex = 0;
	std::vector<std::unique_ptr<CommandBuffer>> _PrimaryCommandBuffers;
	u32 _ActivePrimaryCommandBufferCount = 0;
	std::vector<std::unique_ptr<CommandBuffer>> _SecondaryCommandBuffers;
	u32 _ActiveSecondaryCommandBufferCount = 0;
	CommandBuffer::ResetMode _ResetMode = CommandBuffer::ResetMode::ResetPool;
};
}	// namespace vge
