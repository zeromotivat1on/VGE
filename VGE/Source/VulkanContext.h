#pragma once

#include <vulkan/vulkan.h>

namespace vge
{
	// Global context with main persistent vulkan entities.
	class VulkanContext
	{
	public:
		static inline VkInstance Instance;
		static inline VkPhysicalDevice Gpu;
		static inline VkDevice Device;
		static inline VkQueue GfxQueue;
		static inline VkQueue PresentQueue;
	};
}
