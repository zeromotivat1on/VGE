#pragma once

#include "Core/Error.h"
#include "Render/Core/VkCommon.h"
#include "Render/Core/ShaderModule.h"

VGE_DISABLE_WARNINGS()
#include <glslang/Public/ShaderLang.h>
VGE_ENABLE_WARNINGS()

namespace vge
{
// Helper class to generate SPIRV code from GLSL source.
class GLSLCompiler
{
public:
    GLSLCompiler();
    GLSLCompiler(glslang::EShTargetLanguage, glslang::EShTargetLanguageVersion);

    void UpdateTargetEnvirnoment(glslang::EShTargetLanguage, glslang::EShTargetLanguageVersion);
    void ResetTargetEnvirnoment();
    
    // Compiles GLSL to SPIRV code.
    // @param stage The Vulkan shader stage flag
    // @param glslSource The GLSL source code to be compiled
    // @param entryPoint The entrypoint function name of the shader stage
    // @param shaderVariant The shader variant
    // @param [out] outSpirv The generated SPIRV code
    // @param [out] outInfoLog Stores any log messages during the compilation process
    bool CompileToSpirv(VkShaderStageFlagBits stage,
                        const std::vector<u8>& glslSource,
                        const std::string& entryPoint,
                        const ShaderVariant& shaderVariant,
                        std::vector<u32>& outSpirv,
                        std::string& outInfoLog);

private:
    glslang::EShTargetLanguage _EnvTargetLanguage;
    glslang::EShTargetLanguageVersion _EnvTargetLanguageVersion;
};
} // namespace vge