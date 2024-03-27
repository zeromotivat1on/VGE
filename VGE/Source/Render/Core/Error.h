#pragma once

#include "Logging.h"
#include "VkCommon.h"

#if defined(__clang__)
	// CLANG ENABLE/DISABLE WARNING DEFINITION
	#define VKBP_DISABLE_WARNINGS()												\
	_Pragma("clang diagnostic push")											\
		_Pragma("clang diagnostic ignored \"-Wall\"")							\
			_Pragma("clang diagnostic ignored \"-Wextra\"")						\
				_Pragma("clang diagnostic ignored \"-Wtautological-compare\"")

	#define VGE_ENABLE_WARNINGS() \
		_Pragma("clang diagnostic pop")
#elif defined(__GNUC__) || defined(__GNUG__)
	// GCC ENABLE/DISABLE WARNING DEFINITION
	#define VGE_DISABLE_WARNINGS()												\
	_Pragma("GCC diagnostic push")												\
		_Pragma("GCC diagnostic ignored \"-Wall\"")								\
			_Pragma("GCC diagnostic ignored \"-Wextra\"")						\
				_Pragma("GCC diagnostic ignored \"-Wtautological-compare\"")

	#define VGE_ENABLE_WARNINGS() \
		_Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
	// MSVC ENABLE/DISABLE WARNING DEFINITION
	#define VGE_DISABLE_WARNINGS() \
		__pragma(warning(push, 0))

	#define VGE_ENABLE_WARNINGS() \
		__pragma(warning(pop))
#endif

#define ASSERT(expr) assert(expr)
#define ASSERT_MSG(expr, msg) assert(expr && msg)

// TODO: make support for different CPUs.
#define DEBUG_BREAK() __debugbreak()

#ifndef ENSURE_ENABLED
	#define ENSURE_ENABLED 1
#endif

#ifndef CHECK_ENABLED
	#define CHECK_ENABLED 1
#endif

#if ENSURE_ENABLED
	#define ENSURE(expr)																						\
		if (expr) {}																							\
		else																									\
		{																										\
			LOG(Error, "Ensure condition failed: %s", STRING(expr));											\
			DEBUG_BREAK();																						\
		}
	
	#define ENSURE_MSG(expr, msg, ...)																			\
		if (expr) {}																							\
		else																									\
		{																										\
			LOG(Error, msg, __VA_ARGS__);																		\
			DEBUG_BREAK();																						\
		}

	#define VK_ENSURE(vkResult)																					\
	{																											\
		const VkResult ScopedResult = vkResult;																	\
		if (ScopedResult != VK_SUCCESS)																			\
		{																										\
			vge::NotifyVulkanEnsureFailure(ScopedResult, STRING(vkResult), __FILE__, __LINE__);					\
		}																										\
	}
	
	#define VK_ENSURE_MSG(vkResult, msg)																		\
	{																											\
		const VkResult ScopedResult = vkResult;																	\
		if (ScopedResult != VK_SUCCESS)																			\
		{																										\
			vge::NotifyVulkanEnsureFailure(ScopedResult, STRING(vkResult), __FILE__, __LINE__, msg);			\
		}																										\
	}
#else
	#define ENSURE(expr)
	#define ENSURE_MSG(expr, msg)
	#define VK_ENSURE(vkResult) ENSURE(vkResult == VK_SUCCESS);
	#define VK_ENSURE_MSG(vkResult, msg) ENSURE_MSG(vkResult == VK_SUCCESS, msg);
#endif

#if CHECK_ENABLED
	#define CHECK(expr)																							\
		if (expr) {}																							\
		else																									\
		{																										\
			LOG(Error, "Check condition failed: %s", STRING(expr));												\
		}
	
	#define CHECK_MSG(expr, msg, ...)																			\
		if (expr) {}																							\
		else																									\
		{																										\
			LOG(Error, msg, __VA_ARGS__);																		\
		}
#else
	#define CHECK(expr)
	#define CHECK_MSG(expr)
#endif

namespace vge
{
#if USE_LOGGING
	inline void NotifyVulkanEnsureFailure(VkResult result, const char* function, const char* filename, u32 line, const char* errMessage = nullptr)
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
} // namespace vge
