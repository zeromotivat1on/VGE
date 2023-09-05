#pragma once

#include "Common.h"

namespace vge
{
	using ComponentType = uint8;
	const ComponentType GMaxComponentTypes = 128;

	using Signature = std::bitset<GMaxComponentTypes>;
}
