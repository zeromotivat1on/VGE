#pragma once

#include "Core/Common.h"
#include "Core/RenderPass.h"
#include "Core/RenderTarget.h"

namespace vge
{
class Device;

class Framebuffer
{
public:
	Framebuffer(Device& device, const RenderTarget& renderTarget, const RenderPass& renderPass);

	COPY_CTOR_DEL(Framebuffer);
	Framebuffer(Framebuffer&& other);

	~Framebuffer();

	COPY_OP_DEL(Framebuffer);
	MOVE_OP_DEL(Framebuffer);

public:
	inline VkFramebuffer GetHandle() const { return _Handle; }
	inline const VkExtent2D& GetExtent() const { return _Extent; }

private:
	Device& _Device;
	VkFramebuffer _Handle = VK_NULL_HANDLE;
	VkExtent2D _Extent = {};
};
}	// namespace vge
