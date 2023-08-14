#include "File.h"

std::vector<char> vge::file::ReadShader(const char* filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (!file.is_open())
	{
		LOG(Error, "Failed to open a file %s.", filename);
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
		LOG(Error, "Failed to load a texture file: %s", filename);
		return nullptr;
	}

	outTextureSize = outw * outh * desiredChannelCount;

	return image;
}
