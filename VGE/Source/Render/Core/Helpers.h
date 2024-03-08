#pragma once

#include "Types.h"
#include "Core/Error.h"
#include "GlmCommon.h"
#include "StringConversion.h"
#include <glm/gtx/hash.hpp>
#include <sstream>

namespace vge
{
template <typename T>
inline u32 ToU32(T value)
{
	static_assert(std::is_arithmetic_v<T>, "T must be numeric");
	if (static_cast<uintmax_t>(value) > static_cast<uintmax_t>(std::numeric_limits<u32>::max()))
	{
		ENSURE_MSG(false, "ToU32() failed, value is too big to be converted to u32.");
	}
	return static_cast<u32>(value);
}

template <typename T>
inline std::vector<u8> ToBytes(const T& value)
{
	return std::vector<u8>(reinterpret_cast<const u8*>(&value), reinterpret_cast<const u8*>(&value) + sizeof(T));
}

template <typename T>
inline std::string ToString(const T& value)
{
	std::stringstream ss;
	ss << std::fixed << value;
	return ss.str();
}
	
template <typename T>
inline void HashCombine(size_t& seed, const T& v)
{
	std::hash<T> hasher;
	glm::detail::hash_combine(seed, hasher(v));
}

template <typename T>
inline void Read(std::istringstream& is, T& value)
{
	is.read(reinterpret_cast<char*>(&value), sizeof(T));
}

inline void Read(std::istringstream& is, std::string& value)
{
	size_t size;
	Read(is, size);
	value.resize(size);
	is.read(const_cast<char*>(value.data()), size);
}

template <typename T>
inline void Read(std::istringstream& is, std::set<T>& value)
{
	size_t size;
	Read(is, size);
	for (u32 i = 0; i < size; i++)
	{
		T item;
		is.read(reinterpret_cast<char*>(&item), sizeof(T));
		value.insert(std::move(item));
	}
}

template <typename T>
inline void Read(std::istringstream& is, std::vector<T>& value)
{
	size_t size;
	Read(is, size);
	value.resize(size);
	is.read(reinterpret_cast<char*>(value.data()), value.size() * sizeof(T));
}

template <typename T, class S>
inline void Read(std::istringstream& is, std::map<T, S>& value)
{
	size_t size;
	Read(is, size);

	for (u32 i = 0; i < size; i++)
	{
		std::pair<T, S> item;
		Read(is, item.first);
		Read(is, item.second);

		value.insert(std::move(item));
	}
}

template <typename T, u32 N>
inline void Read(std::istringstream& is, std::array<T, N>& value)
{
	is.read(reinterpret_cast<char*>(value.data()), N * sizeof(T));
}

template <typename T, typename... Args>
inline void Read(std::istringstream& is, T& firstArg, Args &... args)
{
	Read(is, firstArg);
	Read(is, args...);
}

template <typename T>
inline void Write(std::ostringstream& os, const T& value)
{
	os.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

inline void Write(std::ostringstream& os, const std::string& value)
{
	Write(os, value.size());
	os.write(value.data(), value.size());
}

template <typename T>
inline void Write(std::ostringstream& os, const std::set<T>& value)
{
	Write(os, value.size());
	for (const T& item : value)
	{
		os.write(reinterpret_cast<const char*>(&item), sizeof(T));
	}
}

template <typename T>
inline void Write(std::ostringstream& os, const std::vector<T>& value)
{
	Write(os, value.size());
	os.write(reinterpret_cast<const char*>(value.data()), value.size() * sizeof(T));
}

template <typename T, class S>
inline void Write(std::ostringstream& os, const std::map<T, S>& value)
{
	Write(os, value.size());

	for (const std::pair<T, S>& item : value)
	{
		Write(os, item.first);
		Write(os, item.second);
	}
}

template <typename T, u32 N>
inline void Write(std::ostringstream& os, const std::array<T, N>& value)
{
	os.write(reinterpret_cast<const char*>(value.data()), N * sizeof(T));
}

template <typename T, typename... Args>
inline void Write(std::ostringstream& os, const T& firstArg, const Args &... args)
{
	Write(os, firstArg);
	Write(os, args...);
}
}	// namespace vge
