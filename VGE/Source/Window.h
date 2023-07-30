#pragma once

#include "Common.h"

namespace vge
{
	inline GLFWwindow* GWindow = nullptr;

	GLFWwindow* CreateWindow(const char* name, const int32_t width, const int32_t height);
	bool DestroyWindow();
}
