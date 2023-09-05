#define STB_IMAGE_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include "Application.h"
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

	m_EngineLoop.Initialize();
}

void vge::Application::Run()
{
	m_EngineLoop.Start();
}

void vge::Application::Close()
{
	m_EngineLoop.Destroy();
}

bool vge::Application::ShouldClose() const
{
	return GWindow->ShouldClose();
}

int32 vge::Main(int argc, const char** argv)
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
