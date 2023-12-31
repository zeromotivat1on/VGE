#pragma once

#include <vulkan/vulkan.h>
#include <cstdarg>
#include <string>
#include "Types.h"
#include "Macros.h"

#if DEBUG
	#define USE_LOGGING 1
#else
	#define USE_LOGGING 0
#endif

#define LOG_VK_VERBOSE 0

#if USE_LOGGING
	#define LOG(Category, Message, ...)	vge::Logger::PrintLog(vge::LogCategory::Category, Message, __FUNCTION__, __LINE__, __VA_ARGS__);
	#define LOG_RAW(Message, ...)		vge::Logger::PrintLogRaw(Message, __VA_ARGS__);
#else
	#define LOG(Category, Message, ...)
	#define LOG_RAW(Message, ...)
#endif

#ifndef USE_COLORED_LOGS
	#define USE_COLORED_LOGS 1
#endif

#if USE_LOGGING
namespace vge
{
	enum LogCategory : u8
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

		// Change console text color according to a given category.
		static void PaintConsoleText(const LogCategory category);

		// Restore default console text color - white.
		static void PaintDefaultConsoleText();

	private:
		static void PrintLog_Implementation(const LogCategory category, const char* message, va_list args);
		static void PrintLogRaw_Implementation(const char* message, va_list args);
	};

	extern void NotifyVulkanEnsureFailure(VkResult result, const char* function, const char* filename, u32 line, const char* errMessage = "");
}
#endif
