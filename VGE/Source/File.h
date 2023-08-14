#pragma once

#include "Common.h"

namespace vge::file
{
	std::vector<char> ReadShader(const char* filename);
	stbi_uc* LoadTexture(const char* filename, int32& outw, int32& outh, VkDeviceSize& outTextureSize);
}
