#pragma once

#include <type_traits>

#include "Types.h"

// General purpose math functions.
namespace vge::math
{
inline u32 HashPCG(u32 input)
{
	const u32 state = input * 747796405u + 2891336453u;
	const u32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
T RandomNumber(u32& seed)
{
	seed = HashPCG(seed);
	return static_cast<T>(seed) / static_cast<T>(UINT32_MAX);
}
}
