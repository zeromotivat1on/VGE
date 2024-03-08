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
