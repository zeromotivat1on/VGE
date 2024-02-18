#pragma once

#include "Core/Common.h"

namespace vge
{
class Device;

class SemaphorePool
{
public:
	SemaphorePool(Device& device);

	COPY_CTOR_DEL(SemaphorePool);
	MOVE_CTOR_DEL(SemaphorePool);

	~SemaphorePool();

	COPY_OP_DEL(SemaphorePool);
	MOVE_OP_DEL(SemaphorePool);

public:
	VkSemaphore RequestSemaphore();
	VkSemaphore RequestSemaphoreWithOwnership();
	inline void ReleaseOwnedSemaphore(VkSemaphore semaphore);

	void Reset();

	inline u32 GetActiveSemaphoreCount() const { return _ActiveSemaphoreCount; }

private:
	Device& _Device;
	std::vector<VkSemaphore> _Semaphores;
	std::vector<VkSemaphore> _ReleasedSemaphores;
	u32 _ActiveSemaphoreCount = 0;
};
}	// namespace vge
