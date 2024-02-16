#pragma once

#include "Types.h"
#include "Macros.h"
#include <sstream>

namespace vge
{
template <class T>
inline u32 ToU32(T value)
{
	static_assert(std::is_arithmetic<T>::value, "T must be numeric");
	if (static_cast<uintmax_t>(value) > static_cast<uintmax_t>(std::numeric_limits<u32>::max()))
	{
		ENSURE_MSG(false, "ToU32() failed, value is too big to be converted to u32.");
	}
	return static_cast<u32>(value);
}

template <typename T>
inline std::vector<u8> ToBytes(const T& value)
{
	return std::vector<u8>{reinterpret_cast<const u8*>(&value), reinterpret_cast<const u8*>(&value) + sizeof(T)};
}

template <class T>
inline std::string ToString(const T& value)
{
	std::stringstream ss;
	ss << std::fixed << value;
	return ss.str();
}
}