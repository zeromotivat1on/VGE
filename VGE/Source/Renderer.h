#pragma once

#include "Common.h"
#include "MeshModel.h"

namespace vge
{
	inline class Renderer* GRenderer = nullptr;
	
	inline constexpr int32 GMaxDrawFrames = 2;
	inline			 int32 GCurrentFrame  = 0;

	inline constexpr int32 GMaxSceneObjects = 32;

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

		// TODO: for now 1 game object can have only 1 texture which is cringe.
		int32 CreateTexture(const char* filename);
		int32 CreateMeshModel(const char* filename);

		void UpdateModelMatrix(int32 modelIndex, glm::mat4 model) 
		{
			if (modelIndex < m_MeshModels.size()) 
			{ 
				m_MeshModels[modelIndex].SetModelMatrix(model); 
			}
		}

	private:
		std::vector<MeshModel> m_MeshModels = {};
		std::vector<Texture> m_Textures = {};

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

		VkSampler m_TextureSampler = VK_NULL_HANDLE;

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

		VkDescriptorPool m_UniformDescriptorPool = VK_NULL_HANDLE;	// uniform data
		VkDescriptorPool m_SamplerDescriptorPool = VK_NULL_HANDLE;	// texture data (not neccessary to create separate pool)
		
		VkDescriptorSetLayout m_UniformDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_SamplerDescriptorSetLayout = VK_NULL_HANDLE;

		std::vector<VkDescriptorSet> m_UniformDescriptorSets = {};
		
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
		void CreateDescriptorSetLayouts();
		void CreatePushConstantRange();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateTextureSampler();
		//void AllocateDynamicBufferTransferSpace();
		void CreateUniformBuffers();
		void CreateDescriptorPools();
		void CreateUniformDescriptorSets();
		void CreateSyncObjects();

		void RecordCommandBuffers(uint32 ImageIndex);
		void UpdateUniformBuffers(uint32 ImageIndex);
	};

	Renderer* CreateRenderer(GLFWwindow* window);
	bool DestroyRenderer();

	inline void IncrementCurrentFrame() { GCurrentFrame = (GCurrentFrame + 1) % GMaxDrawFrames; }
}