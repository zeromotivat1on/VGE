#pragma once

namespace vge::memory
{
	inline void Memcopy(void* dst, void* src, size_t size)	{ std::memcpy(dst, src, size); }
	inline void Memmove(void* dst, void* src, size_t size)	{ std::memmove(dst, src, size); }
	inline void Memset (void* dst, int32 val, size_t size)	{ std::memset(dst, val, size); }
	inline void Memzero(void* dst, size_t size)				{ std::memset(dst, 0, size); }

	template<typename T, uint32 Count = 1>
	inline T* Allocate() { static_assert(Count > 0); return reinterpret_cast<T*>(::operator new(sizeof(T) * Count)); }

	template<typename T>
	inline void Free(T*& data) { ::operator delete(data); data = nullptr; }

	template<typename T, typename... Args>
	inline T* Emplace(T* where, Args... args) { return new (where) T(std::forward<Args>(args)...); }
}
