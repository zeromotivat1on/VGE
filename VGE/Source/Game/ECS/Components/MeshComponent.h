#pragma once

#include "ECS/Component.h"
#include "Render/Core/VkCommon.h"
#include "Render/Core/Buffer.h"
#include "Render/Core/ShaderModule.h"

namespace vge
{
struct MaterialComponent;

struct VertexAttribute
{
    VkFormat Format = VK_FORMAT_UNDEFINED;
    u32 Stride = 0;
    u32 Offset = 0;
};

// Note: Any changes to material textures or vertex attributes need to be paired with call to ComputeShaderVariant.

struct MeshComponent : public Component
{
public:
    VkIndexType IndexType = VK_INDEX_TYPE_UINT32;
    u32 IndexOffset = 0;
    u32 VerticesCount = 0;
    u32 IndicesCount = 0;
    ShaderVariant ShaderVariant;
    const MaterialComponent* Material = nullptr;
    std::unique_ptr<Buffer> IndexBuffer;
    std::unordered_map<std::string, Buffer> VertexBuffers;
    std::unordered_map<std::string, VertexAttribute> VertexAttributes;

public:
    void ComputeShaderVariant();
    bool GetAttribute(const std::string& attributeName, VertexAttribute& out);
};
}   // namespace vge
