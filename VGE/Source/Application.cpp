#define STB_IMAGE_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include "Application.h"
#include "EngineLoop.h"
#include "Renderer/Window.h"
#include "Renderer/Renderer.h"

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
	ENSURE(GWindow);
	GWindow->Initialize();

	CreateRenderer(vge::GWindow);
	ENSURE(GRenderer);
	GRenderer->Initialize();
}

void vge::Application::Close()
{
	DestroyRenderer();
	DestroyWindow();
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

	CreateApplication(specs);
	ENSURE(GApplication);

	GApplication->Initialize();

	EngineLoop::Start();

	ENSURE(DestroyApplication());

	return EXIT_SUCCESS;
}
