#include "GLSLCompiler.h"

VGE_DISABLE_WARNINGS()
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/StandAlone/DirStackFileIncluder.h>
VGE_ENABLE_WARNINGS()

namespace
{
inline EShLanguage FindShaderLanguage(VkShaderStageFlagBits stage)
{
    switch (stage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT:
        return EShLangVertex;

    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        return EShLangTessControl;

    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
        return EShLangTessEvaluation;

    case VK_SHADER_STAGE_GEOMETRY_BIT:
        return EShLangGeometry;

    case VK_SHADER_STAGE_FRAGMENT_BIT:
        return EShLangFragment;

    case VK_SHADER_STAGE_COMPUTE_BIT:
        return EShLangCompute;

    case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        return EShLangRayGen;

    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        return EShLangAnyHit;

    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        return EShLangClosestHit;

    case VK_SHADER_STAGE_MISS_BIT_KHR:
        return EShLangMiss;

    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        return EShLangIntersect;

    case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
        return EShLangCallable;

    case VK_SHADER_STAGE_MESH_BIT_EXT:
        return EShLangMesh;

    case VK_SHADER_STAGE_TASK_BIT_EXT:
        return EShLangTask;

    default:
        return EShLangVertex;
    }
}
}

vge::GLSLCompiler::GLSLCompiler()
    : _EnvTargetLanguage(glslang::EShTargetLanguage::EShTargetNone),
    _EnvTargetLanguageVersion(static_cast<glslang::EShTargetLanguageVersion>(0))
{
}

vge::GLSLCompiler::GLSLCompiler(glslang::EShTargetLanguage language, glslang::EShTargetLanguageVersion version)
    : _EnvTargetLanguage(language), _EnvTargetLanguageVersion(version)
{
}

void vge::GLSLCompiler::UpdateTargetEnvirnoment(glslang::EShTargetLanguage language, glslang::EShTargetLanguageVersion version)
{
    _EnvTargetLanguage = language;
    _EnvTargetLanguageVersion = version;
}

void vge::GLSLCompiler::ResetTargetEnvirnoment()
{
    _EnvTargetLanguage = glslang::EShTargetLanguage::EShTargetNone;
    _EnvTargetLanguageVersion = static_cast<glslang::EShTargetLanguageVersion>(0);
}

bool vge::GLSLCompiler::CompileToSpirv(
    VkShaderStageFlagBits stage,
    const std::vector<u8>& glslSource,
    const std::string& entryPoint,
    const ShaderVariant& shaderVariant,
    std::vector<u32>& outSpirv,
    std::string& outInfoLog)
{
    glslang::InitializeProcess();

    constexpr EShMessages messages = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);
    const EShLanguage language = FindShaderLanguage(stage);

    const std::string source = std::string(glslSource.begin(), glslSource.end());

    const char* fileNameList[1] = {""};
    const char* shaderSource = source.data();

    glslang::TShader shader(language);
    shader.setStringsWithLengthsAndNames(&shaderSource, nullptr, fileNameList, 1);
    shader.setEntryPoint(entryPoint.c_str());
    shader.setSourceEntryPoint(entryPoint.c_str());
    shader.setPreamble(shaderVariant.GetPreamble().c_str());
    shader.addProcesses(shaderVariant.GetProcesses());
    
    if (_EnvTargetLanguage != glslang::EShTargetLanguage::EShTargetNone)
    {
        shader.setEnvTarget(_EnvTargetLanguage, _EnvTargetLanguageVersion);
    }

    DirStackFileIncluder includeDir;
    includeDir.pushExternalLocalDirectory("Shaders");

    if (!shader.parse(GetDefaultResources(), 100, false, messages, includeDir))
    {
        outInfoLog = std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog());
        return false;
    }

    // Add shader to new program object.
    glslang::TProgram program;
    program.addShader(&shader);

    // Link program.
    if (!program.link(messages))
    {
        outInfoLog = std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
        return false;
    }

    // Save any info log that was generated.
    if (shader.getInfoLog())
    {
        outInfoLog += std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + "\n";
    }

    if (program.getInfoLog())
    {
        outInfoLog += std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
    }

    const glslang::TIntermediate *intermediate = program.getIntermediate(language);

    // Translate to SPIRV.
    if (!intermediate)
    {
        outInfoLog += "Failed to get shared intermediate code.\n";
        return false;
    }

    spv::SpvBuildLogger logger;

    glslang::GlslangToSpv(*intermediate, outSpirv, &logger);

    outInfoLog += logger.getAllMessages() + "\n";

    // Shutdown glslang library.
    glslang::FinalizeProcess();

    return true;
}