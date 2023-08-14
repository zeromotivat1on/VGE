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

		glm::mat4 firstModel(1.0f);
		glm::mat4 secondModel(1.0f);

		firstModel = glm::translate(firstModel, glm::vec3(0.0f, 0.0f, -8.0f));
		firstModel = glm::rotate(firstModel, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

		secondModel = glm::translate(secondModel, glm::vec3(0.0f, 0.0f, -9.0f));
		secondModel = glm::rotate(secondModel, glm::radians(-angle * 2.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		GRenderer->UpdateModel(0, firstModel);
		GRenderer->UpdateModel(1, secondModel);

		GRenderer->Draw();
	}
}
