#include "Mesh.h"

vge::Mesh::Mesh(VkCommandPool transferCmdPool, const std::vector<Vertex>& vertices, const std::vector<uint32>& indices, int32 TextureId)
	: m_VertexCount(vertices.size()), m_IndexCount(indices.size()), m_TextureId(TextureId)
{
	m_VertexBuffer = VertexBuffer(transferCmdPool, vertices);
	m_IndexBuffer = IndexBuffer(transferCmdPool, indices);
}

void vge::Mesh::Destroy()
{
	m_VertexBuffer.Destroy();
	m_IndexBuffer.Destroy();
}
