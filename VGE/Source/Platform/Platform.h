#pragma once

#include "Core/VkCommon.h"
#include "AppInfo.h"
#include "Platform/Window.h"
#include "Platform/Context.h"
#include "Platform/Application.h"
#include "Timer.h"

namespace vge
{
enum class ExitCode
{
	Success = 0, // App executed as expected
	Help,        // App should show help
	Close,       // App has been requested to close at initialization
	FatalError   // App encountered an unexpected error
};

class Platform
{
public:
	Platform(const PlatformContext& context);

	virtual ~Platform() = default;

public:
	static const u32 MinWindowWidth;
	static const u32 MinWindowHeight;

public:
	inline static const std::string& GetExternalStorageDirectory() { return _ExternalStorageDirectory; } // returns the working directory of the application set by the platform
	inline static const std::string& GetTempDirectory() { return _TempDirectory; } // returns the suitable directory for temporary files from the environment variables set in the system
	inline static void SetExternalStorageDirectory(const std::string& dir) { _ExternalStorageDirectory = dir; }

public:
	inline Window& GetWindow() { ASSERT(_Window); return *_Window; }
	inline Application& GetApp() { ASSERT(_ActiveApp); return *_ActiveApp; }
	inline const Application& GetApp() const { ASSERT(_ActiveApp); return *_ActiveApp; }
	inline bool IsAppRequested() const { return _RequestedAppInfo != nullptr; }

	inline void RequestApplication(const AppInfo* app) { _RequestedAppInfo = app; }
	inline void SetFocus(bool focused) { focused = focused; }
	inline void DisableInputProcessing() { _ProcessInputEvents = false; }
	inline void ForceSimulationFps(float fps) { _FixedSimulationFps = true; _SimulationFrameTime = 1 / fps; }

	virtual ExitCode Initialize(/*const std::vector<Plugin*>& plugins*/);
	virtual void Terminate(ExitCode code);
	virtual void Close(); // requests to close the platform at the next available point
	virtual void Resize(u32 width, u32 height);
	virtual void ConsumeInputEvent(const InputEvent& inputEvent);

	bool StartApp();
	ExitCode MainLoop(); // run platform main loop
	void Update(); // advances the application for one frame
	void SetWindowProperties(const Window::OptionalProperties& properties);


	//template <class T>
	//T* get_plugin() const;

	//template <class T>
	//bool using_plugin() const;

	//void on_post_draw(RenderContext& context);

protected:
	//virtual std::vector<spdlog::sink_ptr> get_platform_sinks();

	virtual void CreateWindow(const Window::Properties& properties) = 0;

	//void on_update(float delta_time);
	//void on_app_error(const std::string& app_id);
	//void on_app_start(const std::string& app_id);
	//void on_app_close(const std::string& app_id);
	//void on_platform_close();

protected:
	//std::unique_ptr<CommandParser> parser;
	//std::vector<Plugin*> active_plugins;
	//std::unordered_map<Hook, std::vector<Plugin*>> hooks;
	std::unique_ptr<Window> _Window = nullptr;
	std::unique_ptr<Application> _ActiveApp = nullptr;
	Window::Properties _WindowProperties;	// Source of truth for window state
	bool _FixedSimulationFps = false; // delta time should be fixed with a fabricated value
	float _SimulationFrameTime = 0.016f; // fabricated delta time
	bool _ProcessInputEvents = true; // app should continue processing input events
	bool _Focused = true; // app is currently in focus at an operating system level
	bool _CloseRequested = false;

private:
	static std::string _ExternalStorageDirectory;
	static std::string _TempDirectory;

private:
	Timer _Timer;
	const AppInfo* _RequestedAppInfo = nullptr;
	std::vector<std::string> _Arguments;
	//std::vector<Plugin*> plugins;
};

//template <class T>
//bool Platform::using_plugin() const
//{
//	return !plugins::with_tags<T>(active_plugins).empty();
//}
//
//template <class T>
//T* Platform::get_plugin() const
//{
//	assert(using_plugin<T>() && "Plugin is not enabled but was requested");
//	const auto plugins = plugins::with_tags<T>(active_plugins);
//	return dynamic_cast<T*>(plugins[0]);
//}
}	// namespace vge
