#include "Logging.h"
#include "Color.h"

#if USE_LOGGING
void vge::Logger::PrintLog(const LogCategory category, const char* message, ...)
{
	va_list args;
	va_start(args, message);
	PrintLog_Implementation(category, message, args);
	va_end(args);
}

void vge::Logger::PrintLogRaw(const char* message, ...)
{
	va_list args;
	va_start(args, message);
	PrintLogRaw_Implementation(message, args);
	va_end(args);
}

void vge::Logger::PrintLog_Implementation(const LogCategory category, const char* message, va_list args)
{
	PaintConsoleText(category);
	
	const std::string categoryStr = LogCategoryToString(category);
	const std::string caller = "[%s::%d]: "; // function name + log line
	vprintf((categoryStr + caller + message + '\n').c_str(), args);

	PaintDefaultConsoleText();
}

void vge::Logger::PrintLogRaw_Implementation(const char* message, va_list args)
{
	vprintf(message, args);
}

void vge::Logger::PaintConsoleText(const LogCategory category)
{
	switch (category)
	{
	case LogCategory::Warning:
		std::cout << hue::yellow; return;
	case LogCategory::Error:
		std::cout << hue::red; return;
	default:
		return;
	}
}

void vge::Logger::PaintDefaultConsoleText()
{
	std::cout << hue::reset;
}

std::string vge::Logger::LogCategoryToString(const LogCategory category)
{
	switch (category)
	{
	case LogCategory::Warning:
		std::cout << hue::yellow;
		return "Warning: ";
	case LogCategory::Error:
		std::cout << hue::red;
		return "Error: ";
	default:
		PaintDefaultConsoleText();
		return "Log: ";
	}
}
#endif
