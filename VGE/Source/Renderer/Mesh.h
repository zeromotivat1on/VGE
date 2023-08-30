#pragma once

#include "Common.h"
#include "Buffer.h"

namespace vge
{
	class Device;

	struct ModelData
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0f);
	};

	struct MeshCreateInfo
	{
		const Device* Device = nullptr;
	};

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const Device* device, const std::vector<Vertex>& vertices, const std::vector<uint32>& indices, int32 TextureId);

		inline int32 GetTextureId() const { return m_TextureId; }
		inline size_t GetIndexCount() const { return m_IndexCount; }
		inline size_t GetVertexCount() const { return m_VertexCount; }
		inline ModelData GetModelData() const { return m_ModelData; }
		inline IndexBuffer GetIndexBuffer() const { return m_IndexBuffer; }
		inline VertexBuffer GetVertexBuffer() const{ return m_VertexBuffer; }

		inline void SetModelData(const ModelData& data) { m_ModelData = data; }
		inline void SetModelMatrix(const glm::mat4& model) { m_ModelData.ModelMatrix = model; }

		void Destroy();

	private:
		int32 m_TextureId = INDEX_NONE;
		size_t m_IndexCount = 0;
		size_t m_VertexCount = 0;
		ModelData m_ModelData = {};
		IndexBuffer m_IndexBuffer = {};
		VertexBuffer m_VertexBuffer = {};
	};
}
