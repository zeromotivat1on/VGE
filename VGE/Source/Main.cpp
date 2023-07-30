#include "Common.h"
#include "Window.h"
#include "Renderer.h"

int main(int argc, const char** argv)
{
	vge::CreateWindow("Vulkan Game Engine", 800, 600);
	vge::CreateRenderer(vge::GWindow);
	
	if (vge::GRenderer->Initialize() == EXIT_FAILURE) 
	{
		return EXIT_FAILURE;
	}

	while (!glfwWindowShouldClose(vge::GWindow))
	{
		glfwPollEvents();
	}

	vge::DestroyRenderer();
	vge::DestroyWindow();

	return EXIT_SUCCESS;
}
