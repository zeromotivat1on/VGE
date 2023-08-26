#pragma once

#include "Common.h"

namespace vge 
{
	VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);
}
