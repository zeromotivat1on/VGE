#pragma once

#include <string>
#include <unordered_map>
#include "GlmCommon.h"
#include "Types.h"

namespace vge
{
struct TextureComponent;
    
enum class AlphaMode : u8
{
    Opaque, // alpha value is ignored
    Mask,   // either full opaque or fully transparent
    Blend   // output is combined with the background
};
    
struct MaterialComponent
{
    glm::vec3 Emissive = {0.0f, 0.0f, 0.0f}; // emissive color of the material
    bool DoubleSided = false;
    float AlphaCutoff = 0.5f; // cutoff threshold when in Mask mode
    AlphaMode AlphaMode = AlphaMode::Opaque;
    glm::vec4 BaseColorFactor = glm::vec4(0.0f);
    float MetallicFactor = 0.0f;
    float RoughnessFactor = 0.0f;
    std::unordered_map<std::string, TextureComponent*> Textures;
};
}
