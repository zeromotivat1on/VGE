#pragma once

#include "Platform/Application.h"
#include "Render/Core/VkCommon.h"

namespace vge
{
class VgeApplication : public Application
{
public:
    
};

std::unique_ptr<vge::VgeApplication> CreateVgeApplication();
}   // namespace vge

