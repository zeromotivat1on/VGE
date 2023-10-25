#pragma once

#include "Common.h"
#include "Device.h"
#include "Pipeline.h"
#include "Model.h"
#include "Texture.h"
#include "Swapchain.h"
#include "CommandBuffer.h"
#include "RenderPass.h"

namespace vge
{
	class Window;
	class Device;

	inline class Renderer* GRenderer = nullptr;
	
	inline constexpr i32 GMaxDrawFrames = 3;
	inline			 i32 GRenderFrame  = 0;

	inline constexpr i32 GMaxSceneObjects = 32;

	struct UboViewProjection
	{
		glm::mat4 Projection;	// how camera see the world
		glm::mat4 View;			// where and from what angle camera is viewing
	};

	class Renderer final
	{
	public:
		Renderer(Device* device);
		NOT_COPYABLE(Renderer);
		~Renderer() = default;

	public:
		void Initialize();
		void Destroy();

		CommandBuffer* BeginFrame();
		void EndFrame();

		// TODO: 1 mesh can have only 1 texture which is cringe (or not, idk for now).
		i32 CreateTexture(const c8* filename);
		i32 CreateModel(const c8* filename);

		void RecreateSwapchain();

		inline CommandBuffer* GetCurrentCmdBuffer() { return &m_CommandBuffers[m_Swapchain->GetCurrentImageIndex()]; }
		inline FrameBuffer* GetCurrentFrameBuffer() { return m_Swapchain->GetFramebuffer(m_Swapchain->GetCurrentImageIndex()); }
		inline const Swapchain* GetSwapchain() const { return m_Swapchain.get(); }
		inline VkExtent2D GetSwapchainExtent() const { return m_Swapchain->GetExtent(); }
		inline f32 GetSwapchainAspectRatio() const { return m_Swapchain->GetAspectRatio(); }
		inline VkDescriptorSet GetCurrentInputDescriptorSet() const { return m_InputDescriptorSets[m_Swapchain->GetCurrentImageIndex()]; }
		inline VkDescriptorSet GetCurrentUniformDescriptorSet() const { return m_UniformDescriptorSets[m_Swapchain->GetCurrentImageIndex()]; }
		inline const RenderPass* GetRenderPass() const { return &m_RenderPass; }

		inline void SetView(const glm::mat4& view) { m_UboViewProjection.View = view; }
		inline void SetProjection(const glm::mat4& projection) { m_UboViewProjection.Projection = projection; }
		inline void UpdateModelMatrix(i32 id, glm::mat4 model) { ASSERT(id < m_Models.size()); m_Models[id].SetModelMatrix(model); }
		inline void UpdateUniformBuffers() { UpdateUniformBuffers(m_Swapchain->GetCurrentImageIndex()); }

		inline Model* FindModel(i32 id) { return id < m_Models.size() ? &m_Models[id] : nullptr; }
		inline Texture* FindTexture(i32 id) { return id < m_Textures.size() ? &m_Textures[id] : nullptr; }
		inline Pipeline* FindPipeline(i32 index) { return index < m_Pipelines.size() ? &m_Pipelines[index] : nullptr; }

	private:
		std::vector<Model> m_Models = {};
		std::vector<Texture> m_Textures = {};

		UboViewProjection m_UboViewProjection = {};

		Device* m_Device = nullptr;
		std::unique_ptr<Swapchain> m_Swapchain = nullptr;
		std::unique_ptr<SwapchainRecreateInfo> m_SwapchainRecreateInfo = nullptr;

		VkSampler m_TextureSampler = VK_NULL_HANDLE;

		std::vector<VkSemaphore> m_ImageAvailableSemas = {};
		std::vector<VkSemaphore> m_RenderFinishedSemas = {};
		std::vector<VkFence> m_DrawFences = {};

		std::vector<CommandBuffer> m_CommandBuffers = {};

		// TODO: create separate structure for subpass data (images, view, memory, format).

		VkFormat m_ColorFormat = VK_FORMAT_UNDEFINED;
		VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;
		std::vector<RenderPassAttachment> m_ColorAttachments = {};
		std::vector<RenderPassAttachment> m_DepthAttachments = {};

		VkDescriptorPool m_UniformDescriptorPool = VK_NULL_HANDLE;	// uniform data
		VkDescriptorPool m_SamplerDescriptorPool = VK_NULL_HANDLE;	// texture data (not neccessary to create separate pool)
		VkDescriptorPool m_InputDescriptorPool = VK_NULL_HANDLE;	// input data for separate pipeline and 2 subpass

		std::vector<VkDescriptorSet> m_UniformDescriptorSets = {};
		std::vector<VkDescriptorSet> m_InputDescriptorSets = {};

		VkPushConstantRange m_PushConstantRange = {};

		std::vector<Buffer> m_VpUniformBuffers = {};

		//std::vector<VkBuffer> m_ModelDynamicUniformBuffers = {};
		//std::vector<VkDeviceMemory> m_ModelDynamicUniformBuffersMemory = {};
		//VkDeviceSize m_MinUniformBufferOffset = 0;
		//size_t m_ModelUniformAlignment = 0;
		//ModelData* m_ModelTransferSpace = nullptr;

		// Amount of subpasses in render pass and pipelines in it.
		u32 m_DefaultSubpassCount = 2;
		//VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		RenderPass m_RenderPass = {};
		std::vector<Pipeline> m_Pipelines;

	private:
		void CreateSwapchain();
		void CreateRenderPassColorAttachments();
		void CreateRenderPassDepthAttachments();
		void CreateRenderPass();
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

		void UpdateUniformBuffers(u32 ImageIndex);

		void FreeCommandBuffers();
		void DestroyRenderPassColorAttachments();
		void DestroyRenderPassDepthAttachments();
	};

	inline Renderer* CreateRenderer(Device* device)
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
