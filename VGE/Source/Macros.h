#pragma once

// Settings

#ifdef NDEBUG
	#define DEBUG 0
#else
	#define DEBUG 1
#endif

#ifndef COMPILE_SHADERS_ON_INIT
	#define COMPILE_SHADERS_ON_INIT 1
#endif

// Misc

#define INDEX_NONE -1

#define STD_VECTOR_ALLOC_SIZE(vector) (sizeof(vector[0]) * vector.size())

#define C_ARRAY_NUM(arr) (sizeof(arr) / sizeof(arr[0]))

#define _GLUE(a, b) a ## b
#define GLUE(a, b) _GLUE(a, b)

#define _STRING(x) #x
#define STRING(x) _STRING(x)

#define NOT_COPYABLE(userType)																					\
	userType(const userType&) = delete;																			\
	userType& operator=(const userType&) = delete;

#define NOT_MOVABLE(userType)																					\
	userType(userType&&) = delete;																				\
	userType& operator=(userType&&) = delete;

#define ASSERT(expr) assert(expr);
#define ASSERT_MSG(expr, msg) assert(expr && msg);

// TODO: make support for different CPUs.
#define DEBUG_BREAK() __debugbreak()

#ifndef ENSURE_ENABLED
	#define ENSURE_ENABLED 1
#endif

#ifndef CHECK_ENABLED
	#define CHECK_ENABLED USE_LOGGING && 1
#endif

#if ENSURE_ENABLED
	#define ENSURE(expr)																						\
		if (expr) {}																							\
		else																									\
		{																										\
			LOG(Error, "Ensure condition failed: %s", #expr);													\
			DEBUG_BREAK();																						\
		}
#else
	#define ENSURE(expr)
#endif

#if ENSURE_ENABLED
	#define ENSURE_MSG(expr, msg)																				\
		if (expr) {}																							\
		else																									\
		{																										\
			LOG(Error, msg);																					\
			DEBUG_BREAK();																						\
		}
#else
	#define ENSURE_MSG(expr)
#endif

#if CHECK_ENABLED
	#define CHECK(expr)																							\
		if (expr) {}																							\
		else																									\
		{																										\
			LOG(Error, "Check condition failed: %s", #expr);													\
		}
#else
	#define CHECK(expr)
#endif

#if CHECK_ENABLED
	#define CHECK_MSG(expr, msg)																				\
		if (expr) {}																							\
		else																									\
		{																										\
			LOG(Error, msg);																					\
		}
#else
	#define CHECK_MSG(expr)
#endif

#if ENSURE_ENABLED
	#define VK_ENSURE(vkResult)																					\
	{																											\
		const VkResult ScopedResult = vkResult;																	\
		if (ScopedResult != VK_SUCCESS)																			\
		{																										\
			vge::NotifyVulkanEnsureFailure(ScopedResult, #vkResult, __FILE__, __LINE__);						\
		}																										\
	}
#else
	#define VK_ENSURE(vkResult) ENSURE(vkResult == VK_SUCCESS);
#endif

#if ENSURE_ENABLED
	#define VK_ENSURE_MSG(vkResult, msg)																		\
	{																											\
		const VkResult ScopedResult = vkResult;																	\
		if (ScopedResult != VK_SUCCESS)																			\
		{																										\
			vge::NotifyVulkanEnsureFailure(ScopedResult, #vkResult, __FILE__, __LINE__, msg);					\
		}																										\
	}
#else
	#define VK_ENSURE_MSG(vkResult, msg) ENSURE(vkResult == VK_SUCCESS);
#endif
