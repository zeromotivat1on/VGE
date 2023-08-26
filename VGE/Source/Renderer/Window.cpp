#include "Window.h"

vge::VgeWindow* vge::CreateWindow(const char* name, const int32 width, const int32 height)
{
	if (GWindow) return GWindow;
	return (GWindow = new VgeWindow(name, width, height));
}

bool vge::DestroyWindow()
{
	if (!GWindow) return false;
	GWindow->Destroy();
	delete GWindow;
	GWindow = nullptr;
	return true;
}

void vge::VgeWindow::GetFramebufferSize(int32& outw, int32& outh) const
{
	glfwGetFramebufferSize(m_Handle, &outw, &outh);
}

void vge::VgeWindow::GetInstanceExtensions(std::vector<const char*>& outExtensions) const
{
	uint32 glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32 i = 0; i < glfwExtensionCount; ++i)
	{
		outExtensions.push_back(glfwExtensions[i]);
	}
}

void vge::VgeWindow::CreateSurface(VkInstance instance, VkSurfaceKHR& outSurface) const
{
	VK_ENSURE(glfwCreateWindowSurface(instance, m_Handle, nullptr, &outSurface));
}

vge::VgeWindow::VgeWindow(const char* name, const int32 width, const int32 height)
	: m_Name(name), m_Width(width), m_Height(height)
{}

void vge::VgeWindow::Initialize()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_Handle = glfwCreateWindow(m_Width, m_Height, m_Name, nullptr, nullptr);
}

void vge::VgeWindow::Destroy()
{
	glfwDestroyWindow(m_Handle);
	m_Handle = nullptr;
	glfwTerminate();
}
