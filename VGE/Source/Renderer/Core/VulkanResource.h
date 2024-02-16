#pragma once

#include "Core/Common.h"

namespace vge
{
class Device;

// Simple base class for vulkan objects that want to store THandle and TDevice vulkan types.
template <typename THandle, VkObjectType ObjectType, typename TDevice = vge::Device>
class VulkanResource
{
public:
	VulkanResource(THandle handle = VK_NULL_HANDLE, TDevice* device = nullptr) : _Handle(handle), _Device(device) {}

	COPY_CTOR_DEL(VulkanResource);

	VulkanResource(VulkanResource&& other) : _Handle(other._Handle), _Device(other._Device)
	{
		other._Handle = VK_NULL_HANDLE;
	}

	COPY_OP_DEL(VulkanResource);

	VulkanResource& operator=(VulkanResource&& other)
	{
		_Handle = other._Handle;
		_Device = other._Device;

		other._Handle = VK_NULL_HANDLE;

		return *this;
	}

	virtual ~VulkanResource() = default;

public:
	inline VkObjectType GetObjectType() const { return ObjectType; }
	inline TDevice& GetDevice()			const { ASSERT(_Device); return *device; }
	inline const THandle& GetHandle()	const { return handle; }

protected:
	THandle  _Handle;
	TDevice* _Device;
};
}	// namespace vge

