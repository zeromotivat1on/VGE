#include "Platform/Entrypoint.h"
#include "Context.h"
#include <Windows.h>

std::unique_ptr<vge::PlatformContext> CreatePlatformContext(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    return std::make_unique<vge::WindowsPlatformContext>(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
