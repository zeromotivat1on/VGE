#include "EngineLoop.h"
#include "Application.h"
#include "Profiling.h"
#include "Renderer/Window.h"
#include "Renderer/Renderer.h"

static inline void IncrementAppFrame() { ++vge::GAppFrame; }

float vge::EngineLoop::StartTime = 0.0f;
float vge::EngineLoop::LastTime = 0.0f;
float vge::EngineLoop::DeltaTime = 0.0f;

void vge::EngineLoop::UpdateTime(const float nowTime)
{
	DeltaTime = nowTime - LastTime;
	LastTime = nowTime;
}

void vge::EngineLoop::Start()
{
	StartTime = static_cast<float>(glfwGetTime());

	float angle = 0.0f;

	const int32 firstModelID = GRenderer->CreateModel("Models/cottage/Cottage_FREE.obj");

	while (!GApplication->ShouldClose())
	{
		SCOPE_TIMER("Tick");

		UpdateTime(static_cast<float>(glfwGetTime()));

		GWindow->PollEvents();

		angle += 30.0f * DeltaTime;

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

	const float loopDurationTime = LastTime - StartTime;
	LOG(Log, "Engine loop stats:");
	LOG(Log, " Duration: %.2fs", loopDurationTime);
	LOG(Log, " Iterations: %d", GAppFrame);
	LOG(Log, " Average FPS: %.2f", static_cast<float>(GAppFrame) / loopDurationTime);
}
