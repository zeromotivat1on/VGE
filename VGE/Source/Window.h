#pragma once

#include "Common.h"

namespace vge
{
	inline GLFWwindow* GWindow = nullptr;

	GLFWwindow* CreateWindow(const char* name, const int32 width, const int32 height);
	bool DestroyWindow();

	void GetGlfwInstanceExtensions(std::vector<const char*>& outExtensions);
}
