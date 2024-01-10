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

void vge::NotifyVulkanEnsureFailure(VkResult result, const char* function, const char* filename, u32 line, const char* errMessage)
{
	const char* resultString = nullptr;
	switch (result)
	{
#define VK_RESULT_CASE(x) case x: resultString = STRING(x)
		VK_RESULT_CASE(VK_NOT_READY);							break;
		VK_RESULT_CASE(VK_TIMEOUT);								break;
		VK_RESULT_CASE(VK_EVENT_SET);							break;
		VK_RESULT_CASE(VK_EVENT_RESET);							break;
		VK_RESULT_CASE(VK_INCOMPLETE);							break;
		VK_RESULT_CASE(VK_ERROR_OUT_OF_HOST_MEMORY);			break;
		VK_RESULT_CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY);			break;
		VK_RESULT_CASE(VK_ERROR_INITIALIZATION_FAILED);			break;
		VK_RESULT_CASE(VK_ERROR_DEVICE_LOST);					break;
		VK_RESULT_CASE(VK_ERROR_MEMORY_MAP_FAILED);				break;
		VK_RESULT_CASE(VK_ERROR_LAYER_NOT_PRESENT);				break;
		VK_RESULT_CASE(VK_ERROR_EXTENSION_NOT_PRESENT);			break;
		VK_RESULT_CASE(VK_ERROR_FEATURE_NOT_PRESENT);			break;
		VK_RESULT_CASE(VK_ERROR_INCOMPATIBLE_DRIVER);			break;
		VK_RESULT_CASE(VK_ERROR_TOO_MANY_OBJECTS);				break;
		VK_RESULT_CASE(VK_ERROR_FORMAT_NOT_SUPPORTED);			break;
		VK_RESULT_CASE(VK_ERROR_SURFACE_LOST_KHR);				break;
		VK_RESULT_CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);		break;
		VK_RESULT_CASE(VK_SUBOPTIMAL_KHR);						break;
		VK_RESULT_CASE(VK_ERROR_OUT_OF_DATE_KHR);				break;
		VK_RESULT_CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);		break;
		VK_RESULT_CASE(VK_ERROR_VALIDATION_FAILED_EXT);			break;
		VK_RESULT_CASE(VK_ERROR_INVALID_SHADER_NV);				break;
		VK_RESULT_CASE(VK_ERROR_FRAGMENTED_POOL);				break;
		VK_RESULT_CASE(VK_ERROR_OUT_OF_POOL_MEMORY_KHR);		break;
		VK_RESULT_CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR);	break;
		VK_RESULT_CASE(VK_ERROR_NOT_PERMITTED_EXT);				break;
#undef VK_RESULT_CASE
	default:
		break;
	}

	LOG(Error, "%s failed with VkResult=%s(%d) in %s:%u.", function, resultString, result, filename, line);
	CLOG(errMessage, Error, "User message: %s", errMessage);

	DEBUG_BREAK();
}
#endif
