#pragma once

#include "..\Render\Core\VkCommon.h"
#include "Platform/Window.h"

struct GLFWwindow;

namespace vge
{
class Platform;

class GlfwWindow : public Window
{
public:
	GlfwWindow(Platform* platform, const Window::Properties& properties);

	~GlfwWindow() override;

public:
	VkSurfaceKHR CreateSurface(Instance& instance) override;
	VkSurfaceKHR CreateSurface(VkInstance instance, VkPhysicalDevice physicalDevice) override;
	bool ShouldClose() override;
	void ProcessEvents() override;
	void Close() override;
	float GetDpiFactor() const override;
	float GetContentScaleFactor() const override;
	std::vector<const char*> GetRequiredSurfaceExtensions() const override;

private:
	GLFWwindow* _Handle = nullptr;
};
}	// namespace vge