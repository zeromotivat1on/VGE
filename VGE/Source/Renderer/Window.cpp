#include "Window.h"

void vge::Window::GetInstanceExtensions(std::vector<const char*>& outExtensions) const
{
	uint32 glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32 i = 0; i < glfwExtensionCount; ++i)
	{
		outExtensions.push_back(glfwExtensions[i]);
	}
}

void vge::Window::WaitSizeless() const
{
	while (m_Width == 0 || m_Height == 0)
	{
		glfwWaitEvents();
	}
}

void vge::Window::FramebufferResizeCallback(GLFWwindow* windowRaw, int32 width, int32 height)
{
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowRaw));
	window->m_FramebufferResized = true;
	window->m_Width = width;
	window->m_Height = height;

	LOG(Log, "Dimensions: %dx%d", width, height);

	// TODO: add clever and beautiful way to recreate swapchain.
}

vge::Window::Window(const char* name, const int32 width, const int32 height)
	: m_Name(name), m_Width(width), m_Height(height)
{}

void vge::Window::Initialize()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_Handle = glfwCreateWindow(m_Width, m_Height, m_Name, nullptr, nullptr);
	glfwSetWindowUserPointer(m_Handle, this);
	glfwSetFramebufferSizeCallback(m_Handle, Window::FramebufferResizeCallback);

	LOG(Log, "Dimensions: %dx%d", m_Width, m_Height);
}

void vge::Window::Destroy()
{
	glfwDestroyWindow(m_Handle);
	m_Handle = nullptr;
	glfwTerminate();
}
