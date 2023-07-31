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

		VkDebugUtilsMessengerEXT m_DebugMessenger;

	private:
		void CreateInstance();
		void SetupDebugMessenger();
		void FindGpu();
		void CreateDevice();
	};

	Renderer* CreateRenderer(GLFWwindow* window);
	bool DestroyRenderer();

	void GetGlfwExtensions(std::vector<const char*>& outExtensions);
	void GetRequriedExtensions(std::vector<const char*>& outExtensions);
	bool SupportInstanceExtensions(const std::vector<const char*>& extensions);
	bool SuitableGpu(VkPhysicalDevice gpu);
	QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice gpu);
}