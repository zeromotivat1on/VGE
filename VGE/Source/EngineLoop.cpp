#include "EngineLoop.h"
#include "Window.h"
#include "Renderer.h"
#include "Profiling.h"

void vge::MainLoop()
{
	float angle = 0.0f;
	float deltaTime = 0.0f;
	float lastTime = 0.0f;

	const int32 maleModelIndex = GRenderer->CreateMeshModel("Models/male.obj");

	while (!glfwWindowShouldClose(GWindow))
	{
		SCOPE_TIMER("Tick");
		glfwPollEvents();

		const float now = static_cast<float>(glfwGetTime());
		deltaTime = now - lastTime;
		lastTime = now;

		angle += 60.0f * deltaTime;

		if (angle > 360.0f)
		{
			angle -= 360.0f;
		}

		glm::mat4 firstModel(1.0f);
		firstModel = glm::translate(firstModel, glm::vec3(0.0f, 0.0f, -100.0f));
		firstModel = glm::rotate(firstModel, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

		GRenderer->UpdateModelMatrix(maleModelIndex, firstModel);

		GRenderer->Draw();
	}
}
