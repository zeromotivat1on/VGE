#include "Image.h"
#include "Device.h"
#include "File.h"
#include "Buffer.h"

namespace 
{
	struct ImageTransitionInfo
	{
		const vge::Device* Device = nullptr;
		VkImage Image = VK_NULL_HANDLE;
		VkImageLayout OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout NewLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	void TransitionImageLayout(const ImageTransitionInfo& data)
	{
		vge::ScopeCmdBuffer cmdBuffer(data.Device->GetCommandPool(), data.Device->GetGfxQueue());

		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.oldLayout = data.OldLayout;
		imageMemoryBarrier.newLayout = data.NewLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// do not transfer to other queue
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// do not transfer to other queue
		imageMemoryBarrier.image = data.Image;
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.levelCount = 1;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_NONE;
		VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_NONE;

		// If transtion from new image to image ready to receive data ...
		if (data.OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && data.NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			// Memory access stage must start after ...
			imageMemoryBarrier.srcAccessMask = 0;										// basically any memory access stage
			// ... but must end before ...
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// If transition from transfer destination to shader readable ...
		else if (data.OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && data.NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		vkCmdPipelineBarrier(
			cmdBuffer.GetHandle(),			// command buffer
			srcStageFlags, dstStageFlags,	// pipeline stages (match to src/dst Access Masks from barrier)
			0,								// dependency flags
			0, nullptr,						// memory barrier
			0, nullptr,						// buffer memory barrier
			1, &imageMemoryBarrier			// image memory barrier
		);
	}
}

vge::Image vge::Image::Create(const ImageCreateInfo& data)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = data.Extent.width;
	imageCreateInfo.extent.height = data.Extent.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = data.Format;
	imageCreateInfo.tiling = data.Tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // dont care
	imageCreateInfo.usage = data.Usage;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // whether can be shared between queues

	VmaAllocationCreateInfo vmaAllocCreateInfo = {};
	vmaAllocCreateInfo.usage = data.MemAllocUsage;

	Image image = {};
	image.m_Allocator = data.Device->GetAllocator();
	VK_ENSURE(vmaCreateImage(image.m_Allocator, &imageCreateInfo, &vmaAllocCreateInfo, &image.Handle, &image.Allocation, &image.AllocInfo));

	return image;
}

vge::Image vge::Image::CreateForTexture(const Device* device, const char* filename)
{
	int32 width = 0, height = 0;
	VkDeviceSize textureSize = 0;
	stbi_uc* textureData = file::LoadTexture(filename, width, height, textureSize);

	const VkExtent2D textureExtent = { static_cast<uint32>(width), static_cast<uint32>(height) };

	ScopeStageBuffer stageBuffer(device, textureSize);
	stageBuffer.Get().TransferToGpuMemory(textureData, static_cast<size_t>(textureSize));

	file::FreeTexture(textureData);

	ImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.Device = device;
	imageCreateInfo.Extent = textureExtent;
	imageCreateInfo.Format = VK_FORMAT_R8G8B8A8_UNORM;
	imageCreateInfo.Tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.Usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.MemAllocUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	Image image = Image::Create(imageCreateInfo);

	ImageTransitionInfo transitionInfo = {};
	transitionInfo.Device = device;
	transitionInfo.Image = image.Handle;

	// Transition image to be destination for copy operation.
	{
		transitionInfo.OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		transitionInfo.NewLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		TransitionImageLayout(transitionInfo);
	}

	CopyImageBuffer(device->GetGfxQueue(), device->GetCommandPool(), stageBuffer.Get().Handle, image.Handle, textureExtent);

	// Transition image to be shader readable for shade usage.
	{
		transitionInfo.OldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		transitionInfo.NewLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		TransitionImageLayout(transitionInfo);
	}

	return image;
}

void vge::Image::Destroy()
{
	vmaDestroyImage(m_Allocator, Handle, Allocation);
	Handle = VK_NULL_HANDLE;
	Allocation = VK_NULL_HANDLE;
	memory::memzero(&AllocInfo, sizeof(VmaAllocationInfo));
	m_Allocator = VK_NULL_HANDLE;
}
