#pragma once

#include "VkCommon.h"
#include "Core/Queue.h"
#include "Core/FencePool.h"
#include "Core/BufferPool.h"
#include "Core/CommandPool.h"
#include "Core/RenderTarget.h"
#include "Core/SemaphorePool.h"
#include "Core/DescriptorSet.h"
#include "Core/CommandBuffer.h"
#include "Core/DescriptorPool.h"
#include "Common/ResourceCaching.h"

namespace vge
{
enum class BufferAllocationStrategy : u8
{
	OneAllocationPerBuffer,
	MultipleAllocationsPerBuffer
};

enum class DescriptorManagementStrategy : u8
{
	StoreInCache,
	CreateDirectly
};

/**
* RenderFrame is a container for per-frame data, including BufferPool objects,
* synchronization primitives (semaphores, fences) and the swapchain RenderTarget.
*
* When creating a RenderTarget, we need to provide images that will be used as attachments within a RenderPass. 
* The RenderFrame is responsible for creating a RenderTarget using RenderTarget::CreateFunc. 
* A custom RenderTarget::CreateFunc can be provided if a different render target is required.
*
* A RenderFrame cannot be destroyed individually since frames are managed by the RenderContext, the whole context must be destroyed.
* This is because each RenderFrame holds Vulkan objects, such as the swapchain image.
*/
class RenderFrame
{
public:
	// Block size of a buffer pool in kilobytes.
	static constexpr u32 BufferPoolBlockSize = 256;

	// A map of the supported usages to a multiplier for the BufferPoolBlockSize
	const std::unordered_map<VkBufferUsageFlags, u32> SupportedUsageMap = 
	{
		{ VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 1 },
		{ VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 2 }, // x2 the size of BufferPoolBlockSize since SSBOs are normally much larger than other types of buffers
		{ VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 1 },
		{ VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 1 } 
	};

public:
	RenderFrame(Device& device, std::unique_ptr<RenderTarget>&& renderTarget, size_t thread_count = 1);

	COPY_CTOR_DEL(RenderFrame);
	MOVE_CTOR_DEL(RenderFrame);

	COPY_OP_DEL(RenderFrame);
	MOVE_OP_DEL(RenderFrame);

public:
	inline Device& GetDevice() { return _Device; }
	inline const FencePool& GetFencePool() const { return _FencePool; }
	inline VkFence RequestFence() { return _FencePool.RequestFence();}
	inline const SemaphorePool& GetSemaphorePool() const { return _SemaphorePool; }
	inline VkSemaphore RequestSemaphore() { return _SemaphorePool.RequestSemaphore(); }
	inline VkSemaphore RequestSemaphoreWithOwnership() { return _SemaphorePool.RequestSemaphoreWithOwnership(); }
	// Called when the swapchain changes.
	inline RenderTarget& GetRenderTarget() { return *_SwapchainRenderTarget;}
	inline const RenderTarget& GetRenderTarget() const { return *_SwapchainRenderTarget; }
	
	inline void ReleaseOwnedSemaphore(VkSemaphore semaphore) { _SemaphorePool.ReleaseOwnedSemaphore(semaphore); }
	inline void UpdateRenderTarget(std::unique_ptr<RenderTarget>&& renderTarget) { _SwapchainRenderTarget = std::move(renderTarget);}

	inline void SetBufferAllocationStrategy(BufferAllocationStrategy newStrategy) { _BufferAllocationStrategy = newStrategy; }
	inline void SetDescriptorManagementStrategy(DescriptorManagementStrategy newStrategy) { _DescriptorManagementStrategy = newStrategy; }

	void Reset();

	// Frame should be active at the moment of the command buffer request.
	CommandBuffer& RequestCommandBuffer(
		const Queue&, CommandBuffer::ResetMode = CommandBuffer::ResetMode::ResetPool,
		VkCommandBufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY, size_t threadIndex = 0);

	VkDescriptorSet RequestDescriptorSet(
		const DescriptorSetLayout&, const BindingMap<VkDescriptorBufferInfo>&,
		const BindingMap<VkDescriptorImageInfo>&, bool updateAfterBind, size_t threadIndex = 0);

	void ClearDescriptors();

	BufferAllocation AllocateBuffer(VkBufferUsageFlags, VkDeviceSize, size_t threadIndex = 0);

	void UpdateDescriptorSets(size_t threadIndex = 0);

private:
	static std::vector<u32> CollectBindingsToUpdate(const DescriptorSetLayout&, const BindingMap<VkDescriptorBufferInfo>&, const BindingMap<VkDescriptorImageInfo>&);

	std::vector<std::unique_ptr<CommandPool>>& GetCommandPools(const Queue&, CommandBuffer::ResetMode);

private:
	Device& _Device;
	std::map<u32, std::vector<std::unique_ptr<CommandPool>>> _CommandPools; // command pools for eqch family index
	std::vector<std::unique_ptr<std::unordered_map<size_t, DescriptorPool>>> _DescriptorPools;
	std::vector<std::unique_ptr<std::unordered_map<size_t, DescriptorSet>>> _DescriptorSets;
	FencePool _FencePool;
	SemaphorePool _SemaphorePool;
	size_t _ThreadCount;
	std::unique_ptr<RenderTarget> _SwapchainRenderTarget;
	BufferAllocationStrategy _BufferAllocationStrategy = BufferAllocationStrategy::MultipleAllocationsPerBuffer;
	DescriptorManagementStrategy _DescriptorManagementStrategy = DescriptorManagementStrategy::StoreInCache;
	std::map<VkBufferUsageFlags, std::vector<std::pair<BufferPool, BufferBlock*>>> _BufferPools;
};
}	// namespace vge
