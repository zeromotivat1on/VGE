#include "EngineLoop.h"
#include "Window.h"
#include "Renderer.h"

void vge::MainLoop()
{
	while (!glfwWindowShouldClose(GWindow))
	{
		glfwPollEvents();
		GRenderer->Draw();
	}
}
