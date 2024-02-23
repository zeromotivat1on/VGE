#pragma once

#include "Platform/Context.h"
#include "Windows.h"

namespace vge
{
class WindowsPlatformContext : public PlatformContext
{
public:
    WindowsPlatformContext(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow);
    ~WindowsPlatformContext() override = default;
};
}   // namespace vge
