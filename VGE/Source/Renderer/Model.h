#pragma once

#include "Mesh.h"

namespace vge
{
	class Device;

	struct ModelCreateInfo
	{
		i32 Id = INDEX_NONE;
		const c8* Filename = nullptr;
		const Device* Device = nullptr;
	};

	class Model
	{
	public:
		static Model Create(const ModelCreateInfo& data);

	public:
		Model() = default;

		void Destroy();

		inline i32 GetId() const { return m_Id; }
		inline size_t GetMeshCount() const { return m_Meshes.size(); }
		inline ModelData GetModelData() const { return m_ModelData; }
		inline const c8* GetFilename() const { return m_Filename; }

		inline const Mesh* GetMesh(size_t index) const { return index < GetMeshCount() ? &m_Meshes[index] : nullptr; }
		inline 		 Mesh* GetMesh(size_t index)	   { return index < GetMeshCount() ? &m_Meshes[index] : nullptr; }

		inline void SetModelMatrix(const glm::mat4& modelMatrix) { m_ModelData.ModelMatrix = modelMatrix; }

	private:
		// Recursively load all meshes starting from a given node as root.
		void LoadNode(const Device*, const aiScene* scene, const aiNode* node, const std::vector<i32>& materialToTextureId);
		void LoadMesh(const Device*, const aiScene* scene, const aiMesh* mesh, const std::vector<i32>& materialToTextureId);

	private:
		i32 m_Id = INDEX_NONE;
		ModelData m_ModelData = {};
		const c8* m_Filename = nullptr;
		const Device* m_Device = nullptr;
		std::vector<Mesh> m_Meshes = {};
	};
}
