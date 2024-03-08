#include "Platform.h"

const vge::u32 vge::Platform::MinWindowWidth = 420;
const vge::u32 vge::Platform::MinWindowHeight = 320;
std::string vge::Platform::_ExternalStorageDirectory = "";
std::string vge::Platform::_TempDirectory = "";

vge::Platform::Platform(const vge::PlatformContext& context)
{
	_Arguments = context.GetArguments();

	_ExternalStorageDirectory = context.GetExternalStorageDirectory();
	_TempDirectory = context.GetTempDirectory();
}

vge::ExitCode vge::Platform::Initialize(/*const std::vector<Plugin*>& plugins*/)
{
//	auto sinks = get_platform_sinks();
//
//	auto logger = std::make_shared<spdlog::logger>("logger", sinks.begin(), sinks.end());
//
//#ifdef VKB_DEBUG
//	logger->set_level(spdlog::level::debug);
//#else
//	logger->set_level(spdlog::level::info);
//#endif
//
//	logger->set_pattern(LOGGER_FORMAT);
//	spdlog::set_default_logger(logger);
//
//	LOGI("Logger initialized");
//
//	parser = std::make_unique<CLI11CommandParser>("vulkan_samples", "\n\tVulkan Samples\n\n\t\tA collection of samples to demonstrate the Vulkan best practice.\n", arguments);
//
//	// Process command line arguments
//	if (!parser->parse(associate_plugins(plugins)))
//	{
//		return ExitCode::Help;
//	}
//
//	// Subscribe plugins to requested hooks and store activated plugins
//	for (auto* plugin : plugins)
//	{
//		if (plugin->activate_plugin(this, *parser.get()))
//		{
//			auto& plugin_hooks = plugin->get_hooks();
//			for (auto hook : plugin_hooks)
//			{
//				auto it = hooks.find(hook);
//
//				if (it == hooks.end())
//				{
//					auto r = hooks.emplace(hook, std::vector<Plugin*>{});
//
//					if (r.second)
//					{
//						it = r.first;
//					}
//				}
//
//				it->second.emplace_back(plugin);
//			}
//
//			active_plugins.emplace_back(plugin);
//		}
//	}
//
//	// Platform has been closed by a plugins initialization phase
//	if (close_requested)
//	{
//		return ExitCode::Close;
//	}

	CreateWindow(_WindowProperties);

	if (!_Window)
	{
		LOG(Error, "Window creation failed.");
		return ExitCode::FatalError;
	}

	return ExitCode::Success;
}

vge::ExitCode vge::Platform::MainLoop()
{
	if (!IsAppRequested())
	{
		LOG(Log, "An app was not requested, can not continue.");
		return ExitCode::Close;
	}

	while (!_Window->ShouldClose() && !_CloseRequested)
	{
		// Load the requested app.
		if (IsAppRequested())
		{
			if (!StartApp())
			{
				LOG(Error, "Failed to load requested application.");
				return ExitCode::FatalError;
			}

			// Compensate for load times of the app by rendering the first frame pre-emptively.
			_Timer.Tick();
			_ActiveApp->Update(0.01667f);
		}

		Update();

		if (_ActiveApp && _ActiveApp->ShouldClose())
		{
			std::string id = _ActiveApp->GetName();
			//on_app_close(id);
			_ActiveApp->Finish();
		}

		_Window->ProcessEvents();
	}

	return ExitCode::Success;
}

void vge::Platform::Update()
{
	float deltaTime = static_cast<float>(_Timer.Tick());

	if (_Focused)
	{
		//on_update(delta_time);

		if (_FixedSimulationFps)
		{
			deltaTime = _SimulationFrameTime;
		}

		_ActiveApp->Update(deltaTime);

		//if (auto* app = dynamic_cast<VulkanSample*>(_ActiveApp.get()))
		//{
		//	if (app->has_render_context())
		//	{
		//		on_post_draw(app->get_render_context());
		//	}
		//}
		//else if (auto* app = dynamic_cast<HPPVulkanSample*>(_ActiveApp.get()))
		//{
		//	if (app->has_render_context())
		//	{
		//		on_post_draw(reinterpret_cast<vkb::RenderContext&>(app->get_render_context()));
		//	}
		//}
	}
}

void vge::Platform::Terminate(ExitCode code)
{
	if (code == ExitCode::Help)
	{
		//auto help = parser->help();
		//for (auto& line : help)
		//{
		//	LOGI(line);
		//}
	}

	if (_ActiveApp)
	{
		//std::string id = _ActiveApp->get_name();
		//on_app_close(id);
		_ActiveApp->Finish();
	}

	_ActiveApp.reset();
	_Window.reset();

	//spdlog::drop_all();

	//on_vge::Platform_close();

	// Halt on all unsuccessful exit codes unless ForceClose is in use
//	if (code != ExitCode::Success && !using_plugin<::plugins::ForceClose>())
//	{
//#ifndef ANDROID
//		std::cout << "Press return to continue";
//		std::cin.get();
//#endif
//	}
}

void vge::Platform::Close()
{
	if (_Window)
	{
		_Window->Close();
	}

	// Fallback incase a window is not yet in use.
	_CloseRequested = true;
}

void vge::Platform::SetWindowProperties(const Window::OptionalProperties& properties)
{
	_WindowProperties.Title = properties.Title.has_value() ? properties.Title.value() : _WindowProperties.Title;
	_WindowProperties.Mode = properties.Mode.has_value() ? properties.Mode.value() : _WindowProperties.Mode;
	_WindowProperties.Resizable = properties.Resizable.has_value() ? properties.Resizable.value() : _WindowProperties.Resizable;
	_WindowProperties.Vsync = properties.Vsync.has_value() ? properties.Vsync.value() : _WindowProperties.Vsync;
	_WindowProperties.Extent.width = properties.Extent.width.has_value() ? properties.Extent.width.value() : _WindowProperties.Extent.width;
	_WindowProperties.Extent.height = properties.Extent.height.has_value() ? properties.Extent.height.value() : _WindowProperties.Extent.height;
}

//std::vector<spdlog::sink_ptr> vge::Platform::get_vge::Platform_sinks()
//{
//	std::vector<spdlog::sink_ptr> sinks;
//	sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
//	return sinks;
//}

bool vge::Platform::StartApp()
{
	auto* requestedAppInfo = _RequestedAppInfo;
	// Reset early incase error in preparation stage.
	_RequestedAppInfo = nullptr;

	if (_ActiveApp)
	{
		auto executionTime = _Timer.Stop();
		LOG(Log, "Closing App (Runtime: {:.1f})", executionTime);

		_ActiveApp->Finish();
	}

	_ActiveApp = requestedAppInfo->Create();
	_ActiveApp->SetName(requestedAppInfo->Id);

	if (!_ActiveApp)
	{
		LOG(Error, "Failed to create a valid vulkan app.");
		return false;
	}

	if (!_ActiveApp->Prepare({ false, _Window.get() }))
	{
		LOG(Error, "Failed to prepare vulkan app.");
		return false;
	}

	//on_app_start(requestedAppInfo->id);

	return true;
}

void vge::Platform::ConsumeInputEvent(const InputEvent& inputEvent)
{
	if (_ProcessInputEvents && _ActiveApp)
	{
		_ActiveApp->ConsumeInputEvent(inputEvent);
	}

	if (inputEvent.GetSource() == EventSource::Keyboard)
	{
		const auto& key_event = static_cast<const KeyInputEvent&>(inputEvent);

		if (key_event.GetCode() == KeyCode::Back ||
			key_event.GetCode() == KeyCode::Escape)
		{
			Close();
		}
	}
}

void vge::Platform::Resize(u32 width, u32 height)
{
	const auto extent = Window::Extent{ std::max<u32>(width, MinWindowWidth), std::max<u32>(height, MinWindowHeight) };
	if (_Window && width > 0 && height > 0)
	{
		const auto actualExtent = _Window->Resize(extent);

		if (_ActiveApp)
		{
			_ActiveApp->Resize(actualExtent.width, actualExtent.height);
		}
	}
}

//#define HOOK(enum, func)                \
//	static auto res = hooks.find(enum); \
//	if (res != hooks.end())             \
//	{                                   \
//		for (auto plugin : res->second) \
//		{                               \
//			plugin->func;               \
//		}                               \
//	}
//
//void vge::Platform::on_post_draw(RenderContext& context)
//{
//	HOOK(Hook::PostDraw, on_post_draw(context));
//}
//
//void vge::Platform::on_app_error(const std::string& app_id)
//{
//	HOOK(Hook::OnAppError, on_app_error(app_id));
//}
//
//void vge::Platform::on_update(float delta_time)
//{
//	HOOK(Hook::OnUpdate, on_update(delta_time));
//}
//
//void vge::Platform::on_app_start(const std::string& app_id)
//{
//	HOOK(Hook::OnAppStart, on_app_start(app_id));
//}
//
//void vge::Platform::on_app_close(const std::string& app_id)
//{
//	HOOK(Hook::OnAppClose, on_app_close(app_id));
//}
//
//void vge::Platform::on_vge::Platform_close()
//{
//	HOOK(Hook::Onvge::PlatformClose, on_vge::Platform_close());
//}

//#undef HOOK
