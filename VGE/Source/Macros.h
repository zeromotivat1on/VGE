#pragma once

// Settings

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

#define ENSURE(expr)																							\
if (!(expr))																									\
{																												\
	exit(EXIT_FAILURE);																							\
}																												\

#define ENSURE_MSG(expr, errMessage)																			\
if (!(expr))																									\
{																												\
	LOG(Error, errMessage);																						\
	exit(EXIT_FAILURE);																							\
}																												\

#define VK_ENSURE(vkFunction)																					\
{																												\
	const VkResult ScopedResult = vkFunction;																	\
	if (ScopedResult != VK_SUCCESS)																				\
	{																											\
		vge::NotifyVulkanEnsureFailure(ScopedResult, #vkFunction, __FILE__, __LINE__);							\
	}																											\
}																												\

#define VK_ENSURE_MSG(vkFunction, errMessage)																	\
{																												\
	const VkResult ScopedResult = vkFunction;																	\
	if (ScopedResult != VK_SUCCESS)																				\
	{																											\
		vge::NotifyVulkanEnsureFailure(ScopedResult, #vkFunction, __FILE__, __LINE__, errMessage);				\
	}																											\
}																												\