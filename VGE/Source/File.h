#pragma once

#include "Common.h"
#include "Renderer/RenderCommon.h"

namespace vge::file
{
	std::vector<c8> ReadShader(const c8* filename);
	
	stbi_uc* LoadTexture(const c8* filename, i32& outw, i32& outh, VkDeviceSize& outTextureSize);
	inline void FreeTexture(stbi_uc* data) { stbi_image_free(data); }

	const aiScene* LoadModel(const c8* filename, Assimp::Importer& outImporter);
}
