#pragma once

#include "Common.h"

namespace vge
{
	inline class VgeWindow* GWindow = nullptr;

	class VgeWindow
	{
	public:
		VgeWindow(const char* name, const int32 width, const int32 height);

		void Initialize();
		void Destroy();

		inline void PollEvents() const { glfwPollEvents(); }
		inline bool ShouldClose() const { return glfwWindowShouldClose(m_Handle); }

		void GetFramebufferSize(int32& outw, int32& outh) const;

		void GetInstanceExtensions(std::vector<const char*>& outExtensions) const;
		void CreateSurface(VkInstance instance, VkSurfaceKHR& outSurface) const;

	public:
		GLFWwindow* m_Handle = nullptr;
		const char* m_Name = nullptr; 
		const int32 m_Width = 0; 
		const int32 m_Height = 0;
	};

	VgeWindow* CreateWindow(const char* name, const int32 width, const int32 height);
	bool DestroyWindow();
}
