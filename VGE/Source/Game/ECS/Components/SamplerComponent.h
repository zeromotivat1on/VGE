#pragma once

#include "ECS/Component.h"
#include "Render/Core/Sampler.h"

namespace vge
{
struct SamplerComponent : public Component
{
    std::unique_ptr<Sampler> VkSampler;
};
}   // namespace vge
