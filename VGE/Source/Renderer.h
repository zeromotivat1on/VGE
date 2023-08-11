#pragma once

#include "Common.h"
#include "Mesh.h"

namespace vge
{
	inline class Renderer* GRenderer = nullptr;
	
	inline constexpr int32_t GMaxDrawFrames = 2;
	inline			 int32_t GCurrentFrame  = 0;

	struct MVP
	{
		glm::mat4 Projection;	// how camera see the world
		glm::mat4 View;			// where and from what angle camera is viewing
		glm::mat4 Model;		// world position
	};

	class Renderer final
	{
	public:
		Renderer(GLFWwindow* window);
		~Renderer() = default;

		void Initialize();
		void Draw();
		void Cleanup();

		void UpdateModel(glm::mat4 model) { m_Mvp.Model = model; }

	private:
		std::vector<Mesh> m_Meshes = {};

		MVP m_Mvp = {};

		GLFWwindow* m_Window = nullptr;
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_Gpu = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		
		QueueFamilyIndices m_QueueIndices = {};
		VkQueue m_GfxQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;

		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkFormat m_SwapchainImageFormat = {};
		VkExtent2D m_SwapchainExtent = {};

		std::vector<VkSemaphore> m_ImageAvailableSemas = {};
		std::vector<VkSemaphore> m_RenderFinishedSemas = {};
		std::vector<VkFence> m_DrawFences = {};

		std::vector<SwapchainImage> m_SwapchainImages = {};
		std::vector<VkFramebuffer> m_SwapchainFramebuffers = {};
		std::vector<VkCommandBuffer> m_CommandBuffers = {};

		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_DescriptorSets = {};

		std::vector<VkBuffer> m_UniformBuffers = {};
		std::vector<VkDeviceMemory> m_UniformBuffersMemory = {};

		VkPipeline m_GfxPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_GfxPipelineLayout = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VkCommandPool m_GfxCommandPool = VK_NULL_HANDLE;

		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

	private:
		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void FindGpu();
		void CreateDevice();
		void CreateSwapchain();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void RecordCommandBuffers();
		void CreateSyncObjects();

		void UpdateUniformBuffer(uint32_t ImageIndex);
	};

	Renderer* CreateRenderer(GLFWwindow* window);
	bool DestroyRenderer();

	VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags);

	inline void IncrementCurrentFrame();
}