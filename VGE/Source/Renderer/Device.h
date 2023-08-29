#pragma once

#include "Common.h"

namespace vge
{
	class Window;

	inline class Device* GDevice = nullptr;

	struct QueueFamilyIndices
	{
		int32 GraphicsFamily = -1;
		int32 PresentFamily = -1;

		inline bool IsValid()
		{
			return GraphicsFamily >= 0 && PresentFamily >= 0;
		}
	};

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		std::vector<VkSurfaceFormatKHR> SurfaceFormats;
		std::vector<VkPresentModeKHR> PresentModes;

		inline bool IsValid()
		{
			return !SurfaceFormats.empty() && !PresentModes.empty();
		}
	};

	class Device
	{
	public:
		Device() = default;
		Device(Window* window);
		NOT_COPYABLE(Device);
		NOT_MOVABLE(Device);

		void Initialize();
		void Destroy();

		inline VkCommandPool GetCommandPool() const { return m_CommandPool; }
		inline VkDevice GetDevice() const { return m_Device; }
		inline VkSurfaceKHR GetSurface() const { return m_Surface; }
		inline VmaAllocator GetAllocator() const { return m_Allocator; }
		inline VkQueue GetGfxQueue() const { return m_GfxQueue; }
		inline VkQueue GetPresentQueue() const { return m_PresentQueue; }
		inline QueueFamilyIndices GetQueueIndices() const { return m_QueueIndices; }
		inline SwapchainSupportDetails GetSwapchainDetails() const { return m_SwapchainSupportDetails; }

	private:
		Window* m_Window = nullptr;
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_Gpu = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VmaAllocator m_Allocator = VK_NULL_HANDLE;
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		
		QueueFamilyIndices m_QueueIndices = {};
		VkQueue m_GfxQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;

		SwapchainSupportDetails m_SwapchainSupportDetails = {};

	private:
		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void FindGpu();
		void CreateDevice();
		void FindQueues();
		void CreateCustomAllocator();
		void CreateCommandPool();
	};

	Device* CreateDevice(Window* window);
	bool DestroyDevice();
}
