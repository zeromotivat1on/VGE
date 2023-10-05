#include "Mesh.h"

vge::Mesh vge::Mesh::Create(const MeshCreateInfo& data)
{
	Mesh mesh = {};
	mesh.m_TextureId = data.TextureId;
	mesh.m_VertexBuffer = VertexBuffer::Create(data.Device, data.VertexCount, data.Vertices);
	mesh.m_IndexBuffer = IndexBuffer::Create(data.Device, data.IndexCount, data.Indices);

	return mesh;
}

void vge::Mesh::Destroy()
{
	m_VertexBuffer.Destroy();
	m_IndexBuffer.Destroy();
}
