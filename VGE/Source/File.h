#pragma once

#include "Common.h"

namespace vge::file
{
	std::vector<char> ReadShader(const char* filename);
	
	stbi_uc* LoadTexture(const char* filename, int32& outw, int32& outh, VkDeviceSize& outTextureSize);
	
	const aiScene* LoadModel(const char* filename, Assimp::Importer& outImporter);

	// Get texture names from a given scene, preserves 1 to 1 relationship.
	// If failed to get a texture from material, its name will be empty in out array.
	void LoadTextures(const aiScene* scene, std::vector<const char*>& outTextures);
}
