#include "EngineLoop.h"
#include "Application.h"
#include "Profiling.h"
#include "Renderer/Window.h"
#include "Renderer/Renderer.h"

static inline void IncrementAppFrame() { ++vge::GAppFrame; }

void vge::EngineLoop::Start()
{
	const float loopStartTime = static_cast<float>(glfwGetTime());

	float angle = 0.0f;
	float deltaTime = 0.0f;
	float lastTime = 0.0f;

	const int32 firstModelID = GRenderer->CreateModel("Models/cottage/Cottage_FREE.obj");

	while (!GApplication->ShouldClose())
	{
		SCOPE_TIMER("Tick");

		GWindow->PollEvents();

		const float now = static_cast<float>(glfwGetTime());
		deltaTime = now - lastTime;
		lastTime = now;

		angle += 30.0f * deltaTime;

		if (angle > 360.0f)
		{
			angle -= 360.0f;
		}

		{
			glm::mat4 firstModel(1.0f);
			firstModel = glm::translate(firstModel, glm::vec3(0.0f, 0.0f, -25.0f));
			//firstModel = glm::rotate(firstModel, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			firstModel = glm::rotate(firstModel, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
			firstModel = glm::scale(firstModel, glm::vec3(1.0f, 1.0f, 1.0f));

			GRenderer->UpdateModelMatrix(firstModelID, firstModel);
		}

		GRenderer->Draw();

		IncrementAppFrame();
	}

	const float loopDurationTime = lastTime - loopStartTime;
	LOG(Log, "Engine loop stats:");
	LOG(Log, " Duration: %.2fs", loopDurationTime);
	LOG(Log, " Iterations: %d", GAppFrame);
}
