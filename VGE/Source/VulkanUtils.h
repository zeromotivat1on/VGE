#pragma once

#include <stdint.h>

struct QueueFamilyIndices
{
	int32_t GraphicsFamily = -1;

	inline bool IsValid()
	{
		return GraphicsFamily >= 0;
	}
};
