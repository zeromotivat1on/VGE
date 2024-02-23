#pragma once

#include "Core/VulkanResource.h"

namespace vge
{
class Device;

class Sampler : public VulkanResource<VkSampler, VK_OBJECT_TYPE_SAMPLER, const Device>
{
public:
	Sampler(const Device& device, const VkSamplerCreateInfo& info);

	COPY_CTOR_DEL(Sampler);
	Sampler(Sampler&& sampler);

	~Sampler();

	COPY_OP_DEL(Sampler);
	MOVE_OP_DEL(Sampler);
};
}	// namespace vge