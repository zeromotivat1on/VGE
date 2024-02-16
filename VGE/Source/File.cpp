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

stbi_uc* vge::file::LoadTexture(const char* filename, i32& outw, i32& outh, VkDeviceSize& outTextureSize)
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

const aiScene* vge::file::LoadModel(const char* filename, Assimp::Importer& outImporter)
{
	const aiScene* scene = outImporter.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
	{
		LOG(Error, "Assimp: %s", outImporter.GetErrorString());
		return nullptr;
	}

	return scene;
}

bool vge::file::SyncReadFile(const char* filePath, u8* buffer, size_t bufferSize, size_t& outBytesRead)
{
#pragma warning(suppress : 4996)
	FILE* handle = fopen(filePath, "rb");
	if (handle)
	{
		// BLOCK here until all data has been read.
		size_t bytesRead = fread(buffer, 1, bufferSize, handle);
		i32 err = ferror(handle); // get error if any
		fclose(handle);
		if (err == 0)
		{
			outBytesRead = bytesRead;
			return true;
		}
	}
	outBytesRead = 0;
	return false;
}

std::string vge::file::ReadTextFile(const char* filename)
{
	std::vector<std::string> data;

	std::ifstream file;

	file.open(filename, std::ios::in);

	ENSURE_MSG(file.is_open(), "Failed to open file: %s", filename);

	return std::string((std::istreambuf_iterator<char>(file)),
					   (std::istreambuf_iterator<char>()));
}
