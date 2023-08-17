#pragma once

#include "Mesh.h"

namespace vge
{
	class MeshModel
	{
	public:
		MeshModel() = default;
		//MeshModel(const char* modelFilename);
		MeshModel(const std::vector<Mesh>& Meshes) : m_Meshes(Meshes) {}

		// Recursively load all meshes from this node and its children.
		void LoadNode(VkPhysicalDevice gpu, VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, const aiScene* scene, const aiNode* node, const std::vector<int32>& textureToDescriptorSet);
		void LoadMesh(VkPhysicalDevice gpu, VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, const aiScene* scene, const aiMesh* mesh, const std::vector<int32>& textureToDescriptorSet);

	public:
		size_t GetMeshCount() const { return m_Meshes.size(); }
		Mesh* GetMesh(size_t index) { return index < GetMeshCount() ? &m_Meshes[index] : nullptr; }

		const ModelData& GetModelDataRef() const { return m_ModelData; }
			  ModelData  GetModelData()    const { return m_ModelData; }

		void SetModelMatrix(const glm::mat4& modelMatrix) { m_ModelData.ModelMatrix = modelMatrix; }

		void Destroy();

	private:
		std::vector<Mesh> m_Meshes = {};
		ModelData m_ModelData = {};
	};
}
