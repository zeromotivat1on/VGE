#pragma once

namespace vge::memory
{
	inline void memcopy(void* dst, void* src, size_t size)	{ std::memcpy(dst, src, size); }
	inline void memmove(void* dst, void* src, size_t size)	{ std::memmove(dst, src, size); }
	inline void memset (void* dst, int32 val, size_t size)	{ std::memset(dst, val, size); }
	inline void memzero(void* dst, size_t size)				{ std::memset(dst, 0, size); }
}
