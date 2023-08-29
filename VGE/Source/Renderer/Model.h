#pragma once

#include "Mesh.h"

namespace vge
{
	struct ModelCreateInfo
	{
		int32 Id = INDEX_NONE;
		const char* Filename = nullptr;
		VkCommandPool CmdPool = VK_NULL_HANDLE;
	};

	class Model
	{
	public:
		static Model Create(const ModelCreateInfo& data);

	public:
		Model() = default;

	public:
		void Destroy();

		inline int32 GetId() const { return m_Id; }
		inline size_t GetMeshCount() const { return m_Meshes.size(); }
		inline ModelData GetModelData() const { return m_ModelData; }
		inline const char* GetFilename() const { return m_Filename; }

		inline const Mesh* GetMesh(size_t index) const { return index < GetMeshCount() ? &m_Meshes[index] : nullptr; }
		inline 		 Mesh* GetMesh(size_t index)	   { return index < GetMeshCount() ? &m_Meshes[index] : nullptr; }

		inline void SetModelMatrix(const glm::mat4& modelMatrix) { m_ModelData.ModelMatrix = modelMatrix; }

	private:
		// Recursively load all meshes starting from a given node as root.
		void LoadNode(VkQueue transferQueue, VkCommandPool transferCmdPool, const aiScene* scene, const aiNode* node, const std::vector<int32>& materialToTextureId);
		void LoadMesh(VkQueue transferQueue, VkCommandPool transferCmdPool, const aiScene* scene, const aiMesh* mesh, const std::vector<int32>& materialToTextureId);

	private:
		int32 m_Id = INDEX_NONE;
		ModelData m_ModelData = {};
		const char* m_Filename = nullptr;
		std::vector<Mesh> m_Meshes = {};
	};
}
