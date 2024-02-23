#pragma once

#include <functional>
#include <memory>
#include <string>
#include "Platform/Application.h"

namespace vge
{
using AppCreateFunc = std::function<std::unique_ptr<Application>()>;

class AppInfo
{
public:
    AppInfo(const std::string &id, const AppCreateFunc& createFunc)
        : Id(id), Create(createFunc)
    {}

    std::string Id;
    AppCreateFunc Create;
};
    
}   // namespace vge