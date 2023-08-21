#include "Shader.h"

VkShaderModule vge::CreateShaderModule(VkDevice device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32*>(code.data());

	VkShaderModule shaderModule = VK_NULL_HANDLE;
	VK_ENSURE_MSG(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule), "Failed to create shader module.");

	return shaderModule;
}
