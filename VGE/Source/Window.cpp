#include "Window.h"

GLFWwindow* vge::CreateWindow(const char* name, const int32_t width, const int32_t height)
{
	if (GWindow) return GWindow;
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	return (GWindow = glfwCreateWindow(width, height, name, nullptr, nullptr));
}

bool vge::DestroyWindow()
{
	if (!GWindow) return false;
	glfwDestroyWindow(GWindow);
	glfwTerminate();
	GWindow = nullptr;
	return true;
}

void vge::GetGlfwInstanceExtensions(std::vector<const char*>& outExtensions)
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
	{
		outExtensions.push_back(glfwExtensions[i]);
	}
}