#pragma once

// Settings

#ifdef NDEBUG
	#define DEBUG 0
#else
	#define DEBUG 1
#endif

#ifndef USE_CUSTOM_MATRIX_CALCS
	#define USE_CUSTOM_MATRIX_CALCS 0
#endif

#ifndef COMPILE_SHADERS_ON_INIT
	#define COMPILE_SHADERS_ON_INIT 1
#endif

// Misc

#define INDEX_NONE -1

#define STD_VECTOR_ALLOC_SIZE(vector) (sizeof(vector[0]) * vector.size())

#define C_ARRAY_NUM(arr) (sizeof(arr) / sizeof(arr[0]))

#define CONCAT_INNER(a, b) a ## b
#define CONCAT(a, b) CONCAT_INNER(a, b)

#define STRING_INNER(x) #x
#define STRING(x) STRING_INNER(x)

#define NOT_COPYABLE(userType)																					\
	userType(const userType&) = delete;																			\
	userType& operator=(const userType&) = delete;

#define NOT_MOVABLE(userType)																					\
	userType(userType&&) = delete;																				\
	userType& operator=(userType&&) = delete;

#define ASSERT(expr) assert(expr);
#define ASSERT_MSG(expr, msg) assert(expr && msg);

#define ENSURE(expr)																							\
if (!(expr))																									\
{																												\
	LOG(Error, "Ensure condition failed: %s", #expr);															\
	abort();																									\
}

#define ENSURE_MSG(expr, msg)																					\
if (!(expr))																									\
{																												\
	LOG(Error, msg);																							\
	abort();																									\
}

#define CHECK(expr)																								\
if (!(expr))																									\
{																												\
	LOG(Error, "Check condition failed: %s", #expr);															\
}

#define CHECK_MSG(expr, msg)																					\
if (!(expr))																									\
{																												\
	LOG(Error, msg);																							\
}

#if USE_LOGGING
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

#if USE_LOGGING
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
