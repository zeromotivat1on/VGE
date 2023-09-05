#pragma once

#include "Common.h"
#include "RenderCommon.h"

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
		inline bool WasResized() const { return m_FramebufferResized; }
		inline void ResetResizedFlag() { m_FramebufferResized = false; }
		inline int32 GetWidth() const { return m_Width; }
		inline int32 GetHeight() const { return m_Height; }
		inline VkExtent2D GetExtent() const { return { static_cast<uint32>(m_Width), static_cast<uint32>(m_Height) }; }

		void GetInstanceExtensions(std::vector<const char*>& outExtensions) const;
		void WaitSizeless() const;

	private:
		static void FramebufferResizeCallback(GLFWwindow* windowRaw, int32 width, int32 height);

	private:
		GLFWwindow* m_Handle = nullptr;
		const char* m_Name = nullptr; 
		int32 m_Width = 0; 
		int32 m_Height = 0;
		bool m_FramebufferResized = false;
	};

	inline Window* CreateWindow(const char* name, const int32 width, const int32 height)
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
