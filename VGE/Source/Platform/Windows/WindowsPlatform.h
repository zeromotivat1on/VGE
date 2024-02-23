#pragma once

#include "Platform/Platform.h"

namespace vge
{
class WindowsPlatform : public Platform
{
public:
    WindowsPlatform(const PlatformContext &context);

    ~WindowsPlatform() override = default;

protected:
    void CreateWindow(const Window::Properties &properties) override;
};
}   // namespace vge