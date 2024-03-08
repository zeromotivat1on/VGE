#include "GlfwWindow.h"
#include <GLFW/glfw3.h>
#include "Core/Error.h"
#include "Platform/Platform.h"
#include "Platform/InputEvens.h"

namespace vge
{
namespace
{
void ErrorCallback(int error, const char* description)
{
	LOG(Error, "GLFW %d: %s", error, description);
}

void WindowCloseCallback(GLFWwindow* window)
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	if (auto platform = reinterpret_cast<Platform*>(glfwGetWindowUserPointer(window)))
	{
		platform->Resize(width, height);
	}
}

void WindowFocusCallback(GLFWwindow* window, int focused)
{
	if (auto platform = reinterpret_cast<Platform*>(glfwGetWindowUserPointer(window)))
	{
		platform->SetFocus(focused);
	}
}

inline KeyCode TranslateKeyCode(int key)
{
	static const std::unordered_map<int, KeyCode> keyLookup =
	{
		{GLFW_KEY_SPACE, KeyCode::Space},
		{GLFW_KEY_APOSTROPHE, KeyCode::Apostrophe},
		{GLFW_KEY_COMMA, KeyCode::Comma},
		{GLFW_KEY_MINUS, KeyCode::Minus},
		{GLFW_KEY_PERIOD, KeyCode::Period},
		{GLFW_KEY_SLASH, KeyCode::Slash},
		{GLFW_KEY_0, KeyCode::_0},
		{GLFW_KEY_1, KeyCode::_1},
		{GLFW_KEY_2, KeyCode::_2},
		{GLFW_KEY_3, KeyCode::_3},
		{GLFW_KEY_4, KeyCode::_4},
		{GLFW_KEY_5, KeyCode::_5},
		{GLFW_KEY_6, KeyCode::_6},
		{GLFW_KEY_7, KeyCode::_7},
		{GLFW_KEY_8, KeyCode::_8},
		{GLFW_KEY_9, KeyCode::_9},
		{GLFW_KEY_SEMICOLON, KeyCode::Semicolon},
		{GLFW_KEY_EQUAL, KeyCode::Equal},
		{GLFW_KEY_A, KeyCode::A},
		{GLFW_KEY_B, KeyCode::B},
		{GLFW_KEY_C, KeyCode::C},
		{GLFW_KEY_D, KeyCode::D},
		{GLFW_KEY_E, KeyCode::E},
		{GLFW_KEY_F, KeyCode::F},
		{GLFW_KEY_G, KeyCode::G},
		{GLFW_KEY_H, KeyCode::H},
		{GLFW_KEY_I, KeyCode::I},
		{GLFW_KEY_J, KeyCode::J},
		{GLFW_KEY_K, KeyCode::K},
		{GLFW_KEY_L, KeyCode::L},
		{GLFW_KEY_M, KeyCode::M},
		{GLFW_KEY_N, KeyCode::N},
		{GLFW_KEY_O, KeyCode::O},
		{GLFW_KEY_P, KeyCode::P},
		{GLFW_KEY_Q, KeyCode::Q},
		{GLFW_KEY_R, KeyCode::R},
		{GLFW_KEY_S, KeyCode::S},
		{GLFW_KEY_T, KeyCode::T},
		{GLFW_KEY_U, KeyCode::U},
		{GLFW_KEY_V, KeyCode::V},
		{GLFW_KEY_W, KeyCode::W},
		{GLFW_KEY_X, KeyCode::X},
		{GLFW_KEY_Y, KeyCode::Y},
		{GLFW_KEY_Z, KeyCode::Z},
		{GLFW_KEY_LEFT_BRACKET, KeyCode::LeftBracket},
		{GLFW_KEY_BACKSLASH, KeyCode::Backslash},
		{GLFW_KEY_RIGHT_BRACKET, KeyCode::RightBracket},
		{GLFW_KEY_GRAVE_ACCENT, KeyCode::GraveAccent},
		{GLFW_KEY_ESCAPE, KeyCode::Escape},
		{GLFW_KEY_ENTER, KeyCode::Enter},
		{GLFW_KEY_TAB, KeyCode::Tab},
		{GLFW_KEY_BACKSPACE, KeyCode::Backspace},
		{GLFW_KEY_INSERT, KeyCode::Insert},
		{GLFW_KEY_DELETE, KeyCode::DelKey},
		{GLFW_KEY_RIGHT, KeyCode::Right},
		{GLFW_KEY_LEFT, KeyCode::Left},
		{GLFW_KEY_DOWN, KeyCode::Down},
		{GLFW_KEY_UP, KeyCode::Up},
		{GLFW_KEY_PAGE_UP, KeyCode::PageUp},
		{GLFW_KEY_PAGE_DOWN, KeyCode::PageDown},
		{GLFW_KEY_HOME, KeyCode::Home},
		{GLFW_KEY_END, KeyCode::End},
		{GLFW_KEY_CAPS_LOCK, KeyCode::CapsLock},
		{GLFW_KEY_SCROLL_LOCK, KeyCode::ScrollLock},
		{GLFW_KEY_NUM_LOCK, KeyCode::NumLock},
		{GLFW_KEY_PRINT_SCREEN, KeyCode::PrintScreen},
		{GLFW_KEY_PAUSE, KeyCode::Pause},
		{GLFW_KEY_F1, KeyCode::F1},
		{GLFW_KEY_F2, KeyCode::F2},
		{GLFW_KEY_F3, KeyCode::F3},
		{GLFW_KEY_F4, KeyCode::F4},
		{GLFW_KEY_F5, KeyCode::F5},
		{GLFW_KEY_F6, KeyCode::F6},
		{GLFW_KEY_F7, KeyCode::F7},
		{GLFW_KEY_F8, KeyCode::F8},
		{GLFW_KEY_F9, KeyCode::F9},
		{GLFW_KEY_F10, KeyCode::F10},
		{GLFW_KEY_F11, KeyCode::F11},
		{GLFW_KEY_F12, KeyCode::F12},
		{GLFW_KEY_KP_0, KeyCode::KP_0},
		{GLFW_KEY_KP_1, KeyCode::KP_1},
		{GLFW_KEY_KP_2, KeyCode::KP_2},
		{GLFW_KEY_KP_3, KeyCode::KP_3},
		{GLFW_KEY_KP_4, KeyCode::KP_4},
		{GLFW_KEY_KP_5, KeyCode::KP_5},
		{GLFW_KEY_KP_6, KeyCode::KP_6},
		{GLFW_KEY_KP_7, KeyCode::KP_7},
		{GLFW_KEY_KP_8, KeyCode::KP_8},
		{GLFW_KEY_KP_9, KeyCode::KP_9},
		{GLFW_KEY_KP_DECIMAL, KeyCode::KP_Decimal},
		{GLFW_KEY_KP_DIVIDE, KeyCode::KP_Divide},
		{GLFW_KEY_KP_MULTIPLY, KeyCode::KP_Multiply},
		{GLFW_KEY_KP_SUBTRACT, KeyCode::KP_Subtract},
		{GLFW_KEY_KP_ADD, KeyCode::KP_Add},
		{GLFW_KEY_KP_ENTER, KeyCode::KP_Enter},
		{GLFW_KEY_KP_EQUAL, KeyCode::KP_Equal},
		{GLFW_KEY_LEFT_SHIFT, KeyCode::LeftShift},
		{GLFW_KEY_LEFT_CONTROL, KeyCode::LeftControl},
		{GLFW_KEY_LEFT_ALT, KeyCode::LeftAlt},
		{GLFW_KEY_RIGHT_SHIFT, KeyCode::RightShift},
		{GLFW_KEY_RIGHT_CONTROL, KeyCode::RightControl},
		{GLFW_KEY_RIGHT_ALT, KeyCode::RightAlt},
	};

	auto keyIt = keyLookup.find(key);

	if (keyIt == keyLookup.end())
	{
		return KeyCode::Unknown;
	}

	return keyIt->second;
}

inline KeyAction TranslateKeyAction(int action)
{
	if (action == GLFW_PRESS)
	{
		return KeyAction::Down;
	}
	else if (action == GLFW_RELEASE)
	{
		return KeyAction::Up;
	}
	else if (action == GLFW_REPEAT)
	{
		return KeyAction::Repeat;
	}

	return KeyAction::Unknown;
}

void KeyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
	KeyCode keyCode = TranslateKeyCode(key);
	KeyAction keyAction = TranslateKeyAction(action);

	if (auto platform = reinterpret_cast<Platform*>(glfwGetWindowUserPointer(window)))
	{
		platform->ConsumeInputEvent(KeyInputEvent(keyCode, keyAction));
	}
}

inline MouseButton TranslateMouseButton(int button)
{
	if (button < GLFW_MOUSE_BUTTON_6)
	{
		return static_cast<MouseButton>(button);
	}

	return MouseButton::Unknown;
}

inline MouseAction TranslateMouseAction(int action)
{
	if (action == GLFW_PRESS)
	{
		return MouseAction::Down;
	}
	else if (action == GLFW_RELEASE)
	{
		return MouseAction::Up;
	}

	return MouseAction::Unknown;
}

void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (auto* platform = reinterpret_cast<Platform*>(glfwGetWindowUserPointer(window)))
	{
		platform->ConsumeInputEvent(MouseInputEvent(MouseButton::Unknown, MouseAction::Move, static_cast<float>(xpos), static_cast<float>(ypos)));
	}
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/)
{
	MouseAction mouseAction = TranslateMouseAction(action);

	if (auto* platform = reinterpret_cast<Platform*>(glfwGetWindowUserPointer(window)))
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		platform->ConsumeInputEvent(MouseInputEvent{
			TranslateMouseButton(button),
			mouseAction,
			static_cast<float>(xpos),
			static_cast<float>(ypos) });
	}
}
}	// namespace
}	// namespace vge

vge::GlfwWindow::GlfwWindow(Platform* platform, const Window::Properties& properties) 
	: Window(properties)
{
	ENSURE(glfwInit());

	glfwSetErrorCallback(ErrorCallback);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	switch (properties.Mode)
	{
	case Window::Mode::Fullscreen:
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		_Handle = glfwCreateWindow(mode->width, mode->height, properties.Title.c_str(), monitor, NULL);
		break;
	}

	case Window::Mode::FullscreenBorderless:
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		_Handle = glfwCreateWindow(mode->width, mode->height, properties.Title.c_str(), monitor, NULL);
		break;
	}

	case Window::Mode::FullscreenStretch:
	{
		ENSURE_MSG(false, "Cannot support stretch mode on this platform.");
		break;
	}

	default:
		_Handle = glfwCreateWindow(properties.Extent.width, properties.Extent.height, properties.Title.c_str(), NULL, NULL);
		break;
	}

	Resize(Extent{ properties.Extent.width, properties.Extent.height });

	ENSURE_MSG(_Handle, "Couldn't create glfw window.");

	glfwSetWindowUserPointer(_Handle, platform);

	glfwSetWindowCloseCallback(_Handle, WindowCloseCallback);
	glfwSetWindowSizeCallback(_Handle, WindowSizeCallback);
	glfwSetWindowFocusCallback(_Handle, WindowFocusCallback);
	glfwSetKeyCallback(_Handle, KeyCallback);
	glfwSetCursorPosCallback(_Handle, CursorPositionCallback);
	glfwSetMouseButtonCallback(_Handle, MouseButtonCallback);

	glfwSetInputMode(_Handle, GLFW_STICKY_KEYS, 1);
	glfwSetInputMode(_Handle, GLFW_STICKY_MOUSE_BUTTONS, 1);
}

vge::GlfwWindow::~GlfwWindow()
{
	glfwTerminate();
}

VkSurfaceKHR vge::GlfwWindow::CreateSurface(Instance& instance)
{
	return CreateSurface(instance.GetHandle(), VK_NULL_HANDLE);
}

VkSurfaceKHR vge::GlfwWindow::CreateSurface(VkInstance instance, VkPhysicalDevice)
{
	if (!instance || !_Handle)
	{
		return VK_NULL_HANDLE;
	}

	VkSurfaceKHR surface;

	VkResult result = glfwCreateWindowSurface(instance, _Handle, NULL, &surface);

	if (result != VK_SUCCESS)
	{
		return nullptr;
	}

	return surface;
}

bool vge::GlfwWindow::ShouldClose()
{
	return glfwWindowShouldClose(_Handle);
}

void vge::GlfwWindow::ProcessEvents()
{
	glfwPollEvents();
}

void vge::GlfwWindow::Close()
{
	glfwSetWindowShouldClose(_Handle, GLFW_TRUE);
}

// Calculates the dpi factor using the density from GLFW physical size.
// See https://www.glfw.org/docs/latest/monitor_guide.html#monitor_size
float vge::GlfwWindow::GetDpiFactor() const
{
	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* vidmode = glfwGetVideoMode(primaryMonitor);

	int mmWidth, mmHeight;
	glfwGetMonitorPhysicalSize(primaryMonitor, &mmWidth, &mmHeight);

	// As suggested by the GLFW monitor guide.
	static const float inchToMM = 25.0f;
	static const float winBaseDensity = 96.0f;

	const u32 dpi = static_cast<u32>(vidmode->width / (mmWidth / inchToMM));
	const float dpiFactor = dpi / winBaseDensity;
	return dpiFactor;
}

float vge::GlfwWindow::GetContentScaleFactor() const
{
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(_Handle, &fbWidth, &fbHeight);
	int winWidth, winHeight;
	glfwGetWindowSize(_Handle, &winWidth, &winHeight);

	// We could return a 2D result here instead of a scalar, but non-uniform scaling is very unlikely,
	// and would require significantly more changes in the IMGUI integration.
	return static_cast<float>(fbWidth) / winWidth;
}

std::vector<const char*> vge::GlfwWindow::GetRequiredSurfaceExtensions() const
{
	u32 glfwExtensionCount = 0;
	const char** names = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	return { names, names + glfwExtensionCount };
}
