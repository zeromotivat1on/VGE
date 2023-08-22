#include "Model.h"

void vge::Model::LoadNode(VkQueue transferQueue, VkCommandPool transferCmdPool, const aiScene* scene, const aiNode* node, const std::vector<int32>& textureToDescriptorSet)
{
	if (!node)
	{
		LOG(Warning, "Given assimp node was nullptr.");
		return;
	}

	for (uint32 i = 0; i < node->mNumMeshes; ++i)
	{
		LoadMesh(transferQueue, transferCmdPool, scene, scene->mMeshes[node->mMeshes[i]], textureToDescriptorSet);
	}

	for (uint32 i = 0; i < node->mNumChildren; ++i)
	{
		LoadNode(transferQueue, transferCmdPool, scene, node->mChildren[i], textureToDescriptorSet);
	}
}

void vge::Model::LoadMesh(VkQueue transferQueue, VkCommandPool transferCmdPool, const aiScene* scene, const aiMesh* mesh, const std::vector<int32>& textureToDescriptorSet)
{
	static constexpr glm::vec3 DontCareColor = glm::vec3(0.0f);

	std::vector<Vertex> vertices(mesh->mNumVertices);
	std::vector<uint32> indices = {};

	for (uint32 i = 0; i < mesh->mNumVertices; ++i)
	{
		vertices[i].Color = DontCareColor;

		vertices[i].Position.x = mesh->mVertices[i].x;
		vertices[i].Position.y = mesh->mVertices[i].y;
		vertices[i].Position.z = mesh->mVertices[i].z;

		if (mesh->mTextureCoords[0])
		{
			vertices[i].TexCoords.x = mesh->mTextureCoords[0][i].x;
			vertices[i].TexCoords.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			vertices[i].TexCoords.x = 0.0f;
			vertices[i].TexCoords.x = 0.0f;
		}
	}

	for (uint32 i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (uint32 j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	m_Meshes.emplace_back(transferCmdPool, vertices, indices, textureToDescriptorSet[mesh->mMaterialIndex]);
}

void vge::Model::Destroy()
{
	for (Mesh& mesh : m_Meshes)
	{
		mesh.Destroy();
	}
}
