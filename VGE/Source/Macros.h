#pragma once

// Settings

#ifdef NDEBUG
	#define VGE_DEBUG 0
	#define VGE_VALIDATION_LAYERS 0
#else
	#define VGE_DEBUG 1
	#define VGE_VALIDATION_LAYERS 1
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

#define COPY_CTOR_DEL(T) T(const T&) = delete;
#define MOVE_CTOR_DEL(T) T(T&&) = delete;
#define COPY_OP_DEL(T) T& operator=(const T&) = delete;
#define MOVE_OP_DEL(T) T& operator=(T&&) = delete;

#define NOT_COPYABLE(T) COPY_CTOR_DEL(T); COPY_OP_DEL(T);
#define NOT_MOVABLE(T) MOVE_CTOR_DEL(T); MOVE_OP_DEL(T);

#define COPY_CTOR_DEF(T) T(const T&) = default;
#define MOVE_CTOR_DEF(T) T(T&&) = default;
#define COPY_OP_DEF(T) T& operator=(const T&) = default;
#define MOVE_OP_DEF(T) T& operator=(T&&) = default;

#define ASSERT(expr) assert(expr);
#define ASSERT_MSG(expr, msg) assert(expr && msg);

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
