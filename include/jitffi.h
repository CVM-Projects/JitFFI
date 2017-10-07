#pragma once
#include <memory>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <functional>

namespace JitFFI
{
	class JitFuncPool
	{
	public:
		enum Access {
			ReadOnly,
			ReadWrite
		};

	public:
		explicit JitFuncPool(size_t _size, Access access = ReadWrite) :
			_size(_size),
			data(alloc(_size, access == ReadOnly), [=](void *dp) { free(dp, _size); }) {}

		explicit JitFuncPool(void *data, size_t _size = 0) :
			_size(_size),
			data(data, [](void *dp) {}) {}

		explicit JitFuncPool(const JitFuncPool &data, size_t _size) :
			_size(_size),
			data(data.get(), [](void *dp) {}) {}

		JitFuncPool(const JitFuncPool &) = default;

		template <typename T = void>
		T* get() const {
			return static_cast<T*>(data.get());
		}

		void unprotect() {
			unprotect(data.get(), _size);
		}
		void protect() {
			protect(data.get(), _size);
		}

		size_t size() const {
			return _size;
		}

	private:
		const size_t _size;
		std::shared_ptr<void> data;

	public:
		static void* alloc(size_t _size, bool readonly);
		static void protect(void *dp, size_t _size);
		static void unprotect(void *dp, size_t _size);
		static void free(void *dp, size_t _size);
	};

	class JitFunc
	{
		using byte = uint8_t;
	public:
		explicit JitFunc(JitFuncPool &jfp) : jfp(jfp) {}
		explicit JitFunc(JitFuncPool &&jfp) : jfp(jfp) {}
		explicit JitFunc(JitFuncPool &jfp, size_t offset, size_t size) : jfp(jfp.get<byte>() + offset, size) {}

		template <typename T>
		T* func() const {
			return reinterpret_cast<T*>(get());
		}

		template <typename T = void>
		T* get() const {
			return jfp.get<T>();
		}

		void unprotect() {
			jfp.unprotect();
		}
		void protect() {
			jfp.protect();
		}

		size_t size() const {
			return jfp.size();
		}

	private:
		JitFuncPool jfp;
	};

	class JitFuncCreater
	{
		using byte = uint8_t;
	public:
		explicit JitFuncCreater(JitFunc &data) : data(data) {}

		void push(byte dat) {
			assert(count + 1 < data.size());
			byte *code = data.get<byte>();
			code[count++] = dat;
		}
		void push(byte dat1, byte dat2) {
			push(dat1);
			push(dat2);
		}
		void push(byte dat1, byte dat2, byte dat3) {
			push(dat1, dat2);
			push(dat3);
		}
		void push(byte dat1, byte dat2, byte dat3, byte dat4) {
			push(dat1, dat2, dat3);
			push(dat4);
		}
		void push(byte dat1, byte dat2, byte dat3, byte dat4, byte dat5) {
			push(dat1, dat2, dat3, dat4);
			push(dat5);
		}
		void push_uint16(uint16_t dat) {
			write(&dat, 2);
		}
		void push_uint32(uint32_t dat) {
			write(&dat, 4);
		}
		void push_uint64(uint64_t dat) {
			write(&dat, 8);
		}
		void write(const void *src, size_t msize) {
			assert(count + msize < data.size());
			byte *code = data.get<byte>();
			std::memcpy(code + count, src, msize);
			count += msize;
		}

		byte* begin() const {
			return data.get<byte>();
		}
		byte* end() const {
			return begin() + count;
		}

		JitFunc& get() {
			return data;
		}
		const JitFunc& get() const {
			return data;
		}

	private:
		JitFunc &data;
		size_t count = 0;
	};

	class JitFuncCallerCreater
	{
		using byte = uint8_t;
	public:
		template <typename _FTy>
		JitFuncCallerCreater(JitFuncCreater &jfc, _FTy *func)
			: jfc(jfc), func(reinterpret_cast<void*>(func)) {}

		void sub_rsp();
		void add_rsp();
		byte& sub_rsp_unadjusted();
		void adjust_sub_rsp(byte &d);

		void add_int(uint64_t dat);
		void add_int_uint32(uint32_t dat);
		void add_int_rbx();
		void add_double(uint64_t dat);

		void push(uint64_t);

		void push_rbx();
		void pop_rbx();

		void call();
		void ret();

		JitFuncCreater& data() {
			return jfc;
		}

	private:
		JitFuncCreater &jfc;
		void* func;
		unsigned int push_count = 0;
		bool have_init = false;

#if (defined(_WIN64))
		unsigned int argn;
		unsigned int add_count = 0;

	public:
		void init_addarg_count(unsigned int int_c, unsigned int dou_c, unsigned int mem_c = 0) {
			argn = int_c + dou_c + mem_c;
			have_init = true;
		}
	private:
#elif (defined(__x86_64__))
		unsigned int addarg_int_count = 0;
		unsigned int addarg_double_count = 0;

	public:
		void init_addarg_count(unsigned int int_c, unsigned int dou_c, unsigned int mem_c = 0) {
			addarg_int_count = int_c;
			addarg_double_count = dou_c;
			have_init = true;
		}
	private:
#else
	public:
		void init_addarg_count(unsigned int int_c, unsigned int dou_c) {}
	private:
#endif

		byte get_offset();
		byte get_sub_offset();
		auto get_add_offset();

		using OpHandler = unsigned int(unsigned int);

		void _add_int(const std::function<OpHandler> &handler);
		void _add_double(const std::function<OpHandler> &handler);
	};

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
