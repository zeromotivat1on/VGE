#include "Application.h"
#include "Window.h"
#include "Renderer.h"
#include "EngineLoop.h"

int32 vge::Main(int argc, const char** argv)
{
	ApplicationSpecs specs = {};
	specs.Name = "Vulkan Game Engine";
	specs.InternalName = "Spicy Cake";
	specs.Window.Name = "VGE";
	specs.Window.Width = 800;
	specs.Window.Height = 600;

	CreateApplication(specs);
	if (!GApplication) return EXIT_FAILURE;
	GApplication->Initialize();

	MainLoop();

	DestroyApplication();

	return EXIT_SUCCESS;
}

vge::Application* vge::CreateApplication(const ApplicationSpecs& specs)
{
	if (GApplication) return GApplication;
	return (GApplication = new Application(specs));
}

bool vge::DestroyApplication()
{
	if (!GApplication) return false;
	GApplication->Close();
	delete GApplication;
	GApplication = nullptr;
	return true;
}

vge::Application::Application(const ApplicationSpecs& specs) : Specs(specs)
{}

void vge::Application::Initialize()
{
	CreateWindow(Specs.Window.Name, Specs.Window.Width, Specs.Window.Height);
	CreateRenderer(vge::GWindow);

	GRenderer->Initialize();
}

void vge::Application::Close()
{
	DestroyRenderer();
	DestroyWindow();
}
