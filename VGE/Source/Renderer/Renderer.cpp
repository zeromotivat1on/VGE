#include "Renderer.h"
#include "Application.h"
#include "Device.h"
#include "Swapchain.h"
#include "File.h"
#include "Buffer.h"
#include "Shader.h"

static inline void IncrementRenderFrame() { vge::GRenderFrame = (vge::GRenderFrame + 1) % vge::GMaxDrawFrames; }

vge::Renderer* vge::CreateRenderer(Device* device)
{
	if (GRenderer) return GRenderer;
	return (GRenderer = new Renderer(device));
}

bool vge::DestroyRenderer()
{
	if (!GRenderer) return false;
	GRenderer->Destroy();
	delete GRenderer;
	GRenderer = nullptr;
	return true;
}

vge::Renderer::Renderer(Device* device) : m_Device(device)
{
	ENSURE(m_Device);
}

void vge::Renderer::Initialize()
{
	CreateSwapchain();
	CreateColorBufferImages();
	CreateDepthBufferImages();
	CreateRenderPass();
	CreateDescriptorSetLayouts();
	CreatePushConstantRange();
	CreatePipelines();
	CreateFramebuffers();
	CreateCommandBuffers();
	CreateTextureSampler();
	//AllocateDynamicBufferTransferSpace();
	CreateUniformBuffers();
	CreateDescriptorPools();
	CreateDescriptorSets();
	CreateSyncObjects();

	// Default texture (plain white square 64x64).
	CreateTexture("Textures/plain.png");

	const float aspectRatio = static_cast<float>(m_Swapchain->GetExtentWidth()) / static_cast<float>(m_Swapchain->GetExtentHeight());
	m_UboViewProjection.Projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10000.0f);
	m_UboViewProjection.View = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	m_UboViewProjection.Projection[1][1] *= -1; // invert y-axis as glm uses positive y-axis for up, but vulkan uses it for down
}

void vge::Renderer::Draw()
{
	const VkFence& drawFence = m_DrawFences[GRenderFrame];
	const VkSemaphore& imageAvailableSemaphore = m_ImageAvailableSemas[GRenderFrame];
	const VkSemaphore& renderFinishedSemaphore = m_RenderFinishedSemas[GRenderFrame];

	vkWaitForFences(m_Device->GetHandle(), 1, &drawFence, VK_TRUE, UINT64_MAX);	// wait till open
	vkResetFences(m_Device->GetHandle(), 1, &drawFence);						// close after enter

	const uint32 AcquiredImageIndex = m_Swapchain->AcquireNextImage(imageAvailableSemaphore);

	RecordCommandBuffers(AcquiredImageIndex);
	UpdateUniformBuffers(AcquiredImageIndex);

	VkCommandBuffer& currentCmdBuffer = m_CommandBuffers[AcquiredImageIndex];

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &currentCmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	// Open fence after successful render.
	VK_ENSURE(vkQueueSubmit(m_Device->GetGfxQueue(), 1, &submitInfo, drawFence));

	VkSwapchainKHR swapchain = m_Swapchain->GetHandle();
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &AcquiredImageIndex;

	VK_ENSURE(vkQueuePresentKHR(m_Device->GetPresentQueue(), &presentInfo));

	IncrementRenderFrame();
}

void vge::Renderer::Destroy()
{
	vkDeviceWaitIdle(m_Device->GetHandle());

	for (size_t i = 0; i < m_Models.size(); ++i)
	{
		m_Models[i].Destroy();
	}

	vkDestroySampler(m_Device->GetHandle(), m_TextureSampler, nullptr);

	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		m_Textures[i].Destroy();
	}

	//_aligned_free(m_ModelTransferSpace);

	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		vkDestroyImageView(m_Device->GetHandle(), m_ColorBufferImageViews[i], nullptr);
		m_ColorBufferImages[i].Destroy();

		vkDestroyImageView(m_Device->GetHandle(), m_DepthBufferImageViews[i], nullptr);
		m_DepthBufferImages[i].Destroy();
	}

	vkDestroyDescriptorPool(m_Device->GetHandle(), m_InputDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_Device->GetHandle(), m_InputDescriptorSetLayout, nullptr);

	vkDestroyDescriptorPool(m_Device->GetHandle(), m_SamplerDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_Device->GetHandle(), m_SamplerDescriptorSetLayout, nullptr);

	vkDestroyDescriptorPool(m_Device->GetHandle(), m_UniformDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_Device->GetHandle(), m_UniformDescriptorSetLayout, nullptr);

	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		m_VpUniformBuffers[i].Destroy();
		//vkDestroyBuffer(m_Device, m_ModelDynamicUniformBuffers[i], nullptr);
		//vkFreeMemory(m_Device, m_ModelDynamicUniformBuffersMemory[i], nullptr);
	}

	for (int32 i = 0; i < GMaxDrawFrames; ++i)
	{
		vkDestroyFence(m_Device->GetHandle(), m_DrawFences[i], nullptr);
		vkDestroySemaphore(m_Device->GetHandle(), m_RenderFinishedSemas[i], nullptr);
		vkDestroySemaphore(m_Device->GetHandle(), m_ImageAvailableSemas[i], nullptr);
	}

	m_SecondPipeline.Destroy();
	m_FirstPipeline.Destroy();

	vkDestroyRenderPass(m_Device->GetHandle(), m_RenderPass, nullptr);

	m_Swapchain->Destroy();
}

void vge::Renderer::CreateSwapchain()
{
	m_Swapchain = std::make_unique<Swapchain>(*m_Device);
	m_Swapchain->Initialize();
}

void vge::Renderer::CreateColorBufferImages()
{
	static constexpr VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

	const std::vector<VkFormat> formats = { VK_FORMAT_R8G8B8A8_UNORM };
	m_ColorFormat = GetBestImageFormat(formats, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	m_ColorBufferImages.resize(m_Swapchain->GetImageCount());
	m_ColorBufferImageViews.resize(m_Swapchain->GetImageCount());

	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		CreateImage(m_Device->GetAllocator(), m_Swapchain->GetExtent(), m_ColorFormat, VK_IMAGE_TILING_OPTIMAL, usage, VMA_MEMORY_USAGE_GPU_ONLY, &m_ColorBufferImages[i]);
		CreateImageView(m_ColorBufferImages[i].Handle, m_ColorFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_ColorBufferImageViews[i]);
	}
}

void vge::Renderer::CreateDepthBufferImages()
{
	static constexpr VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

	const std::vector<VkFormat> formats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT };
	m_DepthFormat = GetBestImageFormat(formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	m_DepthBufferImages.resize(m_Swapchain->GetImageCount());
	m_DepthBufferImageViews.resize(m_Swapchain->GetImageCount());

	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		CreateImage(m_Device->GetAllocator(), m_Swapchain->GetExtent(), m_DepthFormat, VK_IMAGE_TILING_OPTIMAL, usage, VMA_MEMORY_USAGE_GPU_ONLY, &m_DepthBufferImages[i]);
		CreateImageView(m_DepthBufferImages[i].Handle, m_DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, m_DepthBufferImageViews[i]);
	}
}

void vge::Renderer::CreateRenderPass()
{
	std::array<VkSubpassDescription, 2> subpasses = {};

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

	// Attachemnts and references for 2 subpass.
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
	// Example: <dependency1> (subpass1) <dependency2> (subpass2) <dependency3>.
	std::array<VkSubpassDependency, 3> subpassDependencies;

	// Start to subpass 1.
	// Image layout transition must happen after ...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	// Stage we want to make sure is finished before layout transition.
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	// ... and before ...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;

	// From subpass 1 to subpass 2.
	// Image layout transition must happen after ...
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// ... and before ...
	subpassDependencies[1].dstSubpass = 1;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	// Subpass 2 to end.
	// Image layout transition must happen after ...
	subpassDependencies[2].srcSubpass = 0; // TODO: should be 0 or 1?
	subpassDependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// ... and before ...
	subpassDependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[2].dependencyFlags = 0;

	// Order should correspond to attachemnt values in attachment descriptions.
	std::array<VkAttachmentDescription, 3> renderPassAttachments = { swapchainColorAttachment, colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32>(renderPassAttachments.size());
	renderPassCreateInfo.pAttachments = renderPassAttachments.data();
	renderPassCreateInfo.subpassCount = static_cast<uint32>(subpasses.size());
	renderPassCreateInfo.pSubpasses = subpasses.data();
	renderPassCreateInfo.dependencyCount = static_cast<uint32>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	VK_ENSURE(vkCreateRenderPass(m_Device->GetHandle(), &renderPassCreateInfo, nullptr, &m_RenderPass));
}

void vge::Renderer::CreateDescriptorSetLayouts()
{
	{
		VkDescriptorSetLayoutBinding vpLayoutBinding = {};
		vpLayoutBinding.binding = 0; // binding for a particular subpass
		vpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vpLayoutBinding.descriptorCount = 1;
		vpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		vpLayoutBinding.pImmutableSamplers = nullptr;

		//VkDescriptorSetLayoutBinding modelLayoutBinding = {};
		//modelLayoutBinding.binding = 1;
		//modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		//modelLayoutBinding.descriptorCount = 1;
		//modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		//modelLayoutBinding.pImmutableSamplers = nullptr;

		const std::array<VkDescriptorSetLayoutBinding, 1> uniformLayoutBindings = { vpLayoutBinding, /*modelLayoutBinding*/ };

		VkDescriptorSetLayoutCreateInfo uniformLayoutCreateInfo = {};
		uniformLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		uniformLayoutCreateInfo.bindingCount = static_cast<uint32>(uniformLayoutBindings.size());
		uniformLayoutCreateInfo.pBindings = uniformLayoutBindings.data();

		VK_ENSURE(vkCreateDescriptorSetLayout(m_Device->GetHandle(), &uniformLayoutCreateInfo, nullptr, &m_UniformDescriptorSetLayout));
	}

	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 0; // should be increased by 1 if this was used in vertex shader as previous one
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		const std::array<VkDescriptorSetLayoutBinding, 1> samplerLayoutBindings = { samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo samplerLayoutCreateInfo = {};
		samplerLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		samplerLayoutCreateInfo.bindingCount = static_cast<uint32>(samplerLayoutBindings.size());
		samplerLayoutCreateInfo.pBindings = samplerLayoutBindings.data();

		VK_ENSURE(vkCreateDescriptorSetLayout(m_Device->GetHandle(), &samplerLayoutCreateInfo, nullptr, &m_SamplerDescriptorSetLayout));
	}

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

		const std::array<VkDescriptorSetLayoutBinding, 2> inputLayoutBindings = { colorInputLayoutBinding, depthInputLayoutBinding };

		VkDescriptorSetLayoutCreateInfo inputLayoutCreateInfo = {};
		inputLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		inputLayoutCreateInfo.bindingCount = static_cast<uint32>(inputLayoutBindings.size());
		inputLayoutCreateInfo.pBindings = inputLayoutBindings.data();

		VK_ENSURE(vkCreateDescriptorSetLayout(m_Device->GetHandle(), &inputLayoutCreateInfo, nullptr, &m_InputDescriptorSetLayout));
	}
}

void vge::Renderer::CreatePushConstantRange()
{
	m_PushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	m_PushConstantRange.offset = 0;
	m_PushConstantRange.size = sizeof(ModelData);
}

void vge::Renderer::CreatePipelines()
{
	// TODO: create convenient abstraction for multiple pipelines creation, e.g map with pipeline and its create data.

	// First pipeline creation. Used to render evrything.
	{
		std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = { m_UniformDescriptorSetLayout, m_SamplerDescriptorSetLayout};

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32>(descriptorSetLayouts.size());
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &m_PushConstantRange;

		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VK_ENSURE(vkCreatePipelineLayout(m_Device->GetHandle(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

		// As this pipeline is used for rendering actual data, we need vertex input.
		const VertexInputDescription vertexDescription = Vertex::GetDescription();
		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<uint32>(vertexDescription.Bindings.size());
		vertexInputCreateInfo.pVertexBindingDescriptions = vertexDescription.Bindings.data();
		vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32>(vertexDescription.Attributes.size());
		vertexInputCreateInfo.pVertexAttributeDescriptions = vertexDescription.Attributes.data();

		PipelineCreateInfo pipelineCreateInfo = Pipeline::DefaultCreateInfo(m_Swapchain->GetExtent());
		pipelineCreateInfo.RenderPass = m_RenderPass;
		pipelineCreateInfo.VertexInfo = vertexInputCreateInfo;
		pipelineCreateInfo.PipelineLayout = pipelineLayout;
		pipelineCreateInfo.SubpassIndex = 0;

		m_FirstPipeline.Initialize("Shaders/Bin/first_vert.spv", "Shaders/Bin/first_frag.spv", pipelineCreateInfo);
	}

	// Second pipeline creation. Used to present data from previous pipeline.
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_InputDescriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VK_ENSURE(vkCreatePipelineLayout(m_Device->GetHandle(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

		// This pipeline just presents data on screen, so we don't need any vertex input here.
		PipelineCreateInfo pipelineCreateInfo = Pipeline::DefaultCreateInfo(m_Swapchain->GetExtent());
		pipelineCreateInfo.RenderPass = m_RenderPass;
		pipelineCreateInfo.PipelineLayout = pipelineLayout;
		pipelineCreateInfo.SubpassIndex = 1;
		pipelineCreateInfo.DepthStencilInfo.depthWriteEnable = VK_FALSE;

		m_SecondPipeline.Initialize("Shaders/Bin/second_vert.spv", "Shaders/Bin/second_frag.spv", pipelineCreateInfo);
	}
}

void vge::Renderer::CreateFramebuffers()
{
	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		std::array<VkImageView, 3> attachments = {};
		attachments[0] = m_Swapchain->GetImage(i)->View;
		attachments[1] = m_ColorBufferImageViews[i];
		attachments[2] = m_DepthBufferImageViews[i];

		m_Swapchain->CreateFramebuffer(m_RenderPass, static_cast<uint32>(attachments.size()), attachments.data());
	}
}

void vge::Renderer::CreateCommandBuffers()
{
	m_CommandBuffers.resize(m_Swapchain->GetFramebufferCount());

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = m_Device->GetCommandPool();
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = static_cast<uint32>(m_CommandBuffers.size());

	VK_ENSURE(vkAllocateCommandBuffers(m_Device->GetHandle(), &cmdBufferAllocInfo, m_CommandBuffers.data()));
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

	VK_ENSURE(vkCreateSampler(m_Device->GetHandle(), &samplerCreateInfo, nullptr, &m_TextureSampler));
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

	constexpr VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	constexpr VkMemoryPropertyFlags props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	constexpr VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_CPU_ONLY;

	for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
	{
		CreateBuffer(m_Device->GetAllocator(), vpBufferSize, usage, memUsage, m_VpUniformBuffers[i]);
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

		VK_ENSURE(vkCreateDescriptorPool(m_Device->GetHandle(), &uniformPoolCreateInfo, nullptr, &m_UniformDescriptorPool));
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

		VK_ENSURE(vkCreateDescriptorPool(m_Device->GetHandle(), &samplerPoolCreateInfo, nullptr, &m_SamplerDescriptorPool));
	}

	{
		VkDescriptorPoolSize colorInputPoolSize = {};
		colorInputPoolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		colorInputPoolSize.descriptorCount = static_cast<uint32>(m_ColorBufferImages.size());

		VkDescriptorPoolSize depthInputPoolSize = {};
		depthInputPoolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		depthInputPoolSize.descriptorCount = static_cast<uint32>(m_DepthBufferImages.size());

		const std::array<VkDescriptorPoolSize, 2> inputPoolSizes = { colorInputPoolSize, depthInputPoolSize };

		VkDescriptorPoolCreateInfo inputPoolCreateInfo = {};
		inputPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		inputPoolCreateInfo.maxSets = static_cast<uint32>(m_Swapchain->GetImageCount());
		inputPoolCreateInfo.poolSizeCount = static_cast<uint32>(inputPoolSizes.size());
		inputPoolCreateInfo.pPoolSizes = inputPoolSizes.data();

		VK_ENSURE(vkCreateDescriptorPool(m_Device->GetHandle(), &inputPoolCreateInfo, nullptr, &m_InputDescriptorPool));
	}
}

void vge::Renderer::CreateDescriptorSets()
{
	{
		m_UniformDescriptorSets.resize(m_Swapchain->GetImageCount());

		std::vector<VkDescriptorSetLayout> setLayouts(m_Swapchain->GetImageCount(), m_UniformDescriptorSetLayout);

		VkDescriptorSetAllocateInfo setAllocInfo = {};
		setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		setAllocInfo.descriptorPool = m_UniformDescriptorPool;
		setAllocInfo.descriptorSetCount = static_cast<uint32>(m_Swapchain->GetImageCount());
		setAllocInfo.pSetLayouts = setLayouts.data(); // 1 to 1 relationship with layout and set

		VK_ENSURE(vkAllocateDescriptorSets(m_Device->GetHandle(), &setAllocInfo, m_UniformDescriptorSets.data()));

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

			vkUpdateDescriptorSets(m_Device->GetHandle(), static_cast<uint32>(setWrites.size()), setWrites.data(), 0, nullptr);
		}
	}

	{
		m_InputDescriptorSets.resize(m_Swapchain->GetImageCount());

		std::vector<VkDescriptorSetLayout> setLayouts(m_Swapchain->GetImageCount(), m_InputDescriptorSetLayout);

		VkDescriptorSetAllocateInfo setAllocInfo = {};
		setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		setAllocInfo.descriptorPool = m_InputDescriptorPool;
		setAllocInfo.descriptorSetCount = static_cast<uint32>(m_Swapchain->GetImageCount());
		setAllocInfo.pSetLayouts = setLayouts.data();

		VK_ENSURE(vkAllocateDescriptorSets(m_Device->GetHandle(), &setAllocInfo, m_InputDescriptorSets.data()));

		for (size_t i = 0; i < m_Swapchain->GetImageCount(); ++i)
		{
			VkDescriptorImageInfo colorInputDescriptorImageInfo = {};
			colorInputDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			colorInputDescriptorImageInfo.imageView = m_ColorBufferImageViews[i];
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
			depthInputDescriptorImageInfo.imageView = m_DepthBufferImageViews[i];
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

			vkUpdateDescriptorSets(m_Device->GetHandle(), static_cast<uint32>(setWrites.size()), setWrites.data(), 0, nullptr);
		}
	}
}

void vge::Renderer::RecordCommandBuffers(uint32 ImageIndex)
{
	const VkCommandBuffer& cmdBuffer = m_CommandBuffers[ImageIndex];
	const VkFramebuffer& framebuffer = m_Swapchain->GetFramebuffer(ImageIndex);

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	// No need in this flag now as we control synchronization by ourselves.
	// cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	std::array<VkClearValue, 3> clearValues = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; // draw black triangle by default, dont care actually
	clearValues[1].color = { 0.3f, 0.3f, 0.2f, 1.0f };
	clearValues[2].depthStencil.depth = 1.0f;

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_RenderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = m_Swapchain->GetExtent();
	renderPassBeginInfo.clearValueCount = static_cast<uint32>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();
	renderPassBeginInfo.framebuffer = framebuffer;

	VK_ENSURE(vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo));

	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	{
		m_FirstPipeline.Bind(cmdBuffer);

		for (Model& model : m_Models)
		{
			const ModelData& modelData = model.GetModelData();
			vkCmdPushConstants(cmdBuffer, m_FirstPipeline.GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelData), &modelData);

			for (size_t MeshIndex = 0; MeshIndex < model.GetMeshCount(); ++MeshIndex)
			{
				const Mesh* mesh = model.GetMesh(MeshIndex);

				if (!mesh)
				{
					continue;
				}

				const std::array<VkDescriptorSet, 2> currentDescriptorSets = { m_UniformDescriptorSets[ImageIndex], m_Textures[mesh->GetTextureId()].GetDescriptor()};

				VkBuffer vertexBuffers[] = { mesh->GetVertexBuffer().Get().Handle };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(cmdBuffer, mesh->GetIndexBuffer().Get().Handle, 0, VK_INDEX_TYPE_UINT32);

				//const uint32 dynamicOffset = static_cast<uint32>(m_ModelUniformAlignment * MeshIndex);
				vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_FirstPipeline.GetLayout(),
					0, static_cast<uint32>(currentDescriptorSets.size()), currentDescriptorSets.data(), 0, nullptr);

				vkCmdDrawIndexed(cmdBuffer, static_cast<uint32>(mesh->GetIndexCount()), 1, 0, 0, 0);
			}
		}

		// Start 2 subpass.

		vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);

		m_SecondPipeline.Bind(cmdBuffer);

		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_SecondPipeline.GetLayout(),
			0, 1, &m_InputDescriptorSets[ImageIndex], 0, nullptr);

		vkCmdDraw(cmdBuffer, 3, 1, 0, 0); // fill screen with one big triangle to draw on
	}

	vkCmdEndRenderPass(cmdBuffer);

	VK_ENSURE(vkEndCommandBuffer(cmdBuffer));
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
		VK_ENSURE(vkCreateSemaphore(m_Device->GetHandle(), &semaphoreCreateInfo, nullptr, &m_ImageAvailableSemas[i]));
		VK_ENSURE(vkCreateSemaphore(m_Device->GetHandle(), &semaphoreCreateInfo, nullptr, &m_RenderFinishedSemas[i]));
		VK_ENSURE(vkCreateFence(m_Device->GetHandle(), &fenceCreateInfo, nullptr, &m_DrawFences[i]));
	}
}

void vge::Renderer::UpdateUniformBuffers(uint32 ImageIndex)
{
	void* data;
	vmaMapMemory(m_Device->GetAllocator(), m_VpUniformBuffers[ImageIndex].Allocation, &data);
	memcpy(data, &m_UboViewProjection, sizeof(UboViewProjection));
	vmaUnmapMemory(m_Device->GetAllocator(), m_VpUniformBuffers[ImageIndex].Allocation);

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

int32 vge::Renderer::CreateTexture(const char* filename)
{
	TextureCreateInfo texCreateInfo = {};
	texCreateInfo.Id = static_cast<int32>(m_Textures.size());
	texCreateInfo.Filename = filename;
	texCreateInfo.CmdPool = m_Device->GetCommandPool();
	texCreateInfo.Sampler = m_TextureSampler;
	texCreateInfo.DescriptorPool = m_SamplerDescriptorPool;
	texCreateInfo.DescriptorLayout = m_SamplerDescriptorSetLayout;

	Texture texture = Texture::Create(texCreateInfo);
	m_Textures.push_back(texture);

	return texture.GetId();
}

int32 vge::Renderer::CreateModel(const char* filename)
{
	ModelCreateInfo modelCreateInfo = {};
	modelCreateInfo.Id = static_cast<int32>(m_Models.size());
	modelCreateInfo.Filename = filename;
	modelCreateInfo.CmdPool = m_Device->GetCommandPool();

	Model model = Model::Create(modelCreateInfo);
	m_Models.push_back(model);

	return model.GetId();
}
