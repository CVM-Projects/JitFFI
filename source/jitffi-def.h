#pragma once
#include "jitffi.h"
#include <algorithm>
#include <functional>

namespace JitFFI
{
	// convert_uintxx:
	//    This function can convert to a match size integer of value.

	inline uint32_t convert_uint32(const void *dat, size_t n)
	{
		if (n == 4) {
			return *reinterpret_cast<const uint32_t*>(dat);
		}
		else if (n == 2) {
			return *reinterpret_cast<const uint16_t*>(dat);
		}
		else if (n == 1) {
			return *reinterpret_cast<const uint8_t*>(dat);
		}
		else {
			assert(false);
			return 0xcccccccc;
		}
	}

	inline uint64_t convert_uint64(const void *dat, size_t n)
	{
		if (n == 8) {
			return *reinterpret_cast<const uint64_t*>(dat);
		}
		else {
			return convert_uint32(dat, n);
		}
	}
	template <typename T>
	inline uint64_t convert_uint32(const T &dat)
	{
		return convert_uint32(&dat, sizeof(T));
	}
	template <typename T>
	inline uint64_t convert_uint64(const T &dat)
	{
		return convert_uint64(&dat, sizeof(T));
	}
}

namespace JitFFI
{
	inline auto get_next_post_f(std::function<unsigned int()> get_next_size, const unsigned int align) {
		bool is_first = true;
		unsigned int count = 0;

		return [=]() mutable {
			unsigned int result;

			unsigned int size = get_next_size();
			if (is_first) {
				count += size;
				result = 0;
				is_first = false;
			}
			else {
				unsigned int pack = std::min<unsigned int>(align, size);
				unsigned int rem = count % pack;

				if (rem != 0) {
					count += pack - rem;
				}
				result = count;
				count += size;
			}
			return result;
		};
	}
}

namespace JitFFI
{
	inline unsigned int get_value_count(size_t size, size_t psize) {
		assert(size < UINT32_MAX);
		unsigned int count = static_cast<unsigned int>(size / psize);
		unsigned int remsize = static_cast<unsigned int>(size % psize);
		return count + ((remsize == 0) ? 0 : 1);
	}

	template <typename T>
	inline unsigned int push_memory(const void *dat, size_t size, std::function<void(T)> func_push) {
		assert(size < UINT32_MAX);
		unsigned int count = static_cast<unsigned int>(size / sizeof(T));
		unsigned int remsize = static_cast<unsigned int>(size % sizeof(T));

		const T *dp = reinterpret_cast<const T*>(dat);

		for (unsigned int i = 0; i != count; ++i) {
			func_push(dp[i]);
		}

		if (remsize != 0) {
			T v = 0;
			const byte *p = reinterpret_cast<const byte*>(dp + count);
			memcpy(&v, p, remsize);
			func_push(v);
		}

		return get_value_count(size, sizeof(T));
	}
}
