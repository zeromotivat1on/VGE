#include "Application.h"
#include "Window.h"
#include "Renderer.h"
#include "EngineLoop.h"

int32_t vge::Main(int argc, const char** argv)
{
	ApplicationSpecs specs = {};
	specs.Name = "Vulkan Game Engine";
	specs.InternalName = "Spicy Cake";
	specs.Window.Name = "VGE";
	specs.Window.Width = 800;
	specs.Window.Height = 600;

	CreateApplication(specs);

	if (GApplication->Initialize() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

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

int32_t vge::Application::Initialize()
{
	vge::CreateWindow(Specs.Window.Name, Specs.Window.Width, Specs.Window.Height);
	vge::CreateRenderer(vge::GWindow);

	if (vge::GRenderer->Initialize() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void vge::Application::Close()
{
	vge::DestroyRenderer();
	vge::DestroyWindow();
}
