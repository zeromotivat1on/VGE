#include "SPIRVReflection.h"
#include "Logging.h"
#include "Core/Helpers.h"

namespace vge
{
namespace
{
template <ShaderResourceType T>
inline void ReadShaderResource(const spirv_cross::Compiler& /*compiler*/,
                               VkShaderStageFlagBits /*stage*/,
                               std::vector<ShaderResource>& /*resources*/,
                               const ShaderVariant& /*variant*/)
{
    LOG(Error, "Reading shader resources of type '%s' is not implemented.", typeid(T).name());
}

template <spv::Decoration T>
inline void ReadResourceDecoration(const spirv_cross::Compiler& /*compiler*/,
                                   const spirv_cross::Resource& /*resource*/,
                                   ShaderResource& /*shader_resource*/,
                                   const ShaderVariant& /* variant */)
{
    LOG(Error, "Reading shader resources of type '%s' is not implemented.", typeid(T).name());
}

template <>
inline void ReadResourceDecoration<spv::DecorationLocation>(const spirv_cross::Compiler& compiler,
                                                            const spirv_cross::Resource& resource,
                                                            ShaderResource& shaderResource,
                                                            const ShaderVariant& variant)
{
    shaderResource.Location = compiler.get_decoration(resource.id, spv::DecorationLocation);
}

template <>
inline void ReadResourceDecoration<spv::DecorationDescriptorSet>(const spirv_cross::Compiler& compiler,
                                                                 const spirv_cross::Resource& resource,
                                                                 ShaderResource& shaderResource,
                                                                 const ShaderVariant& variant)
{
    shaderResource.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
}

template <>
inline void ReadResourceDecoration<spv::DecorationBinding>(const spirv_cross::Compiler& compiler,
                                                           const spirv_cross::Resource& resource,
                                                           ShaderResource& shaderResource,
                                                           const ShaderVariant& variant)
{
    shaderResource.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
}

template <>
inline void ReadResourceDecoration<spv::DecorationInputAttachmentIndex>(const spirv_cross::Compiler& compiler,
                                                                        const spirv_cross::Resource& resource,
                                                                        ShaderResource& shaderResource,
                                                                        const ShaderVariant& variant)
{
    shaderResource.InputAttachmentIndex = compiler.get_decoration(resource.id, spv::DecorationInputAttachmentIndex);
}

template <>
inline void ReadResourceDecoration<spv::DecorationNonWritable>(const spirv_cross::Compiler& compiler,
                                                               const spirv_cross::Resource& resource,
                                                               ShaderResource& shaderResource,
                                                               const ShaderVariant& variant)
{
    shaderResource.Qualifiers |= ShaderResourceQualifiers::NonWritable;
}

template <>
inline void ReadResourceDecoration<spv::DecorationNonReadable>(const spirv_cross::Compiler& compiler,
                                                               const spirv_cross::Resource& resource,
                                                               ShaderResource& shaderResource,
                                                               const ShaderVariant& variant)
{
    shaderResource.Qualifiers |= ShaderResourceQualifiers::NonReadable;
}

inline void ReadResourceVecSize(const spirv_cross::Compiler& compiler,
                                const spirv_cross::Resource& resource,
                                ShaderResource& shaderResource,
                                const ShaderVariant& variant)
{
    const auto& spirvType = compiler.get_type_from_variable(resource.id);

    shaderResource.VecSize = spirvType.vecsize;
    shaderResource.Columns = spirvType.columns;
}

inline void ReadResourceArraySize(const spirv_cross::Compiler& compiler,
                                  const spirv_cross::Resource& resource,
                                  ShaderResource& shaderResource,
                                  const ShaderVariant& variant)
{
    const auto& spirvType = compiler.get_type_from_variable(resource.id);

    shaderResource.ArraySize = spirvType.array.size() ? spirvType.array[0] : 1;
}

inline void ReadResourceSize(const spirv_cross::Compiler& compiler,
                             const spirv_cross::Resource& resource,
                             ShaderResource& shaderResource,
                             const ShaderVariant& variant)
{
    const auto& spirvType = compiler.get_type_from_variable(resource.id);

    size_t arraySize = 0;
    if (variant.GetRuntimeArraySizes().count(resource.name) != 0)
    {
        arraySize = variant.GetRuntimeArraySizes().at(resource.name);
    }

    shaderResource.Size = ToU32(compiler.get_declared_struct_size_runtime_array(spirvType, arraySize));
}

inline void ReadResourceSize(const spirv_cross::Compiler& compiler,
                             const spirv_cross::SPIRConstant& constant,
                             ShaderResource& shaderResource,
                             const ShaderVariant& variant)
{
    const auto& spirvType = compiler.get_type(constant.constant_type);

    switch (spirvType.basetype)
    {
    case spirv_cross::SPIRType::BaseType::Boolean:
    case spirv_cross::SPIRType::BaseType::Char:
    case spirv_cross::SPIRType::BaseType::Int:
    case spirv_cross::SPIRType::BaseType::UInt:
    case spirv_cross::SPIRType::BaseType::Float:
        shaderResource.Size = 4;
        break;
    case spirv_cross::SPIRType::BaseType::Int64:
    case spirv_cross::SPIRType::BaseType::UInt64:
    case spirv_cross::SPIRType::BaseType::Double:
        shaderResource.Size = 8;
        break;
    default:
        shaderResource.Size = 0;
        break;
    }
}

template <>
inline void ReadShaderResource<ShaderResourceType::Input>(const spirv_cross::Compiler& compiler,
                                                          VkShaderStageFlagBits stage,
                                                          std::vector<ShaderResource>& resources,
                                                          const ShaderVariant& variant)
{
    auto inputResources = compiler.get_shader_resources().stage_inputs;

    for (auto& resource : inputResources)
    {
        ShaderResource shaderResource = {};
        shaderResource.Type = ShaderResourceType::Input;
        shaderResource.Stages = stage;
        shaderResource.Name = resource.name;

        ReadResourceVecSize(compiler, resource, shaderResource, variant);
        ReadResourceArraySize(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationLocation>(compiler, resource, shaderResource, variant);

        resources.push_back(shaderResource);
    }
}

template <>
inline void ReadShaderResource<ShaderResourceType::InputAttachment>(const spirv_cross::Compiler& compiler,
                                                                    VkShaderStageFlagBits /*stage*/,
                                                                    std::vector<ShaderResource>& resources,
                                                                    const ShaderVariant& variant)
{
    auto subpassResources = compiler.get_shader_resources().subpass_inputs;

    for (auto& resource : subpassResources)
    {
        ShaderResource shaderResource = {};
        shaderResource.Type = ShaderResourceType::InputAttachment;
        shaderResource.Stages = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderResource.Name = resource.name;

        ReadResourceArraySize(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationInputAttachmentIndex>(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationBinding>(compiler, resource, shaderResource, variant);

        resources.push_back(shaderResource);
    }
}

template <>
inline void ReadShaderResource<ShaderResourceType::Output>(const spirv_cross::Compiler& compiler,
                                                           VkShaderStageFlagBits stage,
                                                           std::vector<ShaderResource>& resources,
                                                           const ShaderVariant& variant)
{
    auto outputResources = compiler.get_shader_resources().stage_outputs;

    for (auto& resource : outputResources)
    {
        ShaderResource shaderResource = {};
        shaderResource.Type = ShaderResourceType::Output;
        shaderResource.Stages = stage;
        shaderResource.Name = resource.name;

        ReadResourceArraySize(compiler, resource, shaderResource, variant);
        ReadResourceVecSize(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationLocation>(compiler, resource, shaderResource, variant);

        resources.push_back(shaderResource);
    }
}

template <>
inline void ReadShaderResource<ShaderResourceType::Image>(const spirv_cross::Compiler& compiler,
                                                          VkShaderStageFlagBits stage,
                                                          std::vector<ShaderResource>& resources,
                                                          const ShaderVariant& variant)
{
    auto imageResources = compiler.get_shader_resources().separate_images;

    for (auto& resource : imageResources)
    {
        ShaderResource shaderResource = {};
        shaderResource.Type = ShaderResourceType::Image;
        shaderResource.Stages = stage;
        shaderResource.Name = resource.name;

        ReadResourceArraySize(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationBinding>(compiler, resource, shaderResource, variant);

        resources.push_back(shaderResource);
    }
}

template <>
inline void ReadShaderResource<ShaderResourceType::ImageSampler>(const spirv_cross::Compiler& compiler,
                                                                 VkShaderStageFlagBits stage,
                                                                 std::vector<ShaderResource>& resources,
                                                                 const ShaderVariant& variant)
{
    auto imageResources = compiler.get_shader_resources().sampled_images;

    for (auto& resource : imageResources)
    {
        ShaderResource shaderResource = {};
        shaderResource.Type = ShaderResourceType::ImageSampler;
        shaderResource.Stages = stage;
        shaderResource.Name = resource.name;

        ReadResourceArraySize(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationBinding>(compiler, resource, shaderResource, variant);

        resources.push_back(shaderResource);
    }
}

template <>
inline void ReadShaderResource<ShaderResourceType::ImageStorage>(const spirv_cross::Compiler& compiler,
                                                                 VkShaderStageFlagBits stage,
                                                                 std::vector<ShaderResource>& resources,
                                                                 const ShaderVariant& variant)
{
    auto storageResources = compiler.get_shader_resources().storage_images;

    for (auto& resource : storageResources)
    {
        ShaderResource shaderResource = {};
        shaderResource.Type = ShaderResourceType::ImageStorage;
        shaderResource.Stages = stage;
        shaderResource.Name = resource.name;

        ReadResourceArraySize(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationNonReadable>(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationNonWritable>(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationBinding>(compiler, resource, shaderResource, variant);

        resources.push_back(shaderResource);
    }
}

template <>
inline void ReadShaderResource<ShaderResourceType::Sampler>(const spirv_cross::Compiler& compiler,
                                                            VkShaderStageFlagBits stage,
                                                            std::vector<ShaderResource>& resources,
                                                            const ShaderVariant& variant)
{
    auto samplerResources = compiler.get_shader_resources().separate_samplers;

    for (auto& resource : samplerResources)
    {
        ShaderResource shaderResource = {};
        shaderResource.Type = ShaderResourceType::Sampler;
        shaderResource.Stages = stage;
        shaderResource.Name = resource.name;

        ReadResourceArraySize(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationBinding>(compiler, resource, shaderResource, variant);

        resources.push_back(shaderResource);
    }
}

template <>
inline void ReadShaderResource<ShaderResourceType::BufferUniform>(const spirv_cross::Compiler& compiler,
                                                                  VkShaderStageFlagBits stage,
                                                                  std::vector<ShaderResource>& resources,
                                                                  const ShaderVariant& variant)
{
    auto uniformResources = compiler.get_shader_resources().uniform_buffers;

    for (auto& resource : uniformResources)
    {
        ShaderResource shaderResource = {};
        shaderResource.Type = ShaderResourceType::BufferUniform;
        shaderResource.Stages = stage;
        shaderResource.Name = resource.name;

        ReadResourceSize(compiler, resource, shaderResource, variant);
        ReadResourceArraySize(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationBinding>(compiler, resource, shaderResource, variant);

        resources.push_back(shaderResource);
    }
}

template <>
inline void ReadShaderResource<ShaderResourceType::BufferStorage>(const spirv_cross::Compiler& compiler,
                                                                  VkShaderStageFlagBits stage,
                                                                  std::vector<ShaderResource>& resources,
                                                                  const ShaderVariant& variant)
{
    auto storageResources = compiler.get_shader_resources().storage_buffers;

    for (auto& resource : storageResources)
    {
        ShaderResource shaderResource = {};
        shaderResource.Type = ShaderResourceType::BufferStorage;
        shaderResource.Stages = stage;
        shaderResource.Name = resource.name;

        ReadResourceSize(compiler, resource, shaderResource, variant);
        ReadResourceArraySize(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationNonReadable>(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationNonWritable>(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, shaderResource, variant);
        ReadResourceDecoration<spv::DecorationBinding>(compiler, resource, shaderResource, variant);

        resources.push_back(shaderResource);
    }
}
} // namespace
} // namespace vge

bool vge::SPIRVReflection::ReflectShaderResources(
    VkShaderStageFlagBits stage,
    const std::vector<uint32_t>& spirv,
    std::vector<ShaderResource>& resources,
    const ShaderVariant& variant)
{
    spirv_cross::CompilerGLSL compiler(spirv);

    auto opts = compiler.get_common_options();
    opts.enable_420pack_extension = true;

    compiler.set_common_options(opts);

    ParseShaderResources(compiler, stage, resources, variant);
    ParsePushConstants(compiler, stage, resources, variant);
    ParseSpecializationConstants(compiler, stage, resources, variant);

    return true;
}

void vge::SPIRVReflection::ParseShaderResources(
    const spirv_cross::Compiler& compiler,
    VkShaderStageFlagBits stage,
    std::vector<ShaderResource>& resources,
    const ShaderVariant& variant)
{
    ReadShaderResource<ShaderResourceType::Input>(compiler, stage, resources, variant);
    ReadShaderResource<ShaderResourceType::InputAttachment>(compiler, stage, resources, variant);
    ReadShaderResource<ShaderResourceType::Output>(compiler, stage, resources, variant);
    ReadShaderResource<ShaderResourceType::Image>(compiler, stage, resources, variant);
    ReadShaderResource<ShaderResourceType::ImageSampler>(compiler, stage, resources, variant);
    ReadShaderResource<ShaderResourceType::ImageStorage>(compiler, stage, resources, variant);
    ReadShaderResource<ShaderResourceType::Sampler>(compiler, stage, resources, variant);
    ReadShaderResource<ShaderResourceType::BufferUniform>(compiler, stage, resources, variant);
    ReadShaderResource<ShaderResourceType::BufferStorage>(compiler, stage, resources, variant);
}

void vge::SPIRVReflection::ParsePushConstants(
    const spirv_cross::Compiler& compiler,
    VkShaderStageFlagBits stage,
    std::vector<ShaderResource>& resources,
    const ShaderVariant& variant)
{
    auto shaderResources = compiler.get_shader_resources();

    for (auto& resource : shaderResources.push_constant_buffers)
    {
        const auto& spivrType = compiler.get_type_from_variable(resource.id);

        std::uint32_t offset = std::numeric_limits<std::uint32_t>::max();

        for (auto i = 0U; i < spivrType.member_types.size(); ++i)
        {
            const auto memOffset = compiler.get_member_decoration(spivrType.self, i, spv::DecorationOffset);
            offset = std::min(offset, memOffset);
        }

        ShaderResource shaderResource = {};
        shaderResource.Type = ShaderResourceType::PushConstant;
        shaderResource.Stages = stage;
        shaderResource.Name = resource.name;
        shaderResource.Offset = offset;

        ReadResourceSize(compiler, resource, shaderResource, variant);

        shaderResource.Size -= shaderResource.Offset;

        resources.push_back(shaderResource);
    }
}

void vge::SPIRVReflection::ParseSpecializationConstants(
    const spirv_cross::Compiler& compiler,
    VkShaderStageFlagBits stage,
    std::vector<ShaderResource>& resources,
    const ShaderVariant& variant)
{
    auto specializationConstants = compiler.get_specialization_constants();

    for (auto& resource : specializationConstants)
    {
        const auto& spirvValue = compiler.get_constant(resource.id);

        ShaderResource shaderResource = {};
        shaderResource.Type = ShaderResourceType::SpecializationConstant;
        shaderResource.Stages = stage;
        shaderResource.Name = compiler.get_name(resource.id);
        shaderResource.Offset = 0;
        shaderResource.ConstantId = resource.constant_id;

        ReadResourceSize(compiler, spirvValue, shaderResource, variant);

        resources.push_back(shaderResource);
    }
}