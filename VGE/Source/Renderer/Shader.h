#pragma once

#include "Common.h"

namespace vge 
{
	class Device;

	enum class ShaderStage : i8
	{
		None = -1,

		Vertex = 0,
		Fragment = 1,

		Count = 2
	};

	struct DescriptorSetLayout
	{
		VkDescriptorSetLayout Handle = VK_NULL_HANDLE;
		std::vector<VkDescriptorSetLayoutBinding> Bindings = {};
	};

	struct ShaderCreateInfo
	{
		vge::Device* Device = nullptr;
		VkShaderStageFlagBits StageFlags = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		std::vector<char>* SpirvChar = {};
		std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings = {};
	};

	class Shader
	{
	public:
		static const char* const DefaultEntryName;

		static VkShaderStageFlagBits GetFlagsFromStage(ShaderStage stage);
		static ShaderStage GetStageFromFlags(VkShaderStageFlagBits flags);

	public:
		Shader() = default;

		void Initialize(const ShaderCreateInfo& data);
		void Destroy();

		// NOTE: Descriptor layout is not crucial for shader to be valid as it may use its own specific data.
		inline bool IsValid() const { return m_Device && m_Module && m_Stage != ShaderStage::None && m_StageFlags != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM; }
		inline ShaderStage GetStage() const { return m_Stage; }
		inline VkShaderModule GetModule() const { return m_Module; }
		inline VkShaderStageFlagBits GetStageFlags() const { return m_StageFlags; }
		inline const DescriptorSetLayout& GetDescriptorSetLayout() const { return m_DescriptorLayout; }

		VkPipelineShaderStageCreateInfo GetStageCreateInfo() const;

	private:
		void CreateModule(const std::vector<char>* SpirvInt8);
		void CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

	private:
		const Device* m_Device = nullptr;
		ShaderStage m_Stage = ShaderStage::None;
		VkShaderModule m_Module = VK_NULL_HANDLE;
		VkShaderStageFlagBits m_StageFlags = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		DescriptorSetLayout m_DescriptorLayout = {};

#if DEBUG
	private:
		void VerifyBindingIndices(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
#endif
	};
}
