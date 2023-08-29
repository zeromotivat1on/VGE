#include "Window.h"

vge::Window* vge::CreateWindow(const char* name, const int32 width, const int32 height)
{
	if (GWindow) return GWindow;
	return (GWindow = new Window(name, width, height));
}

bool vge::DestroyWindow()
{
	if (!GWindow) return false;
	GWindow->Destroy();
	delete GWindow;
	GWindow = nullptr;
	return true;
}

void vge::Window::GetInstanceExtensions(std::vector<const char*>& outExtensions) const
{
	uint32 glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32 i = 0; i < glfwExtensionCount; ++i)
	{
		outExtensions.push_back(glfwExtensions[i]);
	}
}

vge::Window::Window(const char* name, const int32 width, const int32 height)
	: m_Name(name), m_Width(width), m_Height(height)
{}

void vge::Window::Initialize()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_Handle = glfwCreateWindow(m_Width, m_Height, m_Name, nullptr, nullptr);
}

void vge::Window::Destroy()
{
	glfwDestroyWindow(m_Handle);
	m_Handle = nullptr;
	glfwTerminate();
}
