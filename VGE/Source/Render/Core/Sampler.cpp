#include "Sampler.h"
#include "Device.h"

vge::Sampler::Sampler(const Device& device, const VkSamplerCreateInfo& info)
	: VulkanResource(VK_NULL_HANDLE, &device)
{
	VK_ENSURE(vkCreateSampler(device.GetHandle(), &info, nullptr, &_Handle));
}

vge::Sampler::Sampler(Sampler&& other)
	: VulkanResource(std::move(other))
{
}

vge::Sampler::~Sampler()
{
	if (_Handle)
	{
		vkDestroySampler(_Device->GetHandle(), _Handle, nullptr);
	}
}
