#pragma once

#include "Common.h"

namespace vge
{
	using ComponentType = u8;
	inline constexpr ComponentType GMaxComponentTypes = 128;

	using Signature = std::bitset<GMaxComponentTypes>;
}
