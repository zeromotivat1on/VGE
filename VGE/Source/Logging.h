#pragma once

#include <cstdarg>
#include <string>

#ifdef NDEBUG
#define LOG(Category, Message, ...)
#else
#define LOG(Category, Message, ...) Logger::PrintLog(Category, Message, __FUNCTION__, __VA_ARGS__);
#endif

enum LogCategory : uint8_t
{
	Log,
	Warning,
	Error,
};

class Logger final
{
public:
	static void PrintLog(const LogCategory category, const char* message, ...)
	{
		va_list args;
		va_start(args, message);
		PrintLog_Implementation(category, message, args);
		va_end(args);
	}

private:
	static void PrintLog_Implementation(const LogCategory category, const char* message, va_list args)
	{
		std::string categoryStr = [category]()
		{
			switch (category)
			{
			case LogCategory::Warning:
				return "Warning: ";

			case LogCategory::Error:
				return "Error: ";

			default:
				return "Log: ";
			}
		}();

		std::string caller = "[%s]: ";
		vprintf((categoryStr + caller + message + '\n').c_str(), args);
	}
};
