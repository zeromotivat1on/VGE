#pragma once

#include "Common.h"
#include "Mesh.h"

namespace vge
{
	inline class Renderer* GRenderer = nullptr;
	
	inline constexpr int32 GMaxDrawFrames = 2;
	inline			 int32 GCurrentFrame  = 0;

	inline constexpr int32 GMaxSceneObjects = 2;

	struct UboViewProjection
	{
		glm::mat4 Projection;	// how camera see the world
		glm::mat4 View;			// where and from what angle camera is viewing
	};

	class Renderer final
	{
	public:
		Renderer(GLFWwindow* window);
		~Renderer() = default;

		void Initialize();
		void Draw();
		void Cleanup();

		void UpdateModel(int32 modelIndex, glm::mat4 model) { m_Meshes[modelIndex].SetModelMatrix(model); }

	private:
		std::vector<Mesh> m_Meshes = {};

		UboViewProjection m_UboViewProjection = {};

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

		VkImage m_DepthBufferImage = VK_NULL_HANDLE;
		VkImageView m_DepthBufferImageView = VK_NULL_HANDLE;
		VkDeviceMemory m_DepthBufferImageMemory = VK_NULL_HANDLE;
		VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;

		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_DescriptorSets = {};
		VkPushConstantRange m_PushConstantRange = {};

		std::vector<VkBuffer> m_VpUniformBuffers = {};
		std::vector<VkDeviceMemory> m_VpUniformBuffersMemory = {};

		//std::vector<VkBuffer> m_ModelDynamicUniformBuffers = {};
		//std::vector<VkDeviceMemory> m_ModelDynamicUniformBuffersMemory = {};


		//VkDeviceSize m_MinUniformBufferOffset = 0;
		//size_t m_ModelUniformAlignment = 0;
		//ModelData* m_ModelTransferSpace = nullptr;

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
		void CreateDepthBufferImage();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreatePushConstantRange();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		//void AllocateDynamicBufferTransferSpace();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateSyncObjects();

		void RecordCommandBuffers(uint32 ImageIndex);
		void UpdateUniformBuffers(uint32 ImageIndex);
	};

	Renderer* CreateRenderer(GLFWwindow* window);
	bool DestroyRenderer();

	inline void IncrementCurrentFrame();
}