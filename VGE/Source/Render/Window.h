#pragma once

#include "Common.h"
#include "RenderCommon.h"
#include "Core/Error.h"

namespace vge
{
	inline class Window* GWindow = nullptr;

	class Window
	{
	public:
		Window(const char* name, const i32 width, const i32 height);
		NOT_COPYABLE(Window);

	public:
		void Initialize();
		void Destroy();

		inline GLFWwindow* GetHandle() const { return m_Handle; }
		inline void PollEvents() const { glfwPollEvents(); }
		inline bool ShouldClose() const { return glfwWindowShouldClose(m_Handle); }
		inline void GetFramebufferSize(i32& outw, i32& outh) const { glfwGetFramebufferSize(m_Handle, &outw, &outh); }
		//inline void CreateSurface(VkInstance instance, VkSurfaceKHR& outSurface) const { VK_ENSURE(glfwCreateWindowSurface(instance, m_Handle, nullptr, &outSurface)); }
		inline bool WasResized() const { return m_FramebufferResized; }
		inline void ResetResizedFlag() { m_FramebufferResized = false; }
		inline i32 GetWidth() const { return m_Width; }
		inline i32 GetHeight() const { return m_Height; }
		inline VkExtent2D GetExtent() const { return { static_cast<u32>(m_Width), static_cast<u32>(m_Height) }; }
		inline bool IsKeyPressed(i32 key) const { return glfwGetKey(m_Handle, key) == GLFW_PRESS; }

		void GetInstanceExtensions(std::vector<const char*>& outExtensions) const;
		void WaitSizeless() const;

	private:
		static void FramebufferResizeCallback(GLFWwindow* windowRaw, i32 width, i32 height);

	private:
		GLFWwindow* m_Handle = nullptr;
		const char* m_Name = nullptr; 
		i32 m_Width = 0; 
		i32 m_Height = 0;
		bool m_FramebufferResized = false;
	};

	inline Window* CreateWindow(const char* name, const i32 width, const i32 height)
	{
		if (GWindow) return GWindow;
		return (GWindow = new Window(name, width, height));
	}

	inline bool DestroyWindow()
	{
		if (!GWindow) return false;
		GWindow->Destroy();
		delete GWindow;
		GWindow = nullptr;
		return true;
	}
}
