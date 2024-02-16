#include "CommandPool.h"
#include "Device.h"

vge::CommandPool::CommandPool(
	Device& device, u32 queueFamilyIndex, RenderFrame* renderFrame = nullptr,
	size_t threadIndex = 0, CommandBuffer::ResetMode resetMode = CommandBuffer::ResetMode::ResetPool) 
	: _Device(device), _RenderFrame(renderFrame), _ThreadIndex(threadIndex), _ResetMode(resetMode)
{
	VkCommandPoolCreateFlags flags;
	switch (_ResetMode)
	{
	case CommandBuffer::ResetMode::ResetIndividually:
	case CommandBuffer::ResetMode::AlwaysAllocate:
		flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		break;
	case CommandBuffer::ResetMode::ResetPool:
	default:
		flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		break;
	}

	VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	createInfo.queueFamilyIndex = queueFamilyIndex;
	createInfo.flags = flags;

	VK_ENSURE(vkCreateCommandPool(device.GetHandle(), &createInfo, nullptr, &_Handle));
}

vge::CommandPool::~CommandPool()
{
	_PrimaryCommandBuffers.clear();
	_SecondaryCommandBuffers.clear();

	if (_Handle)
	{
		vkDestroyCommandPool(_Device.GetHandle(), _Handle, nullptr);
	}
}

vge::CommandPool::CommandPool(CommandPool&& other) 
	: _Device(other._Device), _Handle(other._Handle), _QueueFamilyIndex(other._QueueFamilyIndex),
	_PrimaryCommandBuffers(std::move(other._PrimaryCommandBuffers)),
	_ActivePrimaryCommandBufferCount(other._ActivePrimaryCommandBufferCount),
	_SecondaryCommandBuffers(std::move(other._SecondaryCommandBuffers)),
	_ActiveSecondaryCommandBufferCount(other._ActiveSecondaryCommandBufferCount),
	_RenderFrame(other._RenderFrame), _ThreadIndex(other._ThreadIndex), _ResetMode(other._ResetMode)
{
	other._Handle = VK_NULL_HANDLE;
	other._QueueFamilyIndex = 0;
	other._ActivePrimaryCommandBufferCount = 0;
	other._ActiveSecondaryCommandBufferCount = 0;
}

VkResult vge::CommandPool::ResetPool()
{
	VkResult result = VK_SUCCESS;

	switch (_ResetMode)
	{
	case CommandBuffer::ResetMode::ResetIndividually:
	{
		result = ResetCommandBuffers();
		break;
	}
	case CommandBuffer::ResetMode::ResetPool:
	{
		result = vkResetCommandPool(_Device.GetHandle(), _Handle, 0);

		if (result != VK_SUCCESS)
		{
			return result;
		}

		result = ResetCommandBuffers();
		break;
	}
	case CommandBuffer::ResetMode::AlwaysAllocate:
	{
		_PrimaryCommandBuffers.clear();
		_ActivePrimaryCommandBufferCount = 0;

		_SecondaryCommandBuffers.clear();
		_ActiveSecondaryCommandBufferCount = 0;

		break;
	}
	default:
		ENSURE_MSG(false, "Unknown reset mode for command pool.");
	}

	return result;
}

VkResult vge::CommandPool::ResetCommandBuffers()
{
	VkResult result = VK_SUCCESS;

	for (auto& cmdBuf : _PrimaryCommandBuffers)
	{
		result = cmdBuf->Reset(_ResetMode);

		if (result != VK_SUCCESS)
		{
			return result;
		}
	}

	_ActivePrimaryCommandBufferCount = 0;

	for (auto& cmdBuf : _SecondaryCommandBuffers)
	{
		result = cmdBuf->Reset(_ResetMode);

		if (result != VK_SUCCESS)
		{
			return result;
		}
	}

	_ActiveSecondaryCommandBufferCount = 0;

	return result;
}

vge::CommandBuffer& vge::CommandPool::RequestCommandBuffer(VkCommandBufferLevel level)
{
	if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
	{
		if (_ActivePrimaryCommandBufferCount < _PrimaryCommandBuffers.size())
		{
			return *_PrimaryCommandBuffers[_ActivePrimaryCommandBufferCount++];
		}

		_PrimaryCommandBuffers.emplace_back(std::make_unique<CommandBuffer>(*this, level));

		_ActivePrimaryCommandBufferCount++;

		return *_PrimaryCommandBuffers.back();
	}
	else
	{
		if (_ActiveSecondaryCommandBufferCount < _SecondaryCommandBuffers.size())
		{
			return *_SecondaryCommandBuffers[_ActiveSecondaryCommandBufferCount++];
		}

		_SecondaryCommandBuffers.emplace_back(std::make_unique<CommandBuffer>(*this, level));

		_ActiveSecondaryCommandBufferCount++;

		return *_SecondaryCommandBuffers.back();
	}
}