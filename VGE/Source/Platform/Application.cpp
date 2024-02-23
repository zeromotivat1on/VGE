#include "Platform/Application.h"

vge::Application::Application() 
	:_Name("Sample Name")
{
}

bool vge::Application::Prepare(const ApplicationOptions& options)
{
	ENSURE(options.Window);

	//auto& _debug_info = get_debug_info();
	//_debug_info.insert<field::MinMax, float>("fps", fps);
	//_debug_info.insert<field::MinMax, float>("frame_time", frame_time);

	_LockSimulationSpeed = options.BenchmarkEnabled;
	_Window = options.Window;

	return true;
}

void vge::Application::Finish()
{
}

bool vge::Application::Resize(const u32 /*width*/, const u32 /*height*/)
{
	return true;
}

void vge::Application::ConsumeInputEvent(const InputEvent& inputEvent)
{
}

void vge::Application::Update(float deltaTime)
{
	_Fps = 1.0f / deltaTime;
	_FrameTime = deltaTime * 1000.0f;
}