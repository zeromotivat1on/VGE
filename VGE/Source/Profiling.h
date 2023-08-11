#pragma once

#include <chrono>
#include "Logging.h"

#ifdef NDEBUG
#define SCOPE_TIMER(LogPrefix)
#else
#define SCOPE_TIMER(LogPrefix) ScopeTimer __scope_timer__LINE__(LogPrefix);
#endif

struct ScopeTimer
{
public:
	ScopeTimer(const std::string_view logPrefix) : m_LogPrefix(logPrefix)
	{
		m_StartTime = std::chrono::high_resolution_clock::now();
	}

	~ScopeTimer()
	{
		const auto endTime = std::chrono::high_resolution_clock::now();
		const float diffTime = std::chrono::duration<float, std::chrono::milliseconds::period>(endTime - m_StartTime).count();
		LOG(Log, "%s: %fms", m_LogPrefix.data(), diffTime);
	}

private:
	std::chrono::steady_clock::time_point m_StartTime;
	std::string_view m_LogPrefix;
};

