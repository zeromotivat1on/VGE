#include "Renderer.h"
#include "Application.h"
#include "Buffer.h"
#include "Shader.h"
#include "Utils.h"
#include "File.h"

static inline void IncrementRenderFrame() { vge::GRenderFrame = (vge::GRenderFrame + 1) % vge::GMaxDrawFrames; }

vge::Renderer::Renderer(Device& device)
	: m_Device(device)
	//, m_FirstPipeline(device), m_SecondPipeline(device)
{
}

void vge::Renderer::Initialize()
{
	CreateSwapchain();
	CreateRenderPassColorAttachments();
	CreateRenderPassDepthAttachments();
	CreateRenderPass();
	CreatePushConstantRange();
	CreatePipelines();
	CreateFramebuffers();
	AllocateCommandBuffers();
	CreateTextureSampler();
	//AllocateDynamicBufferTransferSpace();
	CreateUniformBuffers();
	CreateDescriptorPools();
	CreateDescriptorSets();
	CreateSyncObjects();

	// Default texture (plain white square 64x64).
	CreateTexture("Textures/plain.png");
}

void vge::Renderer::Destroy()
{
	m_Device.WaitIdle();

	for (size_t i = 0; i < m_Models.size(); ++i)
	{
		m_Models[i].Destroy();
	}

	vkDestroySampler(m_Device.GetHandle(), m_TextureSampler, nullptr);

	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		m_Textures[i].Destroy();
	}

	//_aligned_free(m_ModelTransferSpace);

	DestroyRenderPassColorAttachments();
	DestroyRenderPassDepthAttachments();

	vkDestroyDescriptorPool(m_Device.GetHandle(), m_InputDescriptorPool, nullptr);
	vkDestroyDescriptorPool(m_Device.GetHandle(), m_SamplerDescriptorPool, nullptr);
	vkDestroyDescriptorPool(m_Device.GetHandle(), m_UniformDescriptorPool, nullptr);

	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		m_VpUniformBuffers[i].Destroy();
		//vkDestroyBuffer(m_Device, m_ModelDynamicUniformBuffers[i], nullptr);
		//vkFreeMemory(m_Device, m_ModelDynamicUniformBuffersMemory[i], nullptr);
	}

	for (int32 i = 0; i < GMaxDrawFrames; ++i)
	{
		vkDestroyFence(m_Device.GetHandle(), m_DrawFences[i], nullptr);
		vkDestroySemaphore(m_Device.GetHandle(), m_RenderFinishedSemas[i], nullptr);
		vkDestroySemaphore(m_Device.GetHandle(), m_ImageAvailableSemas[i], nullptr);
	}

	for (Pipeline& pipeline : m_Pipelines)
	{
		pipeline.Destroy();
	}

	//vkDestroyRenderPass(m_Device.GetHandle(), m_RenderPass, nullptr);
	m_RenderPass.Destroy();

	m_Swapchain->Destroy(m_SwapchainRecreateInfo.get());
}

VkResult vge::Renderer::BeginFrame()
{
	vkWaitForFences(m_Device.GetHandle(), 1, &m_DrawFences[GRenderFrame], VK_TRUE, UINT64_MAX);	// wait till open
	vkResetFences(m_Device.GetHandle(), 1, &m_DrawFences[GRenderFrame]);						// close after enter

	VkResult acquireImageResult = m_Swapchain->AcquireNextImage(m_ImageAvailableSemas[GRenderFrame]);
	if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapchain();
	}

	return acquireImageResult;
}

void vge::Renderer::EndFrame()
{
	const uint32 imageIndex = m_Swapchain->GetCurrentImageIndex();

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkCommandBuffer cmd = m_CommandBuffers[imageIndex].GetHandle();
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_ImageAvailableSemas[GRenderFrame];
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_RenderFinishedSemas[GRenderFrame];

	// Open fence after successful render.
	VK_ENSURE(vkQueueSubmit(m_Device.GetGfxQueue(), 1, &submitInfo, m_DrawFences[GRenderFrame]));

	VkSwapchainKHR swapchain = m_Swapchain->GetHandle();
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RenderFinishedSemas[GRenderFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;

	VkResult queuePresentResult = vkQueuePresentKHR(m_Device.GetPresentQueue(), &presentInfo);
	if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || queuePresentResult == VK_SUBOPTIMAL_KHR || m_Device.WasWindowResized())
	{
		m_Device.ResetWindowResizedFlag();
		RecreateSwapchain();
	}
	else
	{
		VK_ENSURE(queuePresentResult);
	}

	IncrementRenderFrame();
}

void vge::Renderer::CreateSwapchain()
{
	m_SwapchainRecreateInfo = std::make_unique<SwapchainRecreateInfo>();
	m_SwapchainRecreateInfo->Surface = m_Device.GetInitialSurface();

	m_Swapchain = std::make_unique<Swapchain>(m_Device);
	m_Swapchain->Initialize(m_SwapchainRecreateInfo.get());
}

void vge::Renderer::CreateRenderPassColorAttachments()
{
	const size_t swapchainImageCount = m_Swapchain->GetImageCount();
	m_ColorAttachments.resize(swapchainImageCount);
	m_ColorFormat = Image::GetBestFormat(&m_Device, { VK_FORMAT_R8G8B8A8_UNORM }, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	ImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.Device = &m_Device;
	imageCreateInfo.Extent = m_Swapchain->GetExtent();
	imageCreateInfo.Format = m_ColorFormat;
	imageCreateInfo.Tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	imageCreateInfo.MemAllocUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	ImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.Device = &m_Device;
	viewCreateInfo.Format = m_ColorFormat;
	viewCreateInfo.AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		m_ColorAttachments[i].Image = Image::Create(imageCreateInfo);
		viewCreateInfo.Image = m_ColorAttachments[i].Image.GetHandle();
		m_ColorAttachments[i].View = Image::CreateView(viewCreateInfo);
	}
}

void vge::Renderer::CreateRenderPassDepthAttachments()
{
	const size_t swapchainImageCount = m_Swapchain->GetImageCount();
	m_DepthAttachments.resize(swapchainImageCount);
	m_DepthFormat = Image::GetBestFormat(&m_Device, { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	ImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.Device = &m_Device;
	imageCreateInfo.Extent = m_Swapchain->GetExtent();
	imageCreateInfo.Format = m_DepthFormat;
	imageCreateInfo.Tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	imageCreateInfo.MemAllocUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	ImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.Device = &m_Device;
	viewCreateInfo.Format = m_DepthFormat;
	viewCreateInfo.AspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

	for (size_t i = 0; i < swapchainImageCount; ++i)
	{
		m_DepthAttachments[i].Image = Image::Create(imageCreateInfo);
		viewCreateInfo.Image = m_DepthAttachments[i].Image.GetHandle();
		m_DepthAttachments[i].View = Image::CreateView(viewCreateInfo);
	}
}

void vge::Renderer::CreateRenderPass()
{
	std::vector<VkSubpassDescription> subpasses(m_DefaultSubpassCount);

	// Attachments and references for 1 subpass.
	// This subpass will have color and depth ones.
	// They will be used as inputs later.

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_ColorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// Store op tells how to store data after render pass finish,
	// as we do not use 1 subpass to present images - dont care.
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = m_DepthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentReference = {};
	// Attachment corresponds to index position in array of image views passed to framebuffer create info.
	colorAttachmentReference.attachment = 1;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 2;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &colorAttachmentReference;
	subpasses[0].pDepthStencilAttachment = &depthAttachmentReference;

	// Attachments and references for 2 subpass.
	// This subpass will present images to the screnn.
	// Will use depth and color attachments from previous 1 subpass as inputs.

	VkAttachmentDescription swapchainColorAttachment = {};
	swapchainColorAttachment.format = m_Swapchain->GetImageFormat();
	swapchainColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	swapchainColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// Here we want to store attachment after render pass as we want to present an image to the screen.
	swapchainColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	swapchainColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	swapchainColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	swapchainColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	swapchainColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference swapchainColorAttachmentReference = {};
	swapchainColorAttachmentReference.attachment = 0;
	swapchainColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::array<VkAttachmentReference, 2> inputAttachmentReferences = {};
	inputAttachmentReferences[0].attachment = 1; // reuse color attachment from 1 subpass
	inputAttachmentReferences[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	inputAttachmentReferences[1].attachment = 2; // reuse depth attachment from 1 subpass
	inputAttachmentReferences[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[1].colorAttachmentCount = 1;
	subpasses[1].pColorAttachments = &swapchainColorAttachmentReference;
	subpasses[1].inputAttachmentCount = static_cast<uint32>(inputAttachmentReferences.size());
	subpasses[1].pInputAttachments = inputAttachmentReferences.data();

	// Amount of dependencies = amount of subpasses + 1.
	// <dependency1> (subpass1) <dependency2> (subpass2) <dependency3>.
	std::vector<VkSubpassDependency> dependencies(m_DefaultSubpassCount + 1);

	// Start to subpass 1.
	// Image layout transition must happen after ...
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	// Stage we want to make sure is finished before layout transition.
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	// ... and before ...
	dependencies[0].dstSubpass = 0;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = 0;

	// From subpass 1 to subpass 2.
	// Image layout transition must happen after ...
	dependencies[1].srcSubpass = 0;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// ... and before ...
	dependencies[1].dstSubpass = 1;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = 0;

	// Subpass 2 to end.
	// Image layout transition must happen after ...
	dependencies[2].srcSubpass = 1;
	dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// ... and before ...
	dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[2].dependencyFlags = 0;

	// Order should correspond to attachment values in attachment descriptions.
	std::vector<VkAttachmentDescription> attachments = { swapchainColorAttachment, colorAttachment, depthAttachment };

	RenderPassCreateInfo createInfo = {};
	createInfo.Device = &m_Device;
	createInfo.SubpassCount = m_DefaultSubpassCount;
	createInfo.ColorFormat = m_ColorFormat;
	createInfo.DepthFormat = m_DepthFormat;
	createInfo.Attachments = &attachments;
	createInfo.Subpasses = &subpasses;
	createInfo.Dependencies = &dependencies;

	m_RenderPass.Initialize(createInfo);
}

void vge::Renderer::CreatePushConstantRange()
{
	m_PushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	m_PushConstantRange.offset = 0;
	m_PushConstantRange.size = sizeof(ModelData);
}

void vge::Renderer::CreatePipelines()
{
	m_Pipelines.resize(m_RenderPass.GeSubpassCount(), Pipeline());

	// TODO: create convenient abstraction for multiple pipelines creation, e.g map with pipeline and its create data.

	// First pipeline creation. Used to render evrything.
	// As this pipeline is used for rendering actual data, we need vertex input.
	{
		VkDescriptorSetLayoutBinding vpLayoutBinding = {};
		vpLayoutBinding.binding = 0; // binding for a particular subpass
		vpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vpLayoutBinding.descriptorCount = 1;
		vpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		vpLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 0; // should be increased by 1 if this was used in vertex shader as previous one
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		const VertexInputDescription vertexDescription = Vertex::GetDescription();
		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<uint32>(vertexDescription.Bindings.size());
		vertexInputCreateInfo.pVertexBindingDescriptions = vertexDescription.Bindings.data();
		vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32>(vertexDescription.Attributes.size());
		vertexInputCreateInfo.pVertexAttributeDescriptions = vertexDescription.Attributes.data();

		PipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		Pipeline::DefaultCreateInfo(pipelineCreateInfo);

		pipelineCreateInfo.Device = &m_Device;
		pipelineCreateInfo.PushConstants = { m_PushConstantRange };
		pipelineCreateInfo.ShaderFilenames = { "Shaders/Bin/first_vert.spv", "Shaders/Bin/first_frag.spv" };
		pipelineCreateInfo.DescriptorSetLayoutBindings = { { vpLayoutBinding }, { samplerLayoutBinding } };
		pipelineCreateInfo.RenderPass = &m_RenderPass;
		pipelineCreateInfo.VertexInfo = vertexInputCreateInfo;
		pipelineCreateInfo.SubpassIndex = 0;

		m_Pipelines[0].Initialize(pipelineCreateInfo);
	}

	// Second pipeline creation. Used to present data from previous pipeline.
	// This pipeline just presents data on screen, so we don't need any vertex input here.
	{
		VkDescriptorSetLayoutBinding colorInputLayoutBinding = {};
		colorInputLayoutBinding.binding = 0;
		colorInputLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		colorInputLayoutBinding.descriptorCount = 1;
		colorInputLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding depthInputLayoutBinding = {};
		depthInputLayoutBinding.binding = 1;
		depthInputLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		depthInputLayoutBinding.descriptorCount = 1;
		depthInputLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		PipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		Pipeline::DefaultCreateInfo(pipelineCreateInfo);

		pipelineCreateInfo.Device = &m_Device;
		pipelineCreateInfo.ShaderFilenames = { "Shaders/Bin/second_vert.spv", "Shaders/Bin/second_frag.spv" };
		pipelineCreateInfo.DescriptorSetLayoutBindings = { {}, { colorInputLayoutBinding, depthInputLayoutBinding } };
		pipelineCreateInfo.RenderPass = &m_RenderPass;
		pipelineCreateInfo.SubpassIndex = 1;
		pipelineCreateInfo.DepthStencilInfo.depthWriteEnable = VK_FALSE;

		m_Pipelines[1].Initialize(pipelineCreateInfo);
	}
}

void vge::Renderer::CreateFramebuffers()
{
	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		std::array<VkImageView, 3> attachments = {};
		attachments[0] = m_Swapchain->GetImage(i)->View;
		attachments[1] = m_ColorAttachments[i].View;
		attachments[2] = m_DepthAttachments[i].View;

		m_Swapchain->CreateFramebuffer(&m_RenderPass, static_cast<uint32>(attachments.size()), attachments.data());
	}
}

void vge::Renderer::AllocateCommandBuffers()
{
	const size_t cmdBufferCount = m_Swapchain->GetFramebufferCount();
	m_CommandBuffers.reserve(cmdBufferCount);

	for (size_t i = 0; i < cmdBufferCount; ++i)
	{
		m_CommandBuffers.push_back(CommandBuffer::Allocate(&m_Device));
	}
}

void vge::Renderer::CreateTextureSampler()
{
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;						// how to render when image is magnified on screen
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;						// how to render when image is minified on screen
	// VK_SAMPLER_ADDRESS_MODE_REPEAT - clamp texture coords from 0 to 1 (1.1 -> 0.1 | 2.5 -> 0.5).
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// how to handle texture wrap in U (x) direction
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// how to handle texture wrap in V (y) direction
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// how to handle texture wrap in W (z) direction
	// Useful only when VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER is set.
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;	// border beyond texture
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;				// clamp from 0 to 1
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;						// generally, if enabled, handle texture stretching at strange angles
	samplerCreateInfo.maxAnisotropy = 16;

	VK_ENSURE(vkCreateSampler(m_Device.GetHandle(), &samplerCreateInfo, nullptr, &m_TextureSampler));
}

//void vge::Renderer::AllocateDynamicBufferTransferSpace()
//{
//	// Calculate proper model alignment to ensure its fit into allocated block of memory.
//	m_ModelUniformAlignment = (sizeof(ModelData) + m_MinUniformBufferOffset - 1) & ~(m_MinUniformBufferOffset - 1);
//	m_ModelTransferSpace = (ModelData*)_aligned_malloc(m_ModelUniformAlignment * GMaxSceneObjects, m_ModelUniformAlignment);
//}

void vge::Renderer::CreateUniformBuffers()
{
	constexpr VkDeviceSize vpBufferSize = sizeof(UboViewProjection);
	//const VkDeviceSize modelBufferSize = m_ModelUniformAlignment * GMaxSceneObjects;

	m_VpUniformBuffers.resize(m_Swapchain->GetImageCount());

	//m_ModelDynamicUniformBuffers.resize(m_Swapchain->GetImageCount());
	//m_ModelDynamicUniformBuffersMemory.resize(m_Swapchain->GetImageCount());

	//constexpr VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	//constexpr VkMemoryPropertyFlags props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	//constexpr VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_CPU_ONLY;

	BufferCreateInfo buffCreateInfo = {};
	buffCreateInfo.Device = &m_Device;
	buffCreateInfo.Size = vpBufferSize;
	buffCreateInfo.Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffCreateInfo.MemAllocUsage = VMA_MEMORY_USAGE_CPU_ONLY;

	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		m_VpUniformBuffers[i] = Buffer::Create(buffCreateInfo);
		//CreateBuffer(m_Device.GetAllocator(), vpBufferSize, usage, memUsage, m_VpUniformBuffers[i]);
		//CreateBuffer(m_Gpu, m_Device, modelBufferSize, usage, props, m_ModelDynamicUniformBuffers[i], m_ModelDynamicUniformBuffersMemory[i]);
	}
}

void vge::Renderer::CreateDescriptorPools()
{
	{
		VkDescriptorPoolSize vpPoolSize = {};
		vpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vpPoolSize.descriptorCount = static_cast<uint32>(m_VpUniformBuffers.size());

		//VkDescriptorPoolSize modelPoolSize = {};
		//modelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		//modelPoolSize.descriptorCount = static_cast<uint32>(m_ModelDynamicUniformBuffers.size());

		const std::array<VkDescriptorPoolSize, 1> uniformPoolSizes = { vpPoolSize, /*modelPoolSize*/ };

		VkDescriptorPoolCreateInfo uniformPoolCreateInfo = {};
		uniformPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		uniformPoolCreateInfo.maxSets = static_cast<uint32>(m_Swapchain->GetImageCount());
		uniformPoolCreateInfo.poolSizeCount = static_cast<uint32>(uniformPoolSizes.size());
		uniformPoolCreateInfo.pPoolSizes = uniformPoolSizes.data();

		VK_ENSURE(vkCreateDescriptorPool(m_Device.GetHandle(), &uniformPoolCreateInfo, nullptr, &m_UniformDescriptorPool));
	}

	{
		VkDescriptorPoolSize samplerPoolSize = {};
		samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // in advanced projects descriptor for image and sampler should be separate
		samplerPoolSize.descriptorCount = static_cast<uint32>(GMaxSceneObjects);

		const std::array<VkDescriptorPoolSize, 1> samplerPoolSizes = { samplerPoolSize };

		VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
		samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		samplerPoolCreateInfo.maxSets = static_cast<uint32>(GMaxSceneObjects);
		samplerPoolCreateInfo.poolSizeCount = static_cast<uint32>(samplerPoolSizes.size());
		samplerPoolCreateInfo.pPoolSizes = samplerPoolSizes.data();

		VK_ENSURE(vkCreateDescriptorPool(m_Device.GetHandle(), &samplerPoolCreateInfo, nullptr, &m_SamplerDescriptorPool));
	}

	{
		VkDescriptorPoolSize colorInputPoolSize = {};
		colorInputPoolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		colorInputPoolSize.descriptorCount = static_cast<uint32>(m_ColorAttachments.size());

		VkDescriptorPoolSize depthInputPoolSize = {};
		depthInputPoolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		depthInputPoolSize.descriptorCount = static_cast<uint32>(m_DepthAttachments.size());

		const std::array<VkDescriptorPoolSize, 2> inputPoolSizes = { colorInputPoolSize, depthInputPoolSize };

		VkDescriptorPoolCreateInfo inputPoolCreateInfo = {};
		inputPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		inputPoolCreateInfo.maxSets = static_cast<uint32>(m_Swapchain->GetImageCount());
		inputPoolCreateInfo.poolSizeCount = static_cast<uint32>(inputPoolSizes.size());
		inputPoolCreateInfo.pPoolSizes = inputPoolSizes.data();

		VK_ENSURE(vkCreateDescriptorPool(m_Device.GetHandle(), &inputPoolCreateInfo, nullptr, &m_InputDescriptorPool));
	}
}

void vge::Renderer::CreateDescriptorSets()
{
	{
		AllocateUniformDescriptorSet();
		UpdateUniformDescriptorSet();
	}

	{
		AllocateInputDescriptorSet();
		UpdateInputDescriptorSet();
	}
}

void vge::Renderer::CreateSyncObjects()
{
	m_ImageAvailableSemas.resize(GMaxDrawFrames);
	m_RenderFinishedSemas.resize(GMaxDrawFrames);
	m_DrawFences.resize(GMaxDrawFrames);

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int32 i = 0; i < GMaxDrawFrames; ++i)
	{
		VK_ENSURE(vkCreateSemaphore(m_Device.GetHandle(), &semaphoreCreateInfo, nullptr, &m_ImageAvailableSemas[i]));
		VK_ENSURE(vkCreateSemaphore(m_Device.GetHandle(), &semaphoreCreateInfo, nullptr, &m_RenderFinishedSemas[i]));
		VK_ENSURE(vkCreateFence(m_Device.GetHandle(), &fenceCreateInfo, nullptr, &m_DrawFences[i]));
	}
}

void vge::Renderer::AllocateUniformDescriptorSet()
{
	m_UniformDescriptorSets.resize(m_Swapchain->GetImageCount());

	std::vector<VkDescriptorSetLayout> setLayouts(m_Swapchain->GetImageCount(), m_Pipelines[0].GetShader(ShaderStage::Vertex)->GetDescriptorSetLayout().Handle);

	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = m_UniformDescriptorPool;
	setAllocInfo.descriptorSetCount = static_cast<uint32>(m_Swapchain->GetImageCount());
	setAllocInfo.pSetLayouts = setLayouts.data(); // 1 to 1 relationship with layout and set

	VK_ENSURE(vkAllocateDescriptorSets(m_Device.GetHandle(), &setAllocInfo, m_UniformDescriptorSets.data()));
}

void vge::Renderer::AllocateInputDescriptorSet()
{
	m_InputDescriptorSets.resize(m_Swapchain->GetImageCount());

	std::vector<VkDescriptorSetLayout> setLayouts(m_Swapchain->GetImageCount(), m_Pipelines[1].GetShader(ShaderStage::Fragment)->GetDescriptorSetLayout().Handle);

	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = m_InputDescriptorPool;
	setAllocInfo.descriptorSetCount = static_cast<uint32>(m_Swapchain->GetImageCount());
	setAllocInfo.pSetLayouts = setLayouts.data();

	VK_ENSURE(vkAllocateDescriptorSets(m_Device.GetHandle(), &setAllocInfo, m_InputDescriptorSets.data()));
}

void vge::Renderer::UpdateUniformDescriptorSet()
{
	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		VkDescriptorBufferInfo vpDescriptorBufferInfo = {};
		vpDescriptorBufferInfo.buffer = m_VpUniformBuffers[i].Handle;
		vpDescriptorBufferInfo.offset = 0;
		vpDescriptorBufferInfo.range = sizeof(UboViewProjection);

		VkWriteDescriptorSet vpSetWrite = {};
		vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vpSetWrite.dstSet = m_UniformDescriptorSets[i];
		vpSetWrite.dstBinding = 0;
		vpSetWrite.dstArrayElement = 0;
		vpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vpSetWrite.descriptorCount = 1;
		vpSetWrite.pBufferInfo = &vpDescriptorBufferInfo;

		//VkDescriptorBufferInfo modelDescriptorBufferInfo = {};
		//modelDescriptorBufferInfo.buffer = m_ModelDynamicUniformBuffers[i];
		//modelDescriptorBufferInfo.offset = 0;
		//modelDescriptorBufferInfo.range = m_ModelUniformAlignment;

		//VkWriteDescriptorSet modelSetWrite = {};
		//modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//modelSetWrite.dstSet = m_DescriptorSets[i];
		//modelSetWrite.dstBinding = 1;
		//modelSetWrite.dstArrayElement = 0;
		//modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		//modelSetWrite.descriptorCount = 1;
		//modelSetWrite.pBufferInfo = &modelDescriptorBufferInfo;

		const std::array<VkWriteDescriptorSet, 1> setWrites = { vpSetWrite, /*modelSetWrite*/ };

		vkUpdateDescriptorSets(m_Device.GetHandle(), static_cast<uint32>(setWrites.size()), setWrites.data(), 0, nullptr);
	}
}

void vge::Renderer::UpdateInputDescriptorSet()
{
	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		VkDescriptorImageInfo colorInputDescriptorImageInfo = {};
		colorInputDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		colorInputDescriptorImageInfo.imageView = m_ColorAttachments[i].View;
		colorInputDescriptorImageInfo.sampler = VK_NULL_HANDLE;

		VkWriteDescriptorSet colorSetWrite = {};
		colorSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		colorSetWrite.dstSet = m_InputDescriptorSets[i];
		colorSetWrite.dstBinding = 0;
		colorSetWrite.dstArrayElement = 0;
		colorSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		colorSetWrite.descriptorCount = 1;
		colorSetWrite.pImageInfo = &colorInputDescriptorImageInfo;

		VkDescriptorImageInfo depthInputDescriptorImageInfo = {};
		depthInputDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		depthInputDescriptorImageInfo.imageView = m_DepthAttachments[i].View;
		depthInputDescriptorImageInfo.sampler = VK_NULL_HANDLE;

		VkWriteDescriptorSet depthSetWrite = {};
		depthSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		depthSetWrite.dstSet = m_InputDescriptorSets[i];
		depthSetWrite.dstBinding = 1;
		depthSetWrite.dstArrayElement = 0;
		depthSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		depthSetWrite.descriptorCount = 1;
		depthSetWrite.pImageInfo = &depthInputDescriptorImageInfo;

		const std::array<VkWriteDescriptorSet, 2> setWrites = { colorSetWrite, depthSetWrite };

		vkUpdateDescriptorSets(m_Device.GetHandle(), static_cast<uint32>(setWrites.size()), setWrites.data(), 0, nullptr);
	}
}

void vge::Renderer::UpdateUniformBuffers(uint32 ImageIndex)
{
	m_VpUniformBuffers[ImageIndex].TransferToGpuMemory(&m_UboViewProjection, sizeof(UboViewProjection));

	// Usage of dynamic uniform buffer example. Costly for frequent actions like model matrix update.
	//for (size_t i = 0; i < m_Meshes.size(); ++i)
	//{
	//	ModelData* meshModelStartAddress = (ModelData*)((ptr_size)m_ModelTransferSpace + (i * m_ModelUniformAlignment));
	//	*meshModelStartAddress = m_Meshes[i].GetModel();
	//}

	//vkMapMemory(m_Device, m_ModelDynamicUniformBuffersMemory[ImageIndex], 0, m_ModelUniformAlignment * m_Meshes.size(), 0, &data);
	//memcpy(data, m_ModelTransferSpace, m_ModelUniformAlignment * m_Meshes.size());
	//vkUnmapMemory(m_Device, m_ModelDynamicUniformBuffersMemory[ImageIndex]);
}

void vge::Renderer::RecreateSwapchain()
{
	m_Device.WaitWindowSizeless();
	m_Device.WaitIdle();

	// Destruction.
	DestroyRenderPassColorAttachments();
	DestroyRenderPassDepthAttachments();
	m_Swapchain->Destroy(m_SwapchainRecreateInfo.get());
	m_Swapchain.reset(new Swapchain(m_Device));

	// Creation.
	m_Swapchain->Initialize(m_SwapchainRecreateInfo.get());
	CreateRenderPassColorAttachments();
	CreateRenderPassDepthAttachments();
	CreateFramebuffers();

	if (m_Swapchain->GetFramebufferCount() != m_CommandBuffers.size())
	{
		FreeCommandBuffers();
		AllocateCommandBuffers();
	}

	UpdateInputDescriptorSet();
}

void vge::Renderer::FreeCommandBuffers()
{
	for (CommandBuffer& cmd : m_CommandBuffers)
	{
		cmd.Free();
	}

	m_CommandBuffers.clear();
}

void vge::Renderer::DestroyRenderPassColorAttachments()
{
	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		vkDestroyImageView(m_Device.GetHandle(), m_ColorAttachments[i].View, nullptr);
		m_ColorAttachments[i].Image.Destroy();
	}
}

void vge::Renderer::DestroyRenderPassDepthAttachments()
{
	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		vkDestroyImageView(m_Device.GetHandle(), m_DepthAttachments[i].View, nullptr);
		m_DepthAttachments[i].Image.Destroy();
	}
}

int32 vge::Renderer::CreateTexture(const char* filename)
{
	TextureCreateInfo texCreateInfo = {};
	texCreateInfo.Id = static_cast<int32>(m_Textures.size());
	texCreateInfo.Filename = filename;
	texCreateInfo.Device = &m_Device;
	texCreateInfo.Sampler = m_TextureSampler;
	texCreateInfo.DescriptorPool = m_SamplerDescriptorPool;
	texCreateInfo.DescriptorLayout = m_Pipelines[0].GetShader(ShaderStage::Fragment)->GetDescriptorSetLayout().Handle; // first pipeline is used to render everything

	Texture texture = Texture::Create(texCreateInfo);
	m_Textures.push_back(texture);

	return texture.GetId();
}

int32 vge::Renderer::CreateModel(const char* filename)
{
	ModelCreateInfo modelCreateInfo = {};
	modelCreateInfo.Id = static_cast<int32>(m_Models.size());
	modelCreateInfo.Filename = filename;
	modelCreateInfo.Device = &m_Device;

	Model model = Model::Create(modelCreateInfo);
	m_Models.push_back(model);

	return model.GetId();
}
