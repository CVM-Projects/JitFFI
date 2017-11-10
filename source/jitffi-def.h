#pragma once
#include "jitffi.h"
#include <algorithm>
#include <functional>
#include <stack>

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
			assert(n <= 4);
			uint32_t v = 0;
			memcpy(&v, dat, n);
			return v;
		}
	}

	inline uint64_t convert_uint64(const void *dat, size_t n)
	{
		if (n == 8) {
			return *reinterpret_cast<const uint64_t*>(dat);
		}
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
			assert(n <= 8);
			uint64_t v = 0;
			memcpy(&v, dat, n);
			return v;
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
	inline std::pair<unsigned int, unsigned int> get_next_post_base(unsigned int count, unsigned int size, unsigned int align) {
		unsigned int pack = std::min<unsigned int>(align, size);
		unsigned int rem = count % pack;
		unsigned int result = (rem != 0) ? count + pack - rem : count;
		return { result + size, result };
	}
}

namespace JitFFI
{
	template <typename T>
	class UPtrValVector
	{
	public:
		explicit UPtrValVector(size_t num)
			: data((T*)std::malloc(num * sizeof(T))) {}

		T* get() const {
			return data.get();
		}
		T& operator[](size_t id) const {
			return get()[id];
		}

	private:
		struct Deleter { void operator()(T *p) { std::free(p); } };
		std::unique_ptr<T, Deleter> data;
	};

	template <typename T>
	class StackList
	{
	public:
		void add(const T &v) {
			data.push(v);
		}

		bool get_next(T &v) {
			if (data.empty()) {
				return false;
			}
			else {
				v = data.top();
				data.pop();
				return true;
			}
		}

	private:
		std::stack<T, std::list<T>> data;
	};


	template <typename ArgTypeInfo, typename ArgPassData, typename ArgRetData, ArgPassData(pass_f)(const ArgTypeUnit &), ArgRetData(ret_f)(const ArgTypeUnit &)>
	static ArgTypeInfo create_argtypeinfo(const ArgTypeUnit &restype, const ArgTypeList &atlist) {
		assert(atlist.size() < UINT32_MAX);
		ArgTypeInfo ati(static_cast<uint32_t>(atlist.size()));
		ati.retdata = ret_f(restype);
		unsigned int i = 0;
		for (auto &type : atlist) {
			ati.typelist.get()[i++] = pass_f(*type);
		}
		return ati;
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
