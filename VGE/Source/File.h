#pragma once

#include "Common.h"

namespace vge::file
{
	std::vector<char> ReadShader(const char* filename);
	
	stbi_uc* LoadTexture(const char* filename, int32& outw, int32& outh, VkDeviceSize& outTextureSize);
	
	const aiScene* LoadModel(const char* filename, Assimp::Importer& outImporter);
}
