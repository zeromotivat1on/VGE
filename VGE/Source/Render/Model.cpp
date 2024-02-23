#include "Model.h"
#include "Renderer.h"
#include "File.h"
#include "Utils.h"

vge::Model vge::Model::Create(const ModelCreateInfo& data)
{
	Assimp::Importer importer;
	const aiScene* scene = file::LoadModel(data.Filename, importer);

	std::vector<const char*> texturePaths;
	GetTexturesFromMaterials(scene, texturePaths);

	std::vector<i32> textureToDescriptorSet;
	ResolveTexturesForDescriptors(GRenderer, texturePaths, textureToDescriptorSet);

	Model model = {};
	model.m_Id = data.Id;
	model.m_Filename = data.Filename;
	model.m_Device = data.Device;

	model.LoadNode(data.Device, scene, scene->mRootNode, textureToDescriptorSet);

	LOG(Log, "New - ID: %d, filename: %s", model.GetId(), model.GetFilename());

	return model;
}

void vge::Model::LoadNode(const Device* device, const aiScene* scene, const aiNode* node, const std::vector<i32>& materialToTextureId)
{
	if (!node)
	{
		LOG(Warning, "Given assimp node was nullptr.");
		return;
	}

	for (u32 i = 0; i < node->mNumMeshes; ++i)
	{
		LoadMesh(device, scene, scene->mMeshes[node->mMeshes[i]], materialToTextureId);
	}

	for (u32 i = 0; i < node->mNumChildren; ++i)
	{
		LoadNode(device, scene, node->mChildren[i], materialToTextureId);
	}
}

void vge::Model::LoadMesh(const Device* device, const aiScene* scene, const aiMesh* mesh, const std::vector<i32>& materialToTextureId)
{
	static constexpr glm::vec3 DontCareColor = glm::vec3(0.0f);

	std::vector<Vertex> vertices(mesh->mNumVertices);
	std::vector<u32> indices = {};

	for (u32 i = 0; i < mesh->mNumVertices; ++i)
	{
		vertices[i].Color = DontCareColor;

		vertices[i].Position.x = mesh->mVertices[i].x;
		vertices[i].Position.y = mesh->mVertices[i].y;
		vertices[i].Position.z = mesh->mVertices[i].z;

		if (mesh->mTextureCoords[0])
		{
			vertices[i].UV.x = mesh->mTextureCoords[0][i].x;
			vertices[i].UV.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			vertices[i].UV.x = 0.0f;
			vertices[i].UV.x = 0.0f;
		}
	}

	for (u32 i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace face = mesh->mFaces[i];
		for (u32 j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	MeshCreateInfo meshCreateInfo = {};
	meshCreateInfo.Device = device;
	meshCreateInfo.VertexCount = vertices.size();
	meshCreateInfo.Vertices = vertices.data();
	meshCreateInfo.IndexCount = indices.size();
	meshCreateInfo.Indices = indices.data();
	meshCreateInfo.TextureId = materialToTextureId[mesh->mMaterialIndex];

	m_Meshes.emplace_back(Mesh::Create(meshCreateInfo));
}

void vge::Model::Destroy()
{
	for (Mesh& mesh : m_Meshes)
	{
		mesh.Destroy();
	}
}
