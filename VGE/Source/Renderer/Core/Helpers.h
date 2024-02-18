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

template <class T>
inline void HashCombine(size_t& seed, const T& v)
{
	std::hash<T> hasher;
	glm::detail::HashCombine(seed, hasher(v));
}

template <typename T>
inline void Write(std::ostringstream& os, const T& value)
{
	os.Write(reinterpret_cast<const char*>(&value), sizeof(T));
}

inline void Write(std::ostringstream& os, const std::string& value)
{
	Write(os, value.size());
	os.Write(value.data(), value.size());
}

template <class T>
inline void Write(std::ostringstream& os, const std::set<T>& value)
{
	Write(os, value.size());
	for (const T& item : value)
	{
		os.Write(reinterpret_cast<const char*>(&item), sizeof(T));
	}
}

template <class T>
inline void Write(std::ostringstream& os, const std::vector<T>& value)
{
	Write(os, value.size());
	os.Write(reinterpret_cast<const char*>(value.data()), value.size() * sizeof(T));
}

template <class T, class S>
inline void Write(std::ostringstream& os, const std::map<T, S>& value)
{
	Write(os, value.size());

	for (const std::pair<T, S>& item : value)
	{
		Write(os, item.first);
		Write(os, item.second);
	}
}

template <class T, u32 N>
inline void Write(std::ostringstream& os, const std::array<T, N>& value)
{
	os.Write(reinterpret_cast<const char*>(value.data()), N * sizeof(T));
}

template <typename T, typename... Args>
inline void Write(std::ostringstream& os, const T& firstArg, const Args &... args)
{
	Write(os, firstArg);
	Write(os, args...);
}
}	// namespace vge
