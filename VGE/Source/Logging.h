#pragma once

#include <vulkan/vulkan.h>
#include <cstdarg>
#include <string>
#include "Types.h"

#define LOG_VK_VERBOSE 0

#ifdef NDEBUG
#define LOG(Category, Message, ...)
#define LOG_RAW(Message, ...)
#else
#define LOG(Category, Message, ...)	vge::Logger::PrintLog(vge::LogCategory::Category, Message, __FUNCTION__, __VA_ARGS__);
#define LOG_RAW(Message, ...)		vge::Logger::PrintLogRaw(Message, __VA_ARGS__);
#endif

namespace vge
{
	enum LogCategory : uint8
	{
		Log,
		Warning,
		Error,
	};

	class Logger final
	{
	public:
		// Formatted message log.
		static void PrintLog(const LogCategory category, const char* message, ...);

		// Log given message with args as given.
		static void PrintLogRaw(const char* message, ...);

		static std::string LogCategoryToString(const LogCategory category);

	private:
		static void PrintLog_Implementation(const LogCategory category, const char* message, va_list args);
		static void PrintLogRaw_Implementation(const char* message, va_list args);
	};

	extern void NotifyVulkanEnsureFailure(VkResult result, const char* function, const char* filename, uint32 line, const char* errMessage = "");
}
