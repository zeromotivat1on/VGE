#pragma once

#include "Common.h"
#include "VulkanUtils.h"

namespace vge
{
	inline class Renderer* GRenderer = nullptr;

	class Renderer final
	{
	public:
		Renderer(GLFWwindow* window);
		~Renderer() = default;

		int32_t Initialize();
		void Cleanup();

	private:
		GLFWwindow* m_Window = nullptr;
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_Gpu = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkQueue m_GrpahicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkFormat m_SwapchainImageFormat = {};
		VkExtent2D m_SwapchainExtent = {};
		std::vector<SwapchainImage> m_SwapchainImages = {};

		VkDebugUtilsMessengerEXT m_DebugMessenger;

	private:
		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void FindGpu();
		void CreateDevice();
		void CreateSwapchain();
	};

	Renderer* CreateRenderer(GLFWwindow* window);
	bool DestroyRenderer();

	void GetGlfwInstanceExtensions(std::vector<const char*>& outExtensions);
	void GetRequriedInstanceExtensions(std::vector<const char*>& outExtensions);
	bool SupportInstanceExtensions(const std::vector<const char*>& checkExtensions);
	bool SupportDeviceExtensions(VkPhysicalDevice gpu, const std::vector<const char*>& checkExtensions);
	bool SuitableGpu(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	SwapchainDetails GetSwapchainDetails(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	VkSurfaceFormatKHR GetBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR GetBestPresentMode(const std::vector<VkPresentModeKHR>& modes);
	VkExtent2D GetBestSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities);
	VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags);
}