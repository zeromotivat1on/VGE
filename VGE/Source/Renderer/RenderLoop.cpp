#include "RenderLoop.h"
#include "Application.h"
#include "Renderer.h"
#include "Window.h"

void vge::RenderLoop::Initialize()
{
	const ApplicationSpecs appSpecs = GApplication->Specs;

	CreateWindow(appSpecs.Window.Name, appSpecs.Window.Width, appSpecs.Window.Height);
	ENSURE(GWindow);
	GWindow->Initialize();

	CreateDevice(GWindow);
	ENSURE(GDevice);
	GDevice->Initialize();

	CreateRenderer(GDevice);
	ENSURE(GRenderer);
	GRenderer->Initialize();
}

void vge::RenderLoop::Tick(float deltaTime)
{
	static float angle = 0.0f;
	static const int32 firstModelID = GRenderer->CreateModel("Models/cottage/Cottage_FREE.obj");

	GWindow->PollEvents();

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
}

void vge::RenderLoop::Destroy()
{
	DestroyRenderer();
	DestroyDevice();
	DestroyWindow();
}
