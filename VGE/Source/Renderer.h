#pragma once

#include "Common.h"
#include "Model.h"

namespace vge
{
	inline class Renderer* GRenderer = nullptr;
	
	inline constexpr int32 GMaxDrawFrames = 3;
	inline			 int32 GRenderFrame  = 0;

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
		int32 CreateModel(const char* filename);

		void UpdateModelMatrix(int32 modelIndex, glm::mat4 model) 
		{
			if (modelIndex < m_Models.size()) 
			{ 
				m_Models[modelIndex].SetModelMatrix(model); 
			}
		}

	private:
		std::vector<Model> m_Models = {};
		std::vector<Texture> m_Textures = {};

		UboViewProjection m_UboViewProjection = {};

		GLFWwindow* m_Window = nullptr;
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_Gpu = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VmaAllocator m_Allocator = VK_NULL_HANDLE;
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

		// TODO: create separate structure for subpass data (images, view, memory, format).

		std::vector<VmaImage> m_ColorBufferImages = {};
		std::vector <VkImageView> m_ColorBufferImageViews = {};
		VkFormat m_ColorFormat = VK_FORMAT_UNDEFINED;

		std::vector <VmaImage> m_DepthBufferImages = {};
		std::vector <VkImageView> m_DepthBufferImageViews = {};
		VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;

		VkDescriptorPool m_UniformDescriptorPool = VK_NULL_HANDLE;	// uniform data
		VkDescriptorPool m_SamplerDescriptorPool = VK_NULL_HANDLE;	// texture data (not neccessary to create separate pool)
		VkDescriptorPool m_InputDescriptorPool = VK_NULL_HANDLE;	// input data for separate pipeline and 2 subpass

		VkDescriptorSetLayout m_UniformDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_SamplerDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_InputDescriptorSetLayout = VK_NULL_HANDLE;

		std::vector<VkDescriptorSet> m_UniformDescriptorSets = {};
		std::vector<VkDescriptorSet> m_InputDescriptorSets = {};

		VkPushConstantRange m_PushConstantRange = {};

		std::vector<VmaBuffer> m_VpUniformBuffers = {};
		//std::vector<VkDeviceMemory> m_VpUniformBuffersMemory = {};

		//std::vector<VkBuffer> m_ModelDynamicUniformBuffers = {};
		//std::vector<VkDeviceMemory> m_ModelDynamicUniformBuffersMemory = {};

		//VkDeviceSize m_MinUniformBufferOffset = 0;
		//size_t m_ModelUniformAlignment = 0;
		//ModelData* m_ModelTransferSpace = nullptr;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VkPipeline m_GfxPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_GfxPipelineLayout = VK_NULL_HANDLE;

		VkPipeline m_SecondPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_SecondPipelineLayout = VK_NULL_HANDLE;

		VkCommandPool m_GfxCommandPool = VK_NULL_HANDLE;

		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

	private:
		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void FindGpu();
		void CreateDevice();
		void CreateCustomAllocator();
		void CreateSwapchain();
		void CreateColorBufferImages();
		void CreateDepthBufferImages();
		void CreateRenderPass();
		void CreateDescriptorSetLayouts();
		void CreatePushConstantRange();
		void CreatePipelines();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateTextureSampler();
		//void AllocateDynamicBufferTransferSpace();
		void CreateUniformBuffers();
		void CreateDescriptorPools();
		void CreateDescriptorSets();
		void CreateSyncObjects();

		void RecordCommandBuffers(uint32 ImageIndex);
		void UpdateUniformBuffers(uint32 ImageIndex);
	};

	Renderer* CreateRenderer(GLFWwindow* window);
	bool DestroyRenderer();

	// Get texture names from a given scene, preserves 1 to 1 relationship.
	// If failed to get a texture from material, its name will be empty in out array.
	void GetTexturesFromMaterials(const aiScene* scene, std::vector<const char*>& outTextures);

	// Resolve given textures to be mapped with descriptor sets.
	void ResolveTexturesForDescriptors(Renderer& renderer, const std::vector<const char*>& texturePaths, std::vector<int32>& outTextureToDescriptorSet);

	inline void IncrementRenderFrame() { GRenderFrame = (GRenderFrame + 1) % GMaxDrawFrames; }
}