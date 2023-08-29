#pragma once

#include "Common.h"

namespace vge
{
	inline class Window* GWindow = nullptr;

	class Window
	{
	public:
		Window(const char* name, const int32 width, const int32 height);
		NOT_COPYABLE(Window);

	public:
		void Initialize();
		void Destroy();

		inline void PollEvents() const { glfwPollEvents(); }
		inline bool ShouldClose() const { return glfwWindowShouldClose(m_Handle); }
		inline void GetFramebufferSize(int32& outw, int32& outh) const { glfwGetFramebufferSize(m_Handle, &outw, &outh); }
		inline void CreateSurface(VkInstance instance, VkSurfaceKHR& outSurface) const { VK_ENSURE(glfwCreateWindowSurface(instance, m_Handle, nullptr, &outSurface)); }

		void GetInstanceExtensions(std::vector<const char*>& outExtensions) const;

	public:
		GLFWwindow* m_Handle = nullptr;
		const char* m_Name = nullptr; 
		const int32 m_Width = 0; 
		const int32 m_Height = 0;
	};

	Window* CreateWindow(const char* name, const int32 width, const int32 height);
	bool DestroyWindow();
}
