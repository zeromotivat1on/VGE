#include "Platform/Windows/WindowsPlatform.h"
#include "Applications/VgeApplication.h"
#include <Windows.h>

extern std::unique_ptr<vge::PlatformContext> CreatePlatformContext(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow);

//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
int main(int argc, const char** argv)
{
	const auto vgeAppInfo = std::make_unique<vge::AppInfo>("Vulkan Game Engine", vge::CreateVgeApplication);
	//const auto context = CreatePlatformContext(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	const auto context = CreatePlatformContext(nullptr, nullptr, nullptr, 0);
	auto platform = vge::WindowsPlatform(*context);

	{
		vge::Window::OptionalProperties windowProps;
		windowProps.Title = "VGE";
		windowProps.Resizable = true;
		windowProps.Vsync = vge::Window::Vsync::Default;
		windowProps.Mode = vge::Window::Mode::Default;
		windowProps.Extent = vge::Window::OptionalExtent{ 1024, 780 };
		platform.SetWindowProperties(windowProps);
	}

	{
		platform.RequestApplication(vgeAppInfo.get());
	}
	
	const auto result = platform.Initialize();
	
	if (result == vge::ExitCode::Success)
	{
		platform.MainLoop();
	}
	
	platform.Terminate(result);
	
	return 0;
}
