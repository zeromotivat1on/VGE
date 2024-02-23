#include "WindowsPlatform.h"
#include "Platform/GlfwWindow.h"

vge::WindowsPlatform::WindowsPlatform(const PlatformContext& context)
    : vge::Platform(context)
{
}

void vge::WindowsPlatform::CreateWindow(const Window::Properties& properties)
{
    if (properties.mode == Window::Mode::Headless)
    {
        //_Window = std::make_unique<HeadlessWindow>(properties);
    }
    else
    {
        _Window = std::make_unique<GlfwWindow>(this, properties);
    }
}
