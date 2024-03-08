#pragma once

#include "VkCommon.h"
#include "Core/Error.h"
#include "Core/Swapchain.h"
#include "Core/RenderFrame.h"
#include "Core/RenderTarget.h"

namespace vge
{
class Window;

/**
 * RenderContext acts as a frame manager, with a lifetime that is the same as that of the Application itself.
 * It acts as a container for RenderFrame objects, swapping between them (BeginFrame, EndFrame)
 * and forwarding requests for Vulkan resources to the active frame.
 * Note that it's guaranteed that there is always an active frame.
 * More than one frame can be in-flight in the GPU, thus the need for per-frame resources.
 *
 * It requires a Device to be valid on creation, and will take control of a given Swapchain.
 *
 * For normal rendering (using a Swapchain), the RenderContext can be created by passing in a Swapchain.
 * A RenderFrame will then be created for each Swapchain image.
 *
 * For headless rendering (no Swapchain), the RenderContext can be given a valid Device, and a width and height.
 * A single RenderFrame will then be created.
 */
class RenderContext
{
public:
	// The format to use for the RenderTargets if a Swapchain isn't created.
	static VkFormat DefaultVkFormat;

public:
	RenderContext(
		Device&, VkSurfaceKHR, const Window&, VkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR,
		const std::vector<VkPresentModeKHR>& presentModePriorityList = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR },
		const std::vector<VkSurfaceFormatKHR>& surfaceFormatPriorityList = { {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} });

	COPY_CTOR_DEL(RenderContext);
	MOVE_CTOR_DEL(RenderContext);

	virtual ~RenderContext() = default;

	COPY_OP_DEL(RenderContext);
	MOVE_OP_DEL(RenderContext);

public:
	inline Device& GetDevice() { return _Device; }
	inline RenderFrame& GetActiveFrame() { ASSERT(_FrameActive); ASSERT(_ActiveFrameIndex < _Frames.size()); return *_Frames[_ActiveFrameIndex]; }
	inline RenderFrame& GetLastRenderedFrame() { ASSERT(!_FrameActive); ASSERT(_ActiveFrameIndex < _Frames.size()); return *_Frames[_ActiveFrameIndex]; }
	inline u32 GetActiveFrameIndex() const { ASSERT(_FrameActive); return _ActiveFrameIndex; }
	inline VkSemaphore RequestSemaphore() { return GetActiveFrame().RequestSemaphore(); }
	inline VkSemaphore RequestSemaphoreWithOwnership() { return GetActiveFrame().RequestSemaphoreWithOwnership(); }
	inline VkFormat GetFormat() const { return _Swapchain ? _Swapchain->GetFormat() : DefaultVkFormat; }
	inline const Swapchain& GetSwapchain() const { ASSERT(_Swapchain); return *_Swapchain; }
	inline const VkExtent2D& GetSurfaceExtent() const { return _SurfaceExtent; }
	inline std::vector<std::unique_ptr<RenderFrame>>& GetRenderFrames() { return _Frames; }
	inline bool HasSwapchain() const { return _Swapchain != nullptr; }
	inline void Submit(CommandBuffer& cmd) { Submit({ &cmd }); }
	inline void ReleaseOwnedSemaphore(VkSemaphore semaphore) { GetActiveFrame().ReleaseOwnedSemaphore(semaphore); }
	inline virtual void WaitFrame() { GetActiveFrame().Reset(); } // waits a frame to finish its rendering

	// Prepares the RenderFrames for rendering.
	void Prepare(size_t threadCount = 1, RenderTarget::CreateFunc = RenderTarget::DefaultCreateFunction);

	void UpdateSwapchain(const VkExtent2D& extent);
	void UpdateSwapchain(const u32 imageCount);
	void UpdateSwapchain(const std::set<VkImageUsageFlagBits>& imageUsageFlags);
	void UpdateSwapchain(const VkExtent2D& extent, const VkSurfaceTransformFlagBitsKHR transform);

	void Recreate(); // recreates the RenderFrames, called after every update
	void RecreateSwapchain();

	CommandBuffer& Begin(CommandBuffer::ResetMode = CommandBuffer::ResetMode::ResetPool); // prepares the next available frame for rendering

	void Submit(const std::vector<CommandBuffer*>&);
	void Submit(const Queue&, const std::vector<CommandBuffer*>&);
	VkSemaphore Submit(const Queue&, const std::vector<CommandBuffer*>&, VkSemaphore waitSemaphore, VkPipelineStageFlags waitPipelineStage);

	void BeginFrame();
	void EndFrame(VkSemaphore);

	virtual bool HandleSurfaceChanges(bool forceUpdate = false); // handles surface changes, only applicable if the render context makes use of a swapchain

	VkSemaphore ConsumeAcquiredSemaphore(); // returns the WSI acquire semaphore, only to be used in very special circumstances.

protected:
	VkExtent2D _SurfaceExtent;

private:
	Device& _Device;
	const Window& _Window;
	const Queue& _Queue; // if swapchain exists, then this will be a present supported queue, else a graphics queue
	std::unique_ptr<Swapchain> _Swapchain;
	SwapchainProperties _SwapchainProperties;
	std::vector<std::unique_ptr<RenderFrame>> _Frames;
	VkSemaphore _AcquiredSemaphore;
	bool _Prepared = false;
	u32 _ActiveFrameIndex = 0;
	bool _FrameActive = false;
	RenderTarget::CreateFunc _CreateRenderTargetFunc = RenderTarget::DefaultCreateFunction;
	VkSurfaceTransformFlagBitsKHR _PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	size_t _ThreadCount = 1;
};
}	// namespace vge
