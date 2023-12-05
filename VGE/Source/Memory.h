#pragma once

namespace vge::memory
{
	inline void Memcopy(void* dst, void* src, size_t size)		{ std::memcpy(dst, src, size); }
	inline void Memmove(void* dst, void* src, size_t size)		{ std::memmove(dst, src, size); }
	inline void Memset (void* dst, int32_t val, size_t size)	{ std::memset(dst, val, size); }
	inline void Memzero(void* dst, size_t size)					{ std::memset(dst, 0, size); }

	template<typename T, u32 Count = 1>
	inline T* Alloc() { static_assert(Count > 0); return reinterpret_cast<T*>(::operator new(sizeof(T) * Count)); }

	template<typename T>
	inline void Free(T*& data) { ::operator delete(data); data = nullptr; }

	template<typename T, typename... Args>
	inline T* Emplace(T* where, Args... args) { return new (where) T(std::forward<Args>(args)...); }

	// Shift the given address upwards if/as necessary to
	// ensure it is aligned to the given number of bytes.
	inline uintptr_t AlignAddress(uintptr_t addr, size_t align)
	{
		const size_t mask = align - 1;
		assert((align & mask) == 0); // pwr of 2
		return (addr + mask) & ~mask;
	}

	// Shift the given pointer upwards if/as necessary to
	// ensure it is aligned to the given number of bytes.
	template<typename T>
	inline T* AlignPointer(T* ptr, size_t align)
	{
		const uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
		const uintptr_t addrAligned = AlignAddress(addr, align);
		return reinterpret_cast<T*>(addrAligned);
	}

	// Aligned allocation function. IMPORTANT: 'align' must be a power of 2 (typically 4, 8 or 16).
	inline void* AllocAligned(size_t bytes, size_t align)
	{
		// Allocate 'align' more bytes than we need.
		size_t actualBytes = bytes + align;
		
		// Allocate unaligned block.
		uint8_t* pRawMem = new uint8_t[actualBytes];
		
		// Align the block. If no alignment occurred, shift it up the full 'align' bytes so we always have room to store the shift.
		uint8_t* pAlignedMem = AlignPointer(pRawMem, align);
		if (pAlignedMem == pRawMem)
		{
			pAlignedMem += align;
		}
		
		// Determine the shift, and store it. This works for up to 256-byte alignment.
		ptrdiff_t shift = pAlignedMem - pRawMem;
		assert(shift > 0 && shift <= 256);
		pAlignedMem[-1] = static_cast<uint8_t>(shift & 0xFF);

		return pAlignedMem;
	}

	inline void FreeAligned(void* pMem)
	{
		if (pMem)
		{
			// Convert to U8 pointer.
			uint8_t* pAlignedMem = reinterpret_cast<uint8_t*>(pMem);

			// Extract the shift.
			ptrdiff_t shift = pAlignedMem[-1];
			if (shift == 0)
			{
				shift = 256;
			}

			// Back up to the actual allocated address and array-delete it.
			uint8_t* pRawMem = pAlignedMem - shift;
			delete[] pRawMem;
		}
	}
}
