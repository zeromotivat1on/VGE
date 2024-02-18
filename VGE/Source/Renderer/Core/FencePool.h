#pragma once

#include "Core/Common.h"

namespace vge
{
class Device;

class FencePool
{
public:
	FencePool(Device& device);

	COPY_CTOR_DEL(FencePool);
	MOVE_CTOR_DEL(FencePool);

	~FencePool();

	COPY_OP_DEL(FencePool);
	MOVE_OP_DEL(FencePool);

public:
	VkFence RequestFence();
	VkResult Wait(u32 timeout = DEFAULT_FENCE_TIMEOUT) const;
	VkResult Reset();

private:
	Device& _Device;
	std::vector<VkFence> _Fences;
	u32 _ActiveFenceCount = 0;
};
}	// namespace vge
