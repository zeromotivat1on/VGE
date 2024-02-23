#include "Platform/Windows/WindowsPlatform.h"
#include "Applications/VgeApplication.h"
#include <Windows.h>

extern std::unique_ptr<vge::PlatformContext> CreatePlatformContext(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	const auto context = CreatePlatformContext(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	auto platform = vge::WindowsPlatform(*context);

	{
		vge::Window::OptionalProperties windowProps;
		windowProps.title = "VGE";
		windowProps.resizable = true;
		windowProps.vsync = vge::Window::Vsync::Default;
		windowProps.mode = vge::Window::Mode::Default;
		windowProps.extent = vge::Window::OptionalExtent{ 1024, 780 };
		platform.SetWindowProperties(windowProps);
	}

	{
		platform.RequestApplication(new vge::AppInfo("Vulkan Game Engine", vge::CreateVgeApplication));
	}
	
	const auto result = platform.Initialize();
	
	if (result == vge::ExitCode::Success)
	{
		platform.MainLoop();
	}
	
	platform.Terminate(result);
	
	return 0;
}
