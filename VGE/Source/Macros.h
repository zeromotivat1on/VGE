#pragma once

#define INDEX_NONE -1

#define STD_VECTOR_ALLOC_SIZE(vector) (sizeof(vector[0]) * vector.size())

#define C_ARRAY_NUM(arr) (sizeof(arr) / sizeof(arr[0]))

#define ENSURE(expr, errMessage)																\
if (!(expr))																					\
{																								\
	LOG(Error, ##errMessage);																	\
	exit(EXIT_FAILURE);																			\
}																								\