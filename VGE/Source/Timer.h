#pragma once

#include <chrono>

namespace vge
{
 // Encapsulates basic usage of chrono, providing a means to calculate float durations between time points via function calls.
class Timer
{
public:
	using Clock = std::chrono::steady_clock;
	using Seconds = std::ratio<1>;
	using Milliseconds = std::ratio<1, 1000>;
	using Microseconds = std::ratio<1, 1000000>;
	using Nanoseconds = std::ratio<1, 1000000000>;
	using DefaultResolution = Seconds;

public:
	Timer() : _StartTime(Clock::now()), _PreviousTick(Clock::now()) {}

	virtual ~Timer() = default;

public:
	inline bool IsRunning() const { return _Running; }

	// Starts the timer, Elapsed() now returns the duration since Start().
	void Start()
	{
		if (!_Running)
		{
			_Running = true;
			_StartTime = Clock::now();
		}
	}

	// Laps the timer, Elapsed() now returns the duration since the last Lap().
	void Lap()
	{
		_Lapping = true;
		_LapTime = Clock::now();
	}

	// Stops the timer, Elapsed() now returns 0.
	// @return The total execution time between Start() and Stop().
	template <typename T = DefaultResolution>
	double Stop()
	{
		if (!_Running)
		{
			return 0;
		}

		_Running = false;
		_Lapping = false;

		auto duration = std::chrono::duration<double, T>(Clock::now() - _StartTime);
		_StartTime = Clock::now();
		_LapTime = Clock::now();

		return duration.count();
	}

	// Calculates the time difference between now and when the timer was started if Lap() was called, then between now and when the timer was last lapped.
	// @return The duration between the two time points (default in seconds).
	template <typename T = DefaultResolution>
	double Elapsed()
	{
		if (!_Running)
		{
			return 0;
		}

		Clock::time_point start = _StartTime;

		if (_Lapping)
		{
			start = _LapTime;
		}

		return std::chrono::duration<double, T>(Clock::now() - start).count();
	}

	// Calculates the time difference between now and the last time this function was called
	// @return The duration between the two time points (default in seconds)
	template <typename T = DefaultResolution>
	double Tick()
	{
		auto now = Clock::now();
		auto duration = std::chrono::duration<double, T>(now - _PreviousTick);
		_PreviousTick = now;
		return duration.count();
	}

private:
	bool _Running = false;
	bool _Lapping = false;
	Clock::time_point _StartTime;
	Clock::time_point _LapTime;
	Clock::time_point _PreviousTick;
};
}	// namespace vge