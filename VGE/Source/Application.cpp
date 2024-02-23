#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define VOLK_IMPLEMENTATION 

#include "Application.h"
#include "EngineLoop.h"
#include "Renderer/Window.h"
#include "Renderer/Device.h"
#include "Renderer/Renderer.h"

vge::Application::Application(const ApplicationSpecs& specs) : Specs(specs)
{}

void vge::Application::Initialize()
{
#if COMPILE_SHADERS_ON_INIT
	LOG_RAW("\n----- Shader compilation started -----\n");
	ENSURE(system("compile_shaders.bat") >= 0);
	LOG_RAW("\n----- Shader compilation finished -----\n\n");
#endif

	CreateEngineLoop();
	ENSURE(GEngineLoop);
	GEngineLoop->Initialize();
}

void vge::Application::Run()
{
	GEngineLoop->Start();
}

void vge::Application::Close()
{
	ENSURE(DestroyEngineLoop());
}

bool vge::Application::ShouldClose() const
{
	return GWindow->ShouldClose();
}

vge::i32 vge::Main(int argc, const char** argv)
{
	ApplicationSpecs specs = {};
	specs.Name = "Vulkan Game Engine";
	specs.InternalName = "Spicy Cake";
	specs.Window.Name = "VGE";
	specs.Window.Width = 800;
	specs.Window.Height = 600;

	ENSURE(CreateApplication(specs));
	GApplication->Initialize();
	GApplication->Run();
	ENSURE(DestroyApplication());

	return EXIT_SUCCESS;
}
