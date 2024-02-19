#include "RenderContext.h"

VkFormat vge::RenderContext::DefaultVkFormat = VK_FORMAT_R8G8B8A8_SRGB;

vge::RenderContext::RenderContext(
	Device& device,
	VkSurfaceKHR surface,
	const Window& window,
	VkPresentModeKHR presentMode,
	const std::vector<VkPresentModeKHR>& presentModePriorityList,
	const std::vector<VkSurfaceFormatKHR>& surfaceFormatPriorityList) 
	: 
	_Device(device), 
	_Window(window), 
	_Queue(device.GetSuitableGraphicsQueue()), 
	_SurfaceExtent(window.GetExtent().width, window.GetExtent().height)
{
	if (surface)
	{
		VkSurfaceCapabilitiesKHR surfaceProperties;
		VK_ENSURE(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetGpu().GetHandle(), surface, &surfaceProperties));

		if (surfaceProperties.currentExtent.width == 0xFFFFFFFF)
		{
			_Swapchain = std::make_unique<Swapchain>(device, surface, presentMode, presentModePriorityList, surfaceFormatPriorityList, _SurfaceExtent);
		}
		else
		{
			_Swapchain = std::make_unique<Swapchain>(device, surface, presentMode, presentModePriorityList, surfaceFormatPriorityList);
		}
	}
}

void vge::RenderContext::Prepare(size_t threadCount, RenderTarget::CreateFunc createRenderTargetFunc)
{
	_Device.WaitIdle();

	if (_Swapchain)
	{
		_SurfaceExtent = _Swapchain->GetExtent();

		VkExtent3D extent = { _SurfaceExtent.width, _SurfaceExtent.height, 1 };

		for (auto& imageHandle : _Swapchain->GetImages())
		{
			auto swapchainImage = Image(_Device, imageHandle, extent, _Swapchain->GetFormat(), _Swapchain->get_usage());
			auto renderTarget = createRenderTargetFunc(std::move(swapchainImage));
			_Frames.emplace_back(std::make_unique<RenderFrame>(_Device, std::move(renderTarget), threadCount));
		}
	}
	else
	{
		// Otherwise, create a single RenderFrame.
		_Swapchain = nullptr;

		auto colorImage = Image(_Device, VkExtent3D{ _SurfaceExtent.width, _SurfaceExtent.height, 1 }, DefaultVkFormat,
								VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		auto renderTarget = createRenderTargetFunc(std::move(colorImage));
		_Frames.emplace_back(std::make_unique<RenderFrame>(_Device, std::move(renderTarget), threadCount));
	}

	_CreateRenderTargetFunc = createRenderTargetFunc;
	_ThreadCount = threadCount;
	_Prepared = true;
}

void vge::RenderContext::UpdateSwapchain(const VkExtent2D& extent)
{
	if (!_Swapchain)
	{
		LOG(Warning, "Can't update the swapchains extent in headless mode, skipping.");
		return;
	}

	_Device.GetResourceCache().clear_framebuffers();

	_Swapchain = std::make_unique<Swapchain>(*_Swapchain, extent);

	Recreate();
}

void vge::RenderContext::UpdateSwapchain(const uint32_t imageCount)
{
	if (!_Swapchain)
	{
		LOG(Warning, "Can't update the swapchains image count in headless mode, skipping.");
		return;
	}

	_Device.GetResourceCache().clear_framebuffers();

	_Device.WaitIdle();

	_Swapchain = std::make_unique<Swapchain>(*_Swapchain, imageCount);

	Recreate();
}

void vge::RenderContext::UpdateSwapchain(const std::set<VkImageUsageFlagBits>& imageUsageFlags)
{
	if (!_Swapchain)
	{
		LOG(Warning, "Can't update the swapchains image usage in headless mode, skipping.");
		return;
	}

	_Device.GetResourceCache().clear_framebuffers();

	_Swapchain = std::make_unique<Swapchain>(*_Swapchain, imageUsageFlags);

	Recreate();
}

void vge::RenderContext::UpdateSwapchain(const VkExtent2D& extent, const VkSurfaceTransformFlagBitsKHR transform)
{
	if (!_Swapchain)
	{
		LOG(Warning, "Can't update the swapchains extent and surface transform in headless mode, skipping.");
		return;
	}

	_Device.GetResourceCache().clear_framebuffers();

	u32 width = extent.width;
	u32 height = extent.height;
	if (transform == VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR || transform == VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
	{
		// Pre-rotation: always use native orientation i.e. if rotated, use width and height of identity transform.
		std::swap(width, height);
	}

	_Swapchain = std::make_unique<Swapchain>(*_Swapchain, VkExtent2D{ width, height }, transform);

	// Save the preTransform attribute for future rotations.
	_PreTransform = transform;

	Recreate();
}

void vge::RenderContext::Recreate()
{
	LOG(Log, "Recreated swapchain.");

	VkExtent2D swapchainExtent = _Swapchain->GetExtent();
	VkExtent3D extent = { swapchainExtent.width, swapchainExtent.height, 1 };

	auto frameIt = _Frames.begin();

	for (auto& imageHandle : _Swapchain->get_images())
	{
		Image swapchainImage = Image(_Device, imageHandle, extent, _Swapchain->GetFormat(), _Swapchain->get_usage());

		auto renderTarget = _CreateRenderTargetFunc(std::move(swapchainImage));

		if (frameIt != _Frames.end())
		{
			(*frameIt)->UpdateRenderTarget(std::move(renderTarget));
		}
		else
		{
			// Create a new frame if the new swapchain has more images than current frames.
			_Frames.emplace_back(std::make_unique<RenderFrame>(_Device, std::move(renderTarget), _ThreadCount));
		}

		++frameIt;
	}

	_Device.GetResourceCache().clear_framebuffers();
}

bool vge::RenderContext::HandleSurfaceChanges(bool forceUpdate)
{
	if (!_Swapchain)
	{
		LOG(Warning, "Can't handle surface changes in headless mode, skipping.");
		return false;
	}

	VkSurfaceCapabilitiesKHR surfaceProperties;
	VK_ENSURE(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_Device.GetGpu().GetHandle(), _Swapchain->get_surface(), &surfaceProperties));

	if (surfaceProperties.currentExtent.width == 0xFFFFFFFF)
	{
		return false;
	}

	// Only recreate the swapchain if the dimensions have changed.
	// This method is called on VK_SUBOPTIMAL_KHR, which might not be due to a surface resize.
	if (surfaceProperties.currentExtent.width != _SurfaceExtent.width ||
		surfaceProperties.currentExtent.height != _SurfaceExtent.height ||
		forceUpdate)
	{
		// Recreate swapchain.
		_Device.WaitIdle();

		UpdateSwapchain(surfaceProperties.currentExtent, _PreTransform);

		_SurfaceExtent = surfaceProperties.currentExtent;

		return true;
	}

	return false;
}

vge::CommandBuffer& vge::RenderContext::Begin(CommandBuffer::ResetMode resetMode)
{
	ASSERT_MSG(_Prepared, "RenderContext is not prepared for rendering, call Prepare().");

	if (!_FrameActive)
	{
		BeginFrame();
	}

	if (_AcquiredSemaphore == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Couldn't begin frame");
	}

	const auto& queue = _Device.GetQueueByFlags(VK_QUEUE_GRAPHICS_BIT, 0);
	return GetActiveFrame().RequestCommandBuffer(queue, resetMode);
}

void vge::RenderContext::Submit(const std::vector<CommandBuffer*>& commandBuffers)
{
	ASSERT_MSG(_FrameActive, "RenderContext is inactive, cannot submit command buffer, call Begin().");

	VkSemaphore renderSemaphore = VK_NULL_HANDLE;

	if (_Swapchain)
	{
		ASSERT_MSG(_AcquiredSemaphore, "We do not have acquired semaphore, it was probably consumed?");
		renderSemaphore = Submit(_Queue, commandBuffers, _AcquiredSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	}
	else
	{
		Submit(_Queue, commandBuffers);
	}

	EndFrame(renderSemaphore);
}

void vge::RenderContext::BeginFrame()
{
	// Only handle surface changes if a swapchain exists.
	if (_Swapchain)
	{
		HandleSurfaceChanges();
	}

	ASSERT_MSG(!_FrameActive, "Frame is still active, call EndFrame().");
	ASSERT(_ActiveFrameIndex < _Frames.size());

	auto& prevFrame = *_Frames[_ActiveFrameIndex];

	// We will use the acquired semaphore in a different frame context, so we need to hold ownership.
	_AcquiredSemaphore = prevFrame.RequestSemaphoreWithOwnership();

	if (_Swapchain)
	{
		auto result = _Swapchain->acquire_next_image(_ActiveFrameIndex, _AcquiredSemaphore, VK_NULL_HANDLE);

		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			const bool swapchainUpdated = HandleSurfaceChanges(result == VK_ERROR_OUT_OF_DATE_KHR);

			if (swapchainUpdated)
			{
				result = _Swapchain->acquire_next_image(_ActiveFrameIndex, _AcquiredSemaphore, VK_NULL_HANDLE);
			}
		}

		if (result != VK_SUCCESS)
		{
			prevFrame.Reset();
			return;
		}
	}

	// Now the frame is active again.
	_FrameActive = true;

	// Wait on all resource to be freed from the previous render to this frame.
	WaitFrame();
}

VkSemaphore vge::RenderContext::Submit(const Queue& queue, const std::vector<CommandBuffer*>& commandBuffers, VkSemaphore waitSemaphore, VkPipelineStageFlags waitPipelineStage)
{
	std::vector<VkCommandBuffer> cmdBufHandles(commandBuffers.size(), VK_NULL_HANDLE);
	std::transform(commandBuffers.begin(), commandBuffers.end(), cmdBufHandles.begin(), [](const CommandBuffer* cmdBuf) { return cmdBuf->GetHandle(); });

	RenderFrame& frame = GetActiveFrame();

	VkSemaphore signalSemaphore = frame.RequestSemaphore();

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = ToU32(cmdBufHandles.size());
	submitInfo.pCommandBuffers = cmdBufHandles.data();

	if (waitSemaphore != VK_NULL_HANDLE)
	{
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pWaitDstStageMask = &waitPipelineStage;
	}

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &signalSemaphore;

	VkFence fence = frame.RequestFence();

	queue.Submit({ submitInfo }, fence);

	return signalSemaphore;
}

void vge::RenderContext::Submit(const Queue& queue, const std::vector<CommandBuffer*>& commandBuffers)
{
	std::vector<VkCommandBuffer> cmdBufHandles(commandBuffers.size(), VK_NULL_HANDLE);
	std::transform(commandBuffers.begin(), commandBuffers.end(), cmdBufHandles.begin(), [](const CommandBuffer* cmdBuf) { return cmdBuf->GetHandle(); });

	RenderFrame& frame = GetActiveFrame();

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = ToU32(cmdBufHandles.size());
	submitInfo.pCommandBuffers = cmdBufHandles.data();

	VkFence fence = frame.RequestFence();

	queue.Submit({ submitInfo }, fence);
}

void vge::RenderContext::EndFrame(VkSemaphore semaphore)
{
	ASSERT_MSG(_FrameActive, "Frame is not active, call BeginFrame().");

	if (_Swapchain)
	{
		VkSwapchainKHR vkSwapchain = _Swapchain->get_handle();

		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &semaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &vkSwapchain;
		presentInfo.pImageIndices = &_ActiveFrameIndex;

		VkDisplayPresentInfoKHR dispPresentInfo{};
		if (_Device.IsExtensionSupported(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME) &&
			_Window.GetDisplayPresentInfo(&dispPresentInfo, _SurfaceExtent.width, _SurfaceExtent.height))
		{
			// Add display present info if supported and wanted.
			presentInfo.pNext = &dispPresentInfo;
		}

		VkResult result = _Queue.Present(presentInfo);

		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			HandleSurfaceChanges();
		}
	}

	// Frame is not active anymore.
	if (_AcquiredSemaphore)
	{
		ReleaseOwnedSemaphore(_AcquiredSemaphore);
		_AcquiredSemaphore = VK_NULL_HANDLE;
	}

	_FrameActive = false;
}

VkSemaphore vge::RenderContext::ConsumeAcquiredSemaphore()
{
	ASSERT_MSG(_FrameActive, "Frame is not active, call BeginFrame().");
	auto sem = _AcquiredSemaphore;
	_AcquiredSemaphore = VK_NULL_HANDLE;
	return sem;
}

void vge::RenderContext::RecreateSwapchain()
{
	_Device.WaitIdle();
	_Device.GetResourceCache().clear_framebuffers();

	VkExtent2D swapchainExtent = _Swapchain->GetExtent();
	VkExtent3D extent = { swapchainExtent.width, swapchainExtent.height, 1 };

	auto frameIt = _Frames.begin();

	for (auto& imageHandle : _Swapchain->GetImages())
	{
		auto swapchainImage = Image(_Device, imageHandle, extent, _Swapchain->GetFormat(), _Swapchain->GetUsage());
		auto renderTarget = _CreateRenderTargetFunc(std::move(swapchainImage));
		(*frameIt)->UpdateRenderTarget(std::move(renderTarget));

		++frameIt;
	}
}
