#include "EngineLoop.h"
#include "Window.h"
#include "Renderer.h"
#include "Profiling.h"

void vge::MainLoop()
{
	float angle = 0.0f;
	float deltaTime = 0.0f;
	float lastTime = 0.0f;

	while (!glfwWindowShouldClose(GWindow))
	{
		SCOPE_TIMER("Tick");
		glfwPollEvents();

		float now = static_cast<float>(glfwGetTime());
		deltaTime = now - lastTime;
		lastTime = now;

		angle += 60.0f * deltaTime;
		
		if (angle > 360.0f) 
		{
			angle -= 360.0f;
		}

		GRenderer->UpdateModel(glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)));
		GRenderer->Draw();
	}
}
