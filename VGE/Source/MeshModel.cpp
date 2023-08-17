#include "MeshModel.h"

void vge::MeshModel::LoadNode(VkPhysicalDevice gpu, VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, const aiScene* scene, const aiNode* node, const std::vector<int32>& textureToDescriptorSet)
{
	if (!node)
	{
		LOG(Warning, "Given node was nullptr.");
		return;
	}

	for (uint32 i = 0; i < node->mNumMeshes; ++i)
	{
		LoadMesh(gpu, device, transferQueue, transferCmdPool, scene, scene->mMeshes[node->mMeshes[i]], textureToDescriptorSet);
	}

	for (uint32 i = 0; i < node->mNumChildren; ++i)
	{
		LoadNode(gpu, device, transferQueue, transferCmdPool, scene, node->mChildren[i], textureToDescriptorSet);
	}
}

void vge::MeshModel::LoadMesh(VkPhysicalDevice gpu, VkDevice device, VkQueue transferQueue, VkCommandPool transferCmdPool, const aiScene* scene, const aiMesh* mesh, const std::vector<int32>& textureToDescriptorSet)
{
	std::vector<Vertex> vertices(mesh->mNumVertices);
	std::vector<uint32> indices = {};

	for (uint32 i = 0; i < mesh->mNumVertices; ++i)
	{
		vertices[i].Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
		vertices[i].Color = { 0.0f, 0.0f, 0.0f }; // use white, dont care for now

		if (mesh->mTextureCoords[0])
		{
			vertices[i].TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}
		else
		{
			vertices[i].TexCoords = { 0.0f, 0.0f };
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

	m_Meshes.push_back(Mesh(gpu, device, transferQueue, transferCmdPool, vertices, indices, textureToDescriptorSet[mesh->mMaterialIndex]));
}

void vge::MeshModel::Destroy()
{
	for (Mesh& mesh : m_Meshes)
	{
		mesh.Destroy();
	}
}
