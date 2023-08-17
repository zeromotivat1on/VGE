#include "File.h"

std::vector<char> vge::file::ReadShader(const char* filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (!file.is_open())
	{
		LOG(Error, "Failed to open a file: %s.", filename);
		return {};
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> fileBuffer(fileSize);

	file.seekg(0);
	file.read(fileBuffer.data(), fileSize);

	file.close();

	return fileBuffer;
}

stbi_uc* vge::file::LoadTexture(const char* filename, int32& outw, int32& outh, VkDeviceSize& outTextureSize)
{
	static constexpr int8 desiredChannelCount = 4; // r g b a

	int32 channels = 0;
	stbi_uc* image = stbi_load(filename, &outw, &outh, &channels, STBI_rgb_alpha);

	if (!image)
	{
		LOG(Error, "Failed to load a texture: %s", filename);
		return nullptr;
	}

	outTextureSize = outw * outh * desiredChannelCount;

	return image;
}

const aiScene* vge::file::LoadModel(const char* filename, Assimp::Importer& outImporter)
{
	const aiScene* scene = outImporter.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
	{
		LOG(Error, "Failed to load a model: %s", filename);
		LOG(Error, " Assimp: %s", outImporter.GetErrorString());
		return nullptr;
	}

	return scene;
}

void vge::file::LoadTextures(const aiScene* scene, std::vector<const char*>& outTextures)
{
	outTextures.resize(scene->mNumMaterials, "");

	for (uint32 i = 0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial* material = scene->mMaterials[i];

		if (!material)
		{
			continue;
		}

		// TODO: add possibility to load different textures.
		if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			aiString path;
			// TODO: retreive all textures from material.
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				int32 index = static_cast<int32>(std::string(path.data).rfind("\\"));
				std::string filename = std::string(path.data).substr(index + 1);
				outTextures[i] = filename.c_str();
			}
		}
	}
}
