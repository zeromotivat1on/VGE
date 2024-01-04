#pragma once

#include "Types.h"
#include <type_traits>

// General purpose math functions.
namespace vge::math
{
	inline f32 RandomF32(u32& seed)
	{
		seed = HashPCG(seed);
		return (f32)seed / (f32)UINT32_MAX;
	}

	inline u32 HashPCG(u32 input)
	{
		u32 state = input * 747796405u + 2891336453u;
		u32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}
}
