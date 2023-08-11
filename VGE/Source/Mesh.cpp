#include "Mesh.h"

vge::Mesh::Mesh(VkPhysicalDevice gpu, VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	: m_Gpu(gpu), m_Device(device), m_VertexCount(vertices.size()), m_IndexCount(indices.size())
{
	CreateVertexBuffer(transferQueue, transferCmdPool, vertices);
	CreateIndexBuffer(transferQueue, transferCmdPool, indices);
}

void vge::Mesh::DestroyVertexBuffer() const
{
	vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
	vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);
}

void vge::Mesh::DestroyIndexBuffer() const
{
	vkDestroyBuffer(m_Device, m_IndexBuffer, nullptr);
	vkFreeMemory(m_Device, m_IndexBufferMemory, nullptr);
}

void vge::Mesh::CreateVertexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices)
{
	const VkDeviceSize bufferSize = STD_VECTOR_ALLOC_SIZE(vertices);

	VkBuffer stageBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stageBufferMemory = VK_NULL_HANDLE;

	const VkBufferUsageFlags stageBufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	const VkMemoryPropertyFlags stageBufferMemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	CreateBuffer(m_Gpu, m_Device, bufferSize, stageBufferUsage, stageBufferMemoryFlags, stageBuffer, stageBufferMemory);

	void* data; // cpu accessible data from buffer memory on gpu
	vkMapMemory(m_Device, stageBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(m_Device, stageBufferMemory);

	const VkBufferUsageFlags vertexBufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	const VkMemoryPropertyFlags vertexBufferMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	CreateBuffer(m_Gpu, m_Device, bufferSize, vertexBufferUsage, vertexBufferMemoryFlags, m_VertexBuffer, m_VertexBufferMemory);

	CopyBuffer(m_Device, transferQueue, transferCmdPool, stageBuffer, m_VertexBuffer, bufferSize);

	vkDestroyBuffer(m_Device, stageBuffer, nullptr);
	vkFreeMemory(m_Device, stageBufferMemory, nullptr);
}

void vge::Mesh::CreateIndexBuffer(VkQueue transferQueue, VkCommandPool transferCmdPool, const std::vector<uint32_t>& indices)
{
	const VkDeviceSize bufferSize = STD_VECTOR_ALLOC_SIZE(indices);

	VkBuffer stageBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stageBufferMemory = VK_NULL_HANDLE;

	const VkBufferUsageFlags stageBufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	const VkMemoryPropertyFlags stageBufferMemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	CreateBuffer(m_Gpu, m_Device, bufferSize, stageBufferUsage, stageBufferMemoryFlags, stageBuffer, stageBufferMemory);

	void* data; // cpu accessible data from buffer memory on gpu
	vkMapMemory(m_Device, stageBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(m_Device, stageBufferMemory);

	const VkBufferUsageFlags indexBufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	const VkMemoryPropertyFlags indexBufferMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	CreateBuffer(m_Gpu, m_Device, bufferSize, indexBufferUsage, indexBufferMemoryFlags, m_IndexBuffer, m_IndexBufferMemory);

	CopyBuffer(m_Device, transferQueue, transferCmdPool, stageBuffer, m_IndexBuffer, bufferSize);

	vkDestroyBuffer(m_Device, stageBuffer, nullptr);
	vkFreeMemory(m_Device, stageBufferMemory, nullptr);
}
