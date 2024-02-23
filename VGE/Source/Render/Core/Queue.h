#pragma once

#include "VkCommon.h"

namespace vge
{
class Device;
class CommandBuffer;

class Queue
{
public:
	Queue(Device& device, u32 familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent, u32 index);

	COPY_CTOR_DEF(Queue);
	Queue(Queue&& other);

	COPY_OP_DEL(Queue);
	MOVE_OP_DEL(Queue);

public:
	inline const Device& GetDevice() const { return _Device; }
	inline VkQueue GetHandle() const { return _Handle; }
	inline u32 GetFamilyIndex() const { return _FamilyIndex; }
	inline u32 GetIndex() const { return _Index; }
	inline const VkQueueFamilyProperties& GetProperties() const { return _Properties; }
	inline VkBool32 SupportPresent() const { return _CanPresent; }
	inline VkResult WaitIdle() const { vkQueueWaitIdle(_Handle); }

	VkResult Submit(const std::vector<VkSubmitInfo>& submitInfos, VkFence fence) const;
	VkResult Submit(const CommandBuffer& commandBuffer, VkFence fence) const;
	VkResult Present(const VkPresentInfoKHR& presentInfos) const;

private:
	Device& _Device;
	VkQueue _Handle = VK_NULL_HANDLE;
	u32 _FamilyIndex = 0;
	u32 _Index = 0;
	VkBool32 _CanPresent = VK_FALSE;
	VkQueueFamilyProperties _Properties = {};
};
}	// namespace vge
