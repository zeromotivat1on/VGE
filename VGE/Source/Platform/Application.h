#pragma once

#include <string>
#include "Types.h"
#include "Macros.h"
#include "Logging.h"
#include "Platform/InputEvens.h"

namespace vge
{
class Window;

struct ApplicationOptions
{
	bool BenchmarkEnabled = false;
	vge::Window* Window = nullptr;
};

class Application
{
public:
	Application();

	virtual ~Application() = default;

public:
	// Request the app to close, does not guarantee that the app will close immediately
	inline void Close() { _RequestedClose = true; }
	inline bool ShouldClose() const { return _RequestedClose; }
	inline void SetName(const std::string& name) { _Name = name; }
	inline const std::string& GetName() const { return _Name; }

	virtual bool Prepare(const ApplicationOptions& options);
	virtual void Update(float deltaTime);
	virtual void Finish();
	virtual bool Resize(const u32 width, const u32 height);
	virtual void ConsumeInputEvent(const InputEvent& inputEvent);

	//DebugInfo& get_debug_info();

protected:
	float _Fps = 0.0f;
	float _FrameTime = 0.0f; // in ms
	u32 _FrameCount = 0;
	u32 _LastFrameCount = 0;
	bool _LockSimulationSpeed = false;
	Window* _Window = nullptr;

private:
	std::string _Name;
	//DebugInfo debug_info{};
	bool _RequestedClose = false;
};
}	// namespace vge
