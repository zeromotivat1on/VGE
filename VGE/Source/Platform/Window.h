#pragma once

#include "..\Render\Core\VkCommon.h"
#include "Core/Instance.h"

namespace vge
{
class Window
{
public:
	struct Extent
	{
		u32 width;
		u32 height;
	};

	struct OptionalExtent
	{
		std::optional<u32> width;
		std::optional<u32> height;
	};

	enum class Mode : u8
	{
		Headless,
		Fullscreen,
		FullscreenBorderless,
		FullscreenStretch,
		Default
	};

	enum class Vsync : u8
	{
		Off,
		On,
		Default
	};

	struct OptionalProperties
	{
		std::optional<std::string> Title;
		std::optional<Mode> Mode;
		std::optional<bool> Resizable;
		std::optional<Vsync> Vsync;
		OptionalExtent Extent;
	};

	struct Properties
	{
		std::string Title = "";
		Mode Mode = Mode::Default;
		bool Resizable = true;
		Vsync Vsync = Vsync::Default;
		Extent Extent = { 1280, 720 };
	};

public:
	Window(const Properties& properties);

	virtual ~Window() = default;

public:
	inline const Properties& GetProperties() const { return _Properties; }
	inline const Extent& GetExtent() const { return _Properties.Extent; }
	inline Mode GetWindowMode() const { return _Properties.Mode; }
	inline bool IsHeadless() const { return _Properties.Mode == Mode::Headless; }
	inline virtual float GetContentScaleFactor() const { return 1.0f; } // get the scale factor for systems with heterogeneous window and pixel coordinates
	inline virtual bool GetDisplayPresentInfo(VkDisplayPresentInfoKHR* info, u32 srcWidth, u32 srcHeight) const { return false; }
	inline virtual void ProcessEvents() {}

	virtual VkSurfaceKHR CreateSurface(Instance& instance) = 0;
	virtual VkSurfaceKHR CreateSurface(VkInstance instance, VkPhysicalDevice physicalDevice) = 0;
	virtual bool ShouldClose() = 0;
	virtual void Close() = 0;
	virtual float GetDpiFactor() const = 0;
	virtual std::vector<const char*> GetRequiredSurfaceExtensions() const = 0;

	Extent Resize(const Extent& extent); // attempt to resize the window, not guaranteed to change

protected:
	Properties _Properties;
};
}	// namespace vge
