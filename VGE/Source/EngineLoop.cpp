#include "EngineLoop.h"
#include "Window.h"
#include "Renderer.h"
#include "Profiling.h"

void vge::MainLoop()
{
	while (!glfwWindowShouldClose(GWindow))
	{
		SCOPE_TIMER("Tick");
		glfwPollEvents();
		GRenderer->Draw();
	}
}
