#pragma once

#include "Core/Common.h"
#include "Platform/Window.h"

struct GLFWwindow;

namespace vge
{
class Platform;

class GlfwWindow : public Window
{
public:
	GlfwWindow(Platform* platform, const Window::Properties& properties);

	virtual ~GlfwWindow();

public:
	virtual VkSurfaceKHR CreateSurface(Instance& instance) override;
	virtual VkSurfaceKHR CreateSurface(VkInstance instance, VkPhysicalDevice physicalDevice) override;
	virtual bool ShouldClose() override;
	virtual void ProcessEvents() override;
	virtual void Close() override;
	virtual float GetDpiFactor() const override;
	virtual float GetContentScaleFactor() const override;
	virtual std::vector<const char*> GetRequiredSurfaceExtensions() const override;

private:
	GLFWwindow* handle = nullptr;
};
}	// namespace vge