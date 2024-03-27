#pragma once

#include "Common.h"
#include "Core/Error.h"

namespace vge
{
using ComponentType = u8;
inline constexpr ComponentType GMaxComponentTypes = 128;

using Signature = std::bitset<GMaxComponentTypes>;

// Base Non-Polymorphic class for all components.
struct Component
{
};

// Ensure components are not polymorphic.
static_assert(std::is_polymorphic_v<ComponentType> == false);
    
}
