#pragma once

#include "Common.h"
#include "Device.h"
#include "Pipeline.h"
#include "Model.h"
#include "Texture.h"
#include "Swapchain.h"

namespace vge
{
	class Window;
	class Device;

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
		Renderer(Device& device);
		NOT_COPYABLE(Renderer);
		~Renderer() = default;

	public:
		void Initialize();
		void Draw();
		void Destroy();

		// TODO: 1 mesh can have only 1 texture which is cringe (or not, idk for now).
		int32 CreateTexture(const char* filename);
		int32 CreateModel(const char* filename);

		void RecreateSwapchain();

		inline void UpdateModelMatrix(int32 id, glm::mat4 model) 
		{
			ASSERT(id < m_Models.size()) 
			m_Models[id].SetModelMatrix(model); 
		}

	private:
		std::vector<Model> m_Models = {};
		std::vector<Texture> m_Textures = {};

		UboViewProjection m_UboViewProjection = {};

		Device& m_Device;
		std::unique_ptr<Swapchain> m_Swapchain = nullptr;
		std::unique_ptr<SwapchainRecreateInfo> m_SwapchainRecreateInfo = nullptr;

		VkSampler m_TextureSampler = VK_NULL_HANDLE;

		std::vector<VkSemaphore> m_ImageAvailableSemas = {};
		std::vector<VkSemaphore> m_RenderFinishedSemas = {};
		std::vector<VkFence> m_DrawFences = {};

		std::vector<VkCommandBuffer> m_CommandBuffers = {};

		// TODO: create separate structure for subpass data (images, view, memory, format).

		std::vector<Image> m_ColorBufferImages = {};
		std::vector<VkImageView> m_ColorBufferImageViews = {};
		VkFormat m_ColorFormat = VK_FORMAT_UNDEFINED;

		std::vector<Image> m_DepthBufferImages = {};
		std::vector<VkImageView> m_DepthBufferImageViews = {};
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

		std::vector<Buffer> m_VpUniformBuffers = {};

		//std::vector<VkBuffer> m_ModelDynamicUniformBuffers = {};
		//std::vector<VkDeviceMemory> m_ModelDynamicUniformBuffersMemory = {};

		//VkDeviceSize m_MinUniformBufferOffset = 0;
		//size_t m_ModelUniformAlignment = 0;
		//ModelData* m_ModelTransferSpace = nullptr;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		Pipeline m_FirstPipeline;
		Pipeline m_SecondPipeline;

	private:
		void CreateSwapchain();
		void CreateColorBufferImages();
		void CreateDepthBufferImages();
		void CreateRenderPass();
		void CreateDescriptorSetLayouts();
		void CreatePushConstantRange();
		void CreatePipelines();
		void CreateFramebuffers();
		void AllocateCommandBuffers();
		void CreateTextureSampler();
		//void AllocateDynamicBufferTransferSpace();
		void CreateUniformBuffers();
		void CreateDescriptorPools();
		void CreateDescriptorSets();
		void CreateSyncObjects();

		void AllocateUniformDescriptorSet();
		void AllocateInputDescriptorSet();
		void UpdateUniformDescriptorSet();
		void UpdateInputDescriptorSet();

		void RecordCommandBuffers(uint32 ImageIndex);
		void UpdateUniformBuffers(uint32 ImageIndex);

		void FreeCommandBuffers();
		void DestroyColorBufferImages();
		void DestroyDepthBufferImages();
	};

	inline Renderer* CreateRenderer(Device& device)
	{
		if (GRenderer) return GRenderer;
		return (GRenderer = new Renderer(device));
	}

	inline bool DestroyRenderer()
	{
		if (!GRenderer) return false;
		GRenderer->Destroy();
		delete GRenderer;
		GRenderer = nullptr;
		return true;
	}

}