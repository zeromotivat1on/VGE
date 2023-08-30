#pragma once

#include "Common.h"
#include "Window.h"

namespace vge
{
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

		inline Window* GetWindow() const { return m_Window; }
		inline VkInstance GetInstance() const { return m_Instance; }
		inline VkPhysicalDevice GetGpu() const { return m_Gpu; }
		inline VkDevice GetHandle() const { return m_Handle; }
		inline VkSurfaceKHR GetInitialSurface() const { return m_InitialSurface; }
		inline VmaAllocator GetAllocator() const { return m_Allocator; }
		inline VkCommandPool GetCommandPool() const { return m_CommandPool; }
		inline VkQueue GetGfxQueue() const { return m_GfxQueue; }
		inline VkQueue GetPresentQueue() const { return m_PresentQueue; }
		inline QueueFamilyIndices GetQueueIndices() const { return m_QueueIndices; }

		inline bool WasWindowResized() const { return m_Window->WasResized(); }
		inline void ResetWindowResizedFlag() const { m_Window->ResetResizedFlag(); }
		inline void WaitWindowSizeless() const { m_Window->WaitSizeless(); }
		inline void CreateWindowSurface(VkSurfaceKHR& outSurface) const { m_Window->CreateSurface(m_Instance, outSurface); }

		inline void WaitIdle() const { vkDeviceWaitIdle(m_Handle); }

		SwapchainSupportDetails GetSwapchainSupportDetails(VkSurfaceKHR surface) const;

	private:
		Window* m_Window = nullptr;
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_Gpu = VK_NULL_HANDLE;
		VkDevice m_Handle = VK_NULL_HANDLE;
		VkSurfaceKHR m_InitialSurface = VK_NULL_HANDLE;
		VmaAllocator m_Allocator = VK_NULL_HANDLE;
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		
		VkQueue m_GfxQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
		QueueFamilyIndices m_QueueIndices = {};

	private:
		void CreateInstance();
		void SetupDebugMessenger();
		void CreateInitialSurface();
		void FindGpu();
		void CreateDevice();
		void FindQueues();
		void CreateCustomAllocator();
		void CreateCommandPool();
	};

	Device* CreateDevice(Window* window);
	bool DestroyDevice();
}
