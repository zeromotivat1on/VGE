#pragma once

#include "Common.h"
#include "Buffer.h"

namespace vge
{
	class Device;

	struct ModelData
	{
		alignas(16) glm::mat4 ModelMatrix = glm::mat4(1.0f);
	};

	struct MeshCreateInfo
	{
		const Device* Device = nullptr;
		size_t VertexCount = 0;
		const Vertex* Vertices = nullptr;
		size_t IndexCount = 0;
		const u32* Indices = nullptr;
		i32 TextureId = INDEX_NONE;
	};

	class Mesh
	{
	public:
		static Mesh Create(const MeshCreateInfo& data);

	public:
		Mesh() = default;

		inline i32 GetTextureId() const { return m_TextureId; }
		inline size_t GetIndexCount() const { return m_IndexBuffer.GetIndexCount(); }
		inline size_t GetVertexCount() const { return m_VertexBuffer.GetVertexCount(); }
		inline ModelData GetModelData() const { return m_ModelData; }
		inline IndexBuffer* GetIndexBuffer() { return &m_IndexBuffer; }
		inline VertexBuffer* GetVertexBuffer() { return &m_VertexBuffer; }
		inline const IndexBuffer* GetIndexBuffer() const { return &m_IndexBuffer; }
		inline const VertexBuffer* GetVertexBuffer() const { return &m_VertexBuffer; }

		inline void SetModelData(const ModelData& data) { m_ModelData = data; }
		inline void SetModelMatrix(const glm::mat4& model) { m_ModelData.ModelMatrix = model; }

		void Destroy();

	private:
		i32 m_TextureId = INDEX_NONE;
		ModelData m_ModelData = {};
		IndexBuffer m_IndexBuffer = {};
		VertexBuffer m_VertexBuffer = {};
	};
}
