#include "FileSystem.h"
#include "Core/Error.h"

#include <fstream>
#include <assimp/postprocess.h>

namespace vge
{
namespace
{
std::vector<u8> ReadBinaryFile(const std::string& filename, u32 count)
{
    std::vector<u8> data;

    std::ifstream file;

    file.open(filename, std::ios::in | std::ios::binary);

    ENSURE_MSG(file.is_open(), "Failed to open file: %s.", filename.c_str());

    u64 readCount = count;
    if (count == 0)
    {
        file.seekg(0, std::ios::end);
        readCount = static_cast<u64>(file.tellg());
        file.seekg(0, std::ios::beg);
    }

    data.resize(readCount);

    file.read(reinterpret_cast<char*>(data.data()), readCount);
    file.close();

    return data;
}
}
}

std::string vge::fs::GetExtension(const std::string& uri)
{
    const auto dotPos = uri.find_last_of('.');
    ENSURE_MSG(dotPos != std::string::npos, "Uri has no extension.");
    return uri.substr(dotPos + 1);
}

std::string vge::fs::ReadTextFile(const std::string& filename)
{
    std::ifstream file;

    file.open(filename, std::ios::in);

    ENSURE_MSG(file.is_open(), "Failed to open file: %s", filename.c_str());

    return std::string((std::istreambuf_iterator<char>(file)),
                       (std::istreambuf_iterator<char>()));
}

std::vector<vge::u8> vge::fs::ReadAsset(const std::string& filename, u32 count)
{
    return ReadBinaryFile(filename, count);
}

const aiScene* vge::fs::assimp::ReadAsset(const std::string& filename, Assimp::Importer& importer)
{
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        LOG(Error, "Assimp: %s", importer.GetErrorString());
        return nullptr;
    }

    return scene;
}
