#include "VulkanUtils.h"

namespace vge
{
	VkCommandBuffer BeginOneTimeCmdBuffer(VkDevice device, VkCommandPool cmdPool);
	void			EndOneTimeCmdBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkCommandBuffer cmdBuffer);

	void CreateBuffer(VkPhysicalDevice gpu, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps, VkBuffer& outBuffer, VkDeviceMemory& outMemory);
	void CopyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CopyImageBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, VkBuffer srcBuffer, VkImage dstImage, VkExtent2D extent);

	// Simple wrapper for BeginOneTimeCmdBuffer (ctor) and EndOneTimeCmdBuffer (dtor) functions.
	struct ScopeCmdBuffer
	{
	public:
		ScopeCmdBuffer(VkDevice device, VkCommandPool cmdPool, VkQueue queue)
			: m_Device(device), m_CmdPool(cmdPool), m_Queue(queue)
		{
			m_CmdBuffer = BeginOneTimeCmdBuffer(m_Device, m_CmdPool);
		}

		~ScopeCmdBuffer()
		{
			EndOneTimeCmdBuffer(m_Device, m_CmdPool, m_Queue, m_CmdBuffer);
		}

		VkCommandBuffer GetHandle() const { return m_CmdBuffer; }

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkCommandPool m_CmdPool = VK_NULL_HANDLE;
		VkQueue m_Queue = VK_NULL_HANDLE;
		VkCommandBuffer m_CmdBuffer = VK_NULL_HANDLE;
	};

	// Simple wrapper for scoped stage buffer.
	struct ScopeStageBuffer
	{
	public:
		ScopeStageBuffer(VkPhysicalDevice gpu, VkDevice device, VkDeviceSize size)
			: m_Device(device)
		{
			static constexpr VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			static constexpr VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			CreateBuffer(gpu, device, size, usage, memProps, m_Buffer, m_Memory);
		}

		~ScopeStageBuffer()
		{
			vkDestroyBuffer(m_Device, m_Buffer, nullptr);
			vkFreeMemory(m_Device, m_Memory, nullptr);
		}

		VkBuffer GetHandle() const { return m_Buffer; }
		VkDeviceMemory GetMemory() const { return m_Memory; }

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_Memory = VK_NULL_HANDLE;
	};
}
