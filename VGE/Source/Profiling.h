#pragma once

#include <chrono>
#include "Logging.h"
#include "Macros.h"

#if DEBUG
	#define ENABLE_SCOPE_TIMERS (USE_LOGGING && 1)
#else
	#define ENABLE_SCOPE_TIMERS (USE_LOGGING && 0)
#endif

#if ENABLE_SCOPE_TIMERS
	#define SCOPE_TIMER(LogPrefix) ScopeTimer GLUE(__scope_timer_, __LINE__)(LogPrefix);
#else
	#define SCOPE_TIMER(LogPrefix)
#endif

#if ENABLE_SCOPE_TIMERS
struct ScopeTimer
{
public:
	ScopeTimer(std::string_view logPrefix) : m_LogPrefix(logPrefix)
	{
		m_StartTime = std::chrono::high_resolution_clock::now();
	}

	~ScopeTimer()
	{
		const auto endTime = std::chrono::high_resolution_clock::now();
		const float diffTime = std::chrono::duration<float, std::chrono::milliseconds::period>(endTime - m_StartTime).count();
		LOG(Log, "%s: %.2fms", m_LogPrefix.data(), diffTime);
	}

private:
	std::chrono::steady_clock::time_point m_StartTime;
	std::string_view m_LogPrefix;
};
#endif
