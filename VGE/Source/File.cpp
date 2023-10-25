#include "File.h"

std::vector<vge::c8> vge::file::ReadShader(const c8* filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (!file.is_open())
	{
		LOG(Error, "Failed to open a file: %s.", filename);
		return {};
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<c8> fileBuffer(fileSize);

	file.seekg(0);
	file.read(fileBuffer.data(), fileSize);

	file.close();

	return fileBuffer;
}

stbi_uc* vge::file::LoadTexture(const c8* filename, i32& outw, i32& outh, VkDeviceSize& outTextureSize)
{
	static constexpr i8 desiredChannelCount = 4; // r g b a

	i32 channels = 0;
	stbi_uc* image = stbi_load(filename, &outw, &outh, &channels, STBI_rgb_alpha);

	if (!image)
	{
		LOG(Error, "Failed to load a texture: %s", filename);
		return nullptr;
	}

	outTextureSize = outw * outh * desiredChannelCount;

	return image;
}

const aiScene* vge::file::LoadModel(const c8* filename, Assimp::Importer& outImporter)
{
	const aiScene* scene = outImporter.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
	{
		LOG(Error, "Assimp: %s", outImporter.GetErrorString());
		return nullptr;
	}

	return scene;
}
