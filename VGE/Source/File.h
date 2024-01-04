#pragma once

#include "Common.h"
#include "Renderer/RenderCommon.h"

// General purpose file operations.
namespace vge::file
{
	std::vector<char> ReadShader(const char* filename);
	
	stbi_uc* LoadTexture(const char* filename, i32& outw, i32& outh, VkDeviceSize& outTextureSize);
	inline void FreeTexture(stbi_uc* data) { stbi_image_free(data); }

	const aiScene* LoadModel(const char* filename, Assimp::Importer& outImporter);

	bool SyncReadFile(const char* filePath, u8* buffer, size_t bufferSize, size_t& outBytesRead);
}
