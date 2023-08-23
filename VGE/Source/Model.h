#pragma once

#include "Mesh.h"

namespace vge
{
	class Model
	{
	public:
		Model() = default;

		// Recursively load all meshes starting from a given node as root.
		void LoadNode(VkQueue transferQueue, VkCommandPool transferCmdPool, const aiScene* scene, const aiNode* node, const std::vector<int32>& materialToTextureId);
		void LoadMesh(VkQueue transferQueue, VkCommandPool transferCmdPool, const aiScene* scene, const aiMesh* mesh, const std::vector<int32>& materialToTextureId);

	public:
		size_t GetMeshCount() const { return m_Meshes.size(); }

		const Mesh* GetMesh(size_t index) const { return index < GetMeshCount() ? &m_Meshes[index] : nullptr; }
			  Mesh* GetMesh(size_t index)		{ return index < GetMeshCount() ? &m_Meshes[index] : nullptr; }

		const ModelData& GetModelData() const { return m_ModelData; }
			  ModelData& GetModelData()		  { return m_ModelData; }

		void SetModelMatrix(const glm::mat4& modelMatrix) { m_ModelData.ModelMatrix = modelMatrix; }

		void Destroy();

	private:
		std::vector<Mesh> m_Meshes = {};
		ModelData m_ModelData = {};
	};
}
