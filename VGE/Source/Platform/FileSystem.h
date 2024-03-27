#pragma once

#include <string>
#include <vector>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include "Types.h"

namespace vge
{
namespace fs
{
std::string GetExtension(const std::string& uri);
std::string ReadTextFile(const std::string& filename);
std::vector<u8> ReadAsset(const std::string& filename, u32 count = 0);

namespace assimp
{
const aiScene* ReadAsset(const std::string& filename, Assimp::Importer&);
}
}
}