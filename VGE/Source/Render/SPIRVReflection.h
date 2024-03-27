#pragma once

#include "Core/Error.h"
#include "Render/Core/VkCommon.h"
#include "Render/Core/ShaderModule.h"

VGE_DISABLE_WARNINGS()
#include <spirv/spirv_glsl.hpp>
VGE_ENABLE_WARNINGS()

namespace vge
{
class SPIRVReflection
{
public:
    // Reflects shader resources from SPIRV code.
    // @param stage The Vulkan shader stage flag
    // @param spirv The SPIRV code of shader
    // @param[out] resources The list of reflected shader resources
    // @param variant ShaderVariant used for reflection to specify the size of the runtime arrays in Storage Buffers
    bool ReflectShaderResources(VkShaderStageFlagBits stage,
                                const std::vector<u32>& spirv,
                                std::vector<ShaderResource>& resources,
                                const ShaderVariant& variant);

private:
    void ParseShaderResources(const spirv_cross::Compiler& compiler,
                              VkShaderStageFlagBits stage,
                              std::vector<ShaderResource>& resources,
                              const ShaderVariant& variant);

    void ParsePushConstants(const spirv_cross::Compiler& compiler,
                            VkShaderStageFlagBits stage,
                            std::vector<ShaderResource>& resources,
                            const ShaderVariant& variant);

    void ParseSpecializationConstants(const spirv_cross::Compiler& compiler,
                                      VkShaderStageFlagBits stage,
                                      std::vector<ShaderResource>& resources,
                                      const ShaderVariant& variant);
};
} // namespace vge