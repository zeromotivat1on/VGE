#include "EngineLoop.h"
#include "Window.h"

void vge::MainLoop()
{
	while (!glfwWindowShouldClose(GWindow))
	{
		glfwPollEvents();
	}
}
